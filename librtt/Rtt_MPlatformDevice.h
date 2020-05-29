//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MPlatformDevice_H__
#define _Rtt_MPlatformDevice_H__

#include "Rtt_MCallback.h"
#include "Rtt_DeviceOrientation.h"
#include "Core/Rtt_Assert.h"
#include "Core/Rtt_Real.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class PlatformInputDeviceManager;

// ----------------------------------------------------------------------------

class MPlatformDevice
{
	public:
		typedef enum _EventType
		{
			kUnknownEvent = -1,
			kOrientationEvent = 0,
			kAccelerometerEvent,
			kGyroscopeEvent,
			kLocationEvent,
			kHeadingEvent,
			kMultitouchEvent,
			kCollisionEvent,
			kPreCollisionEvent,
			kPostCollisionEvent,
			kParticleCollisionEvent,
			kMemoryWarningEvent,
			kTrackingEvent,
			kKeyEvent,
			kInputDeviceStatusEvent,
			kMouseEvent,

			kNumTypes
		}
		EventType;

		typedef enum _ActivationType
		{
			kActivateUnknown = -1,
			kActivateControllerUserInteraction = 0,
			kActivationTypeCount
		}
		ActivationType;

		typedef enum _EnvironmentType
		{
			kSimulatorEnvironment = 0,
			kDeviceEnvironment,
			kBrowserEnvironment
		}
		EnvironmentType;

		typedef enum _IdentifierType
		{
			// DO NOT CHANGE ENUM VALUES (Android bridge makes assumptions)
			kDeviceIdentifier = 0,		// The original "deviceId". On some platforms, there was implicit fallback to other identifier types, so we decided to NEVER change the behavior of this API.
			kHardwareIdentifier = 1,	// The hardware id of the device (could be same as kDeviceIdentifier, but there is no fallback to other id mechanisms)
			kOSIdentifier = 2,			// An id created by the OS
			kMacIdentifier = 3,			// The ethernet MAC address
			kUdidIdentifier = 4,		// Some udid scheme that's platform specific
			
			kIOSAdvertisingIdentifier = 5,		// iOS specific advertizing APIs
			kIOSAdvertisingTrackingEnabled = 6,
			kIOSIdentifierForVendor = 7,
			
			kNumIdentifiers
		}
		IdentifierType;
	
	public:
		virtual const char* GetName() const = 0;
		virtual const char* GetManufacturer() const = 0;
		virtual const char* GetModel() const = 0;
		virtual const char* GetUniqueIdentifier( IdentifierType t ) const = 0;
		virtual EnvironmentType GetEnvironment() const = 0;
		virtual const char* GetPlatformName() const = 0;
		virtual const char* GetPlatform() const = 0;
		virtual const char* GetPlatformVersion() const = 0;
		virtual const char* GetArchitectureInfo() const = 0;
		virtual PlatformInputDeviceManager& GetInputDeviceManager() = 0;

	public:
		virtual void Vibrate() const = 0;

	public:
		virtual void BeginNotifications( EventType type ) const = 0;
		virtual void EndNotifications( EventType type ) const = 0;
		virtual bool DoesNotify( EventType type ) const = 0;
		virtual bool HasEventSource( EventType type ) const = 0;
		virtual void SetAccelerometerInterval( U32 frequency ) const = 0;
		virtual void SetGyroscopeInterval( U32 frequency ) const = 0;
	
		// Activate/deactivate system features.
		// Returns true if the feature is supported on that platform. This is to provide backwards compatibility with
		// event-driven system.activate/system.deactivate calls (BeginNotifications/EndNotifications above).
		virtual bool Activate( ActivationType key ) const { return false; }
		virtual bool Deactivate( ActivationType key ) const { return false; }

	public:
		virtual void SetLocationAccuracy( Real meters ) const = 0;
		virtual void SetLocationThreshold( Real meters ) const = 0;

	public:
		virtual DeviceOrientation::Type GetOrientation() const = 0;
};

// Use this to implement MPlatformDevice::DoesNotify
class DeviceNotificationTracker
{
	public:
		typedef MPlatformDevice::EventType EventType;

	public:
		DeviceNotificationTracker() : fNotifications( 0 ) { }

	protected:
		bool Verify( EventType type ) const
		{
			return Rtt_VERIFY( type >= MPlatformDevice::kUnknownEvent
							   && type < MPlatformDevice::kNumTypes ) != 0;
		}

		U32 GetMask( EventType type ) const { return ( 1 << type ); }

	public:
		void BeginNotifications( EventType type ) const { if ( Verify( type ) ) { fNotifications |= GetMask( type ); } }
		void EndNotifications( EventType type ) const { if ( Verify( type ) ) { fNotifications &= ~GetMask( type ); } }
		bool DoesNotify( EventType type ) const { return Verify( type ) && ( fNotifications & GetMask( type ) ); }

	private:
		mutable U32 fNotifications;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MPlatformDevice_H__
