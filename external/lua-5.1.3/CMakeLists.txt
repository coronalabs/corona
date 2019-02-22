cmake_minimum_required(VERSION 3.4.1)
project(lua)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DANDROID_NDK -DRtt_ANDROID_ENV -DNDEBUG -DLUA_USE_DLOPEN -DLUA_USE_POSIX")
add_definitions(-DANDROID_NDK -DRtt_ANDROID_ENV -DNDEBUG -DLUA_USE_DLOPEN -DLUA_USE_POSIX)
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

target_link_libraries ( lua log dl )