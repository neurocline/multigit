# Cross-platform Git

This is the original mainline Git code, patched so that it compiles on modern
C compilers with full warnings enabled, with no GNU extensions, and cross-
platform in the sense that it builds for Linux, Windows, and Mac OS X.

For Linux, it still uses Makefile, but for Windows and Mac OS X there
is a separate premake5.lua file that is used to generate Visual Studio
or Xcode projects (the preferred dev chain for Windows and Mac,
respectively).

## Make it work on modern C and Linux

The first commit updates the code so that it works on modern Linux
and modern GCC. There were several fixes needed:

- Makefile: `-lssl` removed, `-lz` and `-lcrypto` added
- cache.h: `memcmp` now needs header file `<string.h>`
- init-db.c: the first return needs to be `return 0`
- read-cache.d: fix bug with empty index

The last is not an issue building, simply a bug that I ran across while
doing testing. I misunderstood how `update-cache` worked, and created an
index with no files in it. This caused a segment fault when read back in.

## Add builtin SHA-1 and Zlib

Windows doesn't have system libraries for SHA-1 and Zlib (not precisely
true, but close enough), so use an builtin version of both. And, to
be careful, make sure it can be used on Linux, by updating the Makefile
to check for environment variables `BLK_SHA1` and `INTERNAL_ZLIB` to
prefer the builtin versions over the system libraries.
