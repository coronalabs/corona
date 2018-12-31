//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Array.h"
#include "Core/Rtt_String.h"
#include "Rtt_InputDeviceType.h"


namespace Rtt
{

// ----------------------------------------------------------------------------
// Predefined Input Device Types
// ----------------------------------------------------------------------------

/// Indicates that the device is of an unknown type.
const InputDeviceType InputDeviceType::kUnknown(0, "unknown", "Unknown Device");

/// Indicates that the device is a keyboard and will provide key events.
const InputDeviceType InputDeviceType::kKeyboard(1, "keyboard", "Keyboard");

/// Indicates that the device is a mouse and will provide mouse, touch, and axis events.
const InputDeviceType InputDeviceType::kMouse(2, "mouse", "Mouse");

/// Indicates that the device is a stylus and will provide touch and axis events.
const InputDeviceType InputDeviceType::kStylus(3, "stylus", "Stylus");

/// Indicates that the device is a trackball and will provide mouse, touch, and axis events.
const InputDeviceType InputDeviceType::kTrackball(4, "trackball", "Trackball");

/// Indicates that the device is a touchpad will provide axis events.
/// <br>
/// Note that touch coordinates may not be in screen coordinates.
/// An example of this kind of device is the touch surface at the top of an Apple Magic Mouse.
const InputDeviceType InputDeviceType::kTouchpad(5, "touchpad", "Touchpad");

/// Indicates that the device is a touchscreen and will provide touch and axis events.
const InputDeviceType InputDeviceType::kTouchscreen(6, "touchscreen", "Touchscreen");

/// Indicates that the device is a joystick or a gamepad with analog sticks.
/// Typically provides key and axis events.
const InputDeviceType InputDeviceType::kJoystick(7, "joystick", "Joystick");

/// Indicates that the device is a gamepad and will provide key events.
const InputDeviceType InputDeviceType::kGamepad(8, "gamepad", "Gamepad");

/// Indicates that the device is a d-pad and will provide up, down, left, and right key events.
const InputDeviceType InputDeviceType::kDirectionalPad(9, "directionalPad", "Directional Pad");

/// Indicates that the device provides a wheel, gas pedal, and brake pedal axis events.
const InputDeviceType InputDeviceType::kSteeringWheel(10, "steeringWheel", "Steering Wheel");

/// Indicates that the device has a joystick that likely has a rudder, throttle, and hat switch inputs.
const InputDeviceType InputDeviceType::kFlightStick(11, "flightStick", "Flight Stick");

/// Indicates that the device is guitar device (think "Guitar Hero" like device).
const InputDeviceType InputDeviceType::kGuitar(12, "guitar", "Guitar");

/// Indicates that the device is a drum set (think "Rock Band") which provides key events for each drum.
const InputDeviceType InputDeviceType::kDrumSet(13, "drumSet", "Drum Set");

/// Indicates tha the device is a dance pad, which typically only provides key events.
const InputDeviceType InputDeviceType::kDancePad(14, "dancePad", "Dance Pad");


// ----------------------------------------------------------------------------
// Internal Static Collection
// ----------------------------------------------------------------------------

// Collection of input device types.
typedef LightPtrArray<InputDeviceType> InputDeviceTypeCollection;

/// Gets a collection of all predefined input device types.
/// This collection is expected to be populated by this class' private constructor.
/// @return Returns a reference to this class' mutable collection of all known device types.
static InputDeviceTypeCollection& GetCollection()
{
	static InputDeviceTypeCollection sCollection(Rtt_AllocatorCreate());
	return sCollection;
}


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new input device type defaulted to the kUnknown type.
InputDeviceType::InputDeviceType()
:	fIntegerId(kUnknown.fIntegerId),
	fStringId(kUnknown.fStringId),
	fInvariantName(kUnknown.fInvariantName)
{
}

/// Creates a new input device type object.
/// <br>
/// This is a private constructor that is used to create this class' preallocated device type objects.
/// @param integerId The unique integer ID assigned to this device type.
/// @param stringId The unique string ID assigned to this device type.
/// @param invariantName Human readable string ID that is used by the device's descriptor.
InputDeviceType::InputDeviceType(S32 integerId, const char *stringId, const char *invariantName)
:	fIntegerId(integerId),
	fStringId(stringId),
	fInvariantName(invariantName)
{
	GetCollection().Append(this);
}

/// Destroys this device type.
InputDeviceType::~InputDeviceType()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Gets the unique integer ID assigned to this device type.
/// @return Returns a unique integer ID.
S32 InputDeviceType::GetIntegerId() const
{
	return fIntegerId;
}

/// Gets the unique string ID assigned to this device type such as "keyboard", "mouse", "joystick", etc.
/// This string ID is typically used in Lua.
/// @return Returns this device unique string ID.
const char* InputDeviceType::GetStringId() const
{
	return fStringId;
}

/// Get the unique human readable name for this device type such as "Keyboard", "Unknown Device", etc.
/// This string is used to create the device's descriptor string.
/// @return Returns the device's human readable string ID.
const char* InputDeviceType::GetInvariantName() const
{
	return fInvariantName;
}

/// Determines if this device type matches the given device type.
/// @param value Reference to the input device type to compare against.
/// @return Returns true if the device types match. Returns false if not.
bool InputDeviceType::Equals(const InputDeviceType &value) const
{
	return (fStringId == value.fStringId);
}

/// Determines if this device type matches the given device type.
/// @param value Reference to the input device type to compare against.
/// @return Returns true if the device types match. Returns false if not.
bool InputDeviceType::operator==(const InputDeviceType &value) const
{
	return Equals(value);
}

/// Determines if this device type does not match the given device type.
/// @param value Reference to the input device type to compare against.
/// @return Returns true if the device types do not match. Returns false if they do.
bool InputDeviceType::NotEquals(const InputDeviceType &value) const
{
	return !Equals(value);
}

/// Determines if this device type does not match the given device type.
/// @param value Reference to the input device type to compare against.
/// @return Returns true if the device types do not match. Returns false if they do.
bool InputDeviceType::operator!=(const InputDeviceType &value) const
{
	return !Equals(value);
}


// ----------------------------------------------------------------------------
// Public Static Functions
// ----------------------------------------------------------------------------

/// Fetches a device type matching the given ID.
/// @param allocatorPointer Allocator needed to create the device type result.
/// @param id The unique integer ID of the device type.
///           Matches the value returned by the InputDeviceType.GetIntegerId() function.
/// @return Returns a result object indicating if a device type having the given ID was found.
///         The result object's HasSucceeded() function will return true if a match was found,
///         in which case, the result object's GetDeviceType() function will return the matching type.
InputDeviceType::ConversionResult InputDeviceType::FromIntegerId(Rtt_Allocator *allocatorPointer, S32 id)
{
	InputDeviceTypeCollection &collection = GetCollection();
	InputDeviceType *typePointer;

	for (int index = collection.Length() - 1; index >= 0; index--)
	{
		typePointer = collection[index];
		if (typePointer && (id == typePointer->fIntegerId))
		{
			return InputDeviceType::ConversionResult(allocatorPointer, *typePointer);
		}
	}
	return InputDeviceType::ConversionResult();
}

/// Fetches a device type matching the given ID.
/// @param allocatorPointer Allocator needed to create the device type result.
/// @param stringId The unique string ID of the device type.
///                 Matches the string returned by the InputDeviceType.GetStringId() function.
/// @return Returns a result object indicating if a device type having the given ID was found.
///         The result object's HasSucceeded() function will return true if a match was found,
///         in which case, the result object's GetDeviceType() function will return the matching type.
InputDeviceType::ConversionResult InputDeviceType::FromStringId(
	Rtt_Allocator *allocatorPointer, const char *stringId)
{
	InputDeviceTypeCollection &collection = GetCollection();
	InputDeviceType *typePointer;

	if (Rtt_StringIsEmpty(stringId) == false)
	{
		for (int index = collection.Length() - 1; index >= 0; index--)
		{
			typePointer = collection[index];
			if (typePointer && !strcmp(typePointer->GetStringId(), stringId))
			{
				return InputDeviceType::ConversionResult(allocatorPointer, *typePointer);
			}
		}
	}
	return InputDeviceType::ConversionResult();
}

/// Fetches a device type matching the given invariant name.
/// @param allocatorPointer Allocator needed to create the device type result.
/// @param name The device type's unique human readable name.
///             Matches the string returned by the InputDeviceType.GetInvariantName() function.
/// @return Returns a result object indicating if a device type having the given name was found.
///         The result object's HasSucceeded() function will return true if a match was found,
///         in which case, the result object's GetDeviceType() function will return the matching type.
InputDeviceType::ConversionResult InputDeviceType::FromInvariantName(
	Rtt_Allocator *allocatorPointer, const char *name)
{
	InputDeviceTypeCollection &collection = GetCollection();
	InputDeviceType *typePointer;

	if (Rtt_StringIsEmpty(name) == false)
	{
		for (int index = collection.Length() - 1; index >= 0; index--)
		{
			typePointer = collection[index];
			if (typePointer && !strcmp(typePointer->GetInvariantName(), name))
			{
				return InputDeviceType::ConversionResult(allocatorPointer, *typePointer);
			}
		}
	}
	return InputDeviceType::ConversionResult();
}


// ----------------------------------------------------------------------------
// ConversionResult Class Functions
// ----------------------------------------------------------------------------

/// Creates a failure result object.
InputDeviceType::ConversionResult::ConversionResult()
{
}

/// Creates a "success" result object with the given device type.
/// @param allocatorPointer Allocator needed to create the device type.
/// @param type The device type that was created by the conversion function.
InputDeviceType::ConversionResult::ConversionResult(Rtt_Allocator *allocatorPointer, const InputDeviceType &type)
:	fDeviceTypeReference(allocatorPointer, Rtt_NEW(allocatorPointer, InputDeviceType(type)))
{
}

/// Determines if conversion was successful and a device type was retrieved.
/// @return Returns true if the conversion was successful.
///         This means that the GetDeviceType() function will return the result.
///         <br>
///         Returns false if conversion failed.
bool InputDeviceType::ConversionResult::HasSucceeded() const
{
	return fDeviceTypeReference.NotNull();
}

/// Determines if conversion failed.
/// @return Returns true if conversion failed, which means that the GetDeviceType() function will return NULL.
///         <br>
///         Returns false if conversion was successful.
bool InputDeviceType::ConversionResult::HasFailed() const
{
	return fDeviceTypeReference.IsNull();
}

/// Fetches the device type that was created by the conversion function.
/// @return Returns a pointer to the descriptor that was created.
///         <br>
///         Returns NULL if conversion failed.
const InputDeviceType* InputDeviceType::ConversionResult::GetDeviceType() const
{
	return &(*fDeviceTypeReference);
}


// ----------------------------------------------------------------------------

} // namespace Rtt
