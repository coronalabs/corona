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
		NSString *jhome = [[[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:@"../../../../../Corona Simulator.app/Contents/jre/jdk/Contents/Home"] stringByStandardizingPath];
		if([[NSFileManager defaultManager] fileExistsAtPath:jhome]) {
			setenv("JAVA_HOME", [jhome UTF8String], YES);
		} else {
			jhome = [[[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:@"../Corona Simulator.app/Contents/jre/jdk/Contents/Home"] stringByStandardizingPath];
			if([[NSFileManager defaultManager] fileExistsAtPath:jhome]) {
				setenv("JAVA_HOME", [jhome UTF8String], YES);
			}
		}
	
		MacConsolePlatform platform;
		MacPlatformServices services( platform );

		CoronaBuilder builder( platform, services );
		result = builder.Main( argc, argv );
	}
	return result;
}

// ----------------------------------------------------------------------------
