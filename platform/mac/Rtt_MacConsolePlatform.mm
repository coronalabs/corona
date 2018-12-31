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

#include "Core/Rtt_Build.h"

#include "Rtt_MacConsolePlatform.h"

#import <Foundation/Foundation.h>
#import "NSString+Extensions.h"

#if !defined( Rtt_PROJECTOR )
	#include "Rtt_JavaHost.h"
#endif

#include "Core/Rtt_String.h"

/*
#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaResource.h"
#include "Rtt_MacFBConnect.h"
#include "Rtt_MacImageProvider.h"
#include "Rtt_MacMapViewObject.h"
#include "Rtt_MacTextFieldObject.h"
#include "Rtt_MacTextBoxObject.h"
#include "Rtt_MacVideoPlayer.h"
#include "Rtt_MacViewSurface.h"
#include "Rtt_MacVideoObject.h"
#include "Rtt_MacWebPopup.h"
#include "Rtt_MacWebViewObject.h"
#include "Rtt_MacActivityIndicator.h"
#include "Rtt_PlatformInAppStore.h"
#include "Rtt_AppleInAppStore.h"
#include "Rtt_PlatformPlayer.h"
#include "Rtt_PlatformSimulator.h"
#include "Rtt_RenderingStream.h"

#include "Rtt_AppleConnection.h"
#include "Rtt_Authorization.h" // used for string contants for preferences

//#include "Rtt_AppleDictionaryWrapper.h"
#include "Rtt_AppleConnection.h"
#include "Rtt_MacSimulator.h"
#include "Rtt_MacExitCallback.h"
#import "AppDelegate.h"
#import "GLView.h"

#import <AppKit/NSAlert.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSFontManager.h>
#import <AppKit/NSOpenGL.h>
#import <AppKit/NSWindow.h>
#import <Foundation/NSString.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import <netinet/in.h>

//#import <CoreFoundation/CoreFoundation.h>
//#import <Security/Security.h>
//#import <CoreServices/CoreServices.h>

#if (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5)
	#import <Foundation/NSFileManager.h>
	#import <Foundation/NSPathUtilities.h>
	#import <Foundation/NSURL.h>
	#include <CoreFoundation/CoreFoundation.h>
	#include <ApplicationServices/ApplicationServices.h>
#endif

#include "Rtt_Lua.h"


#include <pthread.h>
*/

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static bool
MacOpenURL( const char *url )
{
	bool result = false;

	if ( url )
	{
		NSURL* urlPlatform = [NSURL URLWithString:[NSString stringWithExternalString:url]];
		OSStatus status = LSOpenCFURLRef( (CFURLRef)urlPlatform, NULL );

		// Compare with result codes:
		// http://developer.apple.com/DOCUMENTATION/Carbon/Reference/LaunchServicesReference/Reference/reference.html#//apple_ref/doc/uid/TP30000998-CH4g-BCIHFFIA
		result = Rtt_VERIFY( noErr == status );

		#ifdef Rtt_AUTHORING_SIMULATOR
		if ( ! result )
		{
			Rtt_TRACE_SIM( ( "WARNING: url( %s ) is not supported by the simulator\n", url ) );
		}
		#endif
	}

	return result;
}

// ----------------------------------------------------------------------------

MacConsolePlatform::MacConsolePlatform()
:	fDevice( GetAllocator(), NULL )
{
}

MacConsolePlatform::~MacConsolePlatform()
{
}
	
MPlatformDevice&
MacConsolePlatform::GetDevice() const
{
	return const_cast< MacConsoleDevice& >( fDevice );
}

PlatformSurface*
MacConsolePlatform::CreateScreenSurface() const
{
	return NULL;
}

PlatformSurface*
MacConsolePlatform::CreateOffscreenSurface( const PlatformSurface& ) const
{
	return NULL;
}

bool
MacConsolePlatform::SaveImageToPhotoLibrary(const char* filePath) const
{
    return true;
}
    
