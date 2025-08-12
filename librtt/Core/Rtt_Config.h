//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_Config_H__
#define __Rtt_Config_H__

// Debug
// ----------------------------------------------------------------------------
#if defined( DEBUG ) || defined( _DEBUG )
	#ifndef Rtt_DEBUG
		#define Rtt_DEBUG
	#endif
#endif

// Common
// ----------------------------------------------------------------------------

#if defined( Rtt_FEATURES_ALL )
#endif

#define Rtt_GAMEEDITION

#ifdef Rtt_GAMEEDITION
	#define Rtt_PHYSICS
	#define Rtt_SPRITE
#endif

#ifdef Rtt_EMSCRIPTEN_ENV
	#define Rtt_USE_WEBMIXER
#elif Rtt_ANDROID_ENV
	#define Rtt_USE_OPENSLES
	#define Rtt_USE_ALMIXER
#elif Rtt_NXS_ENV
	//#define Rtt_USE_ALMIXER
#elif Rtt_LINUX_ENV
	#define Rtt_USE_ALMIXER
#else
	#define Rtt_USE_ALMIXER
#endif

// Optimizations
// ----------------------------------------------------------------------------

// Do NOT remove these.
// Turn these off by undef'ing them in the platform-specific section below
//#define Rtt_MATH_INLINE				1
#define Rtt_MINIMIZE_CODE_SIZE		1


// ----------------------------------------------------------------------------
// Platform-specific
// ----------------------------------------------------------------------------

// By default, assume little endian
#define Rtt_LITTLE_ENDIAN	1

#if defined( Rtt_NXS_ENV )
#	undef Rtt_LITTLE_ENDIAN
#	define Rtt_BIG_ENDIAN	1
#endif

// Apple (Mac + iPhone)
// ----------------------------------------------------------------------------

#if defined( Rtt_MAC_ENV )
	#ifdef __BIG_ENDIAN__
		#undef Rtt_LITTLE_ENDIAN
		#define Rtt_BIG_ENDIAN	1
	#else
		#if !defined( __LITTLE_ENDIAN__ )
			#error "Rtt_MAC_ENV ERROR: Unknown Endianness"
		#endif
	#endif

	#define Rtt_APPLE_ENV

	#define Rtt_OPENGL_EXT_APPLE

	#define Rtt_LUA_LFS

	#define Rtt_APPLE_HID

#endif

#if defined( Rtt_IPHONE_ENV )
	#define Rtt_APPLE_ENV

	#define Rtt_OPENGLES
	#define Rtt_DEVICE_ENV

	#if defined( __arm__ )
		#define Rtt_ARM_ASM
	#endif

	#if defined( __arm__ ) || defined( __arm64__ )
	#else
		#define Rtt_DEVICE_SIMULATOR
	#endif

	#define Rtt_LUA_LFS
	#define Rtt_ORIENTATION
	#define Rtt_ACCELEROMETER
	#define Rtt_CORE_MOTION
	#define Rtt_CORE_LOCATION
	#define Rtt_MULTITOUCH
	#define Rtt_AUDIO_SESSION_PROPERTY

#endif

#if defined( Rtt_TVOS_ENV )
	#define Rtt_APPLE_ENV

	#define Rtt_OPENGLES
	#define Rtt_DEVICE_ENV

	#if defined( __arm__ )
		#define Rtt_ARM_ASM
	#endif

	#if defined( __arm__ ) || defined( __arm64__ )
	#else
		#define Rtt_DEVICE_SIMULATOR
	#endif

#endif

// Shared config
#if defined( Rtt_APPLE_ENV )
	#define Rtt_USE_GLOBAL_VARIABLES
	#define Rtt_ALLOCATOR_SYSTEM

	#define Rtt_USE_LIMITS
	#define Rtt_VPRINTF_SUPPORTED

	#define Rtt_NETWORK

	#define Rtt_SQLITE

	#define Rtt_OPENGL_CLIENT_SIDE_ARRAYS 1
	#define Rtt_OPENGL_RESET_VERTEX_ARRAY 1

	#ifdef Rtt_GAMEEDITION
		#define Rtt_SPRITE
	#endif

