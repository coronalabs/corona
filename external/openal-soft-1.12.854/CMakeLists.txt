# CMake build file list for OpenAL

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckSharedLibraryExists)
INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckTypeSize)


PROJECT(OpenAL C)


SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)


OPTION(ALSA    "Check for ALSA backend"                ON)
OPTION(OSS     "Check for OSS backend"                 ON)
OPTION(SOLARIS "Check for Solaris backend"             ON)
OPTION(DSOUND  "Check for DirectSound backend"         ON)
OPTION(WINMM   "Check for Windows Multimedia backend"  ON)
OPTION(PORTAUDIO  "Check for PortAudio backend"        ON)
OPTION(PULSEAUDIO "Check for PulseAudio backend"       ON)

OPTION(DLOPEN  "Check for the dlopen API for loading optional libs"  ON)

OPTION(WERROR  "Treat compile warnings as errors"      OFF)

OPTION(EXAMPLES  "Build example programs"              ON)

OPTION(ALSOFT_CONFIG "Install alsoft.conf configuration file" OFF)


IF(WIN32)
    SET(LIBNAME OpenAL32)
    ADD_DEFINITIONS("-D_WIN32")
ELSE()
    SET(LIBNAME openal)
ENDIF()

SET(LIB_MAJOR_VERSION "1")
SET(LIB_MINOR_VERSION "12")
SET(LIB_BUILD_VERSION "854")
SET(LIB_VERSION "${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_BUILD_VERSION}")
IF(NOT DEFINED LIB_INSTALL_DIR)
  SET(LIB_INSTALL_DIR "lib${LIB_SUFFIX}")
ENDIF(NOT DEFINED LIB_INSTALL_DIR)


CHECK_TYPE_SIZE("long" SIZEOF_LONG)
CHECK_TYPE_SIZE("long long" SIZEOF_LONG_LONG)
CHECK_TYPE_SIZE("unsigned int" SIZEOF_UINT)
CHECK_TYPE_SIZE("void*" SIZEOF_VOIDP)


# Add definitions, compiler switches, etc.
INCLUDE_DIRECTORIES(OpenAL32/Include include "${OpenAL_BINARY_DIR}")

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
        "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
ENDIF()
IF(NOT CMAKE_DEBUG_POSTFIX)
    SET(CMAKE_DEBUG_POSTFIX "" CACHE STRING
        "Library postfix for debug builds. Normally left blank."
        FORCE)
ENDIF()

IF(MSVC)
    # ???
    SET(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -D_DEBUG")
    SET(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -DNDEBUG")
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_DEPRECATE)

    IF(NOT DXSDK_DIR)
        STRING(REGEX REPLACE "\\\\" "/" DXSDK_DIR "$ENV{DXSDK_DIR}")
    ELSE()
        STRING(REGEX REPLACE "\\\\" "/" DXSDK_DIR "${DXSDK_DIR}")
    ENDIF()
    IF(DXSDK_DIR)
        MESSAGE(STATUS "Using DirectX SDK directory: ${DXSDK_DIR}")
        SET(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} "${DXSDK_DIR}/Include")
        INCLUDE_DIRECTORIES("${DXSDK_DIR}/Include")
        LINK_DIRECTORIES("${DXSDK_DIR}/Lib")
    ENDIF()

