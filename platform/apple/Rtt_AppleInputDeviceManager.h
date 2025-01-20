//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __ratatouille__Rtt_AppleInputDeviceManager__
#define __ratatouille__Rtt_AppleInputDeviceManager__

#include "Input/Rtt_PlatformInputDeviceManager.h"
#include "Rtt_AppleInputDevice.h"

@class GCController;

namespace Rtt
{
	/// Stores a collection of all input devices that Corona supports and assigns them
	/// unique identifiers and descriptors for easy lookup within native code and in Lua.
	class AppleInputDeviceManager : public PlatformInputDeviceManager
	{
	public:
		AppleInputDevice::DriverType fCurrentDriverType;
		
		typedef AppleInputDeviceManager Self;

	public:
		AppleInputDeviceManager(Rtt_Allocator *allocatorPointer);
		virtual ~AppleInputDeviceManager();
		
		AppleInputDevice* GetBySerialNumber(NSString* sn);
		AppleInputDevice* GetByMFiDevice(GCController* controller);
		AppleInputDevice* GetByVendorName(NSString *vendor, AppleInputDevice::DriverType driver);

		
		int fLastSeenPlayerIndex;

	protected:
		virtual PlatformInputDevice* CreateUsing(const InputDeviceDescriptor &descriptor) override;
		virtual void Destroy(PlatformInputDevice* devicePointer) override;
		
	};
	
} // namespace Rtt


#endif /* defined(__ratatouille__Rtt_AppleInputDeviceManager__) */
