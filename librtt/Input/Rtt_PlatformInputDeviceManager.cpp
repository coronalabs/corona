//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_PlatformInputDeviceManager.h"
#include "Core/Rtt_Build.h"
#include "Rtt_InputDeviceType.h"
#include "Rtt_PlatformInputDevice.h"


namespace Rtt
{

// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new input device manager.
/// @param allocatorPointer Allocator needed to create input device objects.
PlatformInputDeviceManager::PlatformInputDeviceManager(Rtt_Allocator *allocatorPointer)
:	fDeviceCollection(allocatorPointer),
	fReadOnlyDeviceCollection(&fDeviceCollection)
{
}

/// Destroys this device manager and the input devices it manages.
PlatformInputDeviceManager::~PlatformInputDeviceManager()
{
	PlatformInputDevice *devicePointer;

	// Delete all device objects in the main collection.
	for (int index = fDeviceCollection.GetCount() - 1; index >= 0; index--)
	{
		devicePointer = fDeviceCollection.GetByIndex(index);
		Destroy(devicePointer);
	}
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Adds a new input device to the manager and assigns it a unique ID and descriptor.
/// @param type The type of input device to add such as a keyboard, mouse, gamepad, etc.
/// @return Returns a pointer to a new input device object for the given type.
PlatformInputDevice* PlatformInputDeviceManager::Add(InputDeviceType type)
{
	PlatformInputDevice *devicePointer;

	// Count the number of devices matching the given type.
	// This counts the disconnected devices too, in case they come back later.
	int deviceTypeCount = 0;
	for (int index = 0; index < fDeviceCollection.GetCount(); index++)
	{
		devicePointer = fDeviceCollection.GetByIndex(index);
		if (devicePointer && devicePointer->GetDescriptor().GetDeviceType().Equals(type))
		{
			deviceTypeCount++;
		}
	}

	// Create a new device object with a unique descriptor and add it to the main collection.
	// The descriptor assigns the device a unique number for its type, such as "Joystick 1".
	InputDeviceDescriptor descriptor(GetAllocator(), type, deviceTypeCount + 1);
	devicePointer = CreateUsing(descriptor);
	fDeviceCollection.Add(devicePointer);

	// Return the new device object.
	return devicePointer;
}

/// Gets the allocator that this manager uses when adding new input device objects.
/// @return Returns a pointer to the allocator this manager uses.
Rtt_Allocator* PlatformInputDeviceManager::GetAllocator() const
{
	return fDeviceCollection.GetAllocator();
}

/// Fetches a collection of all input devices that have ever connected to the machine.
/// @return Returns a read-only collection of all input devices.
const ReadOnlyInputDeviceCollection& PlatformInputDeviceManager::GetDevices() const
{
	return fReadOnlyDeviceCollection;
}


// ----------------------------------------------------------------------------
// Protected Member Functions
// ----------------------------------------------------------------------------

/// Called when this device manager needs a new input device object to be created.
/// <br>
/// Can be overriden by a derived class to create a platform specific input device object.
/// @param descriptor Unique descriptor used to identify the new input device.
/// @return Returns a pointer to the newly created input device object.
PlatformInputDevice* PlatformInputDeviceManager::CreateUsing(const InputDeviceDescriptor &descriptor)
{
	return Rtt_NEW(GetAllocator(), PlatformInputDevice(descriptor));
}

/// Called when this device manager needs to destroy the given input device object.
/// <br>
/// Can be overriden by a derived class to delete a platform specific input device object
/// in the same module/library it was created in. This is important on platforms such as
/// Windows where each module has their own heap and you must delete an object from the
/// same module that it was created in, or else a heap error would cause a crash.
/// @param devicePointer Pointer to the input device object to be deleted.
void PlatformInputDeviceManager::Destroy(PlatformInputDevice* devicePointer)
{
	Rtt_DELETE(devicePointer);
}


// ----------------------------------------------------------------------------

} // namespace Rtt
