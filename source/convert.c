/**
 * The MIT License (MIT)
 *
 * Copyright (C) 2023-2025 Carsten Janssen
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
#include <stdlib.h>
#include <string.h>


wchar_t *convert_utf8_to_wcs(const char *str)
{
    int len;
    wchar_t *buf;

    if (!str) return NULL;

    len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    if (len < 1) return NULL;

    buf = malloc((len + 1) * sizeof(wchar_t));
    if (!buf) return NULL;

    if (MultiByteToWideChar(CP_UTF8, 0, str, -1, buf, len) < 1) {
        free(buf);
        return NULL;
    }

    buf[len] = 0;

    return buf;
}


char *convert_wcs_to_str(const wchar_t *wstr)
{
    size_t len, n;
    char *buf;

    if (!wstr) return NULL;

    if (wcstombs_s(&len, NULL, 0, wstr, 0) != 0 || len == 0) {
        return NULL;
    }

    buf = malloc(len + 1);
    if (!buf) return NULL;

    if (wcstombs_s(&n, buf, len+1, wstr, len) != 0 || n == 0) {
        free(buf);
        return NULL;
    }

    buf[len] = 0;

    return buf;
}


wchar_t *convert_str_to_wcs(const char *str)
{
    size_t len, n;
    wchar_t *buf;

    if (!str) return NULL;

    if (mbstowcs_s(&len, NULL, 0, str, 0) != 0 || len == 0) {
        return NULL;
    }

    buf = malloc((len + 1) * sizeof(wchar_t));
    if (!buf) return NULL;

    if (mbstowcs_s(&n, buf, len+1, str, len) != 0 || n == 0) {
        free(buf);
        return NULL;
    }

    buf[len] = 0;

    return buf;
}
