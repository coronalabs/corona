CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
# These are all useless <sigh>
cmake_policy(SET CMP0000 OLD)
cmake_policy(SET CMP0004 OLD)

PROJECT(ALmixer)

INCLUDE(CMakeDependentOption)
SET(CPACK_SET_DESTDIR TRUE)
INCLUDE(CPack)
FIND_PACKAGE(OpenAL)
FIND_PACKAGE(SDL)
# Note: SDL_sound needs to be updated for 2.6+. You'll get annoying warnings.
FIND_PACKAGE(SDL_sound)



SET(ALMIXER_MAJOR_VERSION 0)
SET(ALMIXER_MINOR_VERSION 1)
SET(ALMIXER_PATCH_VERSION 0)

SET(ALMIXER_VERSION ${ALMIXER_MAJOR_VERSION}.${ALMIXER_MINOR_VERSION}.${ALMIXER_PATCH_VERSION})
SET(ALMIXER_COMPATIBILITY_VERSION ${ALMIXER_MAJOR_VERSION}.${ALMIXER_MINOR_VERSION}.0)
# ??? Don't know
SET(ALMIXER_SOVERSION ${ALMIXER_MAJOR_VERSION}.${ALMIXER_MINOR_VERSION}.0)


OPTION(WANTS_BUILD_SHARED_LIBRARY "Set to ON to build dynamic library." ON)
#OPTION(ALMIXER_COMPILE_WITHOUT_SDL "Not supported. Don't use" OFF)
OPTION(ENABLE_ALMIXER_THREADS "Use background thread for ALmixer_Update()" OFF)

IF(APPLE)
	CMAKE_DEPENDENT_OPTION(WANTS_BUILD_FRAMEWORK "Set to ON to build framework instead of dylib. Only valid if BUILD_SHARED_LIBRARY is ON an is OS X." ON "WANTS_BUILD_SHARED_LIBRARY" ON)
ENDIF(APPLE)

SET(ALMIXER_LIBRARY_NAME ALmixer)
IF(APPLE)
	SET(CMAKE_FRAMEWORK_INSTALL_DIR "/Library/Frameworks" CACHE STRING "Directory to install frameworks to.")
	SET(CMAKE_FRAMEWORK_INSTALL_NAME_DIR "@executable_path/../Frameworks" CACHE STRING "install_name path for framework.")
	#SET(CMAKE_DYLIB_INSTALL_NAME_DIR "" CACHE STRING "install_name path for dylib.")
	SET(CMAKE_DYLIB_INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE STRING "install_name path for dylib.")
	SET(ALMIXER_FRAMEWORK_NAME "${ALMIXER_LIBRARY_NAME}.framework")
	SET(ALMIXER_FRAMEWORK_VERSION_NUMBER "A")
	SET(ALMIXER_FRAMEWORK_VERSIONED_EXECUTABLE_DIR "Versions/${ALMIXER_FRAMEWORK_VERSION_NUMBER}/MacOS")
	SET(ALMIXER_FRAMEWORK_VERSIONED_LIB_DIR "Versions/${ALMIXER_FRAMEWORK_VERSION_NUMBER}/lib")
	SET(ALMIXER_FRAMEWORK_CURRENT_EXECUTABLE_DIR "Versions/Current/MacOS")

	# For Apple install_name, is it better to detect if Xcode vs Makefile?
	# Xcode default=1, Makefile=0? Or detect if Framework vs. dylib,
	# Framework=1, dylib=0?
	OPTION(CMAKE_BUILD_WITH_INSTALL_RPATH "Set to YES to set the rpath or install_name on build instead of install." ON)

ELSEIF(UNIX)
	SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" CACHE STRING "rpaths separated by semicolons.")
	OPTION(CMAKE_BUILD_WITH_INSTALL_RPATH "Set to YES to set the rpath or install_name on build instead of install." OFF)

ENDIF(APPLE)



SET(ALMIXER_SOURCE
	${ALmixer_SOURCE_DIR}/ALmixer.c
	${ALmixer_SOURCE_DIR}/CircularQueue.c
	${ALmixer_SOURCE_DIR}/CircularQueue.h
	${ALmixer_SOURCE_DIR}/LinkedList.c
	${ALmixer_SOURCE_DIR}/LinkedList.h
)

SET(PUBLIC_HEADERS
	${ALmixer_SOURCE_DIR}/ALmixer.h
)

#SET(RESOURCE_FILES
#)

ADD_DEFINITIONS(-DALMIXER_BUILD_LIBRARY)
IF(ENABLE_ALMIXER_THREADS)
	ADD_DEFINITIONS(-DENABLE_ALMIXER_THREADS)
ELSE(ENABLE_ALMIXER_THREADS)
	REMOVE_DEFINITIONS(-DENABLE_ALMIXER_THREADS)
