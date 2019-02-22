cmake_minimum_required(VERSION 3.4.1)
project( jnlua5.1 )
include_directories ( ${LibLuaSoPath} )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DANDROID_NDK -DNDEBUG -D_REENTRANT -DRtt_ANDROID_ENV -DLUA_USE_POSIX -DLUA_USE_DLOPEN")

add_library( jnlua5.1 SHARED ${LibJnlua51Path}/jnlua.c )
target_link_libraries ( jnlua5.1 lua log )
