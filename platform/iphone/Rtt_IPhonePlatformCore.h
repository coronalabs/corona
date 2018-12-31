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

#ifndef _Rtt_IPhonePlatformCore_H__
#define _Rtt_IPhonePlatformCore_H__

#include "Rtt_IPhonePlatformBase.h"

#include "Rtt_IPhoneDevice.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class IPhoneImageProvider;
class IPhoneVideoProvider;
class IPhoneVideoPlayer;

// ----------------------------------------------------------------------------

// This is the default implementation of MPlatform for use on iOS devices.
// This is used by CoronaCards.
// (Corona/Native uses the subclass iPhonePlatform).
class IPhonePlatformCore : public IPhonePlatformBase
{
	Rtt_CLASS_NO_COPIES( IPhonePlatformCore )

	public:
		// TODO: Update this to actually use the classes instead of doing a runtime check.
		// This should match apple's UIUserNotificationType enums.
		typedef enum Rtt_UIUserNotificationType  : NSUInteger  {
			Rtt_UIUserNotificationTypeNone     = 0,
			Rtt_UIUserNotificationTypeBadge    = 1 << 0,
			Rtt_UIUserNotificationTypeSound    = 1 << 1,
			Rtt_UIUserNotificationTypeAlert    = 1 << 2,
		} Rtt_UIUserNotificationType;
	
	public:
		typedef IPhonePlatformBase Super;

	public:
		IPhonePlatformCore( CoronaView *view );
		virtual ~IPhonePlatformCore();

	public:
		virtual MPlatformDevice& GetDevice() const;

	public:
		virtual int GetStatusBarHeight() const;
		virtual int GetTopStatusBarHeightPixels() const;
		virtual int GetBottomStatusBarHeightPixels() const;

	protected:
		bool SaveBitmap( PlatformBitmap* bitmap, NSString* filePath, float jpegQuality ) const;

	public:
		virtual bool SaveImageToPhotoLibrary(const char* filePath) const;
		virtual bool SaveBitmap( PlatformBitmap* bitmap, const char* filePath, float jpegQuality ) const;
		virtual bool AddBitmapToPhotoLibrary( PlatformBitmap* bitmap ) const;
		/// virtual bool OpenURL( const char* url ) const;

		/// virtual void SetIdleTimer( bool enabled ) const;
		/// virtual bool GetIdleTimer() const;

		virtual Real GetStandardFontSize() const;
		/// virtual S32 GetFontNames( lua_State *L, int index ) const;

		virtual int PushSystemInfo( lua_State *L, const char *key ) const;

	public:
		void RegisterUserNotificationSettings() const;
		void RegisterUserNotificationSettings(int type) const;
	
	public:
		virtual void SetNativeProperty( lua_State *L, const char *key, int valueIndex ) const;
		virtual int PushNativeProperty( lua_State *L, const char *key ) const;

	private:
		IPhoneDevice fDevice;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhonePlatformCore_H__
