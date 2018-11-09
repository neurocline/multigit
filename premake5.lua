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

function readcache_files()
	files { "read-cache.c", "cache.h" }
	filter { "system:windows" }
			links { "ws2_32.lib" }
end

-- note that this is using platforms/posix-on-win32, which
-- is outside the multigit directory. This will be a package
-- at some point

function console_app(name, mainfile)
	project(name)
	location "build"
	kind "ConsoleApp"
	files { mainfile }
	defines { "BUILTIN_ZLIB", "SHA1_BLK" }
	links { "block-sha1", "zlib", "posix-on-win32" }
	includedirs { "../platforms/posix-on-win32" }
end

console_app("cat-file", "cat-file.c")
	readcache_files()

console_app("commit-tree", "commit-tree.c")
	readcache_files()

console_app("init-db", "init-db.c")
	files { "cache.h" }

console_app("read-tree", "read-tree.c")
	readcache_files()

console_app("show-diff", "show-diff.c")
	readcache_files()

console_app("update-cache", "update-cache.c")
	readcache_files()

console_app("write-tree", "write-tree.c")
	readcache_files()

project "_Reference"
	location "build"
	kind "None"
	files { "*.c", "*.h" }

project "block-sha1"
	location "build"
	kind "StaticLib"
	warnings "Off"
	files { "block-sha1/*.c", "block-sha1/*.h" }

project "zlib"
	location "build"
	kind "StaticLib"
	warnings "Off"
	files { "zlib/*.c", "zlib/*.h" }

project "posix-on-win32"
	location "build"
	kind "StaticLib"
	files { "../platforms/posix-on-win32/**" }
	includedirs { "../platforms/posix-on-win32" }
