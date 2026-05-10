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


#ifdef UTF8_EVERYWHERE
#include "lstat64.i"

#else

#undef _UNICODE
#undef UNICODE
#include <windows.h>
#include <wchar.h>
#include <stdlib.h>
#include "w32-symlink-posix.h"
#include "convert.h"


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
