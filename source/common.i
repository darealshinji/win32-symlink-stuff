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
        xstrncmp(p, _T("\\" "?"  "?" "\\"), 4) == 0)    /* "\??\" NT namespace? */
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


xchar_t *_w(private_return_path)(xchar_t *ptr, xchar_t *buf, size_t numcs)
{
    errno_t rv;

    if (!ptr) {
        errno = private_map_winerr_to_errno(GetLastError());
        return NULL;
    }

    if (!buf) {
        /* return allocated string */
        return ptr;
    }

    /* copy result into target buffer */
    rv = xstrncpy_s(buf, numcs, ptr, _TRUNCATE);
    free(ptr);

    switch (rv)
    {
    case 0:
        break;
    case STRUNCATE:
        errno = ENOMEM; /* Not enough space/cannot allocate memory */
        return NULL;
    default:
        errno = rv;
        return NULL;
    }

    return buf;
}


BOOL _w(private_get_link_target_open_file)(const xchar_t *path, LINK_TARGET *ltarget)
{
    HANDLE handle;

    switch (AW(isSymlink)(path, NULL))
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
    handle = AW(CreateFile)(path,
                            0,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                            NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    return private_get_link_target_from_handle(handle, ltarget);
}


int _w(private_create_path_from_dirfd)(int dirfd, xchar_t *buf, const size_t buflen, const xchar_t *addpath)
{
    const DWORD dwFlags = FILE_NAME_NORMALIZED | VOLUME_NAME_DOS;
    HANDLE hFile;
    DWORD len;
    xchar_t *sub;
    size_t sublen, linklen;

    /* get handle from fd value; don't use CloseHandle() on the returned hFile! */
    hFile = (HANDLE)_get_osfhandle(dirfd);

    if (hFile == INVALID_HANDLE_VALUE ||
        private_handle_is_directory(hFile) == FALSE)
    {
        /* errno was already set correctly */
        return FALSE;
    }

    /* retrieve the path name from hFile */
    len = AW(GetFinalPathNameByHandle)(hFile, buf, buflen, dwFlags);

    if (len == 0) {
        errno = private_map_winerr_to_errno(GetLastError());
        return FALSE;
    } else if (len == buflen) {
        errno = ENOMEM; /* Not enough space/cannot allocate memory */
        return FALSE;
    }

    /* buf + '/' + addpath + '\0' */
    linklen = xstrlen(addpath);

    if ((len + linklen + 2) > buflen) {
        errno = ENOMEM; /* Not enough space/cannot allocate memory */
        return FALSE;
    }

    buf[len] = _T('\\');  /* append path separator */
    sub = buf + len + 1;  /* get sub-buffer */
    sublen = buflen - (len + 1);
    xmemcpy_s(sub, sublen, addpath, linklen + 1);  /* append addpath + NUL */

    return TRUE;
}
