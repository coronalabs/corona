//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "CoronaAssert.h"

// ----------------------------------------------------------------------------

CORONA_API
int CoronaVerify( int cond, const char *reason, const char *func, const char *file, int line )
{
#if defined( CORONA_DEBUG )
	return Rtt_Verify( cond, reason, func, file, line );
#else
	return cond;
#endif // CORONA_DEBUG
}

// ----------------------------------------------------------------------------

