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
#include <tchar.h>
#include <stdlib.h>
#include <string.h>

#include "isSymlink.h"
#include "getLinkTarget.h"


/**
 * https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/ns-ntifs-_reparse_data_buffer
 */
typedef struct _REPARSE_DATA_BUFFER {
  ULONG  ReparseTag;
  USHORT ReparseDataLength;
  USHORT Reserved;
  union {
    /* Symbolic links */
    struct {
      USHORT SubstituteNameOffset;
      USHORT SubstituteNameLength;
      USHORT PrintNameOffset;
      USHORT PrintNameLength;
      ULONG  Flags;
      WCHAR  PathBuffer[1];
    } SymbolicLinkReparseBuffer;

    /* Junction points */
    struct {
      USHORT SubstituteNameOffset;
      USHORT SubstituteNameLength;
      USHORT PrintNameOffset;
      USHORT PrintNameLength;
      WCHAR  PathBuffer[1];
    } MountPointReparseBuffer;

    /* AppExec links (execution aliases) */
    /* (those you can find inside C:\Users\<Username>\AppData\Local\Microsoft\WindowsApps) */
    /* https://www.tiraniddo.dev/2019/09/overview-of-windows-execution-aliases.html */
    /* https://github.com/libuv/libuv/blob/a5c01d4de3695e9d9da34cfd643b5ff0ba582ea7/src/win/winapi.h#L4155 */
    struct {
      ULONG Unused;
      WCHAR StringList[1];
    } AppExecLinkReparseBuffer;

    /* Symbolic links from the LinuX SubSystem (UTF-8 formatted, no trailing NUL) */
    /* https://github.com/JFLarvoire/SysToolsLib/blob/829ecca8d95ade20f5e6241e1226d27e1a2443e7/C/MsvcLibX/include/reparsept.h#L286 */
    struct {
      ULONG Unused;
      char  PathBuffer[1];
    } LxSymbolicLinkReparseBuffer;
  } DUMMYUNIONNAME;
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;


#ifdef _UNICODE
static wchar_t *convert_utf8_to_wcs(const char *lpStr)
{
    int wlen, mbslen;
    wchar_t *pwBuf = NULL;

    mbslen = (int)strlen(lpStr);
    wlen = MultiByteToWideChar(CP_UTF8, 0, lpStr, mbslen, NULL, 0);
    if (wlen < 1) return NULL;

    pwBuf = malloc((wlen + 1) * sizeof(wchar_t));
    if (!pwBuf) return NULL;

    if (MultiByteToWideChar(CP_UTF8, 0, lpStr, mbslen, pwBuf, wlen) < 1) {
        free(pwBuf);
        return NULL;
    }

    pwBuf[wlen] = L'\0';
    return pwBuf;
}
#else
static char *convert_wcs_to_str(const wchar_t *lpWstr)
{
    size_t mbslen, n;
    char *buf;

    if (wcstombs_s(&mbslen, NULL, 0, lpWstr, 0) != 0 || mbslen == 0) {
        return NULL;
    }

    buf = malloc(mbslen + 1);
    if (!buf) return NULL;

    if (wcstombs_s(&n, buf, mbslen+1, lpWstr, mbslen) != 0 || n == 0) {
        free(buf);
        return NULL;
    }

    buf[mbslen] = '\0';
    return buf;
}
#endif


_TCHAR *getLinkTarget(const _TCHAR *lpFileName, ULONG *pReparseTag)
{
    UINT8 data[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
    REPARSE_DATA_BUFFER *pData;
    const wchar_t *pwStr;
    const char *pszStr;
    HANDLE hPath;
    size_t len, i;
    wchar_t *pwBuf;
    char *buf;

    if (!isSymlink(lpFileName)) {
        if (GetLastError() == ERROR_SUCCESS) {
            /* lpFileName exists but is not a symbolic link */
            SetLastError(ERROR_NOT_SUPPORTED);
        }
        return NULL;
    }

    /* open path for reading */
    hPath = CreateFile(lpFileName,
                       0,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                       NULL);

    if (hPath == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    /* retrieve reparse data */
    if (!DeviceIoControl(hPath,
                         FSCTL_GET_REPARSE_POINT,
                         NULL,
                         0,
                         data,
                         sizeof(data),
                         NULL,
                         NULL))
    {
        CloseHandle(hPath);
        return NULL;
    }

    CloseHandle(hPath);

    pData = (REPARSE_DATA_BUFFER *)data;
    pwStr = NULL;
    len = 0;

    if (pReparseTag) {
        *pReparseTag = pData->ReparseTag;
    }

    /* read the link target for each type of symbolic link */
    switch (pData->ReparseTag) {
        /* symbolic links */
        case IO_REPARSE_TAG_SYMLINK:
            pwStr = pData->SymbolicLinkReparseBuffer.PathBuffer +
                (pData->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(wchar_t));
            len = pData->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(wchar_t);
            break;

        /* junctions */
        case IO_REPARSE_TAG_MOUNT_POINT:
            pwStr = pData->MountPointReparseBuffer.PathBuffer +
                (pData->MountPointReparseBuffer.SubstituteNameOffset / sizeof(wchar_t));
            len = pData->MountPointReparseBuffer.SubstituteNameLength / sizeof(wchar_t);
            break;

        /* Windows execution aliases */
        case IO_REPARSE_TAG_APPEXECLINK:
            pwStr = pData->AppExecLinkReparseBuffer.StringList;

            /* NUL separated stringlist. We want the third entry. */
            for (i = 0; i < 2; i++) {
                if ((len = wcslen(pwStr)) == 0) return NULL;
                pwStr += len + 1;
            }

            if ((len = wcslen(pwStr)) == 0) return NULL;

            /* string is already NUL terminated */
#ifdef _UNICODE
            return _wcsdup(pwStr);
#else
            return convert_wcs_to_str(pwStr);
#endif

        /* Linux links */
        case IO_REPARSE_TAG_LX_SYMLINK:
            pszStr = pData->LxSymbolicLinkReparseBuffer.PathBuffer;
            len = pData->ReparseDataLength - sizeof(pData->LxSymbolicLinkReparseBuffer.Unused);

            /* copy string and append NUL byte */
            buf = malloc(len + 1);
            if (!buf) return NULL;
            CopyMemory(buf, pszStr, len);
            buf[len] = '\0';

#ifdef _UNICODE
            pwBuf = convert_utf8_to_wcs(buf);
            free(buf);
            return pwBuf;
#else
            return buf;
#endif

        default:
            return NULL;
    }

    if (!pwStr || len == 0) {
        return NULL;
    }

    /* copy string and append NUL character */
    pwBuf = malloc((len + 1) * sizeof(wchar_t));
    if (!pwBuf) return NULL;
    CopyMemory(pwBuf, pwStr, len * sizeof(wchar_t));
    pwBuf[len] = L'\0';

#ifdef _UNICODE
    return pwBuf;
#else
    buf = convert_wcs_to_str(pwBuf);
    free(pwBuf);
    return buf;
#endif
}
