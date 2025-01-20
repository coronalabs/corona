//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacVideoProvider_H__
#define _Rtt_MacVideoProvider_H__

#include "Rtt_PlatformVideoProvider.h"

// ----------------------------------------------------------------------------

@class MacPickerControllerDelegate;
@class NSImage;
@class NSDictionary;

namespace Rtt
{

class PlatformBitmap;

// ----------------------------------------------------------------------------

class MacVideoProvider : public PlatformVideoProvider
{
	public:
		typedef PlatformVideoProvider Super;

		MacVideoProvider( const ResourceHandle<lua_State> & handle );
		virtual ~MacVideoProvider();

		virtual void DidDismiss( NSImage* image, bool completed );

		char *fDstPath;

	public:
		virtual bool Supports( int source ) const;
		virtual bool Show( int source, lua_State* L, int maxTime, int quality );
	
	protected:
		MacPickerControllerDelegate* fPickerControllerDelegate; // internal Obj-C delegate for callbacks
		bool fInterfaceIsUp; // internal flag to help prevent re-entry while showing interface
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacImageProvider_H__
