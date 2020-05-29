//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <UIKit/UIKit.h>
#import "CoronaCards/CoronaApplicationMain.h"
#import "TestAppDelegate.h"

int main(int argc, char * argv[]) {
	@autoreleasepool {
	    return CoronaApplicationMain(argc, argv, [TestAppDelegate class]);
	}
}
