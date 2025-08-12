//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifdef Rtt_MetalANGLE
	#import <MetalANGLE/MGLKit.h>
	@interface CoronaViewController : MGLKViewController
	@end
#else
	#import <GLKit/GLKit.h>
	@interface CoronaViewController : GLKViewController
	@end
#endif

// ----------------------------------------------------------------------------
