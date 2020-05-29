//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidDevice_H__
#define _Rtt_AndroidDevice_H__

#include "Rtt_MPlatformDevice.h"
#include "Rtt_DeviceOrientation.h"
#include "Rtt_AndroidInputDeviceManager.h"

class NativeToJavaBridge;
// ----------------------------------------------------------------------------

namespace Rtt
{

class AndroidPlatform;
class MCallback;
class String;

// ----------------------------------------------------------------------------

class AndroidDevice : public MPlatformDevice
{
	public:
		typedef MPlatformDevice Super;

	public:
		AndroidDevice( Rtt_Allocator& allocator, NativeToJavaBridge *ntjb );
		~AndroidDevice();

	public:
		virtual const char* GetName() const;
		virtual const char* GetManufacturer() const;
		virtual const char* GetModel() const;
		virtual const char* GetUniqueIdentifier( IdentifierType t ) const;
		virtual EnvironmentType GetEnvironment() const;
		virtual const char* GetPlatformName() const;
		virtual const char* GetPlatform() const;
		virtual const char* GetPlatformVersion() const;
		virtual const char* GetProductName() const;
		virtual const char* GetArchitectureInfo() const;
		virtual PlatformInputDeviceManager& GetInputDeviceManager();

	public:
		virtual void Vibrate() const;

	public:
		virtual void BeginNotifications( EventType type ) const;
		virtual void EndNotifications( EventType type ) const;
		virtual bool DoesNotify( EventType type ) const;
		virtual bool HasEventSource( EventType type ) const;
		virtual void SetAccelerometerInterval( U32 frequency ) const;
		virtual void SetGyroscopeInterval( U32 frequency ) const;

	public:
		virtual void SetLocationAccuracy( Real meters ) const;
		virtual void SetLocationThreshold( Real meters ) const;
		virtual void SetOrientation( DeviceOrientation::Type orientation );

	public:
		virtual DeviceOrientation::Type GetOrientation() const;

	private:
		Rtt_Allocator& fAllocator;
		DeviceNotificationTracker fTracker;
		AndroidInputDeviceManager fInputDeviceManager;
		mutable String *fName;
		mutable String *fManufacturer;
		mutable String *fModel;
		mutable String *fUniqueIdentifier;
		mutable String *fHardwareIdentifier;
		mutable String *fOSIdentifier;
		mutable String *fPlatformVersion;
		mutable String *fProduct;
		DeviceOrientation::Type fOrientation;
		NativeToJavaBridge *fNativeToJavaBridge;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidDevice_H__
