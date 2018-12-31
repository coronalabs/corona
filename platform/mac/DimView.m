//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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
