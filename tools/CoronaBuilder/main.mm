//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_CoronaBuilder.h"

#import <Foundation/Foundation.h>

#include "Rtt_MacConsolePlatform.h"
#include "Rtt_MacPlatformServices.h"

int main( int argc, const char *argv[] )
{
	using namespace Rtt;

	int result = 0;
	@autoreleasepool
	{
		MacConsolePlatform platform;
		MacPlatformServices services( platform );

		CoronaBuilder builder( platform, services );
		result = builder.Main( argc, argv );
	}
	return result;
}

// ----------------------------------------------------------------------------
