//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Cocoa/Cocoa.h>
#import "NSString+Extensions.h"

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
{
	NSRunningApplication *simulator;
}

- (void) clearConsole;
- (void) bringToFront;
- (void)observeValueForKeyPath:(NSString *)keyPath
					  ofObject:(id)object
						change:(NSDictionary *)change
					   context:(void *)context;

@end

