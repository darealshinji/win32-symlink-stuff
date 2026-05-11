/**
 * The MIT License (MIT)
 *
 * Copyright (C) 2025-2026 Carsten Janssen
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
#include <fcntl.h>
#include <io.h>
#include <wchar.h>
#include <errno.h>
#include <wchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "w32-symlink.h"
#include "w32-symlink-posix.h"
#include "convert.h"
#include "common.h"
#include "helper.h"


#if defined(UTF8_EVERYWHERE) || defined(WIDE_CHAR_API)

int _xlstat64(const xchar_t *pathname, struct _stat64 *statbuf)
{
    int fd, errsav, rv;
    HANDLE handle;

    if (!pathname || !*pathname || !statbuf) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    if (!AW(isSymlink)(pathname, NULL)) {
        /* no symlink, use regular stat function */
        return _xstat64(pathname, statbuf);
    }

    /* get symbolic link file handle */
    handle = AW(CreateFile)(pathname,
                            0,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                            NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        errno = private_map_winerr_to_errno(GetLastError());
        return -1;
    }

    /* get file descriptor from handle */
    fd = _open_osfhandle((intptr_t)handle, _O_RDONLY);

    if (fd == -1) {
        CloseHandle(handle);
        errno = EBADF; /* Bad file descriptor */
        return -1;
    }

    /* get status information */
    rv = _fstat64(fd, statbuf);

    /* don't call CloseHandle()! */
    errsav = errno;
    _close(fd);
    errno = errsav;

    return rv;
}

#else /* !UTF8_EVERYWHERE && !WIDE_CHAR_API */

int _lstat64(const char *pathname, struct _stat64 *statbuf)
{
    wchar_t *wcs_path;
    int rv;

    if (!pathname || !*pathname || !statbuf) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    wcs_path = convert_str_to_wcs(pathname);
    rv = _lwstat64(wcs_path, statbuf);
    free(wcs_path);

    return rv;
}

#endif
