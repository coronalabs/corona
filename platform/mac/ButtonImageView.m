//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "ButtonImageView.h"


@implementation ButtonImageView

- (void) mouseUp:(NSEvent*)the_event
{
//	NSLog(@"%@", NSStringFromSelector(_cmd));
//	NSLog(@"target: %@, action: %@", [self target], [self action]);
	[NSApp sendAction:[self action] to:[self target] from:self];
}

@end
