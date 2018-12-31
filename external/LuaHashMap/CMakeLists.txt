CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
# Simple CMake example to build the Logger library and a test program.
# This will build logger into a library and build a program 
# that links against it.

# First you must define a project name. (This probably makes more sense to 
# you if you have Visual Studio experience.)
PROJECT(LuaHashMap)

SET(LUAHASHMAP_MAJOR_VERSION 0)
SET(LUAHASHMAP_MINOR_VERSION 2)
SET(LUAHASHMAP_PATCH_VERSION 0)

SET(LUAHASHMAP_VERSION ${LUAHASHMAP_MAJOR_VERSION}.${LUAHASHMAP_MINOR_VERSION}.${LUAHASHMAP_PATCH_VERSION})
SET(LUAHASHMAP_COMPATIBILITY_VERSION ${LUAHASHMAP_MAJOR_VERSION}.${LUAHASHMAP_MINOR_VERSION}.0)
# ??? Don't know
SET(LUAHASHMAP_SOVERSION ${LUAHASHMAP_MAJOR_VERSION}.${LUAHASHMAP_MINOR_VERSION}.0)



INCLUDE(CMakeDependentOption)
SET(CPACK_SET_DESTDIR TRUE)
INCLUDE(CPack)

IF(USING_LUA_MULTI_SET)
	IF(NOT LUA_LIBRARIES)
		IF(lua_library_module)
			SET(LUA_LIBRARIES lua_library_module)
		ELSE(lua_library_module)
			SET(LUA_LIBRARIES lua_library_static)
		ENDIF(lua_library_module)
	ENDIF(NOT LUA_LIBRARIES)
	IF(NOT LUA_INCLUDE_DIR)
		SET(LUA_INCLUDE_DIR ${Lua_SOURCE_DIR}/src)
	ENDIF(NOT LUA_INCLUDE_DIR)
ELSE(USING_LUA_MULTI_SET)
	FIND_PACKAGE(Lua51 REQUIRED)
ENDIF(USING_LUA_MULTI_SET)


INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})


OPTION(WANTS_BUILD_SHARED_LIBRARY "Set to ON to build dynamic library." ON)

IF(APPLE)
	CMAKE_DEPENDENT_OPTION(WANTS_BUILD_FRAMEWORK "Set to ON to build framework instead of dylib. Only valid if BUILD_SHARED_LIBRARY is ON an is OS X." ON "WANTS_BUILD_SHARED_LIBRARY" ON)
ENDIF(APPLE)

SET(LUAHASHMAP_LIBRARY_NAME LuaHashMap)
IF(APPLE)
	SET(CMAKE_FRAMEWORK_INSTALL_DIR "/Library/Frameworks" CACHE STRING "Directory to install frameworks to.")
	SET(CMAKE_FRAMEWORK_INSTALL_NAME_DIR "@rpath" CACHE STRING "install_name path for framework.")
	#SET(CMAKE_DYLIB_INSTALL_NAME_DIR "" CACHE STRING "install_name path for dylib.")
	SET(CMAKE_DYLIB_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE STRING "install_name path for dylib.")
	#	SET(LUAHASHMAP_FRAMEWORK_NAME "${LUAHASHMAP_LIBRARY_NAME}.framework")
	#	SET(LUAHASHMAP_FRAMEWORK_VERSION_NUMBER "5.1")
	#	SET(LUAHASHMAP_FRAMEWORK_VERSIONED_EXECUTABLE_DIR "Versions/${LUAHASHMAP_FRAMEWORK_VERSION_NUMBER}/MacOS")
	#	SET(LUAHASHMAP_FRAMEWORK_VERSIONED_LIB_DIR "Versions/${LUAHASHMAP_FRAMEWORK_VERSION_NUMBER}/lib")
	#	SET(LUAHASHMAP_FRAMEWORK_CURRENT_EXECUTABLE_DIR "Versions/Current/MacOS")

	# For Apple install_name, is it better to detect if Xcode vs Makefile?
	# Xcode default=1, Makefile=0? Or detect if Framework vs. dylib,
	# Framework=1, dylib=0?
	OPTION(CMAKE_BUILD_WITH_INSTALL_RPATH "Set to YES to set the rpath or install_name on build instead of install." ON)

ELSEIF(UNIX)
	SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" CACHE STRING "rpaths separated by semicolons.")
	OPTION(CMAKE_BUILD_WITH_INSTALL_RPATH "Set to YES to set the rpath or install_name on build instead of install." OFF)

ENDIF(APPLE)




