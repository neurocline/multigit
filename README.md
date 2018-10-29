# Cross-platform Git

This is the original mainline Git code, updated so that it is
cross-platform and up to date in terms of C compiler (no GNU
extensions). It is cross-platform in the senses that it builds
for Linux, Windows and Mac OS X.

It also uses Premake for the makefile, because Unix `make` isn't really
available on Windows, and `nmake` isn't all that close to `make`.

## Notes

The code now assumes a C99 compiler. This is a reasonable assumption.
That means we can use single-line `//` comments as well as multiline `/*...*/` comments.
This also makes it easy to see new comments, they are in single-line style.

The code also expects the compiler to support `#pragma once`.

## How the code was made portable

Our pure source code is in the `cgit` branch, at commit `8c91cbcb8dd5c12ef24b5f35e4fdcc3780568d90`
(which we imported from the mainline Git repo). Make a copy of this into
a new branch, and get typical repo metadata files as well.

```
$ git checkout --orphan cpcgit
$ git rm --cached -r .
... remove files we don't need ...
$ git checkout cgit -- .
$ git checkout master -- .gitignore .gitattributes .editorconfig
... edit ...
```

### Get code compiling on modern Linux

As-is, the code doesn't build on a 2018-era Linux like Ubuntu. Fix that.

- cache.h: `memcmp` now needs header file `<string.h>`
- Makefile: `LIBS` needs `-lz` to link Zlib
- Makefile: `LIBS` needs `-lcrypto` to link SHA1 routines from libcrypto
- Makefile: `LIBS` doesn't need `-lssl` (evidently SHA1 and zlib used to be in here?)
- init-db.c: the first return needs to be `return 0`
- read-cache.c: fix bug with empty index

Once we make those fixes, the code compiles and links on Linux.

The diff:

```
diff -u ../git-original/cache.h ./cache.h
--- ../git-original/cache.h   2018-10-26 19:19:45.753511000 -0700
+++ ./cache.h   2018-10-29 08:04:04.710802800 -0700
@@ -9,6 +9,7 @@
 #include <stdarg.h>
 #include <errno.h>
 #include <sys/mman.h>
+#include <string.h>

 #include <openssl/sha.h>
 #include <zlib.h>
diff -u ../git-original/init-db.c ./init-db.c
--- ../git-original/init-db.c 2018-10-26 19:19:45.755481900 -0700
+++ ./init-db.c 2018-10-29 08:06:28.934389500 -0700
@@ -20,12 +20,12 @@
		if (sha1_dir) {
				struct stat st;
				if (!stat(sha1_dir, &st) < 0 && S_ISDIR(st.st_mode))
-                       return;
+                       return 0;
				fprintf(stderr, "DB_ENVIRONMENT set to bad directory %s: ", sha1_dir);
		}

		/*
-        * The default case is to have a DB per managed directory.
+        * The default case is to have a DB per managed directory.
		 */
		sha1_dir = DEFAULT_DB_ENVIRONMENT;
		fprintf(stderr, "defaulting to private storage area\n");
diff -u ../git-original/Makefile ./Makefile
--- ../git-original/Makefile  2018-10-26 19:19:45.751516200 -0700
+++ ./Makefile  2018-10-29 08:05:39.629685200 -0700
@@ -8,7 +8,7 @@
 install: $(PROG)
		install $(PROG) $(HOME)/bin/

-LIBS= -lssl
+LIBS= -lz -lcrypto

 init-db: init-db.o

diff -u ../git-original/read-cache.c ./read-cache.c
--- ../git-original/read-cache.c      2018-10-26 19:19:45.755481900 -0700
+++ ./read-cache.c      2018-10-29 08:10:12.419215900 -0700
@@ -228,7 +228,7 @@
				map = NULL;
				size = st.st_size;
				errno = EINVAL;
-               if (size > sizeof(struct cache_header))
+               if (size >= sizeof(struct cache_header))
						map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
		}
		close(fd);
```

### Premake

The next thing we do is add Premake binaries and premake5.lua makefile. We need
some sort of cross-platform make, and it's either CMake or Premake5. For now,
use Premake5 because it's slightly easier to get started.

```
$ mkdir bin
$ subl README.md
$ ...
$ ... download Premak5 binaries
```

We write a basic premake5.lua:

