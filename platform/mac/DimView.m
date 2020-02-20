//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "DimView.h"


@implementation DimView

- (void) drawRect:(NSRect)the_rect
{
//    [[NSColor whiteColor] setFill];
	[[NSColor colorWithDeviceRed:0.0f green:0.0f blue:0.0 alpha:0.8f] set];

    NSRectFill(the_rect);
}

#if 0
- (BOOL) acceptsFirstResponder
{
	return YES;
}
- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
	return YES;
}



// Make sure to trap any click events
- (NSView *)hitTest:(NSPoint)aPoint
{
	NSLog(@"%@", NSStringFromSelector(_cmd));
	
	return self;
	/*
	 NSView * clickedView = [super hitTest:aPoint];
	 if (clickedView == nil)
	 {
	 clickedView = self;
	 }
	 
	 return clickedView;
	 */
}

#endif
- (void) mouseDown:(NSEvent*)the_event
{
	NSLog(@"%@", NSStringFromSelector(_cmd));
	//	NSLog(@"target: %@, action: %@", [self target], NSStringFromSelector([self action]));
	//	[NSApp sendAction:[self action] to:[self target] from:self];
}


- (void) mouseUp:(NSEvent*)the_event
{
	NSLog(@"%@", NSStringFromSelector(_cmd));
	//	NSLog(@"target: %@, action: %@", [self target], NSStringFromSelector([self action]));
	//	[NSApp sendAction:[self action] to:[self target] from:self];
}
@end
