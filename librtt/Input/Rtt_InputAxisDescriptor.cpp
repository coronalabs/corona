//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_InputAxisDescriptor.h"
#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include <cstdio>


namespace Rtt
{

// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a descriptor used to identify a specific axis input belonging to one input device.
/// Intended to be used to generate a unique human readable invariant name for one axis input.
/// @param deviceDescriptor Descriptor for the input device that the axis input belongs to.
///                         This descriptor's invariant name will be prefixed to the axis' invariant name.
/// @param axisNumber The number assigned to the axis input that this descriptor will be referencing.
///                   This number will be postfixed to the descriptor's invariant name.
InputAxisDescriptor::InputAxisDescriptor(const InputDeviceDescriptor &deviceDescriptor, S32 axisNumber)
:	fInvariantNamePointer(NULL),			// Pointer made NULL on purpose. Will be created on demand.
	fDeviceDescriptor(deviceDescriptor),
	fAxisNumber(axisNumber)
{
}

/// Creates a copy of the given input axis descriptor.
/// @param descriptor The input axis descriptor to be copied.
InputAxisDescriptor::InputAxisDescriptor(const InputAxisDescriptor &descriptor)
:	fInvariantNamePointer(NULL),			// Pointer made NULL on purpose. Will be created on demand.
	fDeviceDescriptor(descriptor.fDeviceDescriptor),
	fAxisNumber(descriptor.fAxisNumber)
{
}

/// Destroys this descriptor object.
InputAxisDescriptor::~InputAxisDescriptor()
{
	if (fInvariantNamePointer)
	{
		Rtt_DELETE(fInvariantNamePointer);
	}
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Gets the allocator this object uses to create its invariant name.
/// @return Returns a pointer to the allocator.
///         <br>
///         Returns NULL if this descriptor was not assigned an allocator.
Rtt_Allocator* InputAxisDescriptor::GetAllocator() const
{
	return fDeviceDescriptor.GetAllocator();
}

/// Gets the descriptor for the device this axis input belongs to.
/// This descriptor's invariant name is prefixed to this descriptor's invariant name.
/// @return Returns the device descriptor this axis input descriptor is associated with.
const InputDeviceDescriptor& InputAxisDescriptor::GetDeviceDescriptor() const
{
	return fDeviceDescriptor;
}

/// Gets the input axis' assigned number.
/// This number is postfixed to this descriptor's invariant name.
/// @return Returns the axis number assigned to this descriptor.
S32 InputAxisDescriptor::GetAxisNumber() const
{
	return fAxisNumber;
}

/// Gets this descriptor's unique human readable name.
/// @return Returns a unique human readable name used to identify a specific axis on one input device
///         such as "Joystick 1: Axis 1", "Joystick 1: Axis 2", etc.
///         <br>
///         Returns NULL if this descriptor does not have access to an allocator.
const char* InputAxisDescriptor::GetInvariantName() const
{
	// Create the invariant name, if not already done.
	if (NULL == fInvariantNamePointer)
	{
		Rtt_Allocator *allocatorPointer = GetAllocator();
		if (NULL == allocatorPointer)
		{
			return NULL;
		}
		const size_t MAX_CHARACTERS = 32;
		fInvariantNamePointer = Rtt_NEW(allocatorPointer, String(allocatorPointer));
		fInvariantNamePointer->Reserve(MAX_CHARACTERS);
		snprintf(fInvariantNamePointer->GetMutableString(), MAX_CHARACTERS, "%s: Axis %d",
					fDeviceDescriptor.GetInvariantName(), fAxisNumber);
	}

	// Return the name.
	return fInvariantNamePointer->GetString();
}

/// Determines if this descriptor matches the given descriptor.
/// @param descriptor Reference to the descriptor to compare against.
/// @return Returns true if this descriptor references the same device and axis input as the given descriptor.
///         <br>
///         Returns false if this descriptor references a different device or axis input.
bool InputAxisDescriptor::Equals(const InputAxisDescriptor &descriptor) const
{
	return ((fDeviceDescriptor == descriptor.fDeviceDescriptor) && (fAxisNumber == descriptor.fAxisNumber));
}

/// Determines if this descriptor matches the given descriptor.
/// @param descriptor Reference to the descriptor to compare against.
/// @return Returns true if this descriptor references the same device and axis input as the given descriptor.
///         <br>
///         Returns false if this descriptor references a different device or axis input.
bool InputAxisDescriptor::operator==(const InputAxisDescriptor &descriptor) const
{
	return Equals(descriptor);
}

/// Determines if this descriptor does not match the given descriptor.
/// @param descriptor Reference to the descriptor to compare against.
/// @return Returns true if this descriptor does not reference the same device or axis the given descriptor does.
///         <br>
///         Returns false if both descriptors referene the same device and axis input.
bool InputAxisDescriptor::NotEquals(const InputAxisDescriptor &descriptor) const
{
	return !Equals(descriptor);
}

/// Determines if this descriptor does not match the given descriptor.
/// @param descriptor Reference to the descriptor to compare against.
/// @return Returns true if this descriptor does not reference the same device or axis the given descriptor does.
///         <br>
///         Returns false if both descriptors referene the same device and axis input.
bool InputAxisDescriptor::operator!=(const InputAxisDescriptor &descriptor) const
{
	return !Equals(descriptor);
}

// ----------------------------------------------------------------------------

} // namespace Rtt
