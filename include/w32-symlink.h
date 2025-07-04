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
#ifndef W32_SYMLINK_H_INCLUDED
#define W32_SYMLINK_H_INCLUDED

#include <windows.h>
#include <wchar.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ssize_t typedef */
#if !defined(_SSIZE_T_DEFINED) && !defined(HAVE_TYPE_SSIZE_T)
typedef __int64 ssize_t;
#define _SSIZE_T_DEFINED
#endif


/* https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-fscc/c8e77b37-3909-4fe6-a4ea-2b9d423b1ee4 */
#ifndef IO_REPARSE_TAG_SYMLINK
#define IO_REPARSE_TAG_SYMLINK      (0xA000000C)
#endif
#ifndef IO_REPARSE_TAG_MOUNT_POINT
#define IO_REPARSE_TAG_MOUNT_POINT  (0xA0000003)
#endif
#ifndef IO_REPARSE_TAG_APPEXECLINK
#define IO_REPARSE_TAG_APPEXECLINK  (0x8000001B)
#endif
#ifndef IO_REPARSE_TAG_LX_SYMLINK
#define IO_REPARSE_TAG_LX_SYMLINK   (0xA000001D)
#endif


/**
 * Creates a link lpLinkName to the target lpTargetName.
 * The type of link to create depends on the mode.
 * This function is similar to POSIX's `symlink(2)` and `link(2)`.
 *
 * If mode is 'h' or 'H' a hard link (new filename) will be created.
 * In this case lpLinkName will be the new filename and lpTargetName
 * is the old filename.
 *
 * If mode is 'd' or 'D' a symbolic link to a directory will be created.
 *
 * Any other value for mode will create a regular symbolic link.
 */

#ifdef _UNICODE
#define createLink createLinkW
#else
#define createLink createLinkA
#endif

BOOL createLinkA(const char *lpLinkName, const char *lpTargetName, char mode);
BOOL createLinkW(const wchar_t *lpLinkName, const wchar_t *lpTargetName, char mode);



/**
 * getCanonicalPath() returns the canonicalized absolute path form
 * of lpFileName, with all symbolic links and '.' and '..' elements resolved.
 * This function is similar to POSIX's `realpath(3)` or GNU's
 * `canonicalize_file_name(3)`.
 *
 * Consecutive path separators are replaced with a single '\'.
 * The resulting path will begin with "\\?\" followed by the drive letter.
 * 
 * The result must be deallocated with free().
 */

#ifdef _UNICODE
#define getCanonicalPath getCanonicalPathW
#else
#define getCanonicalPath getCanonicalPathA
#endif

char    *getCanonicalPathA(const char *lpFileName);
wchar_t *getCanonicalPathW(const wchar_t *lpFileName);



/**
 * getLinkTarget() will return an allocated string with the link's target.
 * This function is similar to POSIX's `readlink(2)`.
 *
 * pReparseTag is a pointer to the variable where the Reparse Tag will be
 * saved if reading the reparse data was successful. This can be used to
 * figure out what type of link this is.
 */

#ifdef _UNICODE
#define getLinkTarget getLinkTargetW
#else
#define getLinkTarget getLinkTargetA
#endif

char    *getLinkTargetA(const char *lpFileName, ULONG *pReparseTag);
wchar_t *getLinkTargetW(const wchar_t *lpFileName, ULONG *pReparseTag);



/**
 * Return values of isSymlink():
 *  1 (TRUE)    lpFileName is a symbolic link
 *  0 (FALSE)   lpFileName exists and is NOT a symbolic link
 * -1           an error has occured (i.e. if lpFileName does not exist)
 */

#ifdef _UNICODE
#define isSymlink isSymlinkW
#else
#define isSymlink isSymlinkA
#endif

int isSymlinkA(const char *lpFileName);
int isSymlinkW(const wchar_t *lpFileName);




/**
 * The following functions are missing implementations from the POSIX C API.
 * Wide character and "secure" variants are added too.
 * The behavior however is not guaranteed to be exactly as in POSIX.
 */




/**
 * Creates a symbolic link named 'linkpath' pointing to the target named 'target'.
 *
 * On success, zero is returned.
 * On error, -1 is returned, and errno is set to indicate the error.
 */

#ifdef _UNICODE
#define _tsymlink _wsymlink
#else
#define _tsymlink symlink
#endif

