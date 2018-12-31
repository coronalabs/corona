//-----------------------------------------------------------------------------
//
// Corona Labs
//
// easing.lua
//
// Code is MIT licensed; see https://www.coronalabs.com/links/code/license
//
//-----------------------------------------------------------------------------


#ifndef _CoronaLog_H__
#define _CoronaLog_H__

#include "CoronaMacros.h"
#include <stdarg.h>

// Convenience Macros
// ----------------------------------------------------------------------------

// Usage:
//		CORONA_LOG( ( "Is %d + %d always equal to %d?\n", 1, 1, 2 ) );
// 
#define CORONA_LOG( format, ... )	CoronaLog( (format), ## __VA_ARGS__ )

#define CORONA_LOG_WARNING( format, ... )	CoronaLog( "WARNING: " format "\n", ## __VA_ARGS__ )

#define CORONA_LOG_ERROR( format, ... )	CoronaLog( "ERROR: " format "\n", ## __VA_ARGS__ )


// C API
// ----------------------------------------------------------------------------

CORONA_API int CoronaLog( const char *format, ... ) CORONA_PUBLIC_SUFFIX;

CORONA_API int CoronaLogV( const char *format, va_list arguments ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

#endif // _CoronaLog_H__
