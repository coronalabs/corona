cmake_minimum_required(VERSION 3.13)
project( ads )

include_directories( 
	${LuaIncludePath}
	${CORONA_ROOT}/librtt/Corona
)

lua_2_c("${CORONA_ROOT}/plugins/ads/shared/ads.lua")
lua_2_c("${CORONA_ROOT}/plugins/ads/shared/CoronaProvider.ads.lua")

add_library( ads SHARED 
				shared/CoronaAdsLibrary.cpp
				${Lua2CppOutputDir}/ads.c
				${Lua2CppOutputDir}/CoronaProvider.ads.c )

if(ANDROID AND (ANDROID_ABI STREQUAL "arm64-v8a" OR ANDROID_ABI STREQUAL "x86_64"))
	# NDK version < 28
	target_link_options(ads PRIVATE "-Wl,-z,max-page-size=16384")
	# NDK version <= 22
	target_link_options(ads PRIVATE "-Wl,-z,common-page-size=16384")
endif()
target_link_libraries ( ads lua corona )