#endif


// Symbian Series 60
// ----------------------------------------------------------------------------

#if defined( Rtt_SYMBIAN_ENV )

	#define Rtt_ALLOCATOR_SYSTEM

	#define Rtt_OPENGLES
	#define Rtt_OPENGL_CLIENT_SIDE_ARRAYS 1
	#define Rtt_OPENGL_RESET_VERTEX_ARRAY 1
	#define Rtt_DEVICE_ENV

	#if defined( __arm__ )
		#define Rtt_ARM_ASM
	#endif

	#if defined( __arm__ ) || defined( __arm64__ )
	#else
		#define Rtt_DEVICE_SIMULATOR
	#endif

	#define Rtt_REAL_FIXED

	#undef Rtt_MATH_INLINE

	#define Rtt_NO_STRICT_ANSI
	#define Rtt_NO_LONG_DOUBLE

	#define Rtt_VPRINTF_SUPPORTED

	#define Rtt_LUA_LFS

#endif


// Windows
// ----------------------------------------------------------------------------

#if defined( Rtt_WIN_DESKTOP_ENV )
	
	#ifndef Rtt_WIN_ENV
		#define Rtt_WIN_ENV
	#endif
	#ifndef Rtt_AUTHORING_SIMULATOR
		#define Rtt_DEVICE_ENV
	#endif
	#define Rtt_NETWORK
	#define Rtt_LUA_LFS

#if !defined(Rtt_EMSCRIPTEN_ENV) && !defined( Rtt_LINUX_ENV) 
	#define Rtt_DEBUGGER
#endif

#endif

#if defined( Rtt_WIN_PHONE_ENV )

	#ifndef Rtt_WIN_ENV
		#define Rtt_WIN_ENV
	#endif
	#ifndef Rtt_USE_PRECOMPILED_SHADERS
		#define Rtt_USE_PRECOMPILED_SHADERS
	#endif
	#define Rtt_OPENGLES
	#define Rtt_DEVICE_ENV
	#define Rtt_NETWORK

#endif

#if defined( Rtt_WIN_ENV )

	#define Rtt_USE_GLOBAL_VARIABLES
	#define Rtt_VPRINTF_SUPPORTED
	#define Rtt_USE_LIMITS
	#define Rtt_ALLOCATOR_SYSTEM
#if !defined( Rtt_EMSCRIPTEN_ENV ) && !defined( Rtt_LINUX_ENV )
	#define Rtt_SQLITE
#endif
	#define Rtt_OPENGL_CLIENT_SIDE_ARRAYS 1

	#ifdef Rtt_GAMEEDITION
		#define Rtt_SPRITE
	#endif

	#if defined( Rtt_DEBUG ) || defined( _DEBUG )
		// Memory leak tracking
		#ifndef _CRTDBG_MAP_ALLOC
			#define _CRTDBG_MAP_ALLOC
		#endif
		#include <stdlib.h>
		#include <crtdbg.h>
	#endif
	
	#include <sys/stat.h>
	#include "Rtt_FileSystem.h"
	#ifndef fopen
		#define fopen(filePath, mode) Rtt_FileOpen((filePath), (mode))
	#endif
	#ifndef stat
		#define stat(filePath, buffer) Rtt_FileStatus((filePath), (buffer))
	#endif
	#ifndef lstat
		#define lstat(filePath, buffer) Rtt_FileStatus((filePath), (buffer))
	#endif

	// This makes the snprintf() function behave like the C99 equivalent in Visual C++.
	#define snprintf( s, n, format, ... ) _snprintf_s( (s), (n), _TRUNCATE, (format), __VA_ARGS__ )

#endif

#if defined( Rtt_POWERVR_ENV )

	#define Rtt_USE_GLOBAL_VARIABLES
	#define Rtt_VPRINTF_SUPPORTED
	#define Rtt_USE_LIMITS
	#define Rtt_ALLOCATOR_SYSTEM
	#define Rtt_OPENGLES
	#define Rtt_OPENGL_CLIENT_SIDE_ARRAYS 1
	#define Rtt_OPENGL_RESET_VERTEX_ARRAY 1
	#define Rtt_LUA_LFS

