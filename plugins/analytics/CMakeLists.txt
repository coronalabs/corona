cmake_minimum_required(VERSION 3.4.1)
project( analytics )
include_directories (
    ${LibPlAnalyticsPath}
    ${LibNdkGenSharedPath}
    ${CORONA_ROOT}/librtt/Corona
    ${LibLuaSoPath}
)

lua_2_c("${CORONA_ROOT}/plugins/analytics/shared/analytics.lua")
lua_2_c("${CORONA_ROOT}/plugins/analytics/shared/CoronaProvider.analytics.lua")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DANDROID_NDK -DNDEBUG -D_REENTRANT -DRtt_ANDROID_ENV")

add_library( analytics SHARED 
				${LibPlAnalyticsPath}/shared/CoronaAnalyticsLibrary.cpp
				${LibNdkGenSharedPath}/analytics.c
				${LibNdkGenSharedPath}/CoronaProvider.analytics.c )

target_link_libraries ( analytics lua corona )