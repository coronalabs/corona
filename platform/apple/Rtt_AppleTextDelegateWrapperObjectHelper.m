//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "Rtt_AppleTextDelegateWrapperObjectHelper.h"

@implementation Rtt_AppleTextDelegateWrapperObjectHelper

@synthesize textWidget;
@synthesize theRange;
@synthesize replacementString;
@synthesize originalString;

- (void) dealloc
{
	[textWidget release];
	[replacementString release];
	[originalString release];
	[super dealloc];
}

@end
