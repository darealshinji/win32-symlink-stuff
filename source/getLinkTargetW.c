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
#define WIDE_CHAR_API

#undef _UNICODE
#undef UNICODE
#include <windows.h>
#include <wchar.h>
#include <stdlib.h>
#include "w32-symlink.h"
#include "convert.h"
#include "common.h"


wchar_t *getLinkTargetW(const wchar_t *path, ULONG *tag)
{
    LINK_TARGET ltarget = { 0, NULL, NULL };
    wchar_t *wstr = NULL;

    if (!path || !_wprivate_get_link_target_open_file(path, &ltarget)) {
        return NULL;
    }

    if (tag) *tag = ltarget.tag;

    if (ltarget.wide_string) {
        wstr = ltarget.wide_string;
    } else if (ltarget.utf8_string) {
        wstr = convert_utf8_to_wcs(ltarget.utf8_string);
        free(ltarget.utf8_string);
    }

    return wstr;
}