bool
MacConsolePlatform::SaveBitmap( PlatformBitmap* bitmap, const char* filePath, float jpegQuality ) const
{
	return false;
}

bool
MacConsolePlatform::AddBitmapToPhotoLibrary( PlatformBitmap* bitmap ) const
{
	return false;
}

bool
MacConsolePlatform::OpenURL( const char* url ) const
{
	return MacOpenURL( url );
}

int
MacConsolePlatform::CanOpenURL( const char* url ) const
{
	return -1;
}
	
PlatformVideoPlayer *
MacConsolePlatform::GetVideoPlayer( const ResourceHandle<lua_State> & handle ) const
{
	return NULL;
}

PlatformImageProvider*
MacConsolePlatform::GetImageProvider( const ResourceHandle<lua_State> & handle ) const
{
	return NULL;
}

PlatformVideoProvider*
MacConsolePlatform::GetVideoProvider( const ResourceHandle<lua_State> & handle ) const
{
	return NULL;
}

PlatformStoreProvider*
MacConsolePlatform::GetStoreProvider( const ResourceHandle<lua_State>& handle ) const
{
	return NULL;
}

void
MacConsolePlatform::SetStatusBarMode( StatusBarMode newValue ) const
{
	Rtt_ASSERT_NOT_REACHED();
}

MPlatform::StatusBarMode
MacConsolePlatform::GetStatusBarMode() const
{
	Rtt_ASSERT_NOT_REACHED();
	return MPlatform::kHiddenStatusBar;
}

void
MacConsolePlatform::SetIdleTimer( bool enabled ) const
{
}
	
bool
MacConsolePlatform::GetIdleTimer() const
{
	return true;
}

NativeAlertRef
MacConsolePlatform::ShowNativeAlert(
	const char *title,
	const char *msg,
	const char **buttonLabels,
	U32 numButtons,
	LuaResource* resource ) const
{
	return NULL;
}

void
MacConsolePlatform::CancelNativeAlert( NativeAlertRef alert, S32 index ) const
{
}

void
MacConsolePlatform::SetActivityIndicator( bool visible ) const
{
}

PlatformWebPopup*
MacConsolePlatform::GetWebPopup() const
{
	return NULL;
}

PlatformDisplayObject*
MacConsolePlatform::CreateNativeTextBox( const Rect& bounds ) const
{
	return NULL;
}

PlatformDisplayObject*
MacConsolePlatform::CreateNativeTextField( const Rect& bounds ) const
{
	return NULL;
}

void
MacConsolePlatform::SetKeyboardFocus( PlatformDisplayObject *textObject ) const
{
}

PlatformDisplayObject *
MacConsolePlatform::CreateNativeMapView( const Rect& bounds ) const
{
	return NULL;
}

PlatformDisplayObject *
MacConsolePlatform::CreateNativeWebView( const Rect& bounds ) const
{
	return NULL;
}

PlatformDisplayObject *
MacConsolePlatform::CreateNativeVideo( const Rect& bounds ) const
{
	return NULL;
}

S32
MacConsolePlatform::GetFontNames( lua_State *L, int index ) const
{
	return 0;
}

void
MacConsolePlatform::SetTapDelay( Rtt_Real delay ) const
{
}

Rtt_Real
MacConsolePlatform::GetTapDelay() const
{
	return 0.;
}

PlatformFBConnect*
MacConsolePlatform::GetFBConnect() const
{
	return NULL;
}

void
MacConsolePlatform::RuntimeErrorNotification( const char *errorType, const char *message, const char *stacktrace ) const
{
}
    
void
MacConsolePlatform::SetProjectResourceDirectory( const char* filename )
{
}

void MacConsolePlatform::GetSafeAreaInsetsPixels(Rtt_Real &top, Rtt_Real &left, Rtt_Real &bottom, Rtt_Real &right) const
{
	top = left = bottom = right = 0;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

