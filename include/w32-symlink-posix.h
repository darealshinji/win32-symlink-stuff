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
#ifndef W32_SYMLINK_POSIX_H_INCLUDED
#define W32_SYMLINK_POSIX_H_INCLUDED

#include <windows.h>
#include <wchar.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>


/**
 * #define UTF8_EVERYWHERE to assume UTF-8 encoding in "char" strings, otherwise
 * strings will be converted to "wchar_t" and the *W API functions will be used.
 */


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


#define _AT_FDCWD               -100    /* Special value used to indicate the *at functions */
                                        /* should use the current working directory. */
#define _AT_SYMLINK_FOLLOW      0x400   /* Follow symbolic links. */

#ifndef NO_OLDNAMES
# define AT_FDCWD               _AT_FDCWD
# define AT_SYMLINK_FOLLOW      _AT_SYMLINK_FOLLOW
#endif

#ifndef PATH_MAX
#define PATH_MAX                MAX_PATH /* 260 characters */
#endif



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
#define _tsymlink _symlink
#endif

int  _symlink(const char *target, const char *linkpath);
int _wsymlink(const wchar_t *target, const wchar_t *linkpath);

#ifndef NO_OLDNAMES
inline int symlink(const char *target, const char *linkpath) {
    return _symlink(target, linkpath);
}
#endif



/**
 * Creates a symbolic link named 'linkpath' pointing to the target named 'target'
 * relative to the directory referred to by the file descriptor newdirfd.
 *
 * If linkpath is relative then linkpath is interpreted relative to the directory
 * referred to by newdirfd.
 *
 * If linkpath is relative and newdirfd is the special value AT_FDCDW, then
 * linkpath is interpreted relative to the current working directory.
 *
 * If linkpath is absolute then newdirfd is ignored.
 *
 * On success, zero is returned.
 * On error, -1 is returned, and errno is set to indicate the error.
 */

#ifdef _UNICODE
#define _tsymlinkat _wsymlinkat
#else
#define _tsymlinkat _symlinkat
#endif

int  _symlinkat(const char *target, int newdirfd, const char *linkpath);
int _wsymlinkat(const wchar_t *target, int newdirfd, const wchar_t *linkpath);

#ifndef NO_OLDNAMES
inline int symlinkat(const char *target, int newdirfd, const char *linkpath) {
    return _symlinkat(target, newdirfd, linkpath);
}
#endif



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
#define _tlink _link
#endif

int  _link(const char *oldpath, const char *newpath);
int _wlink(const wchar_t *oldpath, const wchar_t *newpath);

#ifndef NO_OLDNAMES
inline int link(const char *oldpath, const char *newpath) {
    return _link(oldpath, newpath);
}
#endif



/**
 * Creates a new link (also known as a hard link) named 'newpath' to an existing
 * file named 'oldpath'.
 *
 * If oldpath is relative it is interpreted relative to the directory referred
 * to by the file descriptor olddirfd.
 *
 * If oldpath is relative and olddirfd is the special value AT_FDCWD, then
 * oldpath is interpreted relative to the current working directory.
 *
 * If oldpath is absolute, then olddirfd is ignored.
 * The parameters newdirfd and newpath are handled the same way.
 *
 * On success, zero is returned.
 * On error, -1 is returned, and errno is set to indicate the error.
 */

#ifdef _UNICODE
#define _tlinkat _wlinkat
#else
#define _tlinkat _linkat
#endif

int  _linkat(int olddirfd, const char *oldpath,
             int newdirfd, const char *newpath, int flags);
int _wlinkat(int olddirfd, const wchar_t *oldpath,
             int newdirfd, const wchar_t *newpath, int flags);

#ifndef NO_OLDNAMES
inline int linkat(int olddirfd, const char *oldpath,
                  int newdirfd, const char *newpath, int flags) {
    return _linkat(olddirfd, oldpath, newdirfd, newpath, flags);
}
#endif



/**
 * Read the value of the link 'path' and save it into the buffer 'buf'.
 * 'bufsize/numwcs' is limited to SSIZE_MAX.
 *
 * readlink() does not append a terminating null byte to buf. It will (silently)
 * truncate the contents (to a length of bufsiz characters), in case the buffer
 * is too small to hold all of the contents.
 *
 * On success the string/character length of the link target is returned.
 * On error, -1 is returned, and errno is set to indicate the error.
 *
 * This function is deprecated in favor of _treadlink_s.
 */

#ifdef _UNICODE
#define _treadlink _wreadlink
#else
#define _treadlink _readlink
#endif

__DEPRECATED /* use _readlink_s instead! */ ssize_t _readlink(
    const char *path, char *buf, size_t bufsize);

__DEPRECATED /* use _wreadlink_s instead! */ ssize_t _wreadlink(
    const wchar_t *path, wchar_t *buf, size_t numwcs);

#ifndef NO_OLDNAMES
__DEPRECATED /* use readlink_s instead! */ ssize_t readlink(
    const char *path, char *buf, size_t bufsize);
#endif



/**
 * Read the value of the link 'path' and save it into the buffer 'buf'.
 *
 * If the 'buf' is too small to hold the contents the function returns with an error.
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
#define _treadlink_s _readlink_s
#endif

char     *_readlink_s(const char *path, char *buf, size_t bufsize);
wchar_t *_wreadlink_s(const wchar_t *path, wchar_t *buf, size_t numwcs);

#ifndef NO_OLDNAMES
inline char *readlink_s(const char *path, char *buf, size_t bufsize) {
    return _readlink_s(path, buf, bufsize);
}
#endif



/**
 * Read the value of the link 'path' and save it into the buffer 'buf'.
 * 'bufsize/numwcs' is limited to SSIZE_MAX.
 *
 * If path is relative, then it is interpreted relative to the directory
 * referred to by the file descriptor dirfd.
 *
 * If path is relative and dirfd is the special value AT_FDCWD, then path
 * is interpreted relative to the current working directory.
 *
 * If path is absolute, then dirfd is ignored.
 *
 * On success the string/character length of the link target is returned.
 * On error, -1 is returned, and errno is set to indicate the error.
 *
 * This function is deprecated in favor of _treadlinkat_s.
 */

