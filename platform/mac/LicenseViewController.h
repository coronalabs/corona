//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Cocoa/Cocoa.h>


@interface LicenseViewController : NSViewController
{
	id forwardMessageObject;
}

@property(nonatomic, assign) id forwardMessageObject;

- (IBAction) acceptLicense:(id)the_sender;
- (IBAction) rejectLicense:(id)the_sender;

@end
