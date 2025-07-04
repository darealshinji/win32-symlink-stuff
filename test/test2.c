#include <windows.h>
#include <wchar.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <time.h>

#include "w32-symlink.h"

#define TEST(x)  puts((x) ? "success" : "failure")


int main()
{
    struct _stat st;
    int rv;
	char buf[MAX_PATH] = {0};
	wchar_t wbuf[MAX_PATH] = {0};
	char timebuf[32] = {0};

    const char *lnk = "link_to_NtDLL";
	const char *lnk2 = "link_to_C";
    const wchar_t *wlnk = L"C:\\Users\\User\\AppData\\Local\\Microsoft\\WindowsApps\\winget.exe";
	const char *ntdll = "c:/WINDOWS/System32/NtDLL.dll";
	const char *c_dir = "c:/";

    DeleteFileA(lnk);
	DeleteFileA(lnk2);
    RemoveDirectoryA(lnk);
    RemoveDirectoryA(lnk2);

    printf("test symlink() [%s]\n", c_dir);
    TEST(symlink(c_dir, lnk2) == 0);
    puts("");

    printf("test symlink() [%s]\n", ntdll);
    TEST(symlink(ntdll, lnk) == 0);
    puts("");

    puts("test realpath()");
    char *path = realpath("./././link_to_NtDLL", NULL);
    TEST(path);

    if (path) {
        puts(path);
        free(path);
    }
    puts("");

    puts("test readlink()");
    TEST(readlink(lnk, buf, _countof(buf)) != -1);

    if (buf[0]) {
        puts(buf);
    }
    puts("");

	wprintf(L"test _wreadlink [%s]\n", wlnk);
    TEST(_wreadlink(wlnk, wbuf, _countof(wbuf)) != -1);

    if (wbuf[0]) {
        _putws(wbuf);
    }
    puts("");

    puts("test _lstat()");
    TEST((rv = _lstat(lnk, &st)) == 0);

    if (rv == 0) {
        printf("st_size = %ju\n", (uintmax_t)st.st_size);

		if (ctime_s(timebuf, _countof(timebuf), &st.st_mtime) == 0) {
			printf("st_mtime ~= %s\n", timebuf);
		}
    }
    puts("");

    puts("test _stat()");
    TEST((rv = _stat(lnk, &st)) == 0);

    if (rv == 0) {
        printf("st_size = %ju\n", (uintmax_t)st.st_size);

		if (ctime_s(timebuf, _countof(timebuf), &st.st_mtime) == 0) {
			printf("st_mtime ~= %s\n", timebuf);
		}
    }

    return 0;
}
