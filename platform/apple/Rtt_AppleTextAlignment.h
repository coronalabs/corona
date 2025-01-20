//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifdef Rtt_MAC_ENV
	#import <AppKit/NSText.h>
#else
	#import <UIKit/UIKit.h>
#endif

namespace Rtt
{

	class AppleAlignment
	{
	
	public:
		static NSTextAlignment AlignmentForString( const char *value );
		static const char* StringForAlignment( NSTextAlignment value );

	};
};

// ----------------------------------------------------------------------------

//#endif
