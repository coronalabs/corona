//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef FOUNDATION_UTILITIES_H
#define FOUNDATION_UTILITIES_H

#import <Foundation/Foundation.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif
	
@interface FoundationUtilities : NSObject

// Returns the application from the bundle. Looks at the CFBundleDisplayName and then CFBundleName, including localized.
+ (NSString*) bundleApplicationName;

@end

	
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // FOUNDATION_UTILITIES_H