int   symlink(const char *target, const char *linkpath);
int _wsymlink(const wchar_t *target, const wchar_t *linkpath);



/**
 * Creates a new link (also known as a hard link) named 'newpath' to an existing
 * file named 'oldpath'.
 *
 * On success, zero is returned.
 * On error, -1 is returned, and errno is set to indicate the error.
 */

#ifdef _UNICODE
#define _tlink _wlink
#else
#define _tlink link
#endif

int   link(const char *oldpath, const char *newpath);
int _wlink(const wchar_t *oldpath, const wchar_t *newpath);



/**
 * Read the value of the link 'path' and save it into the buffer 'buf'.
 *
 * On success the string/character length of the link target is returned.
 * On error, -1 is returned, and errno is set to indicate the error.
 */

#ifdef _UNICODE
#define _treadlink _wreadlink
#else
#define _treadlink readlink
#endif

ssize_t   readlink(const char *path, char *buf, size_t bufsize);
ssize_t _wreadlink(const wchar_t *path, wchar_t *buf, size_t numwcs);



/**
 * Read the value of the link 'path' and save it into the buffer 'buf'.
 * If 'buf' is NULL, 'bufsize/numwcs' is ignored and an allocated string
 * will be returned on success. This string must be deallocated with 'free()'.
 *
 * The design of _treadlink_s is similar to that of _trealpath_s in that it
 * returns a pointer to a potentially allocated string.
 *
 * On success a pointer to the buffer is returned.
 * On error, NULL is returned, the contents of 'buf' are undefined and errno
 * is set to indicate the error.
 */

#ifdef _UNICODE
#define _treadlink_s _wreadlink_s
#else
#define _treadlink_s readlink_s
#endif

char      *readlink_s(const char *path, char *buf, size_t bufsize);
wchar_t *_wreadlink_s(const wchar_t *path, wchar_t *buf, size_t numwcs);



/**
 * Get the canonicalized absolute pathname of 'path' and save it in the buffer 'buf'.
 * If 'buf' is NULL, 'bufsize/numwcs' is ignored and an allocated string
 * will be returned on success. This string must be deallocated with 'free()'.
 *
 * This function is similar to _trealpath except the buffer size is set
 * explicitly and isn't (potentially) limited to MAX_PATH.
 *
 * On success a pointer to the buffer is returned.
 * On error, NULL is returned, the contents of 'buf' are undefined and errno
 * is set to indicate the error.
 */

#ifdef _UNICODE
#define _trealpath_s _wrealpath_s
#else
#define _trealpath_s realpath_s
#endif

char      *realpath_s(const char *path, char *buf, size_t bufsize);
wchar_t *_wrealpath_s(const wchar_t *path, wchar_t *buf, size_t numwcs);



/**
 * Get the canonicalized absolute pathname of 'path' and save it in the buffer
 * pointed to by 'resolved_path'. The size of this buffer must be at least MAX_PATH.
 * If 'resolved_path' is NULL, an allocated string up to MAX_PATH size will be
 * returned on success. This string must be deallocated with 'free()'.
 *
 * On success a pointer to the 'resolved_path' is returned.
 * On error, NULL is returned, the contents of 'resolved_path' are undefined and
 * errno is set to indicate the error.
 */

#ifdef _UNICODE
#define _trealpath _wrealpath
#else
#define _trealpath realpath
#endif

inline char *realpath(const char *path, char *resolved_path) {
    return realpath_s(path, resolved_path, MAX_PATH);
}

inline wchar_t *_wrealpath(const wchar_t *path, wchar_t *resolved_path) {
    return _wrealpath_s(path, resolved_path, MAX_PATH);
}



/**
 * _tlstat is identical to _tstat, except that if pathname is a symbolic link
 * it will provide information about the link itself instead of the target
 * that the link points to.
 */

#ifdef _UNICODE
#define _tlstat _wlstat
#else
#define _tlstat _lstat
#endif

int  _lstat(const char *pathname, struct _stat *statbuf);
int _wlstat(const wchar_t *pathname, struct _stat *statbuf);

inline int lstat(const char *pathname, struct stat *statbuf) {
    return _lstat(pathname, (struct _stat *)statbuf);
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* W32_SYMLINK_H_INCLUDED */
