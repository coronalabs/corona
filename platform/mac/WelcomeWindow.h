//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Cocoa/Cocoa.h>
@class WelcomeToCoronaController;

@interface WelcomeWindow : NSWindow
{
	// Back pointer to controller for convenience.
	WelcomeToCoronaController* windowControllerBackPointer;
}

// weak reference only, do not retain or we will get a retain cycle
@property(nonatomic, assign) WelcomeToCoronaController* windowControllerBackPointer;

@end