# This will find the Doxygen stuff on your system if you want it.
#INCLUDE(${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)
# I need to use my version because the official version doesn't 
# understand the OS X version of Doxygen. It also doesn't define
# DOT_PATH which is a problem if dot is not in the path.
#INCLUDE(${PROJECT_SOURCE_DIR}/CMakeModules/Documentation.cmake OPTIONAL)
INCLUDE(${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)

# Allow the user to specify if Logger should be compiled with or without
# thread/file locking.
OPTION(LUAHASHMAP_ENABLE_DEBUG "Compiles LuaHashMap with internal debug assertions" OFF)

# By default, BUILD_DOCUMENTATION and CMAKE_VERBOSE_MAKEFILE
# are marked as advanced (hidden in the advanced menu).
# I want these options to be seen in the basic menu, so I will
# call the function to toggle them back.
MARK_AS_ADVANCED(CLEAR
	BUILD_DOCUMENTATION
	CMAKE_VERBOSE_MAKEFILE
)

# If the user wants to compile the library with locking, add the 
# appropriate -D flags.
IF(LUAHASHMAP_ENABLE_DEBUG)
	ADD_DEFINITIONS(-DLUAHASHMAP_DEBUG)
ENDIF(LUAHASHMAP_ENABLE_DEBUG)

SET(PUBLIC_HEADERS
	${LuaHashMap_SOURCE_DIR}/LuaHashMap.h
)
SET(LUAHASHMAP_LIBRARY_FILES
	${LuaHashMap_SOURCE_DIR}/LuaHashMap.c
)

IF(WANTS_BUILD_SHARED_LIBRARY)
	ADD_LIBRARY(luahashmap_library_dynamic SHARED ${PUBLIC_HEADERS} ${LUAHASHMAP_LIBRARY_FILES})
	SET_TARGET_PROPERTIES(luahashmap_library_dynamic PROPERTIES OUTPUT_NAME "LuaHashMap")
	#	SET_TARGET_PROPERTIES(luahashmap_library_dynamic PROPERTIES COMPILE_FLAGS "${LUAHASHMAP_C_FLAGS}")
ENDIF(WANTS_BUILD_SHARED_LIBRARY)
# For convenience, I build the static library no matter what,
# but I just want install it if not selected.
#IF(WANTS_BUILD_STATIC_LIBRARY)
	ADD_LIBRARY(luahashmap_library_static STATIC ${PUBLIC_HEADERS} ${LUAHASHMAP_LIBRARY_FILES})
#ENDIF(WANTS_BUILD_STATIC_LIBRARY)

SET_TARGET_PROPERTIES(luahashmap_library_static PROPERTIES OUTPUT_NAME "LuaHashMap")
#SET_TARGET_PROPERTIES(luahashmap_library_static PROPERTIES COMPILE_FLAGS "${LUAHASHMAP_C_FLAGS}")

# Set Dynamic Library and Framework properties
IF(WANTS_BUILD_SHARED_LIBRARY AND WANTS_BUILD_FRAMEWORK)

	SET_TARGET_PROPERTIES(luahashmap_library_dynamic PROPERTIES
		FRAMEWORK TRUE
		#		FRAMEWORK_VERSION "${LUAHASHMAP_FRAMEWORK_VERSION_NUMBER}"
		#  PRIVATE_HEADER "fooPrivate.h;fooBoth.h"
		PUBLIC_HEADER "${PUBLIC_HEADERS}"
		#		RESOURCE "${RESOURCE_FILES}"
		# INSTALL_NAME_DIR ${CMAKE_FRAMEWORK_INSTALL_NAME_DIR}
		# BUILD_WITH_INSTALL_RPATH ${CMAKE_BUILD_WITH_INSTALL_RPATH} 
		#		VERSION ${LUAHASHMAP_VERSION}
		#		SOVERSION ${LUAHASHMAP_COMPATIBILITY_VERSION}
		COMPILE_FLAGS "${LUAHASHMAP_C_FLAGS} -DLUAHASHMAP_BUILD_AS_DLL"
	)
	
	# I moved the INSTALL_NAME_DIR to use SET_PROPERTY instead because
	# SET_TARGET_PROPERTIES will fail if the variable is empty.
	SET_PROPERTY(TARGET luahashmap_library_dynamic PROPERTY INSTALL_NAME_DIR ${CMAKE_FRAMEWORK_INSTALL_NAME_DIR})
	SET_PROPERTY(TARGET luahashmap_library_dynamic PROPERTY BUILD_WITH_INSTALL_RPATH NO)
	# If the user deletes the install_name path, use the gcc default
	# and disable the option completely. But CMake by default places
	# the name of the library in the install_name if 
	# BUILD_WITH_INSTALL_RPATH is ON. So to avoid this, I need to 
	# disable the switch.
	IF(CMAKE_FRAMEWORK_INSTALL_NAME_DIR)
		#		MESSAGE("got dir ${CMAKE_FRAMEWORK_INSTALL_NAME_DIR}")
		SET_PROPERTY(TARGET luahashmap_library_dynamic PROPERTY BUILD_WITH_INSTALL_RPATH ${CMAKE_BUILD_WITH_INSTALL_RPATH})

	ELSE(CMAKE_FRAMEWORK_INSTALL_NAME_DIR)
		#		MESSAGE("no dir ${CMAKE_FRAMEWORK_INSTALL_NAME_DIR}")
		SET_PROPERTY(TARGET luahashmap_library_dynamic PROPERTY BUILD_WITH_INSTALL_RPATH NO)

	ENDIF(CMAKE_FRAMEWORK_INSTALL_NAME_DIR)



	# Short Version is the "marketing version". It is the version
	# the user sees in an information panel.
	SET(MACOSX_FRAMEWORK_SHORT_VERSION_STRING "${LUAHASHMAP_MAJOR_VERSION}.${LUAHASHMAP_MINOR_VERSION}.${LUAHASHMAP_PATCH_VERSION}")
	# Bundle version is the version the OS looks at.
	SET(MACOSX_FRAMEWORK_BUNDLE_VERSION "${LUAHASHMAP_MAJOR_VERSION}.${LUAHASHMAP_MINOR_VERSION}.${LUAHASHMAP_PATCH_VERSION}")
	SET(MACOSX_FRAMEWORK_IDENTIFIER "org.lua.lua")
	#	SET(MACOSX_BUNDLE_BUNDLE_NAME "lua" )

	TARGET_LINK_LIBRARIES(luahashmap_library_dynamic
		${LUA_LIBRARIES}
	)


ELSEIF(WANTS_BUILD_SHARED_LIBRARY)
	IF(APPLE)
		SET_TARGET_PROPERTIES(luahashmap_library_dynamic PROPERTIES
			FRAMEWORK FALSE
			PUBLIC_HEADER "${PUBLIC_HEADERS}"			
			# INSTALL_NAME_DIR ${CMAKE_DYLIB_INSTALL_NAME_DIR}
			# BUILD_WITH_INSTALL_RPATH ${CMAKE_BUILD_WITH_INSTALL_RPATH} 
			VERSION ${LUAHASHMAP_VERSION}
			SOVERSION ${LUAHASHMAP_COMPATIBILITY_VERSION}
			COMPILE_FLAGS "${LUAHASHMAP_C_FLAGS} -DLUAHASHMAP_BUILD_AS_DLL"
		)
		# I moved the INSTALL_NAME_DIR to use SET_PROPERTY instead because
		# SET_TARGET_PROPERTIES will fail if the variable is empty.
		SET_PROPERTY(TARGET luahashmap_library_dynamic PROPERTY INSTALL_NAME_DIR ${CMAKE_DYLIB_INSTALL_NAME_DIR})
		SET_PROPERTY(TARGET luahashmap_library_dynamic PROPERTY BUILD_WITH_INSTALL_RPATH NO)
		# If the user deletes the install_name path, use the gcc default
		# and disable the option completely. But CMake by default places
		# the name of the library in the install_name if 
		# BUILD_WITH_INSTALL_RPATH is ON. So to avoid this, I need to 
		# disable the switch.
		IF(CMAKE_DYLIB_INSTALL_NAME_DIR)
			SET_PROPERTY(TARGET luahashmap_library_dynamic PROPERTY BUILD_WITH_INSTALL_RPATH ${CMAKE_BUILD_WITH_INSTALL_RPATH})
		ELSE(CMAKE_DYLIB_INSTALL_NAME_DIR)
			SET_PROPERTY(TARGET luahashmap_library_dynamic PROPERTY BUILD_WITH_INSTALL_RPATH OFF)
		ENDIF(CMAKE_DYLIB_INSTALL_NAME_DIR)



	ELSEIF(UNIX)
		SET_TARGET_PROPERTIES(luahashmap_library_dynamic PROPERTIES
			PUBLIC_HEADER "${PUBLIC_HEADERS}"				
			INSTALL_RPATH ${CMAKE_INSTALL_RPATH}
			BUILD_WITH_INSTALL_RPATH ${CMAKE_BUILD_WITH_INSTALL_RPATH} 
			VERSION ${LUAHASHMAP_VERSION}
			SOVERSION ${LUAHASHMAP_SOVERSION}
			COMPILE_FLAGS "${LUAHASHMAP_C_FLAGS} -DLUAHASHMAP_BUILD_AS_DLL"
		)
	
	ELSEIF(WIN32)
		SET_TARGET_PROPERTIES(luahashmap_library_dynamic PROPERTIES
			PUBLIC_HEADER "${PUBLIC_HEADERS}"			
			VERSION ${LUAHASHMAP_VERSION}
			SOVERSION ${LUAHASHMAP_COMPATIBILITY_VERSION}
			COMPILE_FLAGS "${LUAHASHMAP_C_FLAGS} -DLUAHASHMAP_BUILD_AS_DLL"
		)

	ELSE(APPLE)
		SET_TARGET_PROPERTIES(luahashmap_library_dynamic PROPERTIES
			PUBLIC_HEADER "${PUBLIC_HEADERS}"						
			VERSION ${LUAHASHMAP_VERSION}
			SOVERSION ${LUAHASHMAP_COMPATIBILITY_VERSION}
			COMPILE_FLAGS "${LUAHASHMAP_C_FLAGS} -DLUAHASHMAP_BUILD_AS_DLL"
		)

	ENDIF(APPLE)

	TARGET_LINK_LIBRARIES(luahashmap_library_dynamic
		${LUA_LIBRARIES}
	)

ENDIF(WANTS_BUILD_SHARED_LIBRARY AND WANTS_BUILD_FRAMEWORK)

# Setup properties for static library
IF(APPLE)
	SET_TARGET_PROPERTIES(luahashmap_library_static PROPERTIES
		PUBLIC_HEADER "${PUBLIC_HEADERS}"					
		VERSION ${LUAHASHMAP_VERSION}
		SOVERSION ${LUAHASHMAP_COMPATIBILITY_VERSION}
	)

ELSEIF(UNIX)
	SET_TARGET_PROPERTIES(luahashmap_library_static PROPERTIES
		PUBLIC_HEADER "${PUBLIC_HEADERS}"					
		VERSION ${LUAHASHMAP_VERSION}
		SOVERSION ${LUAHASHMAP_SOVERSION}
	)

ELSE(APPLE)
	SET_TARGET_PROPERTIES(luahashmap_library_static PROPERTIES
		PUBLIC_HEADER "${PUBLIC_HEADERS}"					
		VERSION ${LUAHASHMAP_VERSION}
		SOVERSION ${LUAHASHMAP_COMPATIBILITY_VERSION}
	)

ENDIF(APPLE)


# test programs that use the library
# As with the ADD_LIBRARY command, you should list all the files
# that are needed to build the executable.
ADD_EXECUTABLE(luahashtest
	luahashtest.c
)
ADD_EXECUTABLE(luahashtestshared
	luahashtestshared.c
)
ADD_EXECUTABLE(luahashtestcpp
	luahashtest.cpp
)
# This tells the build system that the executable must
# be linked against which libraries.
TARGET_LINK_LIBRARIES(luahashtest
	luahashmap_library_static
	${LUA_LIBRARIES}
)
TARGET_LINK_LIBRARIES(luahashtestshared
	luahashmap_library_static
	${LUA_LIBRARIES}
)
TARGET_LINK_LIBRARIES(luahashtestcpp
	luahashmap_library_static
	${LUA_LIBRARIES}
)


# To build the documention, you will have to enable it
# and then do the equivalent of "make doc".
IF (BUILD_DOCUMENTATION)
	IF (DOT)
		SET(HAVE_DOT YES)
	ELSE (DOT)
		SET(HAVE_DOT NO)
	ENDIF (DOT)
	# This processes our Doxyfile.in and substitutes paths to generate
	# a final Doxyfile
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in 
		${PROJECT_BINARY_DIR}/doc/Doxyfile)
	# This creates a new target to build documentation.
	# It runs ${DOXYGEN} which is the full path and executable to
	# Doxygen on your system, set by the FindDoxygen.cmake module
	# (called by FindDocumentation.cmake).
	# It runs the final generated Doxyfile against it.
	# The DOT_PATH is substituted into the Doxyfile.
	ADD_CUSTOM_TARGET(DoxygenDoc ${DOXYGEN}
		${PROJECT_BINARY_DIR}/doc/Doxyfile)
ENDIF (BUILD_DOCUMENTATION)




# Install commands below
IF(APPLE AND WANTS_BUILD_SHARED_LIBRARY AND WANTS_BUILD_FRAMEWORK)
	# Will install framework to /Library/Frameworks directory or user specified
	INSTALL(TARGETS
		luahashmap_library_dynamic
		FRAMEWORK DESTINATION ${CMAKE_FRAMEWORK_INSTALL_DIR}
	)
ELSE()
	IF(WANTS_BUILD_SHARED_LIBRARY)
		SET(WHICH_TARGETS_TO_INSTALL "luahashmap_library_dynamic;luahashmap_library_static")
	ELSE()
		SET(WHICH_TARGETS_TO_INSTALL "luahashmap_library_static")		
	ENDIF()

	INSTALL(TARGETS
		${WHICH_TARGETS_TO_INSTALL}
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		PUBLIC_HEADER DESTINATION include
	)
ENDIF()



# For uninstall (needs cmake_uninstall.cmake.in in the top-level directory)
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


