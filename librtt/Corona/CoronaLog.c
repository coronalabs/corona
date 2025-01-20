//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "CoronaLog.h"

// ----------------------------------------------------------------------------

CORONA_API
int CoronaLog( const char *format, ... )
{
	int result = 0;
    va_list ap;

    va_start( ap, format );
    result = CoronaLogV( format, ap );
    va_end( ap );

	return result;
}

CORONA_API
int CoronaLogV( const char *format, va_list arguments )
{
	return Rtt_VLogException( format, arguments );
}

// ----------------------------------------------------------------------------

