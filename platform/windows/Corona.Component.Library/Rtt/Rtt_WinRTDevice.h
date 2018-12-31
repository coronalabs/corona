// ----------------------------------------------------------------------------
// 
// Rtt_WinRTDevice.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Input/Rtt_PlatformInputDeviceManager.h"
#	include "Rtt_DeviceOrientation.h"
#	include "Rtt_MPlatformDevice.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


#pragma region Forward Declarations
namespace CoronaLabs { namespace Corona { namespace WinRT {
	namespace Interop { namespace Sensors {
		ref class CoronaAccelerometerHandler;
		ref class CoronaGyroscopeHandler;
	} }
	ref class CoronaRuntimeEnvironment;
} } }

#pragma endregion


namespace Rtt
{

#pragma region Forward Declarations
class AndroidPlatform;
class MCallback;
class String;

#pragma endregion


class WinRTDevice : public MPlatformDevice
{
	public:
		typedef MPlatformDevice Super;

		WinRTDevice(Rtt_Allocator& allocator, CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ environment);
		virtual ~WinRTDevice();

		virtual const char* GetName() const;
		virtual const char* GetManufacturer() const;
		virtual const char* GetModel() const;
		virtual const char* GetUniqueIdentifier(IdentifierType t) const;
		virtual EnvironmentType GetEnvironment() const;
		virtual const char* GetPlatformName() const;
		virtual const char* GetPlatform() const;
		virtual const char* GetPlatformVersion() const;
		virtual const char* GetProductName() const;
		virtual const char* GetArchitectureInfo() const;
		virtual PlatformInputDeviceManager& GetInputDeviceManager();
		virtual void Vibrate() const;
		virtual void BeginNotifications(EventType type) const;
		virtual void EndNotifications(EventType type) const;
		virtual bool DoesNotify(EventType type) const;
		virtual bool HasEventSource(EventType type) const;
		virtual void SetAccelerometerInterval(U32 frequency) const;
		virtual void SetGyroscopeInterval(U32 frequency) const;
		virtual void SetLocationAccuracy(Real meters) const;
		virtual void SetLocationThreshold(Real meters) const;
		virtual DeviceOrientation::Type GetOrientation() const;

	private:
		Rtt_Allocator& fAllocator;
		CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ fEnvironment;
		CoronaLabs::Corona::WinRT::Interop::Sensors::CoronaAccelerometerHandler^ fAccelerometerHandler;
		CoronaLabs::Corona::WinRT::Interop::Sensors::CoronaGyroscopeHandler^ fGyroscopeHandler;
		DeviceNotificationTracker fTracker;
		PlatformInputDeviceManager fInputDeviceManager;
};

} // namespace Rtt
