/**
 * The MIT License (MIT)
 *
 * Copyright (C) 2023-2026 Carsten Janssen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE
 */
#undef WIDE_CHAR_API
#undef _UNICODE
#undef UNICODE
#include <windows.h>
#include <winioctl.h> /* FSCTL_GET_REPARSE_POINT */
#include <wchar.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "w32-symlink.h"
#include "convert.h"
#include "helper.h"
#include "common.h"
#include "reparse_data_buffer.h"


typedef struct {
  ULONG    tag;
  wchar_t *wide_string;
  char    *utf8_string;
} LINK_TARGET;



static BOOL get_link_target(const wchar_t *wpath, const char *path, LINK_TARGET *ltarget)
{
    const DWORD dwShare = FILE_SHARE_READ | FILE_SHARE_WRITE;
    const DWORD dwFlags = FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS;
    HANDLE handle;
    uint8_t data[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
    uint8_t *pDataEnd;
    REPARSE_DATA_BUFFER *pData;
    SYMLINK_REPARSE_BUFFER *pSym;
    MOUNTPOINT_REPARSE_BUFFER *pMount;
    NFS_REPARSE_BUFFER *pNfs;
    APPXLINK_REPARSE_BUFFER *pAppX;
    LXSS_SYMLINK_REPARSE_BUFFER *pLxSym;
    wchar_t *wstr;
    size_t off, len, i, buflen, maxlen;
    int ret;

    /* first check if path exists and is a symbolic link */
    ret = wpath ? isSymlinkW(wpath, NULL) : isSymlinkA(path, NULL);

    switch (ret)
    {
        /* it's a symlink */
        case TRUE:
            break;

        /* path exists but is not a symbolic link */
        case FALSE:
            SetLastError(ERROR_NOT_SUPPORTED);
            return FALSE;

        /* error */
        default:
            return FALSE;
    }

    /* open path for reading */
    if (wpath) {
        handle = CreateFileW(wpath, 0, dwShare, NULL, OPEN_EXISTING, dwFlags, NULL);
    } else {
        handle = CreateFileA(path, 0, dwShare, NULL, OPEN_EXISTING, dwFlags, NULL);
    }

    if (handle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    /* retrieve reparse data */
    if (!DeviceIoControl(handle,
                         FSCTL_GET_REPARSE_POINT,
                         NULL,
                         0,
                         data,
                         MAXIMUM_REPARSE_DATA_BUFFER_SIZE,
                         NULL,
                         NULL))
    {
        CloseHandle(handle);
        return FALSE;
    }

    CloseHandle(handle);

    pData = (REPARSE_DATA_BUFFER *)data;
    pDataEnd = data + (MAXIMUM_REPARSE_DATA_BUFFER_SIZE - 1);
    maxlen = MAXIMUM_REPARSE_DATA_BUFFER_SIZE -
             (sizeof(pData->ReparseTag) +
              sizeof(pData->ReparseDataLength) +
              sizeof(pData->Reserved));

    /* check if length exceeds buffer size */
    if (pData->ReparseDataLength >= maxlen) {
        return FALSE;
    }

    ltarget->tag = pData->ReparseTag;
    len = 0;
    wstr = NULL;

    /* read the link target for each type of symbolic link */
    switch (pData->ReparseTag) {
        /* symbolic links */
        case IO_REPARSE_TAG_SYMLINK:
            pSym = (SYMLINK_REPARSE_BUFFER *)pData->DataBuffer;
            buflen = (pDataEnd - (uint8_t *)pSym->PathBuffer) / sizeof(wchar_t);
            off = pSym->SubstituteNameOffset / sizeof(wchar_t);
            len = pSym->SubstituteNameLength / sizeof(wchar_t);

            if (off >= buflen || len >= buflen) {
                return FALSE;
            }

            wstr = pSym->PathBuffer + off;
            break;

        /* junctions */
        case IO_REPARSE_TAG_MOUNT_POINT:
            pMount = (MOUNTPOINT_REPARSE_BUFFER *)pData->DataBuffer;
            buflen = (pDataEnd - (uint8_t *)pMount->PathBuffer) / sizeof(wchar_t);
            off = pMount->SubstituteNameOffset / sizeof(wchar_t);
            len = pMount->SubstituteNameLength / sizeof(wchar_t);

            if (off >= buflen || len >= buflen) {
                return FALSE;
            }

            wstr = pMount->PathBuffer + off;
            break;

        /* Network File System (NFS) component (untested!) */
        case IO_REPARSE_TAG_NFS:
            pNfs = (NFS_REPARSE_BUFFER *)pData->DataBuffer;

            if (pNfs->Type != NFS_SPECFILE_LNK) {
                /* not a symbolic link */
                SetLastError(ERROR_NOT_SUPPORTED);
                return FALSE;
            }

            buflen = pData->ReparseDataLength - sizeof(pNfs->Type);

            if (buflen > NFS_SPECFILE_LNK_MAX_BYTES) {
                return FALSE;
            }

            len = buflen / sizeof(wchar_t);
            wstr = (wchar_t *)pNfs->DataBuffer;
            break;

        /* Windows execution aliases */
        case IO_REPARSE_TAG_APPEXECLINK:
            pAppX = (APPXLINK_REPARSE_BUFFER *)pData->DataBuffer;

            if (pAppX->Id != 3) {
                SetLastError(ERROR_NOT_SUPPORTED);
                return FALSE;
            }

            wstr = pAppX->StringList;
            buflen = (pDataEnd - (uint8_t *)wstr) / sizeof(wchar_t);

            /* NUL separated stringlist. We want the third entry. */
            for (i = 1; i < 3; i++) {
                if ((len = wcsnlen_s(wstr, buflen)) == 0) {
                    return FALSE;
                }
                wstr += len + 1;
                buflen -= len + 1;
            }

            if ((len = wcsnlen_s(wstr, buflen)) == 0) {
                return FALSE;
            }

            ltarget->wide_string = _wcsdup(wstr);
            return TRUE;

        /* Linux links */
        case IO_REPARSE_TAG_LX_SYMLINK:
            pLxSym = (LXSS_SYMLINK_REPARSE_BUFFER *)pData->DataBuffer;

            if (pLxSym->Version != 2) {
                SetLastError(ERROR_NOT_SUPPORTED);
                return FALSE;
            }

            len = pData->ReparseDataLength - sizeof(pLxSym->Version);
            ltarget->utf8_string = malloc(len + 1);
            memcpy_s(ltarget->utf8_string, len + 1, pLxSym->PathBuffer, len);
            ltarget->utf8_string[len] = 0;
            return TRUE;

        default:
            SetLastError(ERROR_NOT_SUPPORTED);
            return FALSE;
    }

    /* copy and NUL-terminate string */
    ltarget->wide_string = malloc((len + 1) * sizeof(wchar_t));
    wmemcpy_s(ltarget->wide_string, len + 1, wstr, len);
    ltarget->wide_string[len] = 0;

    return TRUE;
}


static char *convert_target_path(LINK_TARGET *ltarget)
{
    wchar_t *wstr = NULL;
    char *str = NULL;

    if (ltarget->wide_string) {
        return convert_wcs_to_str(ltarget->wide_string);
    } else if (ltarget->utf8_string) {
#ifdef UTF8_EVERYWHERE
        /* return allocated UTF-8 string */
        str = ltarget->utf8_string;
        ltarget->utf8_string = NULL;
        (void)wstr;
#else
        /* double conversion */
        wstr = convert_utf8_to_wcs(ltarget->utf8_string);
        str = convert_wcs_to_str(wstr);
        free(wstr);
#endif
    }

    return str;
}


wchar_t *getLinkTargetW(const wchar_t *wpath, ULONG *tag)
{
    LINK_TARGET ltarget = { 0, NULL, NULL };
    wchar_t *wstr = NULL;

    if (tag) {
        *tag = 0;
    }

    if (!wpath || !*wpath) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    /* read link data */
    if (!get_link_target(wpath, NULL, &ltarget)) {
        return NULL;
    }

    if (tag) *tag = ltarget.tag;

    if (ltarget.wide_string) {
        /* return allocated string */
        wstr = ltarget.wide_string;
        ltarget.wide_string = NULL;
    } else if (ltarget.utf8_string) {
        wstr = convert_utf8_to_wcs(ltarget.utf8_string);
    }

    free(ltarget.wide_string);
    free(ltarget.utf8_string);

    return wstr;
}


char *getLinkTargetA(const char *path, ULONG *tag)
{
    LINK_TARGET ltarget = { 0, NULL, NULL };
    wchar_t *wpath = NULL;
    char *str = NULL;

    if (tag) {
        *tag = 0;
    }

    if (!path || !*path) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

#ifndef UTF8_EVERYWHERE
    /* convert input path string */
    wpath = convert_str_to_wcs(path);
    path = NULL;
#endif

    /* read link data */
    if (!get_link_target(wpath, path, &ltarget)) {
        return NULL;
    }

    if (tag) *tag = ltarget.tag;

    str = convert_target_path(&ltarget);

    free(ltarget.wide_string);
    free(ltarget.utf8_string);
    free(wpath);

    return str;
}

