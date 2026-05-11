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
#undef _UNICODE
#undef UNICODE
#include <windows.h>
#include <wchar.h>
#include <errno.h>
#include <io.h>
#include <stdlib.h>
#include "w32-symlink.h"
#include "common.h"
#include "helper.h"


#ifndef WIDE_CHAR_API

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

#endif /* !WIDE_CHAR_API */


/**
 * Check if path begins with drive letter + colon + separator.
 *
 * The separator is required because "x:" without a separator
 * refers to the current working directory on drive "x:".
 * So if the current working directory on "c:" was "Users\Joe" then
 * "c:Windows" would actually resolve to "c:\Users\Joe\Windows" and
 * not "c:\Windows".
 */
BOOL _w(private_is_absolute_path)(const xchar_t *p)
{
    /* skip leading namespace specifier */
    if (xstrncmp(p, _T("\\" "\\" "?" "\\"), 4) == 0 || /* "\\?\" file namespace */
        xstrncmp(p, _T("\\" "\\" "." "\\"), 4) == 0 || /* "\\.\" device namespace */
        xstrncmp(p, _T("\\" "?"  "?" "\\"), 4) == 0)   /* "\??\" NT namespace? */
    {
        p += 4;
    }

    /* drive letter + colon + separator, i.e. "C:\" or "z:/" */
    if (xstrnlen_s(p, 3) == 3 &&
        xisalpha(p[0]) &&
        p[1] == _T(':') &&
        (p[2] == _T('\\') || p[2] == _T('/')))
    {
        return TRUE;
    }

    return FALSE;
}


xchar_t *_w(private_create_path_from_dirfd)(int dirfd, const xchar_t *addpath)
{
    BY_HANDLE_FILE_INFORMATION info;
    HANDLE hFile;
    DWORD len;
    xchar_t *buf;
    size_t addlen, buflen;
    int errsav;

    const DWORD flags =
        FILE_NAME_NORMALIZED | /* Normalize the path. -> This is what we want! */
        VOLUME_NAME_DOS;       /* Return path with drive letter (uses "\\?\" syntax). */

    /* get handle from fd value; don't use CloseHandle() on it! */
    hFile = (HANDLE)_get_osfhandle(dirfd);

    if (hFile == INVALID_HANDLE_VALUE) {
        /* errno was already set correctly */
        return NULL;
    }

    /* retrieve file attributes */
    if (GetFileInformationByHandle(hFile, &info) != TRUE) {
        errno = private_map_winerr_to_errno(GetLastError());
        return NULL;
    }

    if (info.dwFileAttributes == INVALID_FILE_ATTRIBUTES) {
        errno = ENODATA;
        return NULL;
    }

    /* check if dirfd represents a directory */
    if (!(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        errno = ENOTDIR;
        return NULL;
    }

    /* get the path name length */
    if ((len = AW(GetFinalPathNameByHandle)(hFile, NULL, 0, flags)) == 0) {
        errno = private_map_winerr_to_errno(GetLastError());
        return NULL;
    }

    addlen = xstrlen(addpath);
    buflen = len + addlen + 2; /* buf + '/' + addpath + '\0' */

    if ((buf = malloc(buflen * sizeof(xchar_t))) == NULL) {
        return NULL;
    }

    /* retrieve the path name from hFile */
    if (AW(GetFinalPathNameByHandle)(hFile, buf, buflen, flags) != len) {
        errsav = private_map_winerr_to_errno(GetLastError());
        free(buf);
        errno = errsav;
        return NULL;
    }

    buf[len] = _T('\\');  /* append path separator */
    xmemcpy_s(buf + (len+1), buflen - (len+1), addpath, addlen + 1);  /* append addpath + NUL */

    return buf;
}
