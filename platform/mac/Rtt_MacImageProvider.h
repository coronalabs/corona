//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacImageProvider_H__
#define _Rtt_MacImageProvider_H__

#include "Rtt_PlatformImageProvider.h"

// ----------------------------------------------------------------------------

@class MacPickerControllerDelegate;
@class NSImage;
@class NSDictionary;

namespace Rtt
{

class PlatformBitmap;

// ----------------------------------------------------------------------------

class MacImageProvider : public PlatformImageProvider
{
	public:
		typedef PlatformImageProvider Super;

		MacImageProvider( const ResourceHandle<lua_State> & handle );
		virtual ~MacImageProvider();

		virtual void DidDismiss( NSImage* image, bool completed );

		char *fDstPath;

	public:
		virtual bool Supports( int source ) const;
		virtual bool Show( int source, const char* filePath, lua_State* L );
	
	protected:
		MacPickerControllerDelegate* fPickerControllerDelegate; // internal Obj-C delegate for callbacks
		bool fInterfaceIsUp; // internal flag to help prevent re-entry while showing interface
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacImageProvider_H__
