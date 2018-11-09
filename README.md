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

## Add Premake

Using a Makefile for Windows is hard to the point of useless. Using a
Makefile for Mac OS X is possible. However, on both platforms, the typical
developer prefers to use Visual Studio or Xcode. So, a premake5.lua
makefile was added that generates a Visual Studio project or an Xcode
project on demand.

Latest Premake binaries are the `bin/` directory, as archive for both
space and convenience. Extract out the one for the platform you want to
build on, then generate:

- Windows: `bin/premake5.exe vs2017`
- Mac OS X: `bin/premake5-mac xcode4`

This generates a workspace with a number of projects, one per Git tool that
the Linux Makefile creates. It also makes a `reference` project for ease of
browsing all the source in the IDE.

Premake can also generate a makefile on Linux (`premake5 gmake`), but there's
little point, as we have a perfectly workable one already.

## Add posix-on-win32 porting layer

Add headers and code to implement the pieces of Posix missing on Windows,
so that the existing code builds and runs on Windows. This is imported
from a separate project, `platforms`.
