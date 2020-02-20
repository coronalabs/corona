//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "ValidationToolOutputViewController.h"


@implementation ValidationToolOutputViewController

@synthesize validationMessage;
/*
- (void) loadView
{
	
}
*/

- (void) dealloc
{
	[validationMessage release];
	[super dealloc];
}

@end
