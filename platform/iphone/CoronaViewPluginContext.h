//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "CoronaRuntime.h"

@class CoronaViewController;

// ----------------------------------------------------------------------------

@interface CoronaViewPluginContext : NSObject <CoronaRuntime>

- (id)initWithOwner:(CoronaViewController *)owner;

@end

// ----------------------------------------------------------------------------
