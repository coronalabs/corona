cmake_minimum_required(VERSION 3.13)
project( jnlua5.1 )
include_directories ( "${LuaIncludePath}" )

add_definitions(
	-DLUA_USE_POSIX 
	-DLUA_USE_DLOPEN
)

add_library( jnlua5.1 SHARED jnlua.c )
if(ANDROID AND (ANDROID_ABI STREQUAL "arm64-v8a" OR ANDROID_ABI STREQUAL "x86_64"))
	# NDK version < 28
	target_link_options(jnlua5.1 PRIVATE "-Wl,-z,max-page-size=16384")
	# NDK version <= 22
	target_link_options(jnlua5.1 PRIVATE "-Wl,-z,common-page-size=16384")
endif()
target_link_libraries ( jnlua5.1 lua log )
