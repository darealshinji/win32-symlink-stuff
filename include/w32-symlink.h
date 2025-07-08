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
#include <sys/types.h>
#include <sys/stat.h>


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __GNUC__
#define __DEPRECATED  __attribute__((deprecated))
#elif defined(_MSC_VER)
#define __DEPRECATED  __declspec(deprecated)
#else
#define __DEPRECATED  /**/
#endif


/* typedef for ssize_t */
#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
#undef ssize_t
#ifdef _WIN64
typedef __int64 ssize_t;
#else
typedef int ssize_t;
#endif /* _WIN64 */
#endif /* _SSIZE_T_DEFINED */


#ifndef SSIZE_MAX
#ifdef _WIN64
#define SSIZE_MAX _I64_MAX
#else
#define SSIZE_MAX INT_MAX
#endif
#endif


/* https://learn.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation */
#define MODERN_MAX_PATH 32767
#ifndef PATH_MAX
#define PATH_MAX MODERN_MAX_PATH
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
 * The following functions are missing implementations from the POSIX C API
 * (and GNU extensions). Wide character and "secure" variants are added too.
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
 * 'bufsize/numwcs' must not exceed SSIZE_MAX.
 *
 * On success the string/character length of the link target is returned.
 * On error, -1 is returned, and errno is set to indicate the error.
 *
 * This function is deprecated in favor of _treadlink_s.
 */

#ifdef _UNICODE
#define _treadlink _wreadlink
#else
#define _treadlink readlink
#endif

__DEPRECATED /* use readlink_s instead! */ ssize_t readlink(
    const char *path, char *buf, size_t bufsize);

__DEPRECATED /* use _wreadlink_s instead! */ ssize_t _wreadlink(
    const wchar_t *path, wchar_t *buf, size_t numwcs);



/**
 * Read the value of the link 'path' and save it into the buffer 'buf'.
 *
 * If 'buf' is NULL, 'bufsize/numwcs' is ignored and an allocated string
 * will be returned on success. This string must be deallocated with 'free()'.
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
 * Get the canonicalized absolute pathname of 'path' and save it in the buffer
 * pointed to by 'resolved_path' up to a maximum of PATH_MAX bytes.
 * If 'resolved_path' is NULL, an allocated string up to PATH_MAX size will be
 * returned on success. This string must be deallocated with 'free()'.
 *
 * On success a pointer to the 'resolved_path' is returned.
 * On error, NULL is returned, the contents of 'resolved_path' are undefined and
 * errno is set to indicate the error.
 *
 * This function is deprecated in favor of _trealpath_s.
 */

#ifdef _UNICODE
#define _trealpath _wrealpath
#else
#define _trealpath realpath
#endif

__DEPRECATED /* use realpath_s instead! */ char *realpath(
    const char *path, char *resolved_path);

__DEPRECATED /* use _wrealpath_s instead! */ wchar_t *_wrealpath(
    const wchar_t *path, wchar_t *resolved_path);



/**
 * Get the canonicalized absolute pathname of 'path' and save it in the buffer 'buf'.
 * If 'buf' is NULL, 'bufsize/numwcs' is ignored and an allocated string
 * will be returned on success. This string must be deallocated with 'free()'.
 *
 * This function is similar to _trealpath except the buffer size is set
 * explicitly and may not be limited to PATH_MAX.
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
 * Get the canonicalized absolute pathname of 'path'. This string must later
 * be deallocated with 'free()'.
 *
 * On success an allocated string is returned.
 * On error, NULL is returned and errno is set to indicate the error.
 */

#ifdef _UNICODE
#define _tcanonicalize_file_name _wcanonicalize_file_name
#else
#define _tcanonicalize_file_name canonicalize_file_name
#endif

inline char *canonicalize_file_name(const char *path) {
    return realpath_s(path, NULL, 0);
}

inline wchar_t *_wcanonicalize_file_name(const wchar_t *path) {
    return _wrealpath_s(path, NULL, 0);
}



/**
 * _lstat is identical to _stat, except that if path is a symbolic link it
 * will provide information about the link itself instead of the target
 * that the link points to.
 */

#ifdef _UNICODE
#define _ltstat      _lwstat
#define _ltstat32    _lwstat32
#define _ltstat64    _lwstat64
#define _ltstati64   _lwstati64
#define _ltstat32i64 _lwstat32i64
#define _ltstat64i32 _lwstat64i32
#define  ltstat       lwstat
#define  ltstat64     lwstat64
#else
#define _ltstat      _lstat
#define _ltstat32    _lstat32
#define _ltstat64    _lstat64
#define _ltstati64   _lstati64
#define _ltstat32i64 _lstat32i64
#define _ltstat64i32 _lstat64i32
#define  ltstat       lstat
#define  ltstat64     lstat64
#endif

int _lstat(const char *path, struct _stat *buffer);
int _lstat32(const char *path, struct _stat32 *buffer);
int _lstat64(const char *path, struct _stat64 *buffer);
int _lstati64(const char *path, struct _stati64 *buffer);
int _lstat32i64(const char *path, struct _stat32i64 *buffer);
int _lstat64i32(const char *path, struct _stat64i32 *buffer);

int _lwstat(const wchar_t *path, struct _stat *buffer);
int _lwstat32(const wchar_t *path, struct _stat32 *buffer);
int _lwstat64(const wchar_t *path, struct _stat64 *buffer);
int _lwstati64(const wchar_t *path, struct _stati64 *buffer);
int _lwstat32i64(const wchar_t *path, struct _stat32i64 *buffer);
int _lwstat64i32(const wchar_t *path, struct _stat64i32 *buffer);

int lstat(const char *path, struct stat *buffer);
int lwstat(const wchar_t *path, struct stat *buffer);


#ifndef stat64
#define stat64 _stat64
#endif

inline int lstat64(const char *path, struct stat64 *buffer) {
   return _lstat64(path, buffer);
}
inline int lwstat64(const wchar_t *path, struct stat64 *buffer) {
   return _lwstat64(path, buffer);
}


#undef __DEPRECATED


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* W32_SYMLINK_H_INCLUDED */
