//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_AppleCallback.h"

#include "Rtt_MCallback.h"

// ----------------------------------------------------------------------------

@implementation AppleCallback

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
@synthesize callback;
#else
- (void)setCallback:(Rtt::MCallback*)newValue
{
	callback = newValue;
}
#endif

- (id)init
{
	self = [super init];
	if ( self )
	{
		callback = NULL;
	}
	return self;
}

- (void)invoke:(id)sender
{
	Rtt_ASSERT( callback );

	(*callback)();
}

@end

// ----------------------------------------------------------------------------

