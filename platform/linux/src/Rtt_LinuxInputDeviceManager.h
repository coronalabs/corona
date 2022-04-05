//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_LinuxInputDeviceManager__
#define __Rtt_LinuxInputDeviceManager__

#include "Input/Rtt_PlatformInputDeviceManager.h"
#include "Rtt_LinuxInputDevice.h"

namespace Rtt
{
	class Runtime;

	/// Stores a collection of all input devices that Corona supports and assigns them
	/// unique identifiers and descriptors for easy lookup within native code and in Lua.
	class LinuxInputDeviceManager : public PlatformInputDeviceManager
	{
	public:
		typedef LinuxInputDeviceManager Self;
		LinuxInputDeviceManager(Rtt_Allocator *allocatorPointer);
		virtual ~LinuxInputDeviceManager();
		LinuxInputDevice* GetBySerialNumber(const char* sn);
		void dispatchEvents(Runtime* runtime);
		void init();
		void registerDevice(const char* dev);

	protected:
		virtual PlatformInputDevice* CreateUsing(const InputDeviceDescriptor &descriptor) override;
		virtual void Destroy(PlatformInputDevice* devicePointer) override;
	};
}; // namespace Rtt

#endif //__Rtt_LinuxInputDeviceManager__
