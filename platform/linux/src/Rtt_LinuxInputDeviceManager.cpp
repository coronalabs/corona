//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_LinuxInputDevice.h"
#include "Rtt_LinuxInputDeviceManager.h"
#include "Rtt_Event.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

namespace Rtt
{
	LinuxInputDeviceManager::LinuxInputDeviceManager(Rtt_Allocator *allocatorPointer)
		: PlatformInputDeviceManager(allocatorPointer)
	{
	}

	LinuxInputDeviceManager::~LinuxInputDeviceManager()
	{
	}

	void LinuxInputDeviceManager::init()
	{
		registerDevice("/dev/input/js0");
		registerDevice("/dev/input/js1");
	}

	void LinuxInputDeviceManager::registerDevice(const char *dev)
	{
		LinuxInputDevice *devicePointer = static_cast<LinuxInputDevice*>(this->Add(InputDeviceType::kJoystick));
		devicePointer->init(dev);
	}

	PlatformInputDevice *LinuxInputDeviceManager::CreateUsing(const InputDeviceDescriptor &descriptor)
	{
		return Rtt_NEW(GetAllocator(), LinuxInputDevice(descriptor));
	}

	LinuxInputDevice *LinuxInputDeviceManager::GetBySerialNumber(const char *sn)
	{
		if (NULL == sn)
		{
			return NULL;
		}

		const ReadOnlyInputDeviceCollection &collection = GetDevices();

		for (int index = 0; index < collection.GetCount(); index++)
		{
			LinuxInputDevice* devicePointer = (LinuxInputDevice*)collection.GetByIndex(index);

			if (devicePointer != NULL) //fixme && ([devicePointer->fSerialNumber isEqualToString:sn]))
			{
				return devicePointer;
			}
		}

		return NULL;
	}

	void LinuxInputDeviceManager::Destroy(PlatformInputDevice *devicePointer)
	{
		Rtt_DELETE(devicePointer);
	}

	void LinuxInputDeviceManager::dispatchEvents(Runtime *runtime)
	{
		const ReadOnlyInputDeviceCollection &collection = GetDevices();

		for (int index = 0; index < collection.GetCount(); index++)
		{
			LinuxInputDevice *devicePointer = (LinuxInputDevice*)collection.GetByIndex(index);

			if (devicePointer != NULL && devicePointer->GetConnectionState() == InputDeviceConnectionState::kConnected)
			{
				devicePointer->dispatchEvents(runtime);
			}
		}
	}
}
