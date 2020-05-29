//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Cocoa/Cocoa.h>


@interface ValidationToolOutputViewController : NSViewController
{
	NSString* validationMessage;
}

// Uses bindings in the nib to set the text view
@property(nonatomic, copy) NSString* validationMessage;

@end