ELSE()
    ADD_DEFINITIONS(-Wall)
    CHECK_C_COMPILER_FLAG(-Wextra HAVE_W_EXTRA)
    IF(HAVE_W_EXTRA)
        ADD_DEFINITIONS(-Wextra)
    ENDIF()

    IF(WERROR)
        ADD_DEFINITIONS(-Werror)
    ENDIF()

    SET(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O2 -funroll-loops -D_DEBUG" CACHE STRING
        "Flags used by the compiler during Release with Debug Info builds."
        FORCE)
    SET(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG" CACHE STRING
        "Flags used by the compiler during release minsize builds."
        FORCE)
    SET(CMAKE_C_FLAGS_RELEASE "-O2 -funroll-loops -fomit-frame-pointer -DNDEBUG" CACHE STRING
        "Flags used by the compiler during release builds"
        FORCE)
    SET(CMAKE_C_FLAGS_DEBUG "-g3 -D_DEBUG" CACHE STRING
        "Flags used by the compiler during debug builds."
        FORCE)

    # Set visibility options if available
    IF(NOT WIN32)
        CHECK_C_SOURCE_COMPILES("int foo() __attribute__((destructor));
                                 int main() {return 0;}" HAVE_GCC_DESTRUCTOR)

        CHECK_C_COMPILER_FLAG(-fvisibility=hidden HAVE_VISIBILITY_SWITCH)
        IF(HAVE_VISIBILITY_SWITCH)
            CHECK_C_SOURCE_COMPILES("int foo() __attribute__((visibility(\"default\")));
                                     int main() {return 0;}" HAVE_GCC_VISIBILITY)
            IF(HAVE_GCC_VISIBILITY)
                ADD_DEFINITIONS(-fvisibility=hidden -DHAVE_GCC_VISIBILITY)
            ENDIF()
        ENDIF()
    ENDIF()
ENDIF()

CHECK_C_SOURCE_COMPILES("int foo(const char *str, ...) __attribute__((format(printf, 1, 2)));
                         int main() {return 0;}" HAVE_GCC_FORMAT)

CHECK_INCLUDE_FILE(fenv.h HAVE_FENV_H)
CHECK_INCLUDE_FILE(float.h HAVE_FLOAT_H)

CHECK_LIBRARY_EXISTS(m  powf   "" HAVE_POWF)
CHECK_LIBRARY_EXISTS(m  sqrtf  "" HAVE_SQRTF)
CHECK_LIBRARY_EXISTS(m  acosf  "" HAVE_ACOSF)
CHECK_LIBRARY_EXISTS(m  atanf  "" HAVE_ATANF)
CHECK_LIBRARY_EXISTS(m  fabsf  "" HAVE_FABSF)
IF(HAVE_FENV_H)
    CHECK_LIBRARY_EXISTS(m  fesetround  "" HAVE_FESETROUND)
ENDIF()
IF(HAVE_SQRTF OR HAVE_ACOSF OR HAVE_ATANF OR HAVE_FABSF OR HAVE_FESETROUND)
    SET(EXTRA_LIBS m ${EXTRA_LIBS})
ENDIF()
CHECK_FUNCTION_EXISTS(strtof HAVE_STRTOF)
CHECK_FUNCTION_EXISTS(_controlfp HAVE__CONTROLFP)

CHECK_FUNCTION_EXISTS(stat HAVE_STAT)
CHECK_FUNCTION_EXISTS(strcasecmp HAVE_STRCASECMP)
IF(NOT HAVE_STRCASECMP)
    CHECK_FUNCTION_EXISTS(_stricmp HAVE__STRICMP)
    IF(NOT HAVE__STRICMP)
        MESSAGE(FATAL_ERROR "No case-insensitive compare function found, please report!")
    ENDIF()

    ADD_DEFINITIONS(-Dstrcasecmp=_stricmp)
ENDIF()

CHECK_FUNCTION_EXISTS(strncasecmp HAVE_STRNCASECMP)
IF(NOT HAVE_STRNCASECMP)
    CHECK_FUNCTION_EXISTS(_strnicmp HAVE__STRNICMP)
    IF(NOT HAVE__STRNICMP)
        MESSAGE(FATAL_ERROR "No case-insensitive size-limitted compare function found, please report!")
    ENDIF()

    ADD_DEFINITIONS(-Dstrncasecmp=_strnicmp)
ENDIF()

CHECK_FUNCTION_EXISTS(snprintf HAVE_SNPRINTF)
IF(NOT HAVE_SNPRINTF)
    CHECK_FUNCTION_EXISTS(_snprintf HAVE__SNPRINTF)
    IF(NOT HAVE__SNPRINTF)
        MESSAGE(FATAL_ERROR "No snprintf function found, please report!")
    ENDIF()

    ADD_DEFINITIONS(-Dsnprintf=_snprintf)
ENDIF()

CHECK_FUNCTION_EXISTS(vsnprintf HAVE_VSNPRINTF)
IF(NOT HAVE_VSNPRINTF)
    CHECK_FUNCTION_EXISTS(_vsnprintf HAVE__VSNPRINTF)
    IF(NOT HAVE__VSNPRINTF)
        MESSAGE(FATAL_ERROR "No vsnprintf function found, please report!")
    ENDIF()

    ADD_DEFINITIONS(-Dvsnprintf=_vsnprintf)
ENDIF()

CHECK_SYMBOL_EXISTS(isnan math.h HAVE_ISNAN)
IF(NOT HAVE_ISNAN)
    CHECK_FUNCTION_EXISTS(_isnan HAVE__ISNAN)
    IF(NOT HAVE__ISNAN)
        MESSAGE(FATAL_ERROR "No isnan function found, please report!")
    ENDIF()

    ADD_DEFINITIONS(-Disnan=_isnan)
ENDIF()


# Check for the dlopen API (for dynamicly loading backend libs)
IF(DLOPEN)
    CHECK_INCLUDE_FILE(dlfcn.h HAVE_DLFCN_H)
    IF(HAVE_DLFCN_H)
        CHECK_LIBRARY_EXISTS(dl dlopen "" HAVE_LIBDL)
        IF(HAVE_LIBDL)
            SET(EXTRA_LIBS dl ${EXTRA_LIBS})
        ENDIF()
    ENDIF()
ENDIF()

# Check if we have Windows headers
CHECK_INCLUDE_FILE(windows.h HAVE_WINDOWS_H -D_WIN32_WINNT=0x0500)
IF(NOT HAVE_WINDOWS_H)
    CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GETTIMEOFDAY)
    IF(NOT HAVE_GETTIMEOFDAY)
        MESSAGE(FATAL_ERROR "No timing function found!")
    ENDIF()

    CHECK_FUNCTION_EXISTS(nanosleep HAVE_NANOSLEEP)
    IF(NOT HAVE_NANOSLEEP)
        MESSAGE(FATAL_ERROR "No sleep function found!")
    ENDIF()

    CHECK_C_COMPILER_FLAG(-pthread HAVE_PTHREAD)
    IF(HAVE_PTHREAD)
        ADD_DEFINITIONS(-pthread)
        SET(EXTRA_LIBS ${EXTRA_LIBS} -pthread)
    ENDIF()

    # We need pthreads outside of Windows
    CHECK_INCLUDE_FILE(pthread.h HAVE_PTHREAD_H)
    IF(NOT HAVE_PTHREAD_H)
        MESSAGE(FATAL_ERROR "PThreads is required for non-Windows builds!")
    ENDIF()
    # Some systems need pthread_np.h to get recursive mutexes
    CHECK_INCLUDE_FILES("pthread.h;pthread_np.h" HAVE_PTHREAD_NP_H)

    # _GNU_SOURCE is needed on some systems for extra attributes, and
    # _REENTRANT is needed for libc thread-safety
    ADD_DEFINITIONS(-D_GNU_SOURCE=1)
    CHECK_LIBRARY_EXISTS(pthread pthread_create "" HAVE_LIBPTHREAD)
    IF(HAVE_LIBPTHREAD)
        SET(EXTRA_LIBS pthread ${EXTRA_LIBS})
    ENDIF()

    CHECK_LIBRARY_EXISTS(pthread pthread_setschedparam "" HAVE_PTHREAD_SETSCHEDPARAM)

    CHECK_LIBRARY_EXISTS(rt clock_gettime "" HAVE_LIBRT)
    IF(HAVE_LIBRT)
        SET(EXTRA_LIBS rt ${EXTRA_LIBS})
    ENDIF()
ENDIF()

# Check for a 64-bit type
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
IF(NOT HAVE_STDINT_H)
    IF(HAVE_WINDOWS_H)
        CHECK_C_SOURCE_COMPILES("\#define _WIN32_WINNT 0x0500
                                 \#include <windows.h>
                                 __int64 foo;
                                 int main() {return 0;}" HAVE___INT64)
    ENDIF()
    IF(NOT HAVE___INT64)
        IF(NOT SIZEOF_LONG MATCHES "8")
            IF(NOT SIZEOF_LONG_LONG MATCHES "8")
                MESSAGE(FATAL_ERROR "No 64-bit types found, please report!")
            ENDIF()
        ENDIF()
    ENDIF()
ENDIF()


SET(OPENAL_OBJS  OpenAL32/alAuxEffectSlot.c
                 OpenAL32/alBuffer.c
                 OpenAL32/alDatabuffer.c
                 OpenAL32/alEffect.c
                 OpenAL32/alError.c
                 OpenAL32/alExtension.c
                 OpenAL32/alFilter.c
                 OpenAL32/alListener.c
                 OpenAL32/alSource.c
                 OpenAL32/alState.c
                 OpenAL32/alThunk.c
)
SET(ALC_OBJS  Alc/ALc.c
              Alc/ALu.c
              Alc/alcConfig.c
              Alc/alcEcho.c
              Alc/alcReverb.c
              Alc/alcRing.c
              Alc/alcThread.c
              Alc/bs2b.c
              Alc/wave.c
)

SET(BACKENDS "")

# Check ALSA backend
IF(ALSA)
    CHECK_INCLUDE_FILE(alsa/asoundlib.h HAVE_ALSA_ASOUNDLIB_H)
    IF(HAVE_ALSA_ASOUNDLIB_H)
        CHECK_SHARED_LIBRARY_EXISTS(asound snd_pcm_open 4 "" HAVE_LIBASOUND)
        IF(HAVE_LIBASOUND OR HAVE_DLFCN_H OR WIN32)
            SET(HAVE_ALSA 1)
            SET(ALC_OBJS  ${ALC_OBJS} Alc/alsa.c)
            IF(HAVE_DLFCN_H OR WIN32)
                SET(BACKENDS  "${BACKENDS} ALSA,")
            ELSE()
                SET(BACKENDS  "${BACKENDS} ALSA \(linked\),")
                SET(EXTRA_LIBS asound ${EXTRA_LIBS})
            ENDIF()
        ENDIF()
    ENDIF()
ENDIF()

# Check OSS backend
IF(OSS)
    CHECK_INCLUDE_FILE(sys/soundcard.h HAVE_SYS_SOUNDCARD_H)
    IF(HAVE_SYS_SOUNDCARD_H)
        SET(HAVE_OSS 1)
        SET(ALC_OBJS  ${ALC_OBJS} Alc/oss.c)
        SET(BACKENDS  "${BACKENDS} OSS,")
    ENDIF()
ENDIF()

# Check Solaris backend
IF(SOLARIS)
    CHECK_INCLUDE_FILE(sys/audioio.h HAVE_SYS_AUDIOIO_H)
    IF(HAVE_SYS_AUDIOIO_H)
        SET(HAVE_SOLARIS 1)
        SET(ALC_OBJS  ${ALC_OBJS} Alc/solaris.c)
        SET(BACKENDS  "${BACKENDS} Solaris,")
    ENDIF()
ENDIF()

# Check DSound/MMSystem backend
IF(DSOUND)
    CHECK_INCLUDE_FILE(dsound.h HAVE_DSOUND_H)
    IF(HAVE_DSOUND_H)
        CHECK_SHARED_LIBRARY_EXISTS(dsound DirectSoundCreate 3 "" HAVE_LIBDSOUND)
        IF(HAVE_LIBDSOUND OR HAVE_DLFCN_H OR WIN32)
            SET(HAVE_DSOUND 1)
            SET(ALC_OBJS  ${ALC_OBJS} Alc/dsound.c)

            IF(HAVE_DLFCN_H OR WIN32)
                SET(BACKENDS  "${BACKENDS} DirectSound,")
            ELSE()
                SET(BACKENDS  "${BACKENDS} DirectSound \(linked\),")
                SET(EXTRA_LIBS dsound ${EXTRA_LIBS})
            ENDIF()
        ENDIF()
    ENDIF()
ENDIF()
IF(HAVE_WINDOWS_H)
    IF(WINMM)
        CHECK_INCLUDE_FILES("windows.h;mmsystem.h" HAVE_MMSYSTEM_H -D_WIN32_WINNT=0x0500)
        IF(HAVE_MMSYSTEM_H)
            CHECK_SHARED_LIBRARY_EXISTS(winmm waveInOpen 6 "" HAVE_LIBWINMM)
            IF(HAVE_LIBWINMM)
                SET(HAVE_WINMM 1)
                SET(ALC_OBJS  ${ALC_OBJS} Alc/winmm.c)
                SET(BACKENDS  "${BACKENDS} WinMM,")

                SET(EXTRA_LIBS winmm ${EXTRA_LIBS})
            ENDIF()
        ENDIF()
    ENDIF()
ENDIF()

# Check PortAudio backend
IF(PORTAUDIO)
    CHECK_INCLUDE_FILE(portaudio.h HAVE_PORTAUDIO_H)
    IF(HAVE_PORTAUDIO_H)
        CHECK_SHARED_LIBRARY_EXISTS(portaudio Pa_Initialize 0 "" HAVE_LIBPORTAUDIO)
        IF(HAVE_LIBPORTAUDIO OR HAVE_DLFCN_H OR WIN32)
            SET(HAVE_PORTAUDIO 1)
            SET(ALC_OBJS  ${ALC_OBJS} Alc/portaudio.c)
            IF(HAVE_DLFCN_H OR WIN32)
                SET(BACKENDS  "${BACKENDS} PortAudio,")
            ELSE()
                SET(BACKENDS  "${BACKENDS} PortAudio \(linked\),")
                SET(EXTRA_LIBS portaudio ${EXTRA_LIBS})
            ENDIF()
        ENDIF()
    ENDIF()
ENDIF()

# Check PortAudio backend
IF(PULSEAUDIO)
    CHECK_INCLUDE_FILE(pulse/pulseaudio.h HAVE_PULSE_PULSEAUDIO_H)
    IF(HAVE_PULSE_PULSEAUDIO_H)
        CHECK_SHARED_LIBRARY_EXISTS(pulse pa_context_new 2 "" HAVE_LIBPULSE)
        IF(HAVE_LIBPULSE OR HAVE_DLFCN_H OR WIN32)
            SET(HAVE_PULSEAUDIO 1)
            SET(ALC_OBJS  ${ALC_OBJS} Alc/pulseaudio.c)
            IF(HAVE_DLFCN_H OR WIN32)
                SET(BACKENDS  "${BACKENDS} PulseAudio,")
            ELSE()
                SET(BACKENDS  "${BACKENDS} PulseAudio \(linked\),")
                SET(EXTRA_LIBS pulse ${EXTRA_LIBS})
            ENDIF()
        ENDIF()
    ENDIF()
ENDIF()

# This is always available
SET(BACKENDS  "${BACKENDS} WaveFile")

# Needed for openal.pc.in
SET(prefix ${CMAKE_INSTALL_PREFIX})
SET(exec_prefix "\${prefix}")
SET(libdir "\${exec_prefix}/${LIB_INSTALL_DIR}")
SET(bindir "\${exec_prefix}/bin")
SET(includedir "\${prefix}/include")
SET(PACKAGE_VERSION "${LIB_VERSION}")

# End configuration
CONFIGURE_FILE(
    "${OpenAL_SOURCE_DIR}/config.h.in"
    "${OpenAL_BINARY_DIR}/config.h")
CONFIGURE_FILE(
    "${OpenAL_SOURCE_DIR}/openal.pc.in"
    "${OpenAL_BINARY_DIR}/openal.pc"
    @ONLY)

# Build a library
IF(NOT LIBTYPE)
    SET(LIBTYPE SHARED)
ENDIF()
ADD_LIBRARY(${LIBNAME} ${LIBTYPE} ${OPENAL_OBJS} ${ALC_OBJS})
SET_TARGET_PROPERTIES(${LIBNAME} PROPERTIES DEFINE_SYMBOL AL_BUILD_LIBRARY
                                            COMPILE_FLAGS -DAL_ALEXT_PROTOTYPES
                                            VERSION ${LIB_VERSION}
                                            SOVERSION ${LIB_MAJOR_VERSION})
IF(WIN32)
    SET_TARGET_PROPERTIES(${LIBNAME} PROPERTIES PREFIX "")
ENDIF()

TARGET_LINK_LIBRARIES(${LIBNAME} ${EXTRA_LIBS})

# Add an install target here
INSTALL(TARGETS ${LIBNAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)
INSTALL(FILES include/AL/al.h
              include/AL/alc.h
              include/AL/alext.h
              include/AL/efx.h
        DESTINATION include/AL
)
INSTALL(FILES "${OpenAL_BINARY_DIR}/openal.pc"
        DESTINATION "${LIB_INSTALL_DIR}/pkgconfig")

# Install alsoft.conf configuration file
IF(ALSOFT_CONFIG)
    INSTALL(FILES alsoftrc.sample
            DESTINATION /etc/openal
            RENAME alsoft.conf
    )
ENDIF()

IF(EXAMPLES)
    ADD_EXECUTABLE(openal-info examples/openal-info.c)
    TARGET_LINK_LIBRARIES(openal-info ${LIBNAME})
    INSTALL(TARGETS openal-info
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION ${LIB_INSTALL_DIR}
            ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    )
ENDIF()

MESSAGE(STATUS "")
MESSAGE(STATUS "Building OpenAL with support for the following backends:")
MESSAGE(STATUS "    ${BACKENDS}")
MESSAGE(STATUS "")

IF(WIN32)
    IF(NOT HAVE_DSOUND)
        MESSAGE(STATUS "WARNING: Building the Windows version without DirectSound output")
        MESSAGE(STATUS "         This is probably NOT what you want!")
        MESSAGE(STATUS "")
    ENDIF()
ENDIF()