```
-- premake5.lua
workspace "cpcgit"
	configurations { "Debug", "Release" }
	platforms { "x32", "x64" }
	location("build")
	language "C++"
	cppdialect "C++17"
	warnings "Extra"

	filter { "platforms:*32" }
		architecture "x86"
	filter { "platforms:*64" }
		architecture "x64"
	filter { "toolset:msc*" }
		defines { "_ITERATOR_DEBUG_LEVEL=0", "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }
	filter { "action:xcode* or toolset:clang*" }
		buildoptions { "-mlzcnt" }

project "cpcgit"
	location "build"
	kind "ConsoleApp"
	files { "*.c", "*.h" }
```

### Prepare for cross-platform code

Create xplat.h for porting code, and xplat_linux.c for Linux-only code.
See contents of xplat.h for a description of the cross-platform routines.

The first pass of this was done by simply generating a Windows Visual Studio
project and then moving header files and functions that Windows didn't know
about to the xplat layer. No Windows or Mac layer was created in this step.

So, on a Windows machine (after extracting `bin/premake5.exe` from
`bin/premake-5.0.0-alpha13-windows.zip`), do:

```
$ bin\premake5.exe vs2017
$ start build\cpcgit.sln
```

Then build and fix until we just have link errors, and then add empty stubs so
we don't have link errors on Windows. Verify build on Linux.

Repeat on a Mac OS X box. We started with Windows because most of the code
would probably work on a Mac.

At the end, the code should be functionally the same as before, and ready for
the next step of writing Windows-specific and Mac-specific code.

Visual Studio pointed out more non-standard code. Zero-sized arrays are a non-standard
extension. The one use of this was in `cache_entry`, and on inspection of the code,
we never actually do `sizeof(cache_entry)` or make arrays of them, so we are free to
change `unsigned char name[0];` to `unsigned char name[1];`. This will be controversial,
I bet.

Alternatively, we could use the ISO C99 "flexible array member", which looks like
`unsigned char name[]`. This is legal in C99, prevents sizeof usage, and prevents the
containing struct from being an element in an array or in another struct. However,
Visual Studio apparently isn't C99-compliant in this respect, it still gives
"warning C4200: nonstandard extension used: zero-sized array in struct/union".
So, we have to use `unsigned char name[1];`, or disable this warning. Let's
disable the warning, and rely on Linux builds to find errors in the code related
to this. We add this to our premake.lua file

```
		disablewarnings { "4200" }
```

There are some warnings about unused local variables, so those were just deleted.
And there were a few command-line programs that didn't actually use argc and argv,
so we just force references to those. The standard "best practice" is to comment-out
the argument names in the function signature, but C99 doesn't like that, it's
apparently only a C++ thing.

The `unpack` function in `read-tree.c` shadows the `sha1` parameter, which isn't
really great (and generates warnings), so the second use was renamed to `sha1_entry`.

Another difference between old-style C and new C is that I guess you used to be
able to add integers to void pointers. The best fix for this was to change
definitions to be `char*` and not `void*`, because the code really was accessing
memory from it as if it were a char/byte array.

There were also signed/unsigned mismatches pointed out by the Visual Studio build.
The Linux makefile didn't declare a warning level, so presumably the default warning
level for GCC is "low". These were fixed with typecasts.

In cases where there was a "potentially uninitialized variable", the variable was
simply initialized.

### Third-party code: Zlib and SHA1

It's foolish to wrap Zlib just to make the code compile on all platforms, since
Zlib is itself cross-platform. The issue is simply that Windows doesn't expect you
to "install" Zlib on a system, whereas Linux and BSD have that heritage.

So we simply put a recent version of Zlib in the project. The choice is whether or
not to use the system Zlib on Linux, or the embedded one. For now, only use the
embedded one on Windows and maybe Mac OS X, but use the system library on Linux.

For SHA1, we just fast-forward a little in the Git source and add the
`block-sha1` code. This avoids needing to link against `libcrypto`, and works
for Windows. We use this for all platforms.

At this point, we have all the Linux-only code in xplat_linux.c, and the Linux binaries
still work. We are ready to make the Windows version work.

By the way, the hack job we did would probably have been rejected by Linus Torvalds,
had someone attempted to suggest cross-platform back in 2005.
