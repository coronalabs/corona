cmake_minimum_required(VERSION 3.4.1)

project("CoronaAAR")

add_definitions( -DNDEBUG -DANDROID_NDK -DRtt_ANDROID_ENV )

get_filename_component(CORONA_ROOT "${CMAKE_SOURCE_DIR}/../../.." ABSOLUTE)

set(Lua2CppOutputDir "${CMAKE_CURRENT_BINARY_DIR}/generated_src")
set(LuaIncludePath "${CORONA_ROOT}/external/lua-5.1.3/src")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384")

include(AndroidNdkModules)
android_ndk_import_module_cpufeatures()
include(CMakeGenLua.cmake)

add_subdirectory("../ndk" ${CMAKE_CURRENT_BINARY_DIR}/libcorona_build/)

SET(ALSOFT_UTILS OFF CACHE BOOL "")
SET(ALSOFT_EXAMPLES OFF CACHE BOOL "")
SET(ALSOFT_TESTS OFF CACHE BOOL "")
SET(ALSOFT_CONFIG OFF CACHE BOOL "")
SET(ALSOFT_HRTF_DEFS OFF CACHE BOOL "")
SET(ALSOFT_AMBDEC_PRESETS OFF CACHE BOOL "")
SET(ALSOFT_INSTALL OFF CACHE BOOL "")
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    SET(ALSOFT_NATIVE_TOOLS_PATH "${CORONA_ROOT}/bin/win" CACHE STRING "Path to prebuilt native tools (leave blank to auto-build)" FORCE)
elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    SET(ALSOFT_NATIVE_TOOLS_PATH "${CORONA_ROOT}/bin/mac" CACHE STRING "Path to prebuilt native tools (leave blank to auto-build)" FORCE)
endif()
add_subdirectory("${CORONA_ROOT}/external/openal-soft" "${CMAKE_CURRENT_BINARY_DIR}/libopenal_build/")

add_subdirectory("${CORONA_ROOT}/external/lua-5.1.3" "${CMAKE_CURRENT_BINARY_DIR}/liblua_build/")
add_subdirectory("${CORONA_ROOT}/external/JNLua/src/main/c" "${CMAKE_CURRENT_BINARY_DIR}/libjnlua5.1_build/")
add_subdirectory("${CORONA_ROOT}/external/mpg123-1.13.1/src/libmpg123" "${CMAKE_CURRENT_BINARY_DIR}/libmpg123_build/")
add_subdirectory("${CORONA_ROOT}/external/ALmixer" "${CMAKE_CURRENT_BINARY_DIR}/libalmixer_build/")
add_subdirectory("${CORONA_ROOT}/plugins/ads" "${CMAKE_CURRENT_BINARY_DIR}/libads_build/")
add_subdirectory("${CORONA_ROOT}/plugins/analytics" "${CMAKE_CURRENT_BINARY_DIR}/libanalytics_build/")
add_subdirectory("${CORONA_ROOT}/plugins/licensing/android" "${CMAKE_CURRENT_BINARY_DIR}/liblicensing_build/")
add_subdirectory("${CORONA_ROOT}/plugins/gameNetwork/android" "${CMAKE_CURRENT_BINARY_DIR}/libgameNetwork_build/")