ENDIF(ENABLE_ALMIXER_THREADS)



IF(WANTS_BUILD_SHARED_LIBRARY)
	#ADD_LIBRARY(ALmixer SHARED ${PUBLIC_HEADERS} ${ALMIXER_SOURCE} ${RESOURCE_FILES})
	ADD_LIBRARY(ALmixer SHARED ${PUBLIC_HEADERS} ${ALMIXER_SOURCE})
ELSE(WANTS_BUILD_SHARED_LIBRARY)
	ADD_LIBRARY(ALmixer STATIC ${PUBLIC_HEADERS} ${ALMIXER_SOURCE} ${RESOURCE_FILES})
ENDIF(WANTS_BUILD_SHARED_LIBRARY)
#SET_TARGET_PROPERTIES(ALmixer PROPERTIES OUTPUT_NAME "ALmixer")

# Set Dynamic Library and Framework properties
IF(WANTS_BUILD_SHARED_LIBRARY AND WANTS_BUILD_FRAMEWORK)

	SET_TARGET_PROPERTIES(ALmixer PROPERTIES
		FRAMEWORK TRUE
		FRAMEWORK_VERSION "${ALMIXER_FRAMEWORK_VERSION_NUMBER}"
		#  PRIVATE_HEADER "fooPrivate.h;fooBoth.h"
		PUBLIC_HEADER "${PUBLIC_HEADERS}"
		RESOURCE "${RESOURCE_FILES}"
		# INSTALL_NAME_DIR ${CMAKE_FRAMEWORK_INSTALL_NAME_DIR}
		# BUILD_WITH_INSTALL_RPATH ${CMAKE_BUILD_WITH_INSTALL_RPATH} 
		VERSION ${ALMIXER_VERSION}
		SOVERSION ${ALMIXER_COMPATIBILITY_VERSION}
#		COMPILE_FLAGS "${ALMIXER_C_FLAGS}"
	)
	
	# I moved the INSTALL_NAME_DIR to use SET_PROPERTY instead because
	# SET_TARGET_PROPERTIES will fail if the variable is empty.
	SET_PROPERTY(TARGET ALmixer PROPERTY INSTALL_NAME_DIR ${CMAKE_FRAMEWORK_INSTALL_NAME_DIR})
	SET_PROPERTY(TARGET ALmixer PROPERTY BUILD_WITH_INSTALL_RPATH NO)
	# If the user deletes the install_name path, use the gcc default
	# and disable the option completely. But CMake by default places
	# the name of the library in the install_name if 
	# BUILD_WITH_INSTALL_RPATH is ON. So to avoid this, I need to 
	# disable the switch.
	IF(CMAKE_FRAMEWORK_INSTALL_NAME_DIR)
		#		MESSAGE("got dir ${CMAKE_FRAMEWORK_INSTALL_NAME_DIR}")
		SET_PROPERTY(TARGET ALmixer PROPERTY BUILD_WITH_INSTALL_RPATH ${CMAKE_BUILD_WITH_INSTALL_RPATH})

	ELSE(CMAKE_FRAMEWORK_INSTALL_NAME_DIR)
		#		MESSAGE("no dir ${CMAKE_FRAMEWORK_INSTALL_NAME_DIR}")
		SET_PROPERTY(TARGET ALmixer PROPERTY BUILD_WITH_INSTALL_RPATH NO)

	ENDIF(CMAKE_FRAMEWORK_INSTALL_NAME_DIR)



	# Short Version is the "marketing version". It is the version
	# the user sees in an information panel.
	SET(MACOSX_FRAMEWORK_SHORT_VERSION_STRING "${ALMIXER_MAJOR_VERSION}.${ALMIXER_MINOR_VERSION}.${ALMIXER_PATCH_VERSION}")
	# Bundle version is the version the OS looks at.
	SET(MACOSX_FRAMEWORK_BUNDLE_VERSION "${ALMIXER_MAJOR_VERSION}.${ALMIXER_MINOR_VERSION}.${ALMIXER_PATCH_VERSION}")
	SET(MACOSX_FRAMEWORK_IDENTIFIER "net.playcontrol.almixer")

