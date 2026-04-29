cmake_minimum_required(VERSION 3.13)
project(lua)

add_definitions(
	-DLUA_USE_DLOPEN
	-DLUA_USE_POSIX
)

#if(CMAKE_ANDROID_ARCH MATCHES "arm")
#	add_definitions(-D_ARM_ASSEM_)
#endif()

add_library( lua SHARED
    src/lapi.c
	src/lauxlib.c
	src/lbaselib.c
	src/lcode.c
	src/ldblib.c
	src/ldebug.c
	src/ldo.c
	src/ldump.c
	src/lfunc.c
	src/lgc.c
	src/linit.c
	src/liolib.c
	src/llex.c
	src/lmathlib.c
	src/lmem.c
	src/loadlib.c
	src/lobject.c
	src/lopcodes.c
	src/loslib.c
	src/lparser.c
	src/lstate.c
	src/lstring.c
	src/lstrlib.c
	src/ltable.c
	src/ltablib.c
	src/ltm.c
	src/luac.c
	src/lundump.c
	src/lvm.c
	src/lzio.c
	src/print.c
)

if(ANDROID AND (ANDROID_ABI STREQUAL "arm64-v8a" OR ANDROID_ABI STREQUAL "x86_64"))
	# NDK version < 28
	target_link_options(lua PRIVATE "-Wl,-z,max-page-size=16384")
	# NDK version <= 22
	target_link_options(lua PRIVATE "-Wl,-z,common-page-size=16384")
endif()
target_link_libraries ( lua log dl )
