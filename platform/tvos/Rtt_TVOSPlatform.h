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

		virtual void SetActivityIndicator( bool visible ) const;

		virtual bool CanShowPopup( const char *name ) const;
		virtual bool ShowPopup( lua_State *L, const char *name, int optionsIndex ) const;
		virtual bool HidePopup( const char *name ) const;

		virtual void SetTapDelay( Rtt_Real delay ) const;
		virtual Rtt_Real GetTapDelay() const;

	public:
		virtual void RuntimeErrorNotification( const char *errorType, const char *message, const char *stacktrace ) const;

	private:
		TVOSDevice fDevice;
//		mutable IPhoneVideoPlayer *fVideoPlayer;
		mutable AppleStoreProvider *fInAppStoreProvider;
		UIView *fActivityView;
		id fPopupControllerDelegate;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TVOSPlatform_H__
