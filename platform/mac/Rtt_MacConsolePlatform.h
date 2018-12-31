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

#ifndef _Rtt_MacConsolePlatform_H__
#define _Rtt_MacConsolePlatform_H__

#include "Rtt_ApplePlatform.h"

#include "Rtt_MacDevice.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class MacConsoleDevice;

// ----------------------------------------------------------------------------

class MacConsolePlatform : public ApplePlatform
{
	public:
		typedef ApplePlatform Super;

	public:
		MacConsolePlatform();
		virtual ~MacConsolePlatform();


	public:
		virtual MPlatformDevice& GetDevice() const;
		virtual PlatformSurface* CreateScreenSurface() const;
		virtual PlatformSurface* CreateOffscreenSurface( const PlatformSurface& parent ) const;
    
        virtual bool SaveImageToPhotoLibrary(const char* filePath) const;
		virtual bool SaveBitmap( PlatformBitmap* bitmap, const char* filePath, float jpegQuality ) const;
		virtual bool AddBitmapToPhotoLibrary( PlatformBitmap* bitmap ) const;
		virtual bool OpenURL( const char* url ) const;
		virtual int CanOpenURL( const char* url ) const;

		virtual PlatformVideoPlayer * GetVideoPlayer( const ResourceHandle<lua_State> & handle ) const;
		virtual PlatformImageProvider* GetImageProvider( const ResourceHandle<lua_State> & handle ) const;
		virtual PlatformStoreProvider* GetStoreProvider( const ResourceHandle<lua_State>& handle ) const;
		virtual PlatformVideoProvider* GetVideoProvider( const ResourceHandle<lua_State> & handle ) const;

		virtual void SetStatusBarMode( StatusBarMode newValue ) const;
		virtual StatusBarMode GetStatusBarMode() const;

		virtual void SetIdleTimer( bool enabled ) const;
		virtual bool GetIdleTimer() const;

		virtual NativeAlertRef ShowNativeAlert(
			const char *title,
			const char *msg,
			const char **buttonLabels,
			U32 numButtons,
			LuaResource* resource ) const;
		virtual void CancelNativeAlert( NativeAlertRef alert, S32 index ) const;

		virtual void SetActivityIndicator( bool visible ) const;

		virtual PlatformWebPopup* GetWebPopup() const;

		virtual PlatformDisplayObject* CreateNativeTextBox( const Rect& bounds ) const;
		virtual PlatformDisplayObject* CreateNativeTextField( const Rect& bounds ) const;
		virtual void SetKeyboardFocus( PlatformDisplayObject *textObject ) const;

		virtual PlatformDisplayObject* CreateNativeMapView( const Rect& bounds ) const;
		virtual PlatformDisplayObject* CreateNativeWebView( const Rect& bounds ) const;
		virtual PlatformDisplayObject* CreateNativeVideo( const Rect& bounds ) const;
        virtual PlatformFBConnect* GetFBConnect() const;

		virtual S32 GetFontNames( lua_State *L, int index ) const;

		virtual void SetTapDelay( Rtt_Real delay ) const;
		virtual Rtt_Real GetTapDelay() const;

		virtual void RuntimeErrorNotification( const char *errorType, const char *message, const char *stacktrace ) const;
        virtual void SetProjectResourceDirectory( const char* filename );

		virtual void GetSafeAreaInsetsPixels(Rtt_Real &top, Rtt_Real &left, Rtt_Real &bottom, Rtt_Real &right) const;
		
	private:
		MacConsoleDevice fDevice;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacConsolePlatform_H__
