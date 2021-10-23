cmake_minimum_required(VERSION 3.4.1)

project("Solar2D")

# Set a default build type if none was specified

#set(default_build_type "Release")
set(default_build_type "Debug")
if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")

  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)

  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


get_filename_component(CORONA_ROOT "${CMAKE_SOURCE_DIR}" ABSOLUTE)
message("CORONA_ROOT: " ${CORONA_ROOT})

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
	
	set(ALSOFT_NATIVE_TOOLS_PATH "${CORONA_ROOT}/bin/linux" CACHE STRING "Path to prebuilt native tools (leave blank to auto-build)" FORCE)
	set(Lua2CppOutputDir "${CMAKE_CURRENT_BINARY_DIR}/generated_src")
	set(LuaIncludePath "${CORONA_ROOT}/external/lua-5.1.3/src")

	include(CMakeGenLua.cmake)
	include(platform/linux/CMakeList.txt)

else()
	message(FATAL_ERROR "CMakeList.txt is not ported to current OS: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

