//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_InputDeviceCollection.h"
#include "Rtt_InputDeviceConnectionState.h"
#include "Rtt_InputDeviceDescriptor.h"


namespace Rtt
{

// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new empty input device collection.
/// @param allocatorPointer Allocator needed to create this collection. Cannot be NULL.
InputDeviceCollection::InputDeviceCollection(Rtt_Allocator *allocatorPointer)
:	fAllocatorPointer(allocatorPointer),
	fCollection(allocatorPointer)
{
}

/// Creates a new collection containing a copy of all the objects in the given collection.
/// @param collection The collection to be copied.
InputDeviceCollection::InputDeviceCollection(const InputDeviceCollection &collection)
:	fAllocatorPointer(collection.GetAllocator()),
	fCollection(collection.GetAllocator())
{
	Add(collection);
}

/// Destroys this collection.
InputDeviceCollection::~InputDeviceCollection()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Adds the given device object's pointer to this collection.
/// <br>
/// Will not be added if the object already exists in the collection.
/// This collection will not store duplicate device pointers.
/// @param devicePointer The input device to be added to the collection.
void InputDeviceCollection::Add(PlatformInputDevice *devicePointer)
{
	// Validate.
	if (NULL == devicePointer)
	{
		return;
	}

	// Do not continue if the given device already exists in the collection.
	if (Contains(devicePointer))
	{
		return;
	}

	// Add the given device to the collection.
	fCollection.Append(devicePointer);
}

/// Adds all of the device object pointers contained in the given collection to this collection.
/// <br>
/// Note that this function will not add duplicate device pointers to this collection.
void InputDeviceCollection::Add(const InputDeviceCollection &collection)
{
	S32 count = collection.GetCount();
	for (S32 index = 0; index < count; index++)
	{
		Add(collection.GetByIndex(index));
	}
}

/// Gets the allocator this collection is using.
/// @return Returns a pointer to the allocator this collection is using.
Rtt_Allocator* InputDeviceCollection::GetAllocator() const
{
	return fAllocatorPointer;
}

/// Gets the number of device pointers stored in this collection.
/// @return Returns the number of devices stored in this collection.
///         <br>
///         Will return zero if this collection is empty.
S32 InputDeviceCollection::GetCount() const
{
	return fCollection.Length();
}

/// Fetches a device from this collection by its descriptor.
/// @param descriptor A descriptor which uniquely identifies a device, such as "Joystick 1".
/// @return Returns a pointer to the device, if found.
///         <br>
///         Returns NULL if a device matching the given descriptor was not found.
PlatformInputDevice* InputDeviceCollection::GetBy(const InputDeviceDescriptor &descriptor) const
{
	PlatformInputDevice* devicePointer;
	S32 count = fCollection.Length();

	for (S32 index = 0; index < count; index++)
	{
		devicePointer = fCollection[index];
		if (devicePointer && devicePointer->GetDescriptor().Equals(descriptor))
		{
			return devicePointer;
		}
	}
	return NULL;
}

/// Fetches a device from this collection by its zero based index.
/// @param index Zero based index to the device. Must be less than GetCount().
/// @return Returns a pointer to the indexed device.
///         <br>
///         Returns NULL if the given index is out of bounds.
PlatformInputDevice* InputDeviceCollection::GetByIndex(S32 index) const
{
	// Validate.
	if ((index < 0) || (index >= GetCount()))
	{
		return NULL;
	}

	// Return the indexed device pointer.
	return fCollection[index];
}

/// Fetches a device from this collection by its unique descriptor name.
/// @param name The name provided by the InputDeviceDescriptor.GetInvariantName() function.
/// @return Returns a pointer to the device, if a match was found.
///         <br>
///         Returns NULL if the given descriptor name was not found or if the given argument was NULL.
PlatformInputDevice* InputDeviceCollection::GetByDescriptorName(const char *name) const
{
	// Validate.
	if (NULL == name)
	{
		return NULL;
	}

	// Attempt to fetch a device by the given descriptor name.
	PlatformInputDevice* devicePointer;
	S32 count = fCollection.Length();
	for (S32 index = 0; index < count; index++)
	{
		devicePointer = fCollection[index];
		if (devicePointer)
		{
			const char *nextName = devicePointer->GetDescriptor().GetInvariantName();
			if (nextName && !strcmp(nextName, name))
			{
				return devicePointer;
			}
		}
	}

	// Descriptor name not found.
	return NULL;
}

/// Fetches a device from this collection by its unique descriptor integer ID.
/// @param id Unique integer ID that is returned by the InputDeviceDescriptor.GetIntegerId() function.
/// @return Returns a pointer to the device matching the given descriptor ID.
///         <br>
///         Returns NULL if the given descriptor ID was not found.
PlatformInputDevice* InputDeviceCollection::GetByDescriptorId(S64 id) const
{
	PlatformInputDevice* devicePointer;
	S32 count = fCollection.Length();

	for (S32 index = 0; index < count; index++)
	{
		devicePointer = fCollection[index];
		if (devicePointer && (devicePointer->GetDescriptor().GetIntegerId() == id))
		{
			return devicePointer;
		}
	}
	return NULL;
}

/// Copies devices that are currently connected/available to the given collection.
/// @param collection The collection to copy connected devices to.
/// @return Returns true if at least 1 connected device has been added to the given collection.
///         <br>
///         Returns false if no connected devices were found in this collection or if
///         the connected devices already exist in the given collection.
bool InputDeviceCollection::CopyConnectedDevicesTo(InputDeviceCollection &collection) const
{
	PlatformInputDevice* devicePointer;
	S32 count = fCollection.Length();
	bool hasCopiedItems = false;

	for (S32 index = 0; index < count; index++)
	{
		devicePointer = fCollection[index];
		if (devicePointer && devicePointer->GetConnectionState().IsConnected())
		{
			if (collection.Contains(devicePointer) == false)
			{
				collection.Add(devicePointer);
				hasCopiedItems = true;
			}
		}
	}
	return hasCopiedItems;
}

/// Determines if the given input device exists in this collection by its pointer.
/// @param devicePointer Memory address of the input device to search for.
/// @return Returns true if the device was found.
///         <br>
///         Returns false if not found or if given a NULL argument.
bool InputDeviceCollection::Contains(PlatformInputDevice *devicePointer) const
{
	// Validate.
	if (NULL == devicePointer)
	{
		return false;
	}

	// Find the device by its memory address.
	S32 count = fCollection.Length();
	for (S32 index = 0; index < count; index++)
	{
		if (devicePointer == fCollection[index])
		{
			return true;
		}
	}
	return false;
}

/// Removes a device from this collection by its pointer.
/// @param devicePointer Memory address of the input device to search for and remove.
/// @return Returns true if the device pointer was successfully found and removed from the collection.
///         <br>
///         Returns false if the device pointer was not found or if given a NULL argument.
bool InputDeviceCollection::Remove(PlatformInputDevice *devicePointer)
{
	// Validate.
	if (NULL == devicePointer)
	{
		return false;
	}

	// Find the device in the collection by its memory address.
	S32 count = fCollection.Length();
	for (S32 index = 0; index < count; index++)
	{
		if (devicePointer == fCollection[index])
		{
			// Device found. Remove it.
			fCollection.Remove(index, 1, false);
			return true;
		}
	}

	// Device pointer not found.
	return false;
}

/// Removes a device from this collection by its descriptor.
/// @param descriptor A descriptor which uniquely identifies a device, such as "Joystick 1".
/// @return Returns true if the device matching the given descriptor was found and removed.
///         <br>
///         Returns false if a device having the given descriptor was not found.
bool InputDeviceCollection::RemoveBy(const InputDeviceDescriptor &descriptor)
{
	PlatformInputDevice *devicePointer;
	bool wasRemoved = false;

	for (S32 index = fCollection.Length() - 1; index >= 0; index--)
	{
		devicePointer = fCollection[index];
		if (devicePointer && devicePointer->GetDescriptor().Equals(descriptor))
		{
			fCollection.Remove(index, 1, false);
			wasRemoved = true;
		}
	}
	return wasRemoved;
}

/// Removes a device from the collection by its zero based index.
/// @param index Zero based index to the device in the collection.
/// @return Returns true if the index device was removed successfully.
///         <br>
///         Returns false if given an index that is out of bounds.
bool InputDeviceCollection::RemoveByIndex(S32 index)
{
	// Validate.
	if ((index < 0) || (index >= GetCount()))
	{
		return false;
	}

	// Remove the indexed device from the collection.
	fCollection.Remove(index, 1, false);
	return true;
}

/// Removes a device from this collection by its unique descriptor name.
/// @param name The name provided by the InputDeviceDescriptor.GetInvariantName() function.
/// @return Returns true if a device having the given descriptor name was found and removed.
///         <br>
///         Returns false if the given descriptor name was not found in the collection.
bool InputDeviceCollection::RemoveByDescriptorName(const char *name)
{
	PlatformInputDevice *devicePointer;
	bool wasRemoved = false;

	// Validate.
	if (name == NULL)
	{
		return false;
	}

	// Remove device(s) from the collection matching the given descriptor name.
	for (S32 index = fCollection.Length() - 1; index >= 0; index--)
	{
		devicePointer = fCollection[index];
		if (devicePointer)
		{
			const char *nextName = devicePointer->GetDescriptor().GetInvariantName();
			if (nextName && !strcmp(nextName, name))
			{
				fCollection.Remove(index, 1, false);
				wasRemoved = true;
			}
		}
	}
	return wasRemoved;
}

/// Removes a device from this collection by its unique descriptor integer ID.
/// @param id The unique integer ID returned by the InputDeviceDescriptor.GetIntegerId() function.
/// @return Returns true if the a device having the given ID was successfully removed from the collection.
///         <br>
///         Returns false if the given ID was not found in the collection.
bool InputDeviceCollection::RemoveByDescriptorId(S64 id)
{
	PlatformInputDevice *devicePointer;
	bool wasRemoved = false;

	for (S32 index = fCollection.Length() - 1; index >= 0; index--)
	{
		devicePointer = fCollection[index];
		if (devicePointer && (devicePointer->GetDescriptor().GetIntegerId() == id))
		{
			fCollection.Remove(index, 1, false);
			wasRemoved = true;
		}
	}
	return wasRemoved;
}

/// Removes all devices from this collection.
void InputDeviceCollection::Clear()
{
	fCollection.Empty();
}

/// Clears this collection and copies the device pointers from the given collection to this collection.
/// @param collection The collection to copy device pointers from.
void InputDeviceCollection::operator=(const InputDeviceCollection &collection)
{
	Clear();
	Add(collection);
}


// ----------------------------------------------------------------------------

} // namespace Rtt
