/**
 * The MIT License (MIT)
 *
 * Copyright (C) 2023 djcj@gmx.de
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
#include <windows.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>

#include "convert.h"
#include "getCanonicalPath.h"
#include "getLinkTarget.h"

#ifndef GetFinalPathNameByHandle
extern DWORD GetFinalPathNameByHandleW(HANDLE hFile, LPWSTR lpszFilePath, DWORD cchFilePath, DWORD dwFlags);
#endif


/**
 * Result must be deallocated with free().
 */
static wchar_t *canonical_path(const wchar_t *path)
{
    wchar_t *buf = NULL;
    HANDLE handle;
    DWORD len;

    const DWORD flags =
        FILE_NAME_NORMALIZED | /* Normalize the path. -> This is what we want! */
        VOLUME_NAME_DOS;       /* Return path with drive letter (uses "\\?\" syntax). */

    /* open for reading */
    handle = CreateFileW(path,
                         0,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         OPEN_EXISTING,
                         FILE_FLAG_BACKUP_SEMANTICS,
                         NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    /* figure out length */
    len = GetFinalPathNameByHandleW(handle, NULL, 0, flags);

    if (len > 0) {
        buf = malloc((len + 1) * sizeof(wchar_t));

        /* resolve path from handle */
        if (buf && GetFinalPathNameByHandleW(handle, buf, len+1, flags) > 0) {
            buf[len] = 0;
            CloseHandle(handle);
            return buf;
        }
    }

    CloseHandle(handle);
    if (buf) free(buf);

    return NULL;
}

/**
 * Check if path begins with drive letter + colon + separator.
 *
 * The separator is required because "x:" without a separator
 * refers to the current working directory on drive "x:".
 * So if the current working directory on "c:" was "Users\Joe" then
 * "c:Windows" would actually resolve to "c:\Users\Joe\Windows" and
 * not "c:\Windows".
 */
static BOOL is_absolute_path(const wchar_t *p)
{
    /* skip leading namespace specifier */
    if (wcslen(p) > 3 && p[0] == L'\\' &&
        (wcsncmp(p, L"\\" "\\" "?" "\\", 4) == 0 || /* "\\?\" file namespace */
         wcsncmp(p, L"\\" "\\" "." "\\", 4) == 0 || /* "\\.\" device namespace */
         wcsncmp(p, L"\\" "?"  "?" "\\", 4) == 0))  /* "\??\" NT namespace? */
    {
        p += 4;
    }

    /* drive letter + colon + separator, i.e. "C:\" or "z:/" */
    if (wcslen(p) > 2 && p[1] == L':' &&
        (p[2] == L'\\' || p[2] == L'/') &&
        ((p[0] >= L'A' && p[0] <= L'Z') ||
         (p[0] >= L'a' && p[0] <= L'z')))
    {
        return TRUE;
    }

    return FALSE;
}

char *getCanonicalPathA(const char *path)
{
    wchar_t *wcs_in, *wcs_out;
    char *buf;

    /* convert string */
    wcs_in = convert_str_to_wcs(path);
    if (!wcs_in) return NULL;

    /* call wide character function */
    wcs_out = getCanonicalPathW(wcs_in);
    free(wcs_in);
    if (!wcs_out) return NULL;

    /* convert string */
    buf = convert_wcs_to_str(wcs_out);
    free(wcs_out);

    return buf;
}

/**
 * Result must be deallocated with free().
 */
wchar_t *getCanonicalPathW(const wchar_t *path)
{
    wchar_t *buf, *link;
    ULONG tag = 0;

    buf = canonical_path(path);
    if (buf) return buf;

    /* canonical_path() has failed.
     * Treat path as a symbolic link and try to get its target. */
    link = getLinkTargetW(path, (ULONG *)&tag);
    if (!link) return NULL;

    /* We cannot reliably resolve LXSS symlinks to their Windows counterparts
     * using GetFinalPathNameByHandle.
     * For example a link target "C:/Windows" could be a relative path inside a
     * Wine prefix whereas "/mnt/c/Windows" may actually be the real location
     * of "C:\Windows". */
    if (tag == IO_REPARSE_TAG_LX_SYMLINK) {
        SetLastError(ERROR_NOT_SUPPORTED);
        return NULL;
    }

    /* Try again with the link target.
     * This kind of second attempt seems to be required on AppExec links.
     * To prevent an erronous canonicalization we should only do a second
     * attempt on an absolute path. */
    if (is_absolute_path(link)) {
        buf = canonical_path(link);
    } else {
        /* buf is still set to NULL */
        SetLastError(ERROR_NOT_SUPPORTED);
    }

    free(link);

    return buf;
}
