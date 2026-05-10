/**
 * The MIT License (MIT)
 *
 * Copyright (C) 2026 Carsten Janssen
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
#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#undef _UNICODE
#undef UNICODE
#include <windows.h>
#include <wchar.h>


typedef struct {
  ULONG    tag;
  wchar_t *wide_string;
  char    *utf8_string;
} LINK_TARGET;


int private_map_winerr_to_errno(DWORD dwErr);

BOOL  _private_is_absolute_path(const char *p);
BOOL _wprivate_is_absolute_path(const wchar_t *p);

char     *_private_return_path(char *ptr, char *buf, size_t numcs);
wchar_t *_wprivate_return_path(wchar_t *ptr, wchar_t *buf, size_t numcs);

BOOL private_get_link_target_from_handle(HANDLE handle, LINK_TARGET *ltarget);

BOOL  _private_get_link_target_open_file(const char *path, LINK_TARGET *ltarget);
BOOL _wprivate_get_link_target_open_file(const wchar_t *path, LINK_TARGET *ltarget);

char     *_private_create_path_from_dirfd(int dirfd, const char *addpath);
wchar_t *_wprivate_create_path_from_dirfd(int dirfd, const wchar_t *addpath);

#endif /* COMMON_H_INCLUDED */
