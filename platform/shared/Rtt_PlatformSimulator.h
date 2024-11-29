//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformSimulator_H__
#define _Rtt_PlatformSimulator_H__

#include "Rtt_DeviceOrientation.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformDevice.h"
#include "Rtt_String.h"
#include "Rtt_TargetDevice.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class MPlatform;
class PlatformPlayer;

// ----------------------------------------------------------------------------

struct SimulatorOptions
{
	bool connectToDebugger;
	bool isInteractive;
};

// Base class
// 
// A PlatformSimulator instance owns a platform-specific Player instance.
// 
// Each platform should have its own subclass.
// 
// The MPlatform is owned by this class --- it must outlive the player instance
// b/c the player instance owns the Runtime.
class PlatformSimulator
{
	Rtt_CLASS_NO_COPIES( PlatformSimulator )

	public:
		typedef PlatformSimulator Self;

		typedef void(*PlatformFinalizer)(const MPlatform*);

	public:
		template< typename T >
		static void Finalizer( const MPlatform* p )
		{
			delete static_cast< T* >( const_cast< MPlatform* >( p ) );
		}

	public:
		typedef enum _PropertyMask
		{
			kUninitializedMask = 0x0,

			kIsAppStartedMask = 0x0001,
			kIsOrientationLocked = 0x0002,

			// Event Masks
			kOrientationEventMask	= 0x0100,
			kAccelerometerEventMask = 0x0200,
			kLocationEventMask		= 0x0400,
			kHeadingEventMask		= 0x0800,
			kMultitouchEventMask	= 0x1000,
			kGyroscopeEventMask		= 0x2000,
			kMouseEventMask			= 0x4000
		}
		PropertyMask;

	public:
		struct Config
		{
			Config( Rtt_Allocator & allocator );
			~Config();

            bool configLoaded;
			TargetDevice::Platform platform;
			float screenOriginX;
			float screenOriginY;
			float screenWidth;
			float screenHeight;
			float safeScreenInsetStatusBar;
			float safeScreenInsetTop;
			float safeScreenInsetLeft;
			float safeScreenInsetBottom;
			float safeScreenInsetRight;
			float safeLandscapeScreenInsetStatusBar;
			float safeLandscapeScreenInsetTop;
			float safeLandscapeScreenInsetLeft;
			float safeLandscapeScreenInsetBottom;
			float safeLandscapeScreenInsetRight;
			String osName;
			String deviceImageFile;
			String displayManufacturer;
			String displayName;
			String statusBarDefaultFile;
			String statusBarTranslucentFile;
			String statusBarBlackFile;
			String statusBarLightTransparentFile;
			String statusBarDarkTransparentFile;
			String screenDressingFile;
			bool supportsScreenRotation;
			bool supportsExitRequests;
			bool supportsInputDevices;
			bool supportsKeyEvents;
			bool supportsKeyEventsFromKeyboard;
			bool supportsBackKey;
			bool supportsMouse;
			bool supportsMultipleAlerts;
			bool isAlertButtonOrderRightToLeft;
			bool isUprightOrientationPortrait;
			bool hasAccelerometer;
			bool hasGyroscope;
			String windowTitleBarName;
			float defaultFontSize;

			// iOS skin-specific
			S32 iosPointWidth;
			S32 iosPointHeight;

			// Android skin-specific
			S32 androidDisplayApproximateDpi;

			S32 GetAdaptiveWidth() const;
			S32 GetAdaptiveHeight() const;
		};

	public:
		PlatformSimulator( PlatformFinalizer finalizer );
		virtual ~PlatformSimulator();

	public:
		virtual const char *GetPlatformName() const;
		virtual const char *GetPlatform() const;

	protected:
		void Initialize( MPlatform* platform, MCallback *viewCallback = NULL );

		// build.settings
		void LoadBuildSettings( const MPlatform& platform );

