//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_ReadOnlyInputAxisCollection.h"
#include "Rtt_InputAxisDescriptor.h"
#include "Rtt_InputAxisCollection.h"
#include "Rtt_PlatformInputAxis.h"


namespace Rtt
{

// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new read-only collection which wraps the given mutable collection.
/// @param collectionPointer Pointer to the collection that this read-only container will wrap.
ReadOnlyInputAxisCollection::ReadOnlyInputAxisCollection(InputAxisCollection *collectionPointer)
:	fCollectionPointer(collectionPointer)
{
}

/// Destroys this collection.
ReadOnlyInputAxisCollection::~ReadOnlyInputAxisCollection()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Gets the allocator this collection is using.
/// @return Returns a pointer to the allocator this collection is using.
Rtt_Allocator* ReadOnlyInputAxisCollection::GetAllocator() const
{
	if (NULL == fCollectionPointer)
	{
		return NULL;
	}
	return fCollectionPointer->GetAllocator();
}

/// Gets the number of input axis pointers stored in this collection.
/// @return Returns the number of input axes stored in this collection.
///         <br>
///         Will return zero if this collection is empty.
S32 ReadOnlyInputAxisCollection::GetCount() const
{
	if (NULL == fCollectionPointer)
	{
		return 0;
	}
	return fCollectionPointer->GetCount();
}

/// Fetches an input axis from this collection by its descriptor.
/// @param descriptor A descriptor which uniquely identifies a device's axis input, such as "Joystick 1: Axis 1".
/// @return Returns a pointer to the input axis, if found.
///         <br>
///         Returns NULL if an input axis matching the given descriptor was not found.
PlatformInputAxis* ReadOnlyInputAxisCollection::GetBy(const InputAxisDescriptor &descriptor) const
{
	if (NULL == fCollectionPointer)
	{
		return NULL;
	}
	return fCollectionPointer->GetBy(descriptor);
}

/// Fetches an input axis from this collection by its zero based index.
/// @param index Zero based index to the input axis in the collection. Must be less than GetCount().
/// @return Returns a pointer to the indexed input axis.
///         <br>
///         Returns NULL if the given index is out of bounds.
PlatformInputAxis* ReadOnlyInputAxisCollection::GetByIndex(S32 index) const
{
	if (NULL == fCollectionPointer)
	{
		return NULL;
	}
	return fCollectionPointer->GetByIndex(index);
}

/// Fetches an input axis from this collection by the axis number assigned to it by the device.
/// @param number One based number assigned to the input axis.
/// @return Returns a pointer to the input axis having the given axis number.
///         <br>
///         Returns NULL if the given axis number was not found in the collection.
PlatformInputAxis* ReadOnlyInputAxisCollection::GetByAxisNumber(S32 number) const
{
	if (NULL == fCollectionPointer)
	{
		return NULL;
	}
	return fCollectionPointer->GetByAxisNumber(number);
}

/// Fetches an input axis from this collection by its unique descriptor name.
/// @param name The name provided by the InputAxisDescriptor.GetInvariantName() function.
/// @return Returns a pointer to the input axis, if a match was found.
///         <br>
///         Returns NULL if the given descriptor name was not found or if the given argument was NULL.
PlatformInputAxis* ReadOnlyInputAxisCollection::GetByDescriptorName(const char *name) const
{
	if (NULL == fCollectionPointer)
	{
		return NULL;
	}
	return fCollectionPointer->GetByDescriptorName(name);
}

/// Determines if the given input axis exists in this collection by its pointer.
/// @param axisPointer Memory address of the input axis to search for.
/// @return Returns true if the input axis was found.
///         <br>
///         Returns false if not found or if given a NULL argument.
bool ReadOnlyInputAxisCollection::Contains(PlatformInputAxis *axisPointer) const
{
	if (NULL == fCollectionPointer)
	{
		return false;
	}
	return fCollectionPointer->Contains(axisPointer);
}

// ----------------------------------------------------------------------------

} // namespace Rtt
