//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhonePlatform_H__
#define _Rtt_IPhonePlatform_H__

#include "Rtt_IPhonePlatformCore.h"

#include "Rtt_IPhoneDevice.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class IPhoneImageProvider;
class IPhoneVideoProvider;
class AppleStoreProvider;
class IPhoneVideoPlayer;
class IPhoneWebPopup;

// ----------------------------------------------------------------------------

// This is the implementation of MPlatform for use by Corona/Native.
// (CoronaCards uses the parent class IPhonePlatformCore)
class IPhonePlatform : public IPhonePlatformCore
{
	Rtt_CLASS_NO_COPIES( IPhonePlatform )

	public:
		typedef IPhonePlatformCore Super;

	public:
		IPhonePlatform( CoronaView *view );
		virtual ~IPhonePlatform();

	public:
		virtual PlatformVideoPlayer* GetVideoPlayer( const ResourceHandle<lua_State> & handle ) const;
		virtual PlatformImageProvider* GetImageProvider( const ResourceHandle<lua_State> & handle ) const;
		virtual PlatformVideoProvider* GetVideoProvider( const ResourceHandle<lua_State> & handle ) const;
		virtual PlatformStoreProvider* GetStoreProvider( const ResourceHandle<lua_State>& handle ) const;

		virtual void SetStatusBarMode( StatusBarMode newValue ) const;
		virtual StatusBarMode GetStatusBarMode() const;

		virtual void SetActivityIndicator( bool visible ) const;

		virtual PlatformWebPopup* GetWebPopup() const;

		virtual bool CanShowPopup( const char *name ) const;
		virtual bool ShowPopup( lua_State *L, const char *name, int optionsIndex ) const;
		virtual bool HidePopup( const char *name ) const;

		virtual PlatformDisplayObject* CreateNativeMapView( const Rect& bounds ) const;
		virtual PlatformDisplayObject* CreateNativeWebView( const Rect& bounds ) const;
		virtual PlatformDisplayObject* CreateNativeVideo( const Rect& bounds ) const;

		virtual void SetTapDelay( Rtt_Real delay ) const;
		virtual Rtt_Real GetTapDelay() const;

	public:
		virtual void* CreateAndScheduleNotification( lua_State *L, int index ) const;
		virtual void ReleaseNotification( void *notificationId ) const;
		virtual void CancelNotification( void *notificationId ) const;

		void SetNativeProperty( lua_State *L, const char *key, int valueIndex ) const;
		int PushNativeProperty( lua_State *L, const char *key ) const;

	public:
		virtual void RuntimeErrorNotification( const char *errorType, const char *message, const char *stacktrace ) const;

	private:
		mutable IPhoneVideoPlayer *fVideoPlayer;
		mutable IPhoneImageProvider *fImageProvider;
		mutable IPhoneVideoProvider *fVideoProvider;
		mutable AppleStoreProvider *fInAppStoreProvider;
		UIView *fActivityView;
		id fPopupControllerDelegate;
		mutable IPhoneWebPopup *fWebPopup;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhonePlatform_H__
