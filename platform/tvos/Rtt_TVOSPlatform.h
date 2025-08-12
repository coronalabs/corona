//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_TVOSPlatform_H__
#define _Rtt_TVOSPlatform_H__

#include "Rtt_IPhonePlatformBase.h"

#include "Rtt_TVOSDevice.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class IPhoneImageProvider;
class IPhoneVideoProvider;
class AppleStoreProvider;
class IPhoneVideoPlayer;
class IPhoneWebPopup;

// ----------------------------------------------------------------------------

class TVOSPlatform : public IPhonePlatformBase
{
	Rtt_CLASS_NO_COPIES( TVOSPlatform )

	public:
		typedef IPhonePlatformBase Super;

	public:
		TVOSPlatform( CoronaView *view );
		virtual ~TVOSPlatform();

	public:
		virtual MPlatformDevice& GetDevice() const;

	public:
		virtual PlatformStoreProvider* GetStoreProvider( const ResourceHandle<lua_State>& handle ) const;
		virtual PlatformVideoPlayer* GetVideoPlayer( const ResourceHandle<lua_State> & handle ) const;

		virtual void SetActivityIndicator( bool visible ) const;

		virtual bool CanShowPopup( const char *name ) const;
		virtual bool ShowPopup( lua_State *L, const char *name, int optionsIndex ) const;
		virtual bool HidePopup( const char *name ) const;
		virtual bool SaveBitmap( PlatformBitmap* bitmap, const char* filePath, float jpegQuality ) const override;


		virtual void SetTapDelay( Rtt_Real delay ) const;
		virtual Rtt_Real GetTapDelay() const;

	public:
		virtual void RuntimeErrorNotification( const char *errorType, const char *message, const char *stacktrace ) const;

	private:
		TVOSDevice fDevice;
		mutable IPhoneVideoPlayer *fVideoPlayer;
		mutable AppleStoreProvider *fInAppStoreProvider;
		UIView *fActivityView;
		id fPopupControllerDelegate;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TVOSPlatform_H__
