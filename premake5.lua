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
		disablewarnings { "4200" } -- so we can use C99 flexible array syntax without a warning
	filter { "action:xcode* or toolset:clang*" }
		buildoptions { "-mlzcnt" }

project "reference"
	location "build"
	kind "ConsoleApp"
	files { "*.c", "*.h" }

project "cat-file"
	location "build"
	kind "ConsoleApp"
	files { "cat-file.c", "read-cache.c", "cache.h" }

project "commit-tree"
	location "build"
	kind "ConsoleApp"
	files { "commit-tree.c", "read-cache.c", "cache.h" }

project "init-db"
	location "build"
	kind "ConsoleApp"
	files { "init-db.c" }

project "read-tree"
	location "build"
	kind "ConsoleApp"
	files { "read-tree.c", "read-cache.c", "cache.h" }

project "show-diff"
	location "build"
	kind "ConsoleApp"
	files { "show-diff.c", "read-cache.c", "cache.h" }

project "update-cache"
	location "build"
	kind "ConsoleApp"
	files { "update-cache.c", "read-cache.c", "cache.h" }

project "write-tree"
	location "build"
	kind "ConsoleApp"
	files { "write-tree.c", "read-cache.c", "cache.h" }
