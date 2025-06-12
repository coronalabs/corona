cmake_minimum_required(VERSION 3.4.1)
project( ads )
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,max-page-size=16384")
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

target_link_libraries ( ads lua corona )
