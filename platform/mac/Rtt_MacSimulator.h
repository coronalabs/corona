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

#ifndef _Rtt_MacSimulator_H__
#define _Rtt_MacSimulator_H__

#include "Rtt_PlatformSimulator.h"
#include "Rtt_AppleInputDeviceManager.h"
#include "Rtt_AppleInputHIDDeviceListener.h"
#include "Rtt_AppleInputMFIDeviceListener.h"

// ----------------------------------------------------------------------------

//@class NSAutoreleasePool;
@class GLView;
@class NSDictionary;
@class NSMutableDictionary;
@class NSWindow;
@class NSWindowController;
@class NSString;
//@class SkinnableWindow;

namespace Rtt
{

class MacViewCallback;

// ----------------------------------------------------------------------------

class MacSimulator : public PlatformSimulator
{
	public:
		typedef PlatformSimulator Super;
		typedef MacSimulator Self;

	public:
		MacSimulator( bool headless = false );
		virtual ~MacSimulator();

	public:
		virtual void Initialize(
			const char deviceConfigFile[],
			const char resourcePath[] );
		virtual const char *GetPlatformName() const;
		virtual const char *GetPlatform() const;

	public:
		virtual void DidRotate( bool clockwise, DeviceOrientation::Type start, DeviceOrientation::Type end );

		// Optional callback for systems when the zoom/scale change finishes.
		// On Mac, it is used to save the scale factor to user preferences so the next launch can be reopened at the same level.
		virtual void DidChangeScale( float scalefactor );
	
	public:
		virtual void SetStatusBarMode( MPlatform::StatusBarMode newValue );
		virtual MPlatform::StatusBarMode GetStatusBarMode() const;

	public:
		GLView* GetScreenView() const;
		NSWindow* GetWindow() const { return (NSWindow*)fWindow; }

		virtual void Shake();
		bool Back();
		bool SupportsBackKey();
		virtual const char *GetOSName() const;


	public:
		NSDictionary* GetProperties() const { return (NSDictionary*)fProperties; }

	public:
		// Physical pixel w,h of the device
		float GetScreenWidth() const { return fScreenWidth; }
		float GetScreenHeight() const { return fScreenHeight; }

	public:
		virtual void WillSuspend();
		virtual void DidResume();
	
	protected:
//		virtual PlatformStringRef CreatePlatformString( const char* src ) const;
//		virtual void ReleasePlatformString( PlatformStringRef str ) const;

	private:
//		NSAutoreleasePool* fPool;
//		SkinnableWindow* fWindow;
		NSWindow* fWindow;
		NSWindowController* fWindowController;
		NSMutableDictionary* fProperties;
		float fScreenWidth;
		float fScreenHeight;
		bool fSupportsScreenRotation;
		bool fSupportsBackButton;
		bool fHeadless;
		NSString* fDeviceSkinIdentifier; // To save the scale factor for the current skin, we need to know which skin we are on.
        NSString* fDeviceName;
		MacViewCallback *fViewCallback;
		AppleInputHIDDeviceListener *fMacHidDeviceListener;
		AppleInputMFiDeviceListener	*fMacMFIListener;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacSimulator_H__