#ifdef _UNICODE
#define _treadlinkat _wreadlinkat
#else
#define _treadlinkat _readlinkat
#endif
__DEPRECATED /* use _readlinkat_s instead! */ ssize_t _readlinkat(
    int dirfd, const char *path, char *buf, size_t bufsize);

__DEPRECATED /* use _wreadlinkat_s instead! */ ssize_t _wreadlinkat(
    int dirfd, const wchar_t *path, wchar_t *buf, size_t numcs);

#ifndef NO_OLDNAMES
__DEPRECATED /* use readlinkat_s instead! */ ssize_t readlinkat(
    int dirfd, const char *path, char *buf, size_t bufsize);
#endif



/**
 * Read the value of the link 'path' and save it into the buffer 'buf'.
 *
 * If 'buf' is NULL, 'bufsize/numwcs' is ignored and an allocated string
 * will be returned on success. This string must be deallocated with 'free()'.
 *
 * If path is relative, then it is interpreted relative to the directory
 * referred to by the file descriptor dirfd.
 *
 * If path is relative and dirfd is the special value AT_FDCWD, then path
 * is interpreted relative to the current working directory.
 *
 * If path is absolute, then dirfd is ignored.
 *
 * On success a pointer to the buffer is returned.
 * On error, NULL is returned, the contents of 'buf' are undefined and errno
 * is set to indicate the error.
 */

#ifdef _UNICODE
#define _treadlinkat_s _wreadlinkat_s
#else
#define _treadlinkat_s _readlinkat_s
#endif
char     *_readlinkat_s(int dirfd, const char *path, char *buf, size_t bufsize);
wchar_t *_wreadlinkat_s(int dirfd, const wchar_t *path, wchar_t *buf, size_t numcs);

#ifndef NO_OLDNAMES
inline char *readlinkat_s(int dirfd, const char *path, char *buf, size_t bufsize) {
    return _readlinkat_s(dirfd, path, buf, bufsize);
}
#endif



/**
 * Get the canonicalized absolute pathname of 'path' and save it in the buffer
 * pointed to by 'resolved_path' up to a maximum of PATH_MAX bytes.
 *
 * If 'resolved_path' is NULL, an allocated string up to PATH_MAX size will be
 * returned on success. This string must be deallocated with 'free()'.
 *
 * On success a pointer to the 'resolved_path' is returned.
 * On error, NULL is returned, the contents of 'resolved_path' are undefined and
 * errno is set to indicate the error.
 *
 * This function is deprecated in favor of _tcanonicalize_file_name.
 */

#ifdef _UNICODE
#define _trealpath _wrealpath
#else
#define _trealpath _realpath
#endif

__DEPRECATED /* use _canonicalize_file_name instead! */ char *_realpath(
    const char *path, char *resolved_path);

__DEPRECATED /* use _wcanonicalize_file_name instead! */ wchar_t *_wrealpath(
    const wchar_t *path, wchar_t *resolved_path);

#ifndef NO_OLDNAMES
__DEPRECATED /* use canonicalize_file_name instead! */ char *realpath(
    const char *path, char *resolved_path);
#endif



/**
 * Get the canonicalized absolute pathname of 'path'. This string must later
 * be deallocated with 'free()'.
 *
 * This function is identical to a call to _trealpath(path, NULL, 0).
 *
 * On success an allocated string is returned.
 * On error, NULL is returned and errno is set to indicate the error.
 */

#ifdef _UNICODE
#define _tcanonicalize_file_name _wcanonicalize_file_name
#else
#define _tcanonicalize_file_name _canonicalize_file_name
#endif

char     *_canonicalize_file_name(const char *path);
wchar_t *_wcanonicalize_file_name(const wchar_t *path);

#ifndef NO_OLDNAMES
inline char *canonicalize_file_name(const char *path) {
    return _canonicalize_file_name(path);
}
#endif



/**
 * _lstat is identical to _stat, except that if path is a symbolic link it
 * will provide information about the link itself instead of the target
 * that the link points to.
 *
 * If in doubt which function to pick, use _lstat64() or _lwstat64().
 */

#ifdef _UNICODE
# define _ltstat        _lwstat
# define _ltstat32      _lwstat32
# define _ltstat64      _lwstat64
# define _ltstati64     _lwstati64
# define _ltstat32i64   _lwstat32i64
# define _ltstat64i32   _lwstat64i32
#else
# define _ltstat        _lstat
# define _ltstat32      _lstat32
# define _ltstat64      _lstat64
# define _ltstati64     _lstati64
# define _ltstat32i64   _lstat32i64
# define _ltstat64i32   _lstat64i32
#endif
#ifndef NO_OLDNAMES
# ifdef _UNICODE
#  define ltstat         lwstat
# else
#  define ltstat         lstat
# endif
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

#ifndef NO_OLDNAMES
int lstat(const char *path, struct stat *buffer);
int lwstat(const wchar_t *path, struct stat *buffer);
#endif


#undef __DEPRECATED


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* W32_SYMLINK_H_INCLUDED */
