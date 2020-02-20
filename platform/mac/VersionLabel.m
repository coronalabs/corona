//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "VersionLabel.h"
#include "Core/Rtt_Version.h"
// #include "FontHelper.h" // Code currently disabled because we don't use it anymore

@implementation VersionLabel

- (void) awakeFromNib
{
	// Update: We don't have permission to use these fonts.
	// This code block is disabled, but if we ever need to load custom fonts, this is how it is done.
//	FontHelper_ChangeFontOnLabel(@"FrutigerLTStd-Roman", self);

	[self setStringValue:[NSString stringWithExternalString:Rtt_STRING_HUMAN_FRIENDLY_VERSION]];
}

@end
