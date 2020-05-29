//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Foundation/Foundation.h>

@interface NSString ( Extensions )

// Use in preference to [NSString stringWithUTF8String:] because it handles unexpected encodings better
+ (_Nonnull instancetype) stringWithExternalString:(const char  * _Nullable) nullTerminatedCString;

// Use in preference to [NSString containsString:] because that isn't available on 10.9
- (BOOL) contains:(NSString * _Nonnull) str;

@end