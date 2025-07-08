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
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <errno.h>
#include <wchar.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "convert.h"
#include "w32-symlink.h"


/* try to map some Windows error codes that might appear
 * to an errno value (mostly file operation error codes) */
static int map_winerr_to_errno(DWORD dwErr)
{
    switch (dwErr)
    {
    case ERROR_SUCCESS:
        return 0;

    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
        return ENOENT;

    case ERROR_BAD_UNIT:
        return ENODEV;

    case ERROR_FILE_EXISTS:
        return EEXIST;

    case ERROR_FILE_TOO_LARGE:
        return EFBIG;

    case ERROR_NOT_ENOUGH_MEMORY:
        return ENOMEM;

    case ERROR_OPERATION_IN_PROGRESS:
        return EINPROGRESS;

    case ERROR_INVALID_PARAMETER:
        return EINVAL;

    case ERROR_TOO_MANY_OPEN_FILES:
        return EMFILE;

    case ERROR_TOO_MANY_LINKS:
        return EMLINK;

    case ERROR_FILENAME_EXCED_RANGE:
        return ENAMETOOLONG;

    case ERROR_DISK_FULL:
        return ENOSPC;

    case ERROR_DIR_NOT_EMPTY:
        return ENOTEMPTY;

    case ERROR_BUFFER_OVERFLOW:
        return EOVERFLOW;

    default:
        break;
    }

    return -1;
}


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


int _wsymlink(const wchar_t *target, const wchar_t *linkpath)
{
    char mode = 0;
    DWORD dwAttr;

    if (!target || !*target || !linkpath || !*linkpath) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    dwAttr = GetFileAttributesW(target);

    /* set mode if target exists and is a directory */
    if (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
        mode = 'D';
    }

    if (createLinkW(linkpath, target, mode) == FALSE) {
        errno = map_winerr_to_errno(GetLastError());
        return -1;
    }

    return 0;
}


int link(const char *oldpath, const char *newpath)
{
    if (!oldpath || !*oldpath || !newpath || !*newpath) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    if (createLinkA(oldpath, newpath, 'H') == FALSE) {
        errno = map_winerr_to_errno(GetLastError());
        return -1;
    }

    return 0;
}


int _wlink(const wchar_t *oldpath, const wchar_t *newpath)
{
    if (!oldpath || !*oldpath || !newpath || !*newpath) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    if (createLinkW(oldpath, newpath, 'H') == FALSE) {
        errno = map_winerr_to_errno(GetLastError());
        return -1;
    }

    return 0;
}


ssize_t readlink(const char *path, char *buf, size_t bufsize)
{
    char *ptr;

    if (!path || !*path || !buf || bufsize == 0 || bufsize > SSIZE_MAX) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    ptr = readlink_s(path, buf, bufsize);

    if (!ptr) {
        return -1;
    }

    return (ssize_t)strlen(buf);
}


ssize_t _wreadlink(const wchar_t *path, wchar_t *buf, size_t numwcs)
{
    wchar_t *ptr;

    if (!path || !*path || !buf || numwcs == 0 || numwcs > SSIZE_MAX) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    ptr = _wreadlink_s(path, buf, numwcs);

    if (!ptr) {
        return -1;
    }

    return (ssize_t)wcslen(buf);
}


char *readlink_s(const char *path, char *buf, size_t bufsize)
{
    errno_t rv;
    char *ptr;

    if (!path || !*path || (buf && bufsize == 0)) {
        errno = EINVAL; /* Invalid argument */
        return NULL;
    }

    ptr = getLinkTargetA(path, NULL);

    if (!ptr) {
        errno = map_winerr_to_errno(GetLastError());
        return NULL;
    }

    if (!buf) {
        /* return allocated string */
        return ptr;
    }

    /* copy result into target buffer */
    rv = strncpy_s(buf, bufsize, ptr, _TRUNCATE);
    free(ptr);

    switch (rv)
    {
    case 0:
        break;
    case STRUNCATE:
        errno = ENOMEM; /* Not enough space/cannot allocate memory */
        return NULL;
    default:
        errno = rv;
        return NULL;
    }

    return buf;
}


