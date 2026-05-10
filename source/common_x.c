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
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "w32-symlink.h"
#include "common.h"
#include "reparse_data_buffer.h"



/* try to map some Windows error codes that might appear
 * to an errno value (mostly file operation error codes) */
int private_map_winerr_to_errno(DWORD dwErr)
{
    switch (dwErr)
    {
    case ERROR_SUCCESS:
        return 0;

    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
        return ENOENT;

    case ERROR_BAD_UNIT:
        return ENODEV;

    case ERROR_FILE_EXISTS:
        return EEXIST;

    case ERROR_FILE_TOO_LARGE:
        return EFBIG;

    case ERROR_NOT_ENOUGH_MEMORY:
        return ENOMEM;

    case ERROR_OPERATION_IN_PROGRESS:
        return EINPROGRESS;

    case ERROR_INVALID_PARAMETER:
        return EINVAL;

    case ERROR_TOO_MANY_OPEN_FILES:
        return EMFILE;

    case ERROR_TOO_MANY_LINKS:
        return EMLINK;

    case ERROR_FILENAME_EXCED_RANGE:
        return ENAMETOOLONG;

    case ERROR_DISK_FULL:
        return ENOSPC;

    case ERROR_DIR_NOT_EMPTY:
        return ENOTEMPTY;

    case ERROR_NOT_SUPPORTED:
        return ENOTSUP;

    case ERROR_BUFFER_OVERFLOW:
        return EOVERFLOW;

    default:
        break;
    }

    return -1;
}


BOOL private_get_link_target_from_handle(HANDLE handle, LINK_TARGET *ltarget)
{
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

    pData = (REPARSE_DATA_BUFFER *)data;
    pDataEnd = data + (MAXIMUM_REPARSE_DATA_BUFFER_SIZE - 1);
    maxlen = MAXIMUM_REPARSE_DATA_BUFFER_SIZE -
             (sizeof(pData->ReparseTag) +
              sizeof(pData->ReparseDataLength) +
              sizeof(pData->Reserved));

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
