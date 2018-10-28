# Cross-platform Git

This is the original mainline Git code, updated so that it is
cross-platform and up to date in terms of C compiler (no GNU
extensions). It is cross-platform in the senses that it builds
for Linux, Windows and Mac OS X.

It also uses Premake for the makefile, because Unix make isn't really
available on Windows, and nmake isn't all that close to make.
