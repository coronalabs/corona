//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <UIKit/UIKit.h>
#include <sys/time.h>

int main(int argc, char *argv[]) {
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	// Scene-based apps get no delegate from a main nib, so the delegate class is named here (see AppDelegate.h)
	int retVal = UIApplicationMain(argc, argv, nil, @"AppDelegate");
	[pool release];
	return retVal;
}
