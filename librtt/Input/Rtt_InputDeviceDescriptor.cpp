//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_InputDeviceDescriptor.h"
#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include <cstdio>


namespace Rtt
{

// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a descriptor used to identify a specific input device.
/// Intended to be used generate a unique human readable invariant name for a device.
/// @param allocatorPointer Allocator needed by this object to generate an invariant name.
/// @param deviceType The type of input device that this descriptor will be referencing.
///                   The device type's invariant name will be prefixed to this descriptor's invariant name.
/// @param deviceNumber The number assigned to the device that this descriptor will be referencing.
///                     This number will be postfixed to the descriptor's invariant name.
InputDeviceDescriptor::InputDeviceDescriptor(
	Rtt_Allocator *allocatorPointer, InputDeviceType deviceType, S32 deviceNumber)
:	fAllocatorPointer(allocatorPointer),
	fInvariantNamePointer(NULL),
	fDeviceType(deviceType),
	fDeviceNumber(deviceNumber)
{
}

/// Creates a copy of the given input device descriptor.
/// @param descriptor The device descriptor to be copied.
InputDeviceDescriptor::InputDeviceDescriptor(const InputDeviceDescriptor &descriptor)
:	fAllocatorPointer(descriptor.fAllocatorPointer),
	fInvariantNamePointer(NULL),			// Pointer made NULL on purpose. Will be created on demand.
	fDeviceType(descriptor.fDeviceType),
	fDeviceNumber(descriptor.fDeviceNumber)
{
}

/// Destroys this descriptor object.
InputDeviceDescriptor::~InputDeviceDescriptor()
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
Rtt_Allocator* InputDeviceDescriptor::GetAllocator() const
{
	return fAllocatorPointer;
}

/// Gets the type of input device this descriptor is associated with.
/// @return Returns a device type such as "keyboard", "mouse", "joystick", etc.
InputDeviceType InputDeviceDescriptor::GetDeviceType() const
{
	return fDeviceType;
}

/// Gets the device's assigned number.
/// @return Returns the device number assigned to this descriptor.
S32 InputDeviceDescriptor::GetDeviceNumber() const
{
	return fDeviceNumber;
}

/// Gets this descriptor's unique integer ID.
/// <br>
/// This ID is used internally within Corona and is not exposed to Lua.
/// For example, Corona uses it on Android on the Java side to easily identify descirptors across the JNI bridge.
/// @return Returns this descriptor's unique integer ID.
S64 InputDeviceDescriptor::GetIntegerId() const
{
	// The descriptor's ID is a 64-bit integer made up of 2 parts.
	// |----------------------------|-----------------------------|
	// |  Most Significant 32-Bits  |  Least Significant 32-Bits  |
	// |----------------------------|-----------------------------|
	// |             Device Number  |     Device Type Integer ID  |
	// |----------------------------|-----------------------------|
	S64 descriptorId = (S64)fDeviceNumber;
	descriptorId <<= 32;
	descriptorId |= (S64)fDeviceType.GetIntegerId();
	return descriptorId;
}

/// Gets this descriptor's unique human readable name.
/// @return Returns a unique human readable name used to identify a specific input device
///         such as "Keyboard 1", "Joystick 1", "Joystick 2", etc.
///         <br>
///         Returns NULL if this descriptor was not assigned an allocator.
const char* InputDeviceDescriptor::GetInvariantName() const
{
	// Create the invariant name, if not already done.
	if (NULL == fInvariantNamePointer)
	{
		if (NULL == fAllocatorPointer)
		{
			return NULL;
		}
		const size_t MAX_CHARACTERS = 32;
		fInvariantNamePointer = Rtt_NEW(fAllocatorPointer, String(fAllocatorPointer));
		fInvariantNamePointer->Reserve(MAX_CHARACTERS);
		snprintf(fInvariantNamePointer->GetMutableString(), MAX_CHARACTERS, "%s %d",
					fDeviceType.GetInvariantName(), fDeviceNumber);
	}

	// Return the name.
	return fInvariantNamePointer->GetString();
}

/// Determines if this descriptor matches the given descriptor.
/// @param descriptor Reference to the descriptor to compare against.
/// @return Returns true if this descriptor references the same device as the given descriptor.
///         <br>
///         Returns false if this descriptor references a different device.
bool InputDeviceDescriptor::Equals(const InputDeviceDescriptor &descriptor) const
{
	return ((fDeviceType == descriptor.fDeviceType) && (fDeviceNumber == descriptor.fDeviceNumber));
}

/// Determines if this descriptor matches the given descriptor.
/// @param descriptor Reference to the descriptor to compare against.
/// @return Returns true if this descriptor references the same device as the given descriptor.
///         <br>
///         Returns false if this descriptor references a different device.
bool InputDeviceDescriptor::operator==(const InputDeviceDescriptor &descriptor) const
{
	return Equals(descriptor);
}

/// Determines if this descriptor does not match the given descriptor.
/// @param descriptor Reference to the descriptor to compare against.
/// @return Returns true if this descriptor does not reference the same device the given descriptor does.
///         <br>
///         Returns false if both descriptors referene the same device.
bool InputDeviceDescriptor::NotEquals(const InputDeviceDescriptor &descriptor) const
{
	return !Equals(descriptor);
}

/// Determines if this descriptor does not match the given descriptor.
/// @param descriptor Reference to the descriptor to compare against.
/// @return Returns true if this descriptor does not reference the same device the given descriptor does.
///         <br>
///         Returns false if both descriptors referene the same device.
bool InputDeviceDescriptor::operator!=(const InputDeviceDescriptor &descriptor) const
{
	return !Equals(descriptor);
}


// ----------------------------------------------------------------------------
// Public Static Functions
// ----------------------------------------------------------------------------

/// Creates a descriptor from the given unique descriptor integer ID.
/// @param allocatorPointer Allocator needed to create the descriptor.
/// @param id The descriptor's unique integer ID that is returned by InputDeviceDescriptor.GetIntegerId() function.
/// @return Returns the result providing the created descriptor via its GetDescriptor() function.
///         <br>
///         The result object's HasFailed() function will return true if this function was given an invalid ID.
InputDeviceDescriptor::ConversionResult InputDeviceDescriptor::FromIntegerId(
	Rtt_Allocator *allocatorPointer, S64 id)
{
	// Extract the device type from the given ID.
	S32 deviceTypeIntegerId = (S32)(id & 0xFFFFFFFF);
	InputDeviceType::ConversionResult result;
	result = InputDeviceType::FromIntegerId(allocatorPointer, deviceTypeIntegerId);
	if (result.HasFailed())
	{
		// The device type ID is invalid. Return a failure result.
		return InputDeviceDescriptor::ConversionResult();
	}

	// Extract the device number from the given ID.
	S32 deviceNumber = (S32)(id >> 32);

	// Create a new descriptor and return it via a result object.
	InputDeviceDescriptor descriptor(allocatorPointer, *(result.GetDeviceType()), deviceNumber);
	return InputDeviceDescriptor::ConversionResult(descriptor);
}


// ----------------------------------------------------------------------------
// ConversionResult Class Functions
// ----------------------------------------------------------------------------

/// Creates a failure result object.
InputDeviceDescriptor::ConversionResult::ConversionResult()
{
}

/// Creates a "" result object with the given descriptor.
/// @param descriptor The descriptor that was created by the conversion function.
InputDeviceDescriptor::ConversionResult::ConversionResult(const InputDeviceDescriptor &descriptor)
:	fDescriptorReference(descriptor.GetAllocator(),
	                     Rtt_NEW(descriptor.GetAllocator(), InputDeviceDescriptor(descriptor)))
{
}

/// Determines if conversion was successful and a descriptor was created.
/// @return Returns true if the conversion was successful.
///         This means that the GetDescriptor() function will return the result.
///         <br>
///         Returns false if conversion failed.
bool InputDeviceDescriptor::ConversionResult::HasSucceeded() const
{
	return fDescriptorReference.NotNull();
}

/// Determines if conversion failed.
/// @return Returns true if conversion failed, which means that the GetDescriptor() function will return NULL.
///         <br>
///         Returns false if conversion was successful.
bool InputDeviceDescriptor::ConversionResult::HasFailed() const
{
	return fDescriptorReference.IsNull();
}

/// Fetches the descriptor that was created by the conversion function.
/// @return Returns a pointer to the descriptor that was created.
///         <br>
///         Returns NULL if conversion failed.
const InputDeviceDescriptor* InputDeviceDescriptor::ConversionResult::GetDescriptor() const
{
	return &(*fDescriptorReference);
}


// ----------------------------------------------------------------------------

} // namespace Rtt
