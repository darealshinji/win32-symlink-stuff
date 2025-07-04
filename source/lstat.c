/**
 * The MIT License (MIT)
 *
 * Copyright (C) 2025 Carsten Janssen
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

#include <wchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "w32-symlink.h"


/* the stat() API on Windows is a mess ... */

#define COPY_TO_BUFFER \
  buffer->st_dev = st.st_dev; \
  buffer->st_ino = st.st_ino; \
  buffer->st_mode = st.st_mode; \
  buffer->st_nlink = st.st_nlink; \
  buffer->st_uid = st.st_uid; \
  buffer->st_gid = st.st_gid; \
  buffer->st_rdev = st.st_rdev; \
  buffer->st_size = st.st_size; \
  buffer->st_atime = st.st_atime; \
  buffer->st_mtime = st.st_mtime; \
  buffer->st_ctime = st.st_ctime;

#define IMPLEMENT_LSTAT(LSTAT, LWSTAT, TYPE) \
\
  int LSTAT(const char *path, TYPE *buffer) \
  { \
    struct _stat64 st; \
    int rv = _lstat64(path, &st); \
    if (rv == 0) { COPY_TO_BUFFER; } \
    return rv; \
  } \
\
  int LWSTAT(const wchar_t *path, TYPE *buffer) \
  { \
    struct _stat64 st; \
    int rv = _lwstat64(path, &st); \
    if (rv == 0) { COPY_TO_BUFFER; } \
    return rv; \
  }


IMPLEMENT_LSTAT(lstat, lwstat, struct stat)
IMPLEMENT_LSTAT(_lstat, _lwstat, struct _stat)
IMPLEMENT_LSTAT(_lstat32, _lwstat32, struct _stat32)
IMPLEMENT_LSTAT(_lstati64, _lwstati64, struct _stati64)
IMPLEMENT_LSTAT(_lstat32i64, _lwstat32i64, struct _stat32i64)
IMPLEMENT_LSTAT(_lstat64i32, _lwstat64i32, struct _stat64i32)

