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

#import "ResponderPassthroughTextField.h"
// #include "FontHelper.h" // Code currently disabled because we don't use it anymore


@implementation ResponderPassthroughTextField

- (void) awakeFromNib
{
	// Update: We don't have permission to use these fonts.
	// This code block is disabled, but if we ever need to load custom fonts, this is how it is done.
//	FontHelper_ChangeFontOnLabel(@"FrutigerLTStd-Roman", self);

	// instead of handling the event, pass it to the button below it to get the visual press effect which Gilbert likes
	[self setNextResponder:passToButton];
}

- (void) mouseUp:(NSEvent*)the_event
{
	//	NSLog(@"%@", NSStringFromSelector(_cmd));
	//	NSLog(@"target: %@, action: %@", [self target], NSStringFromSelector([self action]));
	//	[NSApp sendAction:[self action] to:[self target] from:self];
	// instead of handling the event, pass it to the button below it to get the visual press effect which Gilbert likes
	[[self nextResponder] mouseUp:the_event];
}



@end
