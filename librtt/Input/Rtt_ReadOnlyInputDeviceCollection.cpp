//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_ReadOnlyInputDeviceCollection.h"
#include "Rtt_InputDeviceDescriptor.h"
#include "Rtt_InputDeviceCollection.h"
#include "Rtt_PlatformInputDevice.h"


namespace Rtt
{

// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new read-only collection which wraps the given mutable collection.
/// @param collectionPointer Pointer to the collection that this read-only container will wrap.
ReadOnlyInputDeviceCollection::ReadOnlyInputDeviceCollection(InputDeviceCollection *collectionPointer)
:	fCollectionPointer(collectionPointer)
{
}

/// Destroys this collection.
ReadOnlyInputDeviceCollection::~ReadOnlyInputDeviceCollection()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Gets the allocator this collection is using.
/// @return Returns a pointer to the allocator this collection is using.
Rtt_Allocator* ReadOnlyInputDeviceCollection::GetAllocator() const
{
	if (NULL == fCollectionPointer)
	{
		return NULL;
	}
	return fCollectionPointer->GetAllocator();
}

/// Gets the number of device pointers stored in this collection.
/// @return Returns the number of devices stored in this collection.
///         <br>
///         Will return zero if this collection is empty.
S32 ReadOnlyInputDeviceCollection::GetCount() const
{
	if (NULL == fCollectionPointer)
	{
		return 0;
	}
	return fCollectionPointer->GetCount();
}

/// Fetches a device from this collection by its descriptor.
/// @param descriptor A descriptor which uniquely identifies a device, such as "Joystick 1".
/// @return Returns a pointer to the device, if found.
///         <br>
///         Returns NULL if a device matching the given descriptor was not found.
PlatformInputDevice* ReadOnlyInputDeviceCollection::GetBy(const InputDeviceDescriptor &descriptor) const
{
	if (NULL == fCollectionPointer)
	{
		return NULL;
	}
	return fCollectionPointer->GetBy(descriptor);
}

/// Fetches a device from this collection by its zero based index.
/// @param index Zero based index to the device. Must be less than GetCount().
/// @return Returns a pointer to the indexed device.
///         <br>
///         Returns NULL if the given index is out of bounds.
PlatformInputDevice* ReadOnlyInputDeviceCollection::GetByIndex(S32 index) const
{
	if (NULL == fCollectionPointer)
	{
		return NULL;
	}
	return fCollectionPointer->GetByIndex(index);
}

/// Fetches a device from this collection by its unique descriptor name.
/// @param name The name provided by the InputDeviceDescriptor.GetInvariantName() function.
/// @return Returns a pointer to the device, if a match was found.
///         <br>
///         Returns NULL if the given descriptor name was not found or if the given argument was NULL.
PlatformInputDevice* ReadOnlyInputDeviceCollection::GetByDescriptorName(const char *name) const
{
	if (NULL == fCollectionPointer)
	{
		return NULL;
	}
	return fCollectionPointer->GetByDescriptorName(name);
}

/// Fetches a device from this collection by its unique descriptor integer ID.
/// @param id Unique integer ID that is returned by the InputDeviceDescriptor.GetIntegerId() function.
/// @return Returns a pointer to the device matching the given descriptor ID.
///         <br>
///         Returns NULL if the given descriptor ID was not found.
PlatformInputDevice* ReadOnlyInputDeviceCollection::GetByDescriptorId(S64 id) const
{
	if (NULL == fCollectionPointer)
	{
		return NULL;
	}
	return fCollectionPointer->GetByDescriptorId(id);
}

/// Copies devices that are currently connected/available to the given collection.
/// @param collection The collection to copy connected devices to.
/// @return Returns true if at least 1 connected device has been added to the given collection.
///         <br>
///         Returns false if no connected devices were found in this collection or if
///         the connected devices already exist in the given collection.
bool ReadOnlyInputDeviceCollection::CopyConnectedDevicesTo(InputDeviceCollection &collection) const
{
	if (NULL == fCollectionPointer)
	{
		return false;
	}
	return fCollectionPointer->CopyConnectedDevicesTo(collection);
}

/// Determines if the given input device exists in this collection by its pointer.
/// @param devicePointer Memory address of the input device to search for.
/// @return Returns true if the device was found.
///         <br>
///         Returns false if not found or if given a NULL argument.
bool ReadOnlyInputDeviceCollection::Contains(PlatformInputDevice *devicePointer) const
{
	if (NULL == fCollectionPointer)
	{
		return false;
	}
	return fCollectionPointer->Contains(devicePointer);
}

// ----------------------------------------------------------------------------

} // namespace Rtt
