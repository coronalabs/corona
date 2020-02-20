//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacDevice_H__
#define _Rtt_MacDevice_H__

#include "Rtt_MPlatformDevice.h"
#include "Rtt_DeviceOrientation.h"

#if defined( Rtt_NO_GUI )
	#include "Input/Rtt_PlatformInputDeviceManager.h"
#else
	#include "Rtt_AppleInputDeviceManager.h"
#endif

// ----------------------------------------------------------------------------

@class AppleCallback;
@class NSString;
@class CoronaView;

namespace Rtt
{

class MCallback;
class PlatformSimulator;

// ----------------------------------------------------------------------------

class MacConsoleDevice : public MPlatformDevice
{
	public:
		typedef MPlatformDevice Super;

	public:
		MacConsoleDevice( Rtt_Allocator &allocator, CoronaView *view );
		virtual ~MacConsoleDevice();

	public:
		void SetManufacturer( NSString *newValue );
		void SetModel( NSString *newValue );

	public:
		const char* GetDeviceIdentifier() const;

	public:
		virtual const char* GetName() const;
		virtual const char* GetManufacturer() const;
		virtual const char* GetModel() const;
		virtual const char* GetUniqueIdentifier( IdentifierType t ) const;
		virtual EnvironmentType GetEnvironment() const;
		virtual const char* GetPlatformName() const;
		virtual const char* GetPlatform() const;
		virtual const char* GetPlatformVersion() const;
		virtual const char* GetArchitectureInfo() const;
		virtual PlatformInputDeviceManager& GetInputDeviceManager() override;
	
	public:
		// No-ops
		virtual void Vibrate() const;
		virtual void BeginNotifications( EventType type ) const;
		virtual void EndNotifications( EventType type ) const;
		virtual bool DoesNotify( EventType type ) const;
		virtual bool HasEventSource( EventType type ) const;
		virtual void SetAccelerometerInterval( U32 frequency ) const;
		virtual void SetGyroscopeInterval( U32 frequency ) const;

	public:
		virtual void SetLocationAccuracy( Real meters ) const;
		virtual void SetLocationThreshold( Real meters ) const;

	public:
		virtual DeviceOrientation::Type GetOrientation() const;

	protected:
		CoronaView *fView; // Weak ref

	private:
		Rtt_Allocator &fAllocator;
		mutable NSString *fDeviceManufacturer;
		mutable NSString *fDeviceModel;
		mutable NSString *fSystemVersion;
		mutable char* fDeviceIdentifier;
		DeviceNotificationTracker fTracker;
#if defined( Rtt_NO_GUI )
		PlatformInputDeviceManager fInputDeviceManager;
#else
		AppleInputDeviceManager fInputDeviceManager;
#endif

	
};

// TODO: Move this to a separate file
#if !defined( Rtt_WEB_PLUGIN )

class MacDevice : public MacConsoleDevice
{
	public:
		typedef MacConsoleDevice Super;

	public:
//		static DeviceOrientation::Type ToOrientationType( UIDeviceOrientation orientation );

	public:
		MacDevice( Rtt_Allocator &allocator, PlatformSimulator& simulator );

	public:
		virtual void Vibrate() const;

	public:
		virtual void BeginNotifications( EventType type ) const;
		virtual void EndNotifications( EventType type ) const;
		virtual bool HasEventSource( EventType type ) const;
		virtual void SetAccelerometerInterval( U32 frequency ) const;
		virtual void SetGyroscopeInterval( U32 frequency ) const;
		virtual const char *GetPlatform() const;

	public:
		virtual void SetLocationAccuracy( Real meters ) const;
		virtual void SetLocationThreshold( Real meters ) const;

	public:
		virtual DeviceOrientation::Type GetOrientation() const;

	public:
		PlatformSimulator& GetSimulator() const { return fSimulator; }

	public:
//		DeviceOrientation::Type GetPreviousOrientationAndUpdate( UIDeviceOrientation newValue );

	private:
		PlatformSimulator& fSimulator;
		mutable DeviceOrientation::Type fPreviousOrientation;
};

#endif // Rtt_WEB_PLUGIN
	
class MacAppDevice : public MacConsoleDevice
{
	public:
		typedef MacConsoleDevice Super;

	public:
		MacAppDevice( Rtt_Allocator &allocator, DeviceOrientation::Type orientation );
		virtual DeviceOrientation::Type GetOrientation() const;

	private:
		DeviceOrientation::Type fDeviceOrientation;
};


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacDevice_H__
