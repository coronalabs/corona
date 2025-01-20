//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "LicenseViewController.h"


@implementation LicenseViewController

@synthesize forwardMessageObject;

- (IBAction) acceptLicense:(id)the_sender
{
	[forwardMessageObject acceptLicense:the_sender];
}

- (IBAction) rejectLicense:(id)the_sender
{
	[forwardMessageObject rejectLicense:the_sender];
}

@end
