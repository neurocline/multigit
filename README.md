# Cross-platform Git

This is the original mainline Git code, updated so that it is
cross-platform and up to date in terms of C compiler (no GNU
extensions). It is cross-platform in the senses that it builds
for Linux, Windows and Mac OS X.

It also uses Premake for the makefile, because Unix make isn't really
available on Windows, and nmake isn't all that close to make.

## Notes

The code now assumes a C99 compiler. This is a reasonable assumption.
That means we can use single-line `//` comments as well as multiline `/*...*/` comments.
This also makes it easy to see new comments, they are in single-line style.

The code also expects the compiler to support `#pragma once`.

## How the code was made portable

Our pure source code is in the `cgit` branch, at commit `8c91cbcb8dd5c12ef24b5f35e4fdcc3780568d90`
(which we imported from the mainline Git repo).
