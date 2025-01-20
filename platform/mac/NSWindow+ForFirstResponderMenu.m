//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <AppKit/AppKit.h>

@implementation NSWindow (ForFirstResponderMenu)

// Useful category so all our windows in the Simulator respond to the IBAction center:(id) method.
// For first responder so we can wire easily in the MainMenu
- (IBAction) center:(id)sender
{
	// calls the regular NSWindow center
	// Boo! animator has no effect as of 10.8 so we can't animate it. (Don't try to manually do this or we need to figure out multiple display behaviors with centering.) Filed radar://12127130
	[[self animator] center];
}

@end

