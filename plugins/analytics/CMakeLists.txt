cmake_minimum_required(VERSION 3.13)
project( analytics )

include_directories( 
	${LuaIncludePath}
	${CORONA_ROOT}/librtt/Corona
)

lua_2_c("${CORONA_ROOT}/plugins/analytics/shared/analytics.lua")
lua_2_c("${CORONA_ROOT}/plugins/analytics/shared/CoronaProvider.analytics.lua")

add_library( analytics SHARED 
				shared/CoronaAnalyticsLibrary.cpp
				${Lua2CppOutputDir}/analytics.c
				${Lua2CppOutputDir}/CoronaProvider.analytics.c )

if(ANDROID AND (ANDROID_ABI STREQUAL "arm64-v8a" OR ANDROID_ABI STREQUAL "x86_64"))
	# NDK version < 28
	target_link_options(analytics PRIVATE "-Wl,-z,max-page-size=16384")
	# NDK version <= 22
	target_link_options(analytics PRIVATE "-Wl,-z,common-page-size=16384")
endif()
target_link_libraries ( analytics lua corona )
