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

function lib_files()
	files { "read-cache.c", "cache.h" }
end

function console_app(name, mainfile)
	project(name)
	location "build"
	kind "ConsoleApp"
	includedirs { "posix-on-win32" }
	files { mainfile }
end

project "_Reference"
	location "build"
	kind "None"
	files { "*.c", "*.h" }

console_app("cat-file", "cat-file.c")
	lib_files()

console_app("commit-tree", "commit-tree.c")
	lib_files()

console_app("init-db", "init-db.c")

console_app("read-tree", "read-tree.c")
	lib_files()

console_app("show-diff", "show-diff.c")
	lib_files()

console_app("update-cache", "update-cache.c")
	lib_files()

console_app("write-tree", "write-tree.c")
	lib_files()
