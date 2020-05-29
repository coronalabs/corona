//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Cocoa/Cocoa.h>


@interface PasswordViewController : NSViewController
{
	id forwardMessageObject;
}

@property(nonatomic, assign) id forwardMessageObject;

- (IBAction) cancelPasswordLogin:(id)the_sender;
- (IBAction) submitPasswordLogin:(id)the_sender;

@end
