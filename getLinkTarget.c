/**
 * The MIT License (MIT)
 *
 * Copyright (C) 2023 djcj@gmx.de
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
#include <windows.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

#include "convert.h"
#include "isSymlink.h"
#include "getLinkTarget.h"


/* https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/ns-ntifs-_reparse_data_buffer */
typedef struct {
  ULONG  ReparseTag;
  USHORT ReparseDataLength;
  USHORT Reserved;
  UINT8  DataBuffer[1];
} REPARSE_DATA_BUFFER;

/* Symbolic links */
typedef struct {
  ULONG  ReparseTag;
  USHORT ReparseDataLength;
  USHORT Reserved;
  USHORT SubstituteNameOffset;
  USHORT SubstituteNameLength;
  USHORT PrintNameOffset;
  USHORT PrintNameLength;
  ULONG  Flags;
  WCHAR  PathBuffer[1];
} SYMLINK_REPARSE_BUFFER;

/* Junction points */
typedef struct {
  ULONG  ReparseTag;
  USHORT ReparseDataLength;
  USHORT Reserved;
  USHORT SubstituteNameOffset;
  USHORT SubstituteNameLength;
  USHORT PrintNameOffset;
  USHORT PrintNameLength;
  WCHAR  PathBuffer[1];
} MOUNTPOINT_REPARSE_BUFFER;

/* AppExec links (execution aliases) */
/* (those you can find inside C:\Users\<Username>\AppData\Local\Microsoft\WindowsApps) */
/* https://www.tiraniddo.dev/2019/09/overview-of-windows-execution-aliases.html */
/* https://github.com/libuv/libuv/blob/a5c01d4de3695e9d9da34cfd643b5ff0ba582ea7/src/win/winapi.h#L4155 */
typedef struct {
  ULONG  ReparseTag;
  USHORT ReparseDataLength;
  USHORT Reserved;
  ULONG  Unused;
  WCHAR  StringList[1];
} APPXLINK_REPARSE_BUFFER;

/* Symbolic links from the LinuX SubSystem (UTF-8 formatted, no trailing NUL) */
/* https://github.com/JFLarvoire/SysToolsLib/blob/829ecca8d95ade20f5e6241e1226d27e1a2443e7/C/MsvcLibX/include/reparsept.h#L286 */
typedef struct {
  ULONG  ReparseTag;
  USHORT ReparseDataLength;
  USHORT Reserved;
  ULONG  Unused;
  char   PathBuffer[1];
} LXSS_SYMLINK_REPARSE_BUFFER;

typedef struct {
  ULONG    tag;
  wchar_t *wide_string;
  char    *utf8_string;
} LINK_TARGET;


static BOOL get_link_target(const wchar_t *path, LINK_TARGET *ltarget)
{
    UINT8 data[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
    UINT8 *pDataEnd;
    REPARSE_DATA_BUFFER *pData;
    SYMLINK_REPARSE_BUFFER *pSym;
    MOUNTPOINT_REPARSE_BUFFER *pMount;
    APPXLINK_REPARSE_BUFFER *pAppX;
    LXSS_SYMLINK_REPARSE_BUFFER *pLxSym;
    HANDLE handle;
    wchar_t *wstr;
    size_t off, len, i, buflen, maxlen;

/*
    const size_t maxlen = MAXIMUM_REPARSE_DATA_BUFFER_SIZE -
                          (sizeof(pData->ReparseTag) +
                           sizeof(pData->ReparseDataLength) +
                           sizeof(pData->Reserved));
*/

    pData = (REPARSE_DATA_BUFFER *)data;
    pDataEnd = data + (MAXIMUM_REPARSE_DATA_BUFFER_SIZE - 1);
    maxlen = (pDataEnd - pData->DataBuffer) + 1;

    switch (isSymlinkW(path))
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
    handle = CreateFileW(path,
                         0,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         OPEN_EXISTING,
                         FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                         NULL);

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
            pSym = (SYMLINK_REPARSE_BUFFER *)data;
            buflen = (pDataEnd - (UINT8 *)pSym->PathBuffer) / sizeof(wchar_t);
            off = pSym->SubstituteNameOffset / sizeof(wchar_t);
            len = pSym->SubstituteNameLength / sizeof(wchar_t);

            if (off >= buflen || len >= buflen) {
                return FALSE;
            }

            wstr = pSym->PathBuffer + off;
            break;

        /* junctions */
        case IO_REPARSE_TAG_MOUNT_POINT:
            pMount = (MOUNTPOINT_REPARSE_BUFFER *)data;
            buflen = (pDataEnd - (UINT8 *)pMount->PathBuffer) / sizeof(wchar_t);
            off = pMount->SubstituteNameOffset / sizeof(wchar_t);
            len = pMount->SubstituteNameLength / sizeof(wchar_t);

            if (off >= buflen || len >= buflen) {
                return FALSE;
            }

            wstr = pMount->PathBuffer + off;
            break;

        /* Windows execution aliases */
        case IO_REPARSE_TAG_APPEXECLINK:
            pAppX = (APPXLINK_REPARSE_BUFFER *)data;
            wstr = pAppX->StringList;
            buflen = (pDataEnd - (UINT8 *)wstr) / sizeof(wchar_t);

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
            pLxSym = (LXSS_SYMLINK_REPARSE_BUFFER *)data;
            len = pLxSym->ReparseDataLength - sizeof(pLxSym->Unused);
            ltarget->utf8_string = malloc(len + 1);
            memcpy_s(ltarget->utf8_string, len + 1, pLxSym->PathBuffer, len);
            ltarget->utf8_string[len] = 0;
            return TRUE;

        default:
            SetLastError(ERROR_NOT_SUPPORTED);
            return FALSE;
    }

    ltarget->wide_string = malloc((len + 1) * sizeof(wchar_t));
    wmemcpy_s(ltarget->wide_string, len + 1, wstr, len);
    ltarget->wide_string[len] = 0;

    return TRUE;
}

char *getLinkTargetA(const char *path, ULONG *tag)
{
    LINK_TARGET ltarget = { 0, NULL, NULL };
    char *str;
    wchar_t *wstr;

    if (!path) return NULL;
    wstr = convert_str_to_wcs(path);

    if (!get_link_target(wstr, &ltarget)) {
        free(wstr);
        return NULL;
    }
    free(wstr);

    if (tag) *tag = ltarget.tag;

    if (ltarget.tag == IO_REPARSE_TAG_LX_SYMLINK) {
        return ltarget.utf8_string;
    }

    /* convert wide char string and free() string */
    if (!ltarget.wide_string) return NULL;
    str = convert_wcs_to_str(ltarget.wide_string);
    free(ltarget.wide_string);

    return str;
}

wchar_t *getLinkTargetW(const wchar_t *path, ULONG *tag)
{
    LINK_TARGET ltarget = { 0, NULL, NULL };
    wchar_t *wstr;

    if (!path || !get_link_target(path, &ltarget)) {
        return NULL;
    }

    if (tag) *tag = ltarget.tag;

    if (ltarget.tag != IO_REPARSE_TAG_LX_SYMLINK) {
        return ltarget.wide_string;
    }

    /* convert string and free() wide char string */
    if (!ltarget.utf8_string) return NULL;
    wstr = convert_utf8_to_wcs(ltarget.utf8_string);
    free(ltarget.utf8_string);

    return wstr;
}
