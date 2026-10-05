//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

class MacGUIPlatform;
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
		virtual bool DidChangeFold( bool unfolded );

	private:
		DeviceOrientation::Type SkinNaturalOrientation( const Config& config ) const;
		NSSize ScreenSizeForEngine( float width, float height, const Config& config ) const;
		DeviceOrientation::Type ApplyScreenSize( const Config& config, GLView *view );

	public:

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
		// Properties that describe the simulated screen; a foldable device changes them when it folds
		void SetScreenProperties( const Config& config );

	private:
//		NSAutoreleasePool* fPool;
//		SkinnableWindow* fWindow;
		NSString* fDeviceConfigFile;
		MacGUIPlatform* fMacPlatform;
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