wchar_t *_wreadlink_s(const wchar_t *path, wchar_t *buf, size_t numwcs)
{
    errno_t rv;
    wchar_t *ptr;

    if (!path || !*path || (buf && numwcs == 0)) {
        errno = EINVAL; /* Invalid argument */
        return NULL;
    }

    ptr = getLinkTargetW(path, NULL);

    if (!ptr) {
        errno = map_winerr_to_errno(GetLastError());
        return NULL;
    }

    if (!buf) {
        /* return allocated string */
        return ptr;
    }

    /* copy result into target buffer */
    rv = wcsncpy_s(buf, numwcs, ptr, _TRUNCATE);
    free(ptr);

    switch (rv)
    {
    case 0:
        break;
    case STRUNCATE:
        errno = ENOMEM; /* Not enough space/cannot allocate memory */
        return NULL;
    default:
        errno = rv;
        return NULL;
    }

    return buf;
}


char *realpath_s(const char *path, char *buf, size_t bufsize)
{
    errno_t rv;
    char *ptr;

    if (!path || !*path || (buf && bufsize == 0)) {
        errno = EINVAL; /* Invalid argument */
        return NULL;
    }

    ptr = getCanonicalPathA(path);

    if (!ptr) {
        errno = map_winerr_to_errno(GetLastError());
        return NULL;
    }

    if (!buf) {
        /* return allocated string */
        return ptr;
    }

    /* copy result into target buffer */
    rv = strncpy_s(buf, bufsize, ptr, _TRUNCATE);
    free(ptr);

    switch (rv)
    {
    case 0:
        break;
    case STRUNCATE:
        errno = ENOMEM; /* Not enough space/cannot allocate memory */
        return NULL;
    default:
        errno = rv;
        return NULL;
    }

    return buf;
}


wchar_t *_wrealpath_s(const wchar_t *path, wchar_t *buf, size_t numwcs)
{
    errno_t rv;
    wchar_t *ptr;

    if (!path || !*path || (buf && numwcs == 0)) {
        errno = EINVAL; /* Invalid argument */
        return NULL;
    }

    ptr = getCanonicalPathW(path);

    if (!ptr) {
        errno = map_winerr_to_errno(GetLastError());
        return NULL;
    }

    if (!buf) {
        /* return allocated string */
        return ptr;
    }

    /* copy result into target buffer */
    rv = wcsncpy_s(buf, numwcs, ptr, _TRUNCATE);
    free(ptr);

    switch (rv)
    {
    case 0:
        break;
    case STRUNCATE:
        errno = ENOMEM; /* Not enough space/cannot allocate memory */
        return NULL;
    default:
        errno = rv;
        return NULL;
    }

    return buf;
}


char *realpath(const char *path, char *resolved_path)
{
    char buf[PATH_MAX];
    char *ptr;

    ptr = realpath_s(path, buf, PATH_MAX);

    if (!ptr) {
        /* error (including truncation from exceeding PATH_MAX) */
        return NULL;
    }

    if (!resolved_path) {
        /* return allocated copy */
        return _strdup(buf);
    }

    return memcpy(resolved_path, buf, strlen(buf)+1);
}


wchar_t *_wrealpath(const wchar_t *path, wchar_t *resolved_path)
{
    wchar_t buf[PATH_MAX];
    wchar_t *ptr;

    ptr = _wrealpath_s(path, buf, PATH_MAX);

    if (!ptr) {
        /* error (including truncation from exceeding PATH_MAX) */
        return NULL;
    }

    if (!resolved_path) {
        /* return allocated copy */
        return _wcsdup(buf);
    }

    return wmemcpy(resolved_path, buf, wcslen(buf)+1);
}


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


int _lwstat64(const wchar_t *pathname, struct _stat64 *statbuf)
{
    int fd, errsav, rv;
    HANDLE handle;

    if (!pathname || !*pathname || !statbuf) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    if (!isSymlinkW(pathname)) {
        /* no symlink, use regular _wstat() function */
        return _wstat64(pathname, statbuf);
    }

    /* get symbolic link file handle */
    handle = CreateFileW(pathname,
                         0,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         OPEN_EXISTING,
                         FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                         NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        errno = map_winerr_to_errno(GetLastError());
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
