//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneVideoProvider_H__
#define _Rtt_IPhoneVideoProvider_H__

#include "Rtt_PlatformVideoProvider.h"
#include "Rtt_IPhoneMediaProvider.h"
// ----------------------------------------------------------------------------

@class IPhoneMediaProvider;
@class IPhoneVideoPickerControllerDelegate;
@class NSDictionary;
@class NSURL;

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class IPhoneVideoProvider : public PlatformVideoProvider
{
	public:
		typedef IPhoneVideoProvider Self;
		typedef PlatformVideoProvider Super;

	public:
		IPhoneVideoProvider( const ResourceHandle<lua_State> & handle );
		virtual ~IPhoneVideoProvider();

	protected:
		void Initialize();

	public:
		virtual bool Supports( int source ) const;
		virtual bool Show( int source, lua_State* L, int maxTime, int quality );
//		virtual void SetProperty( U32 mask, bool newValue );

	public:
		void DidDismiss( NSURL* movie, int duration, long size );
		// Needed internally for Obj-C callback to dismiss popover on iPad
		void DismissPopoverController();

	private:
		IPhoneMediaProvider* fMediaProvider;
		IPhoneVideoPickerControllerDelegate* fDelegate;
		bool iOS5statusBarHidden; // workaround for statusbar coming back in iOS 5.0 when hidden
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneVideoProvider_H__
