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

#include "posix.i"


#if !defined(UTF8_EVERYWHERE)

#include "convert.h"


int symlink(const char *target, const char *linkpath)
{
    int rv;
    wchar_t *wcs_linkpath, *wcs_target;

    if (!target || !*target || !linkpath || !*linkpath) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    if ((wcs_target = convert_str_to_wcs(target)) == NULL) {
        return -1;
    }

    if ((wcs_linkpath = convert_str_to_wcs(linkpath)) == NULL) {
        free(wcs_target);
        return -1;
    }

    rv = _wsymlink(wcs_target, wcs_linkpath);

    free(wcs_target);
    free(wcs_linkpath);

    return rv;
}

#endif
