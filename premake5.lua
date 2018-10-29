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
