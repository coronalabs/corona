//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_AppleFont_H__
#define _Rtt_AppleFont_H__

#ifdef Rtt_MAC_ENV

	#import <Foundation/NSGeometry.h>
	#import <AppKit/NSFont.h>

	@class NSFont;
	typedef NSFont AppleFont;
	typedef NSSize AppleSize;

	namespace Rtt
	{
		class MacFont;
		typedef MacFont AppleFontWrapper;
	}

	#include "Rtt_MacFont.h"

#elif defined( Rtt_IPHONE_ENV ) || defined( Rtt_TVOS_ENV )

	#import <CoreGraphics/CGGeometry.h>
	#import <UIKit/UIFont.h>
	#import <UIKit/UIInterface.h>

	@class UIFont;
	typedef UIFont AppleFont;
	typedef CGSize AppleSize;

	namespace Rtt
	{
		class IPhoneFont;
		typedef IPhoneFont AppleFontWrapper;
	}

	#include "Rtt_IPhoneFont.h"

#else
	#error Unsupported platform
#endif

	namespace Rtt
	{
		class AppleFontHelper
		{
		public:
			static AppleFont* AllocNativeFont( const char *fontName, Rtt_Real size, NSString *directory );
			static void ClearCacheAndLoadedFonts();
		private:
			static NSMutableDictionary<NSString*, NSString*>* sFontCache;
			static NSMutableSet<NSURL*> * sActivatedFonts;
		};
	}

// ----------------------------------------------------------------------------

#endif // _Rtt_AppleFont_H__
