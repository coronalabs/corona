//-----------------------------------------------------------------------------
//
// Corona Labs
//
// easing.lua
//
// Code is MIT licensed; see https://www.coronalabs.com/links/code/license
//
//-----------------------------------------------------------------------------

#ifndef _CoronaMacros_H__
#define _CoronaMacros_H__

// ----------------------------------------------------------------------------

// String
// ----------------------------------------------------------------------------

#define CORONA_MACRO_TO_STRING( a )			CORONA_MACRO_TO_STRING_WRAPPER( a )
#define CORONA_MACRO_TO_STRING_WRAPPER( a )	#a


// Concatenation
// ----------------------------------------------------------------------------

#define CORONA_CONCATENATE( a, b )				CORONA_CONCATENATE_ARGS_WRAPPER( a, b )
#define CORONA_CONCATENATE_ARGS_WRAPPER(a, b)	CORONA_CONCATENATE_ARGS( a, b )
#define CORONA_CONCATENATE_ARGS( a, b )		a##b

#define CORONA_CONCATENATE3( a, b, c )			CORONA_CONCATENATE( CORONA_CONCATENATE(a,b), c )

// Linkage visibility
// ----------------------------------------------------------------------------

#if defined( __APPLE__ ) && !defined( Rtt_EMSCRIPTEN_ENV )
	#define CORONA_PUBLIC __attribute__((visibility("default")))
	#define CORONA_PUBLIC_SUFFIX
#elif __ANDROID__
	#define CORONA_PUBLIC
	#define CORONA_PUBLIC_SUFFIX __attribute__((visibility("default")))
/*
#elif defined( _OTHER_PLATFORM_ )
	#define CORONA_PUBLIC
*/
#else
	#define CORONA_PUBLIC
	#define CORONA_PUBLIC_SUFFIX
#endif


// C Extern
// ----------------------------------------------------------------------------

#ifdef __cplusplus
	#define CORONA_EXTERN_C					extern "C"
#else
	#define CORONA_EXTERN_C					extern
#endif


// C Export/Import
// ----------------------------------------------------------------------------

// Windows uses different linkage visibility attributes that depend on whether
// the declaration is used in the implementation or in a public header.
#if (( defined( _WIN32 ) || defined( _WIN64 ) || defined( WINAPI_FAMILY ) ) && !defined(Rtt_ANDROID_ENV) && !defined(Rtt_NINTENDO_ENV) )
	#define CORONA_EXPORT					CORONA_EXTERN_C __declspec( dllexport )
	#define CORONA_IMPORT					CORONA_EXTERN_C __declspec( dllimport )

	#if defined( CORONA_API_EXPORT_ENABLED )
		#define CORONA_API						CORONA_EXPORT
	#else
		#define CORONA_API						CORONA_IMPORT
	#endif
#else
	#define CORONA_EXPORT					CORONA_PUBLIC CORONA_EXTERN_C
	#define CORONA_IMPORT					CORONA_PUBLIC CORONA_EXTERN_C

	#define CORONA_API						CORONA_PUBLIC CORONA_EXTERN_C
#endif


// C API
// ----------------------------------------------------------------------------


/*
#ifdef CORONA_API_HEADER
	#define CORONA_API						CORONA_IMPORT
#else
	#define CORONA_API						CORONA_EXPORT
#endif
*/


// Function string
// ----------------------------------------------------------------------------

#if defined( __GNUC__ ) || defined( __ARMCC_VERSION ) || defined( __MWERKS__ )

    #define CORONA_FUNCTION __PRETTY_FUNCTION__

#elif defined( _MSC_VER )

    #define CORONA_FUNCTION __FUNCSIG__

#elif defined( __STDC_VERSION__ ) && ( __STDC_VERSION__ >= 199901 )

    #define CORONA_FUNCTION __func__

#else

    #define CORONA_FUNCTION "function " ## __FILE__ ## "(" ## #__LINE__ ## ")"

#endif

// Inline
// ----------------------------------------------------------------------------

#ifdef __cplusplus
#define CORONA_INLINE			inline
#else
#define CORONA_INLINE
#endif
// ----------------------------------------------------------------------------

#endif // _CoronaMacros_H__
