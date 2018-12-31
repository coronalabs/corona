//-----------------------------------------------------------------------------
//
// Corona Labs
//
// easing.lua
//
// Code is MIT licensed; see https://www.coronalabs.com/links/code/license
//
//-----------------------------------------------------------------------------

#ifndef _CoronaAssert_H__
#define _CoronaAssert_H__

#include "CoronaMacros.h"

// ----------------------------------------------------------------------------

#if defined( CORONA_DEBUG )

CORONA_API int CoronaVerify( int cond, const char *reason, const char *func, const char *file, int line ) CORONA_PUBLIC_SUFFIX;

#define CORONA_VERIFY( cond )	CoronaVerify( 0 != (cond), #cond, CORONA_FUNCTION, __FILE__, __LINE__ )
#define CORONA_ASSERT( cond )	(void)CORONA_VERIFY( cond )
#define CORONA_ASSERT_MSG( cond, msg )	(void)CoronaVerify( 0 != (cond), msg, CORONA_FUNCTION, __FILE__, __LINE__ )

#else

#define CORONA_VERIFY( cond )	( cond )
#define CORONA_ASSERT( cond )
#define CORONA_ASSERT_MSG( cond, msg )

#endif

// ----------------------------------------------------------------------------

#if defined( CORONA_DEBUG )

	#define CORONA_STATIC_ASSERT( condition )	typedef char CORONA_STATIC_ASSERT_##__FILE__##__LINE__[ (condition) ? 1 : -1 ]

#else

	#define CORONA_STATIC_ASSERT( condition )

#endif

// ----------------------------------------------------------------------------

#define CORONA_ASSERT_NOT_REACHED( )		CORONA_ASSERT_MSG( 0, "Code should NOT be reached" )
#define CORONA_ASSERT_NOT_IMPLEMENTED( )	CORONA_ASSERT_MSG( 0, "Code NOT implemented" )


// ----------------------------------------------------------------------------

#endif // _CoronaAssert_H__