ELSEIF(WANTS_BUILD_SHARED_LIBRARY)
	IF(APPLE)
		SET_TARGET_PROPERTIES(ALmixer PROPERTIES
			FRAMEWORK FALSE
			# INSTALL_NAME_DIR ${CMAKE_DYLIB_INSTALL_NAME_DIR}
			# BUILD_WITH_INSTALL_RPATH ${CMAKE_BUILD_WITH_INSTALL_RPATH} 
			VERSION ${ALMIXER_VERSION}
			SOVERSION ${ALMIXER_COMPATIBILITY_VERSION}
#			COMPILE_FLAGS "${ALMIXER_C_FLAGS}"
		)
		# I moved the INSTALL_NAME_DIR to use SET_PROPERTY instead because
		# SET_TARGET_PROPERTIES will fail if the variable is empty.
		SET_PROPERTY(TARGET ALmixer PROPERTY INSTALL_NAME_DIR ${CMAKE_DYLIB_INSTALL_NAME_DIR})
		SET_PROPERTY(TARGET ALmixer PROPERTY BUILD_WITH_INSTALL_RPATH NO)
		# If the user deletes the install_name path, use the gcc default
		# and disable the option completely. But CMake by default places
		# the name of the library in the install_name if 
		# BUILD_WITH_INSTALL_RPATH is ON. So to avoid this, I need to 
		# disable the switch.
		IF(CMAKE_DYLIB_INSTALL_NAME_DIR)
			SET_PROPERTY(TARGET ALmixer PROPERTY BUILD_WITH_INSTALL_RPATH ${CMAKE_BUILD_WITH_INSTALL_RPATH})
		ELSE(CMAKE_DYLIB_INSTALL_NAME_DIR)
			SET_PROPERTY(TARGET ALmixer PROPERTY BUILD_WITH_INSTALL_RPATH OFF)
		ENDIF(CMAKE_DYLIB_INSTALL_NAME_DIR)



	ELSEIF(UNIX)
		SET_TARGET_PROPERTIES(ALmixer PROPERTIES
			INSTALL_RPATH ${CMAKE_INSTALL_RPATH}
			BUILD_WITH_INSTALL_RPATH ${CMAKE_BUILD_WITH_INSTALL_RPATH} 
			VERSION ${ALMIXER_VERSION}
			SOVERSION ${ALMIXER_SOVERSION}
#			COMPILE_FLAGS "${ALMIXER_C_FLAGS}"
		)
	
	ELSEIF(WIN32)
		#		SET_TARGET_PROPERTIES(ALmixer PROPERTIES
		#	VERSION ${ALMIXER_VERSION}
		#	SOVERSION ${ALMIXER_COMPATIBILITY_VERSION}
		#)

	ELSE(APPLE)
		SET_TARGET_PROPERTIES(ALmixer PROPERTIES
			VERSION ${ALMIXER_VERSION}
			SOVERSION ${ALMIXER_COMPATIBILITY_VERSION}
#			COMPILE_FLAGS "${ALMIXER_C_FLAGS}"
		)

	ENDIF(APPLE)


ENDIF(WANTS_BUILD_SHARED_LIBRARY AND WANTS_BUILD_FRAMEWORK)

INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR} ${SDL_INCLUDE_DIR} ${SDL_SOUND_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(ALmixer ${OPENAL_LIBRARY} ${SDL_SOUND_LIBRARIES} ${SDL_LIBRARY}) 



# For Doxygen
# This will find the Doxygen stuff on your system if you want it.
# I need to use my version because the official version doesn't 
# understand the OS X version of Doxygen. It also doesn't define
# DOT_PATH which is a problem if dot is not in the path.
INCLUDE(${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)
#INCLUDE(${PROJECT_SOURCE_DIR}/CMakeModules/Documentation.cmake OPTIONAL)
OPTION(BUILD_DOCUMENTATION "Build Almixer reference documentation using doxygen (use: make DoxygenDoc)" OFF)
MARK_AS_ADVANCED(CLEAR BUILD_DOCUMENTATION)
# To build the documention, you will have to enable it
# and then do the equivalent of "make DoxygenDoc".
IF(BUILD_DOCUMENTATION)

    OPTION(BUILD_REF_DOCS_SEARCHENGINE "Enable doxygen's search engine (requires that documentation to be installed on a php enabled web server)" OFF)
    IF(BUILD_REF_DOCS_SEARCHENGINE)
        SET(SEARCHENGINE YES)
    ELSE()
        SET(SEARCHENGINE NO)
    ENDIF()

    OPTION(BUILD_REF_DOCS_TAGFILE "Generate a tag file named osg.tag on the documentation web server" OFF)
    IF(BUILD_REF_DOCS_TAGFILE)
		SET(GENERATE_TAGFILE "${PROJECT_BINARY_DIR}/doc/ALmixerDocumentation/ALmixer.tag")
    ELSE()
        SET(GENERATE_TAGFILE "")
    ENDIF()

    IF(DOT)
        SET(HAVE_DOT YES)
    ELSE()
        SET(HAVE_DOT NO)
    ENDIF()
    
    # If html help generation was requested. DOCUMENTATION_HTML_HELP is defined by Documentation.cmake
    SET(GENERATE_HTMLHELP "NO")
    IF(DOCUMENTATION_HTML_HELP)
        # on windows Documentation.cmake finds the html help workshop fi it exists. On u*ix we might have it with wine but no way to point it out
        IF(NOT WIN32)
            SET(HTML_HELP_COMPILER "" CACHE FILEPATH "Enter location of the HTML help compiler to let doxygen compile html")
            MARK_AS_ADVANCED(HTML_HELP_COMPILER)
        ENDIF()
        # this var sets a proper value in .doxygen files when coniguring them below
        SET(GENERATE_HTMLHELP "YES")
    endif()
  
    # This processes our doxyfile.cmake and substitutes paths to generate
    # a final Doxyfile
	CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/doc/ALmixer.doxyfile.cmake
		${PROJECT_BINARY_DIR}/doc/ALmixer.doxyfile
    )
# INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/doc/ALmixerDocumentation DESTINATION doc COMPONENT ALmixer-doc)


    # This creates a new target to build documentation.
    # It runs ${DOXYGEN} which is the full path and executable to
    # Doxygen on your system, set by the FindDoxygen.cmake module
    # (called by FindDocumentation.cmake).
    # It runs the final generated Doxyfile against it.
    # The DOT_PATH is substituted into the Doxyfile.
    ADD_CUSTOM_TARGET(DoxygenDoc ${DOXYGEN}
        ${PROJECT_BINARY_DIR}/doc/ALmixer.doxyfile
    )
ENDIF(BUILD_DOCUMENTATION)




# Install commands below
IF(APPLE AND WANTS_BUILD_SHARED_LIBRARY AND WANTS_BUILD_FRAMEWORK)
	# Will install framework to /Library/Frameworks directory or user specified
	INSTALL(TARGETS
		ALmixer
		FRAMEWORK DESTINATION ${CMAKE_FRAMEWORK_INSTALL_DIR}
	)
ELSE(APPLE AND WANTS_BUILD_SHARED_LIBRARY AND WANTS_BUILD_FRAMEWORK)
	INSTALL(TARGETS
		ALmixer
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		PUBLIC_HEADER DESTINATION include
	)
ENDIF(APPLE AND WANTS_BUILD_SHARED_LIBRARY AND WANTS_BUILD_FRAMEWORK)





# For uninstall (needs cmake_uninstall.cmake.in in the top-level directory)
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


# The Examples
ADD_SUBDIRECTORY(EXAMPLES)



# Set defaults for Universal Binaries. We want 32-bit Intel/PPC on 10.4 
# and 32/64-bit Intel/PPC on >= 10.5. Anything <= 10.3 doesn't support.
IF(APPLE)
    # These are just defaults/recommendations, but how we want to build
    # out of the box. But the user needs to be able to change these options.
    # So we must only set the values the first time CMake is run, or we 
    # will overwrite any changes the user sets.
    # FORCE is used because the options are not reflected in the UI otherwise.
    # Seems like a good place to add version specific compiler flags too.
    IF(NOT ALMIXER_CONFIG_HAS_BEEN_RUN_BEFORE)
        # This is really fragile, but CMake doesn't provide the OS system 
        # version information we need. (Darwin versions can be changed 
        # independently of OS X versions.)
        # It does look like CMake handles the CMAKE_OSX_SYSROOT automatically.
        IF(EXISTS /Developer/SDKs/MacOSX10.5.sdk)
            SET(CMAKE_OSX_ARCHITECTURES "ppc;i386;x86_64" CACHE STRING "Build architectures for OSX" FORCE)
            SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.5" CACHE STRING "Flags used by the compiler during all build types." FORCE)
        ELSE(EXISTS /Developer/SDKs/MacOSX10.5.sdk)
            IF(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
                SET(CMAKE_OSX_ARCHITECTURES "ppc;i386" CACHE STRING "Build architectures for OSX" FORCE)
                SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.4" CACHE STRING "Flags used by the compiler during all build types." FORCE)
            ELSE(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
                # No Universal Binary support
                # Should break down further to set the -mmacosx-version-min,
                # but the SDK detection is too unreliable here.
            ENDIF(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
        ENDIF(EXISTS /Developer/SDKs/MacOSX10.5.sdk)
    ENDIF(NOT ALMIXER_CONFIG_HAS_BEEN_RUN_BEFORE)
ENDIF(APPLE)

# This needs to be run very last so other parts of the scripts can take
# advantage of this.
IF(NOT ALMIXER_CONFIG_HAS_BEEN_RUN_BEFORE)
    SET(ALMIXER_CONFIG_HAS_BEEN_RUN_BEFORE 1 CACHE INTERNAL "Flag to track whether this is the first time running CMake or if CMake has been configured before")
ENDIF(NOT ALMIXER_CONFIG_HAS_BEEN_RUN_BEFORE)



