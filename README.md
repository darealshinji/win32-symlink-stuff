Win32 C helper functions to read or create symbolic and hard links,
including POSIX functions such as `lstat()` and `readlink()`.

The POSIX API is based on Linux man pages descriptions but it's not
guaranteed to be fully POSIX compliant. The lstat() functions are
also based on MSDN documentation.

This is experimental and not everything has been tested yet.
I recommend using the non-POSIX functions if possible.

Build the library with `-DUTF8_EVERYWHERE` to assume UTF-8 encoding in
"char" strings, otherwise strings will internally be converted to "wchar_t"
before being passed to underlying functions.
