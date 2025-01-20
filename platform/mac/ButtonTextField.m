//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "ButtonTextField.h"
// #include "FontHelper.h" // Code currently disabled because we don't use it anymore


@implementation ButtonTextField

// Update: We don't have permission to use these fonts.
// This code block is disabled, but if we ever need to load custom fonts, this is how it is done.
/*
- (void) awakeFromNib
{
	NSFont* old_label_font = [self font];
	NSString* old_font_name = [old_label_font fontName];
	if(YES == [old_font_name hasSuffix:@"-Bold"])
	{
		FontHelper_ChangeFontOnLabel(@"FrutigerLTStd-Bold", self);		
	}
	else
	{
		FontHelper_ChangeFontOnLabel(@"FrutigerLTStd-Roman", self);		
	}
}
*/
- (void) mouseUp:(NSEvent*)the_event
{
//	NSLog(@"%@", NSStringFromSelector(_cmd));
//	NSLog(@"target: %@, action: %@", [self target], NSStringFromSelector([self action]));
	[NSApp sendAction:[self action] to:[self target] from:self];
	// instead of handling the event, pass it to the button below it to get the visual press effect which Gilbert likes
//	[[self nextResponder] mouseUp:the_event];
}


@end
