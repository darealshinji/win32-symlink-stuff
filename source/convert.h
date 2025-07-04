#ifndef W32_SYMLINK_CONVERT_H_INCLUDED
#define W32_SYMLINK_CONVERT_H_INCLUDED

#include <wchar.h>


/**
 * String conversion.
 * The returned string must be deallocated with free().
 */
char    *convert_wcs_to_str(const wchar_t *wcs);
wchar_t *convert_str_to_wcs(const char *str);
wchar_t *convert_utf8_to_wcs(const char *str);

#endif /* W32_SYMLINK_CONVERT_H_INCLUDED */
