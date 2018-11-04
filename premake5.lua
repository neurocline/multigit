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
	kind "None"
	files { "*.c", "*.h" }
	includedirs { "posix-on-win32" }

project "cat-file"
	location "build"
	kind "ConsoleApp"
	links { "block-sha1", "zlib" }
	files { "cat-file.c", "read-cache.c", "xplat_windows.c", "cache.h" }
	includedirs { "posix-on-win32" }
	filter { "system:windows" }
		links { "ws2_32.lib" }

project "commit-tree"
	location "build"
	kind "ConsoleApp"
	links { "block-sha1", "zlib" }
	files { "commit-tree.c", "read-cache.c", "xplat_windows.c", "cache.h" }
	includedirs { "posix-on-win32" }
	filter { "system:windows" }
		links { "ws2_32.lib" }

project "init-db"
	location "build"
	kind "ConsoleApp"
	links { "block-sha1", "zlib" }
	files { "init-db.c", "xplat_windows.c" }
	includedirs { "posix-on-win32" }
	filter { "system:windows" }
		links { "ws2_32.lib" }

project "read-tree"
	location "build"
	kind "ConsoleApp"
	links { "block-sha1", "zlib" }
	files { "read-tree.c", "read-cache.c", "xplat_windows.c", "cache.h" }
	includedirs { "posix-on-win32" }
	filter { "system:windows" }
		links { "ws2_32.lib" }

project "show-diff"
	location "build"
	kind "ConsoleApp"
	links { "block-sha1", "zlib" }
	files { "show-diff.c", "read-cache.c", "xplat_windows.c", "cache.h" }
	includedirs { "posix-on-win32" }
	filter { "system:windows" }
		links { "ws2_32.lib" }

project "update-cache"
	location "build"
	kind "ConsoleApp"
	links { "block-sha1", "zlib" }
	files { "update-cache.c", "read-cache.c", "xplat_windows.c", "cache.h" }
	includedirs { "posix-on-win32" }
	filter { "system:windows" }
		links { "ws2_32.lib" }

project "write-tree"
	location "build"
	kind "ConsoleApp"
	links { "block-sha1", "zlib" }
	files { "write-tree.c", "read-cache.c", "xplat_windows.c", "cache.h" }
	includedirs { "posix-on-win32" }
	filter { "system:windows" }
		links { "ws2_32.lib" }

project "block-sha1"
	location "build"
	kind "StaticLib"
	warnings "Off"
	files { "block-sha1/sha1.c", "block-sha1/sha1.h" }
	includedirs { "posix-on-win32" }
	filter { "system:windows" }
		links { "ws2_32.lib" }

project "zlib"
	location "build"
	kind "StaticLib"
	warnings "Off"
	files { "zlib/*.c", "zlib/*.h" }
	includedirs { "posix-on-win32" }

project "posix-on-win32"
	location "build"
	kind "StaticLib"
	files { "posix-on-win32/*.c", "posix-on-win32/*.h" }
	includedirs { "posix-on-win32" }
