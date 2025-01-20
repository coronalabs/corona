//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "CoronaDelegate.h"

// ----------------------------------------------------------------------------

@interface CoronaAppDelegate : NSObject<CoronaDelegate>

- (id)initWithEnterpriseDelegate:(id)enterpriseDelegate;
+ (BOOL)handlesUrl:(NSURL*)url;

@end

// ----------------------------------------------------------------------------
