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

#include "createLink.i"

#else

#undef _UNICODE
#undef UNICODE
#include <windows.h>
#include <wchar.h>
#include <stdlib.h>
#include "w32-symlink.h"
#include "convert.h"


BOOL createLinkA(const char *link, const char *target, char mode)
{
    wchar_t *wcs_link, *wcs_target;
    BOOL ret;

    /* convert strings */
    wcs_link = convert_str_to_wcs(link);
    if (!wcs_link) return FALSE;

    wcs_target = convert_str_to_wcs(target);

    if (!wcs_target) {
        free(wcs_link);
        return FALSE;
    }

    /* call wide character function */
    ret = createLinkW(wcs_link, wcs_target, mode);

    free(wcs_link);
    free(wcs_target);

    return ret;
}

#endif

