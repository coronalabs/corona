//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Macros_H__
#define _Rtt_Macros_H__

// ----------------------------------------------------------------------------

// Unused
// ----------------------------------------------------------------------------
#define Rtt_UNUSED( var ) ((void)(var))

// String
// ----------------------------------------------------------------------------

#define Rtt_MACRO_TO_STRING( a )			Rtt_MACRO_TO_STRING_WRAPPER( a )
#define Rtt_MACRO_TO_STRING_WRAPPER( a )	#a


// Concatenation
// ----------------------------------------------------------------------------

#define Rtt_CONCATENATE( a, b )				Rtt_CONCATENATE_ARGS_WRAPPER( a, b )
#define Rtt_CONCATENATE_ARGS_WRAPPER(a, b)	Rtt_CONCATENATE_ARGS( a, b )
#define Rtt_CONCATENATE_ARGS( a, b )		a##b

#define Rtt_CONCATENATE3( a, b, c )			Rtt_CONCATENATE( Rtt_CONCATENATE(a,b), c )

// C Export
// ----------------------------------------------------------------------------

#ifdef __cplusplus
	#define Rtt_EXPORT						extern "C"
	#define Rtt_EXPORT_BEGIN					extern "C" {
	#define Rtt_EXPORT_END					}
#else
	#define Rtt_EXPORT
	#define Rtt_EXPORT_BEGIN
	#define Rtt_EXPORT_END
#endif

// Function string
// ----------------------------------------------------------------------------

#if defined( __GNUC__ ) || defined( __ARMCC_VERSION ) || defined( __MWERKS__ )

    #define Rtt_FUNCTION __PRETTY_FUNCTION__

#elif defined( _MSC_VER )

    #define Rtt_FUNCTION __FUNCSIG__

#elif defined( __STDC_VERSION__ ) && ( __STDC_VERSION__ >= 199901 )

    #define Rtt_FUNCTION __func__

#else

    #define Rtt_FUNCTION "function " ## __FILE__ ## "(" ## #__LINE__ ## ")"

#endif

// Inline
// ----------------------------------------------------------------------------

#define Rtt_INLINE			inline

#if defined( __GNUC__ )

	#if __GNUC__ > 3
	#define Rtt_FORCE_INLINE	inline __attribute__((always_inline))
	#else
	#define Rtt_FORCE_INLINE	Rtt_INLINE
	#endif

#elif defined( _MSC_VER ) || defined( __ARMCC_VERSION )

	#define Rtt_FORCE_INLINE	__forceinline

//	#if ( __ARMCC_VERSION < 200000 )
//		#define Rtt_FORCE_INLINE	Rtt_INLINE
//	#else
//		#define Rtt_FORCE_INLINE	__forceinline
//	#endif

#else

	#define Rtt_FORCE_INLINE	Rtt_INLINE

#endif


// Restrict
// ----------------------------------------------------------------------------

#if defined( _MSC_VER )
	// There is a Microsoft VC++ compiler bug with the __restrict keyword when doing release builds.
	// The only work-around is to avoid using it until Microsoft fixes it. So, we #define it to nothing.
	#define Rtt_RESTRICT
#else
	#define Rtt_RESTRICT __restrict
#endif

// For Apple Garbage Collection
// ----------------------------------------------------------------------------
#if defined( Rtt_APPLE_ENV )
	#ifndef __OBJC_GC__
		#define Rtt_STRONG /* empty */
	#else
		#define Rtt_STRONG __strong
	#endif
#else
	#define Rtt_STRONG /* empty */
#endif


// Class Constant
// ----------------------------------------------------------------------------

#ifdef Rtt_CLASS_DECL_MEMBER_INITIALIZATION
	#define Rtt_CLASSCONSTANT( T, variable, expression )	\
		static const T variable = ( expression )
#else
	#define Rtt_CLASSCONSTANT( T, variable, expression )	\
		enum { variable = ( expression ) }
#endif

// Class Properties
// ----------------------------------------------------------------------------

#define Rtt_CLASS_SELF( T )							\
	public:											\
		typedef T	Self;

#define Rtt_CLASS_LUA_WRAPPER( T )					\
	public:											\
		typedef Lua##T	LuaWrapper;

#define Rtt_CLASS_NO_COPIES( T )					\
	private:										\
		T( const T& );								\
		const T& operator=( const T& );

#define Rtt_CLASS_NO_DEFAULT_METHODS( T )			\
	private:										\
		T( );										\
		~T( );										\
													\
		T( const T& );								\
		const T& operator=( const T& );

#define Rtt_CLASS_NO_DYNAMIC_ALLOCATION				\
	private:										\
		static void* operator new(size_t size);		\
		static void operator delete(void * p);		\
													\
		static void* operator new[](size_t size);	\
		static void operator delete[](void * p);

// ----------------------------------------------------------------------------

#ifdef Rtt_WIN_ENV
// On Windows, certain APIs need to be exported so they can be used by DLLs
#define  Rtt_DECL_API	__declspec(dllexport)

#else

#define  Rtt_DECL_API

#endif

#endif // _Rtt_Macros_H__