	public:
		static const char kPlatformKeyNameIPhone[];
		static const char kPlatformKeyNameAndroid[];
		static const char kPlatformKeyNameMac[];
		// Currently loads the Lua state, computes the modules for a platform, closes the lua state.
		// isdefined is true if the components array is defined in the build.settings. If the field is omitted, it is false, intended to let us know if we should revert to the old behavior. An empty array {} should be provided if you want to specify no modules are desired.
		// iserror is true if the build.settings fails to load/run.
		// U32 is from the LuaContext::ModuleMask
		// platform keys are like: "iphone" or "android" or "mac", etc
		U32 GetModulesFromBuildSettings( const char* platformkeyname, bool& isdefined, bool& iserror );
	
		// Device configuration --- not to be confused with config.lua
		static void LoadConfig(const char deviceConfigFile[], Config& rConfig);
//TODO: Deprecate this function. MPlatform object is never used here and it is expensive to create one when not needed.
		static void LoadConfig(const char deviceConfigFile[], const MPlatform& platform, Config& rConfig);
		static void ValidateSettings( const MPlatform& platform );

//		virtual PlatformStringRef CreatePlatformString( const char* src ) const = 0;
//		virtual void ReleasePlatformString( PlatformStringRef str ) const = 0;


	public:
		Rtt_INLINE bool IsProperty( U32 mask ) const { return (fProperties & mask) != 0; }
		Rtt_INLINE void ToggleProperty( U32 mask ) { fProperties ^= mask; }
		void SetProperty( U32 mask, bool value );

	public:
		void Start( const SimulatorOptions& options );

	public:
		void BeginNotifications( MPlatformDevice::EventType type ) const;
		void EndNotifications( MPlatformDevice::EventType type ) const;

		void Rotate( bool clockwise );
		virtual void Shake();
		DeviceOrientation::Type GetOrientation() const { return (DeviceOrientation::Type)fOrientation; }
		virtual const char *GetOSName() const { return "simulator"; }

		virtual void DidRotate( bool clockwise, DeviceOrientation::Type start, DeviceOrientation::Type end ) = 0;

		// Optional callback for systems when the zoom/scale change finishes.
		// On Mac, it is used to save the scale factor to user preferences so the next launch can be reopened at the same level.
		virtual void DidChangeScale( float scalefactor );

        // call before passing point to TouchEvent, etc.
		void AdjustPoint( float& ptX, float& ptY, float viewWidth, float viewHeight, float zoomFactor );

	public:
		bool IsOrientationSupported( DeviceOrientation::Type orientation );

	protected:
		void SetOrientationSupported( DeviceOrientation::Type orientation );

	protected:
		void SetIsTransparent( bool newValue ) { fIsTransparent = newValue; }
		bool GetIsTransparent() const { return fIsTransparent; }
	
	public:
		const PlatformPlayer* GetPlayer() const { return fPlayer; }
		PlatformPlayer* GetPlayer() { return fPlayer; }

	public:
		virtual void SetStatusBarMode( MPlatform::StatusBarMode newValue ) = 0;
		virtual MPlatform::StatusBarMode GetStatusBarMode() const = 0;

	public:
		virtual void WillSuspend();
		virtual void DidSuspend();
		virtual void WillResume();
		virtual void DidResume();
		void ToggleSuspendResume(bool sendApplicationEvents = true);

	private:
		void DidStart(); // Called first time DidResume() is invoked
		void WillExit(); // Called by the dtor

	private:
		MPlatform* fPlatform; // Platform must outlive Player
		PlatformPlayer* fPlayer;
		PlatformFinalizer fPlatformFinalizer;
		mutable U32 fProperties;
		S8 fOrientation;
		U8 fSupportedOrientations;
		DeviceOrientation::Type fLastSupportedOrientation;
		S32 fLastDeviceWidth;
		S32 fLastDeviceHeight;
		bool fIsTransparent;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformSimulator_H__
