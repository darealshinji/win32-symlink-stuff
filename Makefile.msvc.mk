CFLAGS   = -W3 -O2 -I..\include
CPPFLAGS = -DWIN32_LEAN_AND_MEAN
#CPPFLAGS = -DUTF8_EVERYWHERE
LIB_EXE  = lib.exe

# (cd source && ls -1 *.c posix/*.c) | sed 's,^,\t,; s,$, \\,'
SRCS = \
	common.c \
	common_a.c \
	common_w.c \
	convert.c \
	createLinkA.c \
	createLinkW.c \
	getCanonicalPathA.c \
	getCanonicalPathW.c \
	getLinkTargetA.c \
	getLinkTargetW.c \
	isSymlinkA.c \
	isSymlinkW.c \
	posix/link_a.c \
	posix/link_w.c \
	posix/linkat_a.c \
	posix/linkat_w.c \
	posix/lstat.c \
	posix/lstat64_a.c \
	posix/lstat64_w.c \
	posix/readlink_a.c \
	posix/readlink_w.c \
	posix/realpath_a.c \
	posix/realpath_w.c \
	posix/symlink_a.c \
	posix/symlink_w.c \
	posix/symlinkat_a.c \
	posix/symlinkat_w.c \
	$(NULL)

ARCHIVE = symlink.lib
TEST_FILES = test\test1.exe test\test2.exe test\test3.exe


all: $(ARCHIVE)

tests: $(TEST_FILES)

clean:
	-del /Q *.lib test\*.exe test\*.obj source\*.obj

$(ARCHIVE):
	cd source && $(CC) -nologo -MP $(CFLAGS) $(CPPFLAGS) -c $(SRCS) && $(LIB_EXE) *.obj -out:..\$(ARCHIVE)

test/test1.exe: $(ARCHIVE)
	cd test && $(CC) -nologo -MP $(CFLAGS) $(CPPFLAGS) test1.c -Fe:test1.exe -link ..\$(ARCHIVE) $(LFLAGS)

test/test2.exe: $(ARCHIVE)
	cd test && $(CC) -nologo -MP $(CFLAGS) $(CPPFLAGS) test2.c -Fe:test2.exe -link ..\$(ARCHIVE) $(LFLAGS)

test/test3.exe: $(ARCHIVE)
	cd test && $(CC) -nologo -MP $(CFLAGS) $(CPPFLAGS) test3.c -Fe:test3.exe -link ..\$(ARCHIVE) $(LFLAGS)

