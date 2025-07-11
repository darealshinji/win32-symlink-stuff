CFLAGS  = /W3 /O2 /I..\include
LIB_EXE = lib.exe

SRCS = convert.c \
	createLink.c \
	getCanonicalPath.c \
	getLinkTarget.c \
	isSymlink.c \
	lstat.c \
	posix.c

ARCHIVE = symlink.lib
TEST_FILES = test\test1.exe test\test2.exe test\test3.exe


all: $(ARCHIVE)

tests: $(TEST_FILES)

clean:
	-del /Q *.lib test\*.exe test\*.obj source\*.obj

$(ARCHIVE):
	cd source && $(CC) /nologo /MP $(CFLAGS) /c $(SRCS) && $(LIB_EXE) *.obj /out:..\$(ARCHIVE)

test/test1.exe: $(ARCHIVE)
	cd test && $(CC) /nologo /MP $(CFLAGS) test1.c /Fe:test1.exe /link ..\$(ARCHIVE) $(LFLAGS)

test/test2.exe: $(ARCHIVE)
	cd test && $(CC) /nologo /MP $(CFLAGS) test2.c /Fe:test2.exe /link ..\$(ARCHIVE) $(LFLAGS)

test/test3.exe: $(ARCHIVE)
	cd test && $(CC) /nologo /MP $(CFLAGS) test3.c /Fe:test3.exe /link ..\$(ARCHIVE) $(LFLAGS)

