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
#include <stdlib.h>
#include "w32-symlink.h"
#include "common.h"
#include "convert.h"
#include "helper.h"


static BOOL target_is_directory(const xchar_t *target);


#if defined(UTF8_EVERYWHERE) || defined(WIDE_CHAR_API)
static BOOL target_is_directory(const xchar_t *target)
{
    DWORD dwAttr = AW(GetFileAttributes)(target);

    return (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY));
}
#endif


int _w(symlink)(const xchar_t *target, const xchar_t *linkpath)
{
    char mode = 0;

    if (!target || !*target || !linkpath || !*linkpath) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    if (target_is_directory(target)) {
        mode = 'D';
    }

    if (AW(createLink)(linkpath, target, mode) != TRUE) {
        errno = private_map_winerr_to_errno(GetLastError());
        return -1;
    }

    return 0;
}

