cmake_minimum_required(VERSION 3.4.1)
project( jnlua5.1 )
include_directories ( "${LuaIncludePath}" )

add_definitions(
	-DLUA_USE_POSIX 
	-DLUA_USE_DLOPEN
)

add_library( jnlua5.1 SHARED jnlua.c )
target_link_libraries ( jnlua5.1 lua log )