#endif


// Android
// ----------------------------------------------------------------------------

#if defined( Rtt_NOOK_ENV )
	#ifndef Rtt_ANDROID_ENV
		#define Rtt_ANDROID_ENV
	#endif
#endif

#if defined( Rtt_ANDROID_ENV )

	#define Rtt_DEVICE_ENV

	#define Rtt_USE_GLOBAL_VARIABLES
	#define Rtt_ALLOCATOR_SYSTEM
	#define Rtt_VPRINTF_SUPPORTED

	#define Rtt_OPENGLES
	#define Rtt_OPENGL_CLIENT_SIDE_ARRAYS 1
	#define Rtt_EGL
	#define Rtt_SQLITE
	#define Rtt_NETWORK
	#define Rtt_LUA_LFS

	#define Rtt_FLURRY

#endif


// WebOS
// ----------------------------------------------------------------------------

#if defined( Rtt_WEBOS_ENV )

	#define Rtt_DEVICE_ENV

	#define Rtt_USE_GLOBAL_VARIABLES
	#define Rtt_ALLOCATOR_SYSTEM
	#define Rtt_USE_LIMITS
	#define Rtt_VPRINTF_SUPPORTED

	#define Rtt_OPENGLES
	#define Rtt_OPENGL_CLIENT_SIDE_ARRAYS 1
	#define Rtt_OPENGL_RESET_VERTEX_ARRAY 1
	#define Rtt_SQLITE
	#define Rtt_NETWORK
	#define Rtt_LUA_LFS

#endif


// Emscripten
// ----------------------------------------------------------------------------

#if defined( Rtt_EMSCRIPTEN_ENV )
	#undef Rtt_OPENGL_CLIENT_SIDE_ARRAYS

	#define Rtt_DEVICE_ENV

	#define Rtt_USE_GLOBAL_VARIABLES
	#define Rtt_ALLOCATOR_SYSTEM
	#define Rtt_USE_LIMITS
	#define Rtt_VPRINTF_SUPPORTED
	#define Rtt_NETWORK

	// disabled in order to reduce app size
	// #define Rtt_SQLITE

	#if EMSCRIPTEN
		#define Rtt_OPENGLES
		#define Rtt_EGL
		#define Rtt_BROWSER_ENV
	#else
		#define Rtt_OPENGL_EXT_APPLE
		#define Rtt_LUA_C_MODULE_DYLIB
	#endif
#endif

//
// NxS 
//
#if defined( Rtt_NXS_ENV )

#define Rtt_OPENGL_CLIENT_SIDE_ARRAYS 1
#define Rtt_USE_GLOBAL_VARIABLES
#define Rtt_VPRINTF_SUPPORTED
#define Rtt_USE_LIMITS
#define Rtt_ALLOCATOR_SYSTEM
#define Rtt_LUA_LFS
#define Rtt_NETWORK
#define Rtt_SQLITE

#include <sys/stat.h>
#include "Rtt_NX_Allocator.h"
#include "Rtt_FileSystem.h"

#endif

//
// Linux
//
#if defined( Rtt_LINUX_ENV )
	#define GL_GLEXT_PROTOTYPES
	#define Rtt_USE_GLOBAL_VARIABLES
	#define Rtt_VPRINTF_SUPPORTED
	#define Rtt_USE_LIMITS
	#define Rtt_ALLOCATOR_SYSTEM
	#define Rtt_OPENGL_CLIENT_SIDE_ARRAYS 1
	#define Rtt_LUA_LFS
	#define Rtt_SQLITE
	//#define Rtt_NETWORK
#endif

// Authoring Simulator
// ----------------------------------------------------------------------------

#ifdef Rtt_AUTHORING_SIMULATOR

	#define Rtt_DEBUGGER

#endif

// ----------------------------------------------------------------------------

#endif // __Rtt_Config_H__
