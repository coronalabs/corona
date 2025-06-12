cmake_minimum_required(VERSION 3.4.1)
project( analytics )
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,max-page-size=16384")
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

target_link_libraries ( analytics lua corona )
