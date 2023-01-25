//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_MPlatformDevice.h"
#include "Rtt_DeviceOrientation.h"
#include "Input/Rtt_PlatformInputDeviceManager.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AndroidPlatform;
class MCallback;
class String;


class EmscriptenDevice : public MPlatformDevice
{
	public:
		typedef MPlatformDevice Super;

		EmscriptenDevice(Rtt_Allocator& allocator);
		virtual ~EmscriptenDevice();

		virtual const char* GetModel() const override;
		virtual const char* GetName() const override;
		virtual const char* GetUniqueIdentifier(IdentifierType t) const override;
		virtual EnvironmentType GetEnvironment() const override;
		virtual const char* GetPlatformName() const override;
		virtual const char* GetPlatformVersion() const override;
		virtual const char* GetProductName() const;
		virtual const char* GetArchitectureInfo() const override;
		virtual PlatformInputDeviceManager& GetInputDeviceManager() override;
		virtual void Vibrate() const override;
		virtual void BeginNotifications(EventType type) const override;
		virtual void EndNotifications(EventType type) const override;
		virtual bool DoesNotify(EventType type) const override;
		virtual bool HasEventSource(EventType type) const override;
		virtual void SetAccelerometerInterval(U32 frequency) const override;
		virtual void SetGyroscopeInterval(U32 frequency) const override;
		virtual void SetLocationAccuracy(Real meters) const override;
		virtual void SetLocationThreshold(Real meters) const override;
		virtual void SetOrientation(DeviceOrientation::Type orientation);
		virtual DeviceOrientation::Type GetOrientation() const override;
		virtual const char* GetPlatform() const override;
		virtual const char* GetManufacturer() const override;

	private:
		Rtt_Allocator& fAllocator;
		DeviceNotificationTracker fTracker;
		DeviceOrientation::Type fOrientation;
		PlatformInputDeviceManager fInputDeviceManager;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
