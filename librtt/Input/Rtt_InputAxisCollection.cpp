//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_InputAxisCollection.h"
#include "Rtt_InputAxisDescriptor.h"
#include "Rtt_PlatformInputAxis.h"


namespace Rtt
{

// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new empty input axis collection.
/// @param allocatorPointer Allocator needed to create this collection. Cannot be NULL.
InputAxisCollection::InputAxisCollection(Rtt_Allocator *allocatorPointer)
:	fAllocatorPointer(allocatorPointer),
	fCollection(allocatorPointer)
{
}

/// Creates a new collection containing a copy of all the objects in the given collection.
/// @param collection The collection to be copied.
InputAxisCollection::InputAxisCollection(const InputAxisCollection &collection)
:	fAllocatorPointer(collection.GetAllocator()),
	fCollection(collection.GetAllocator())
{
	Add(collection);
}

/// Destroys this collection.
InputAxisCollection::~InputAxisCollection()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Adds the given input axis object's pointer to this collection.
/// <br>
/// Will not be added if the object already exists in the collection.
/// This collection will not store duplicate object pointers.
/// @param axisPointer The input axis to be added to the collection.
void InputAxisCollection::Add(PlatformInputAxis *axisPointer)
{
	// Validate.
	if (NULL == axisPointer)
	{
		return;
	}

	// Do not continue if the given pointer already exists in the collection.
	if (Contains(axisPointer))
	{
		return;
	}

	// Add the given object pointer to the collection.
	fCollection.Append(axisPointer);
}

/// Adds all of the input axis object pointers contained in the given collection to this collection.
/// <br>
/// Note that this function will not add duplicate input axis pointers to this collection.
void InputAxisCollection::Add(const InputAxisCollection &collection)
{
	S32 count = collection.GetCount();
	for (S32 index = 0; index < count; index++)
	{
		Add(collection.GetByIndex(index));
	}
}

/// Gets the allocator this collection is using.
/// @return Returns a pointer to the allocator this collection is using.
Rtt_Allocator* InputAxisCollection::GetAllocator() const
{
	return fAllocatorPointer;
}

/// Gets the number of input axis pointers stored in this collection.
/// @return Returns the number of input axes stored in this collection.
///         <br>
///         Will return zero if this collection is empty.
S32 InputAxisCollection::GetCount() const
{
	return fCollection.Length();
}

/// Fetches an input axis from this collection by its descriptor.
/// @param descriptor A descriptor which uniquely identifies a device's axis input, such as "Joystick 1: Axis 1".
/// @return Returns a pointer to the input axis, if found.
///         <br>
///         Returns NULL if an input axis matching the given descriptor was not found.
PlatformInputAxis* InputAxisCollection::GetBy(const InputAxisDescriptor &descriptor) const
{
	PlatformInputAxis* axisPointer;
	S32 count = fCollection.Length();

	for (S32 index = 0; index < count; index++)
	{
		axisPointer = fCollection[index];
		if (axisPointer && axisPointer->GetDescriptor().Equals(descriptor))
		{
			return axisPointer;
		}
	}
	return NULL;
}

/// Fetches an input axis from this collection by its zero based index.
/// @param index Zero based index to the input axis in the collection. Must be less than GetCount().
/// @return Returns a pointer to the indexed input axis.
///         <br>
///         Returns NULL if the given index is out of bounds.
PlatformInputAxis* InputAxisCollection::GetByIndex(S32 index) const
{
	// Validate.
	if ((index < 0) || (index >= GetCount()))
	{
		return NULL;
	}

	// Return the indexed input axis pointer.
	return fCollection[index];
}

/// Fetches an input axis from this collection by the axis number assigned to it by the device.
/// @param number One based number assigned to the input axis.
/// @return Returns a pointer to the input axis having the given axis number.
///         <br>
///         Returns NULL if the given axis number was not found in the collection.
PlatformInputAxis* InputAxisCollection::GetByAxisNumber(S32 number) const
{
	PlatformInputAxis* axisPointer;
	S32 count = fCollection.Length();
	for (S32 index = 0; index < count; index++)
	{
		axisPointer = fCollection[index];
		if (axisPointer && (axisPointer->GetDescriptor().GetAxisNumber() == number))
		{
			return axisPointer;
		}
	}
	return NULL;
}

/// Fetches an input axis from this collection by its unique descriptor name.
/// @param name The name provided by the InputAxisDescriptor.GetInvariantName() function.
/// @return Returns a pointer to the input axis, if a match was found.
///         <br>
///         Returns NULL if the given descriptor name was not found or if the given argument was NULL.
PlatformInputAxis* InputAxisCollection::GetByDescriptorName(const char *name) const
{
	// Validate.
	if (NULL == name)
	{
		return NULL;
	}

	// Attempt to fetch an input axis by the given descriptor name.
	PlatformInputAxis* axisPointer;
	S32 count = fCollection.Length();
	for (S32 index = 0; index < count; index++)
	{
		axisPointer = fCollection[index];
		if (axisPointer)
		{
			const char *nextName = axisPointer->GetDescriptor().GetInvariantName();
			if (nextName && !strcmp(nextName, name))
			{
				return axisPointer;
			}
		}
	}

	// Descriptor name not found.
	return NULL;
}

/// Determines if the given input axis exists in this collection by its pointer.
/// @param axisPointer Memory address of the input axis to search for.
/// @return Returns true if the input axis was found.
///         <br>
///         Returns false if not found or if given a NULL argument.
bool InputAxisCollection::Contains(PlatformInputAxis *axisPointer) const
{
	// Validate.
	if (NULL == axisPointer)
	{
		return false;
	}

	// Find the input axis by its memory address.
	S32 count = fCollection.Length();
	for (S32 index = 0; index < count; index++)
	{
		if (axisPointer == fCollection[index])
		{
			return true;
		}
	}
	return false;
}

/// Removes an input axis from this collection by its pointer.
/// @param axisPointer Memory address of the input axis to search for and remove.
/// @return Returns true if the input axis pointer was successfully found and removed from the collection.
///         <br>
///         Returns false if the input axis pointer was not found or if given a NULL argument.
bool InputAxisCollection::Remove(PlatformInputAxis *axisPointer)
{
	// Validate.
	if (NULL == axisPointer)
	{
		return false;
	}

	// Find the input axis in the collection by its memory address.
	S32 count = fCollection.Length();
	for (S32 index = 0; index < count; index++)
	{
		if (axisPointer == fCollection[index])
		{
			// Input axis found. Remove it.
			fCollection.Remove(index, 1, false);
			return true;
		}
	}

	// Input axis pointer not found.
	return false;
}

/// Removes an input axis from this collection by its descriptor.
/// @param descriptor A descriptor which uniquely identifies a device's input axis, such as "Joystick 1: Axis 1".
/// @return Returns true if the input axis matching the given descriptor was found and removed.
///         <br>
///         Returns false if the given descriptor was not found in the collection.
bool InputAxisCollection::RemoveBy(const InputAxisDescriptor &descriptor)
{
	PlatformInputAxis *axisPointer;
	bool wasRemoved = false;

	for (S32 index = fCollection.Length() - 1; index >= 0; index--)
	{
		axisPointer = fCollection[index];
		if (axisPointer && axisPointer->GetDescriptor().Equals(descriptor))
		{
			fCollection.Remove(index, 1, false);
			wasRemoved = true;
		}
	}
	return wasRemoved;
}

/// Removes an input axis from the collection by its zero based index.
/// @param index Zero based index to the input axis in the collection.
/// @return Returns true if the indexed input axis was removed successfully.
///         <br>
///         Returns false if given an index that is out of bounds.
bool InputAxisCollection::RemoveByIndex(S32 index)
{
	// Validate.
	if ((index < 0) || (index >= GetCount()))
	{
		return false;
	}

	// Remove the indexed input axis from the collection.
	fCollection.Remove(index, 1, false);
	return true;
}

/// Removes an input axis from the collection by its assigned axis number.
/// @param number One based number assigned to the input axis.
/// @return Returns true if an input axis having the given axis number was found and removed.
///         <br>
///         Returns false if the given axis number was not found in the collection.
bool InputAxisCollection::RemoveByAxisNumber(S32 number)
{
	PlatformInputAxis *axisPointer;
	bool wasRemoved = false;

	for (S32 index = fCollection.Length() - 1; index >= 0; index--)
	{
		axisPointer = fCollection[index];
		if (axisPointer && (axisPointer->GetDescriptor().GetAxisNumber() == number))
		{
			fCollection.Remove(index, 1, false);
			wasRemoved = true;
		}
	}
	return wasRemoved;
}

/// Removes an input axis from this collection by its unique descriptor name.
/// @param name The name provided by the InputAxisDescriptor.GetInvariantName() function.
/// @return Returns true if an input axis having the given descriptor name was found and removed.
///         <br>
///         Returns false if the given descriptor name was not found in the collection.
bool InputAxisCollection::RemoveByDescriptorName(const char *name)
{
	PlatformInputAxis *axisPointer;
	bool wasRemoved = false;

	// Validate.
	if (name == NULL)
	{
		return false;
	}

	// Remove input axes from the collection matching the given descriptor name.
	for (S32 index = fCollection.Length() - 1; index >= 0; index--)
	{
		axisPointer = fCollection[index];
		if (axisPointer)
		{
			const char *nextName = axisPointer->GetDescriptor().GetInvariantName();
			if (nextName && !strcmp(nextName, name))
			{
				fCollection.Remove(index, 1, false);
				wasRemoved = true;
			}
		}
	}
	return wasRemoved;
}

/// Removes all input axes from this collection.
void InputAxisCollection::Clear()
{
	fCollection.Empty();
}

/// Clears this collection and copies the input axis pointers from the given collection to this collection.
/// @param collection The collection to copy input axis pointers from.
void InputAxisCollection::operator=(const InputAxisCollection &collection)
{
	Clear();
	Add(collection);
}


// ----------------------------------------------------------------------------

} // namespace Rtt
