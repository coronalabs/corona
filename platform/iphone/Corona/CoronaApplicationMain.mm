//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

// ----------------------------------------------------------------------------

FOUNDATION_EXPORT void CoronaSetDelegateClass( Class c );
FOUNDATION_EXPORT int CoronaApplicationMain( int argc, char *argv[], Class coronaDelegateClass );

// ----------------------------------------------------------------------------

FOUNDATION_EXPORT int CoronaApplicationMain( int argc, char *argv[], Class coronaDelegateClass )
{
	CoronaSetDelegateClass( coronaDelegateClass );

	return UIApplicationMain( argc, argv, nil, nil );
}