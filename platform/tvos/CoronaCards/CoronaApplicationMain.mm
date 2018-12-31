// ----------------------------------------------------------------------------
// 
// CoronaApplicationMain.mm
// Copyright (c) 2015 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Walter
//
// ----------------------------------------------------------------------------

#import "CoronaCards/CoronaApplicationMain.h"
#import "CoronaCards/CoronaMainAppDelegate.h"

#import <UIKit/UIKit.h>

FOUNDATION_EXPORT void CoronaSetDelegateClass( Class c );
FOUNDATION_EXPORT int CoronaApplicationMain( int argc, char *argv[], Class coronaDelegateClass );

// ----------------------------------------------------------------------------

FOUNDATION_EXPORT int CoronaApplicationMain( int argc, char *argv[], Class coronaDelegateClass )
{
	CoronaSetDelegateClass( coronaDelegateClass );

 	return UIApplicationMain( argc, argv, nil, NSStringFromClass([CoronaMainAppDelegate class]) );
}