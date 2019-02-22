cmake_minimum_required(VERSION 3.4.1)
project( ads )
include_directories ( ${LibPluginAdsPath} ${LibNdkGenSharedPath} ${CORONA_ROOT}/librtt/Corona ${LibLuaSoPath})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DANDROID_NDK -DNDEBUG -D_REENTRANT -DRtt_ANDROID_ENV")

lua_2_c("${CORONA_ROOT}/plugins/ads/shared/ads.lua")
lua_2_c("${CORONA_ROOT}/plugins/ads/shared/CoronaProvider.ads.lua")

add_library( ads SHARED 
				${LibPluginAdsPath}/shared/CoronaAdsLibrary.cpp
				${LibNdkGenSharedPath}/ads.c
				${LibNdkGenSharedPath}/CoronaProvider.ads.c )

target_link_libraries ( ads lua corona )