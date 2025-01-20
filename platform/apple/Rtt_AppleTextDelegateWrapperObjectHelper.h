//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_AppleTextDelegateWrapperObjectHelper_H__
#define _Rtt_AppleTextDelegateWrapperObjectHelper_H__

#import <Foundation/Foundation.h>

@interface Rtt_AppleTextDelegateWrapperObjectHelper : NSObject
{
	// using id because it is easier to reuse this data structor for UITextField and UITextView (and possibly NS-counterparts).
	id textWidget;
	NSRange theRange;
	NSString* replacementString; // the newChars
	NSString* originalString; // the newChars
}

@property(nonatomic, retain) id textWidget;
@property(nonatomic, assign) NSRange theRange;
@property(nonatomic, copy) NSString* replacementString;
@property(nonatomic, copy) NSString* originalString;

@end

#endif // _Rtt_AppleTextDelegateWrapperObjectHelper_H__
