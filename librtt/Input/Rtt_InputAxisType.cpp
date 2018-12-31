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
#include "Rtt_InputAxisType.h"


namespace Rtt
{

// ----------------------------------------------------------------------------
// Predefined Input Axis Types
// ----------------------------------------------------------------------------

/// Indicates that the axis type is unknown.
const InputAxisType InputAxisType::kUnknown(0, "unknown");

/// Indicates that the device provides x-axis input, such as from a joystick.
const InputAxisType InputAxisType::kX(1, "x");

/// Indicates that the device provides y-axis input, such as from a joystick.
const InputAxisType InputAxisType::kY(2, "y");

/// Indicates that the device provides z-axis input, such as from a joystick.
const InputAxisType InputAxisType::kZ(3, "z");

/// Indicates that the device provides rotation input around the x-axis.
const InputAxisType InputAxisType::kRotationX(4, "rotationX");

/// Indicates that the device provides rotation input around the y-axis.
const InputAxisType InputAxisType::kRotationY(5, "rotationY");

/// Indicates that the device provides rotation input around the z-axis.
const InputAxisType InputAxisType::kRotationZ(6, "rotationZ");

/// Indicates that the device provides x-axis input from the left thumbstick on a gamepad.
const InputAxisType InputAxisType::kLeftX(7, "leftX");

/// Indicates that the device provides y-axis input from the left thumbstick on a gamepad.
const InputAxisType InputAxisType::kLeftY(8, "leftY");

/// Indicates that the device provides x-axis input from the right thumbstick on a gamepad.
const InputAxisType InputAxisType::kRightX(9, "rightX");

/// Indicates that the device provides y-axis input from the right thumbstick on a gamepad.
const InputAxisType InputAxisType::kRightY(10, "rightY");

/// Indicates that the device provides x-axis input from a joystick's hat switch.
const InputAxisType InputAxisType::kHatX(11, "hatX");

/// Indicates that the device provides y-axis input from a joystick's hat switch.
const InputAxisType InputAxisType::kHatY(12, "hatY");

/// Indicates that the device provides left trigger input from a gamepad.
const InputAxisType InputAxisType::kLeftTrigger(13, "leftTrigger");

/// Indicates that the device provides right trigger input from a gamepad.
const InputAxisType InputAxisType::kRightTrigger(14, "rightTrigger");

/// Indicates that the device provides gas pedal input or right trigger input from a gamepad.
const InputAxisType InputAxisType::kGas(15, "gas");

/// Indicates that the device provides brake pedal input or left trigger input from a gamepad.
const InputAxisType InputAxisType::kBrake(16, "brake");

/// Indicates that the device provides steering wheel input.
const InputAxisType InputAxisType::kWheel(17, "wheel");

/// Indicates that the device provides rudder input.
const InputAxisType InputAxisType::kRudder(18, "rudder");

/// Indicates that the device provides throttle or slider input.
const InputAxisType InputAxisType::kThrottle(19, "throttle");

/// Indicates that the device provides vertical scroll input, such as from a mouse's scroll wheel.
const InputAxisType InputAxisType::kVerticalScroll(20, "verticalScroll");

/// Indicates that the device provides horizontal scroll input, such as from a mouse's scroll wheel.
const InputAxisType InputAxisType::kHorizontalScroll(21, "horizontalScroll");

/// Indicates that the device provides the direction of a stylus or finger relative to a surface.
const InputAxisType InputAxisType::kOrientation(22, "orientation");

/// Indicates that the device provides the distance that a stylus or finger is from the screen.
const InputAxisType InputAxisType::kHoverDistance(23, "hoverDistance");

/// Indicates that the device provides the length of the major axis of the stylus or finger
/// that is above the touchscreen or touchpad.
const InputAxisType InputAxisType::kHoverMajor(24, "hoverMajor");

/// Indicates that the device provides the length of the minor axis of the stylus or finger
/// that is above the touchscreen or touchpad.
const InputAxisType InputAxisType::kHoverMinor(25, "hoverMinor");

/// Indicates that the device provides the surface area that is in contact of the touchscreen or touchpad.
const InputAxisType InputAxisType::kTouchSize(26, "touchSize");

/// Indicates that the device provides the length of the major axis of the stylus or finger
/// that is in contact with the touchscreen or touchpad.
const InputAxisType InputAxisType::kTouchMajor(27, "touchMajor");

/// Indicates that the device provides the length of the minor axis of the stylus or finger
/// that is in contact with the touchscreen or touchpad.
const InputAxisType InputAxisType::kTouchMinor(28, "touchMinor");

/// Indicates that the device provides the amount of pressure applied to the touchscreen or touchpad.
/// Can also be used by a mouse or trackball to indicate that a button was pressed.
const InputAxisType InputAxisType::kPressure(29, "pressure");

/// Indicates that the device provides the angle that a stylus is being held in relative to a surface.
const InputAxisType InputAxisType::kTilt(30, "tilt");

/// Generic axis input 1 belonging to a device.
const InputAxisType InputAxisType::kGeneric1(31, "generic1");

/// Generic axis input 2 belonging to a device.
const InputAxisType InputAxisType::kGeneric2(32, "generic2");

/// Generic axis input 3 belonging to a device.
const InputAxisType InputAxisType::kGeneric3(33, "generic3");

/// Generic axis input 4 belonging to a device.
const InputAxisType InputAxisType::kGeneric4(34, "generic4");

/// Generic axis input 5 belonging to a device.
const InputAxisType InputAxisType::kGeneric5(35, "generic5");

/// Generic axis input 6 belonging to a device.
const InputAxisType InputAxisType::kGeneric6(36, "generic6");

/// Generic axis input 7 belonging to a device.
const InputAxisType InputAxisType::kGeneric7(37, "generic7");

/// Generic axis input 8 belonging to a device.
const InputAxisType InputAxisType::kGeneric8(38, "generic8");

/// Generic axis input 9 belonging to a device.
const InputAxisType InputAxisType::kGeneric9(39, "generic9");

/// Generic axis input 10 belonging to a device.
const InputAxisType InputAxisType::kGeneric10(40, "generic10");

/// Generic axis input 11 belonging to a device.
const InputAxisType InputAxisType::kGeneric11(41, "generic11");

/// Generic axis input 12 belonging to a device.
const InputAxisType InputAxisType::kGeneric12(42, "generic12");

/// Generic axis input 13 belonging to a device.
const InputAxisType InputAxisType::kGeneric13(43, "generic13");

/// Generic axis input 14 belonging to a device.
const InputAxisType InputAxisType::kGeneric14(44, "generic14");

/// Generic axis input 15 belonging to a device.
const InputAxisType InputAxisType::kGeneric15(45, "generic15");

/// Generic axis input 16 belonging to a device.
const InputAxisType InputAxisType::kGeneric16(46, "generic16");

/// Hatswitch reporting position of hat in degrees.
/// Raw values on PS4 controller are from 0 to 360. 360 indicates neutral position, and 0 - top, 90 - right etc
const InputAxisType InputAxisType::kHatSwitch(47, "hatSwitch");

/// Indicates that a guitar device provides the angle of a whammy bar.
const InputAxisType InputAxisType::kWhammyBar(48, "whammyBar");


// ----------------------------------------------------------------------------
// Internal Static Collection
// ----------------------------------------------------------------------------

// Collection of input axis types.
typedef LightPtrArray<InputAxisType> InputAxisTypeCollection;

/// Gets a collection of all predefined input axis types.
/// This collection is expected to be populated by this class' private constructor.
/// @return Returns a reference to this class' mutable collection of all known axis types.
static InputAxisTypeCollection& GetCollection()
{
	static InputAxisTypeCollection sCollection(Rtt_AllocatorCreate());
	return sCollection;
}


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new input axis type defaulted to the kUnknown type.
InputAxisType::InputAxisType()
:	fIntegerId(kUnknown.fIntegerId),
	fStringId(kUnknown.fStringId)
{
}

/// Creates a new input axis type object.
/// <br>
/// This is a private constructor that is used to create this class' preallocated axis type objects.
/// @param integerId The unique integer ID assigned to this axis type.
/// @param stringId The unique string ID assigned to this axis type.
InputAxisType::InputAxisType(S32 integerId, const char *stringId)
:	fIntegerId(integerId),
	fStringId(stringId)
{
	GetCollection().Append(this);
}

/// Destroys this axis type.
InputAxisType::~InputAxisType()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Gets the unique integer ID assigned to this axis type.
/// @return Returns a unique integer ID.
S32 InputAxisType::GetIntegerId() const
{
	return fIntegerId;
}

/// Gets the unique string ID assigned to this axis type such as "x", "y", "leftTrigger", etc.
/// This string ID is typically used in Lua.
/// @return Returns this axis' unique string ID.
const char* InputAxisType::GetStringId() const
{
	return fStringId;
}

/// Determines if this axis type matches the given axis type.
/// @param value Reference to the input axis type to compare against.
/// @return Returns true if the axis types match. Returns false if not.
bool InputAxisType::Equals(const InputAxisType &value) const
{
	return (fStringId == value.fStringId);
}

/// Determines if this axis type matches the given axis type.
/// @param value Reference to the input axis type to compare against.
/// @return Returns true if the axis types match. Returns false if not.
bool InputAxisType::operator==(const InputAxisType &value) const
{
	return Equals(value);
}

/// Determines if this axis type does not match the given axis type.
/// @param value Reference to the input axis type to compare against.
/// @return Returns true if the axis types do not match. Returns false if they do.
bool InputAxisType::NotEquals(const InputAxisType &value) const
{
	return !Equals(value);
}

/// Determines if this axis type does not match the given axis type.
/// @param value Reference to the input axis type to compare against.
/// @return Returns true if the axis types do not match. Returns false if they do.
bool InputAxisType::operator!=(const InputAxisType &value) const
{
	return !Equals(value);
}


// ----------------------------------------------------------------------------
// Public Static Functions
// ----------------------------------------------------------------------------

/// Fetches an axis type matching the given ID.
/// @param allocatorPointer Allocator needed to create the axis type result.
/// @param id The unique integer ID of the axis type.
///           Matches the value returned by the InputAxisType.GetIntegerId() function.
/// @return Returns a result object indicating if an axis type having the given ID was found.
///         The result object's HasSucceeded() function will return true if a match was found,
///         in which case, the result object's GetAxisType() function will return the matching type.
InputAxisType::ConversionResult InputAxisType::FromIntegerId(Rtt_Allocator *allocatorPointer, S32 id)
{
	InputAxisTypeCollection &collection = GetCollection();
	InputAxisType *typePointer;

	for (int index = collection.Length() - 1; index >= 0; index--)
	{
		typePointer = collection[index];
		if (typePointer && (id == typePointer->fIntegerId))
		{
			return InputAxisType::ConversionResult(allocatorPointer, *typePointer);
		}
	}
	return InputAxisType::ConversionResult();
}

/// Fetches an axis type matching the given ID.
/// @param allocatorPointer Allocator needed to create the axis type result.
/// @param stringId The unique string ID of the axis type.
///                 Matches the string returned by the InputAxisType.GetStringId() function.
/// @return Returns a result object indicating if an axis type having the given ID was found.
///         The result object's HasSucceeded() function will return true if a match was found,
///         in which case, the result object's GetAxisType() function will return the matching type.
InputAxisType::ConversionResult InputAxisType::FromStringId(
	Rtt_Allocator *allocatorPointer, const char *stringId)
{
	InputAxisTypeCollection &collection = GetCollection();
	InputAxisType *typePointer;

	if (Rtt_StringIsEmpty(stringId) == false)
	{
		for (int index = collection.Length() - 1; index >= 0; index--)
		{
			typePointer = collection[index];
			if (typePointer && !strcmp(typePointer->GetStringId(), stringId))
			{
				return InputAxisType::ConversionResult(allocatorPointer, *typePointer);
			}
		}
	}
	return InputAxisType::ConversionResult();
}


// ----------------------------------------------------------------------------
// ConversionResult Class Functions
// ----------------------------------------------------------------------------

/// Creates a failure result object.
InputAxisType::ConversionResult::ConversionResult()
{
}

/// Creates a "success" result object with the given axis type.
/// @param allocatorPointer Allocator needed to create the axis type.
/// @param type The axis type that was created by the conversion function.
InputAxisType::ConversionResult::ConversionResult(Rtt_Allocator *allocatorPointer, const InputAxisType &type)
:	fAxisTypeReference(allocatorPointer, Rtt_NEW(allocatorPointer, InputAxisType(type)))
{
}

/// Determines if conversion was successful and an axis type was retrieved.
/// @return Returns true if the conversion was successful.
///         This means that the GetAxisType() function will return the result.
///         <br>
///         Returns false if conversion failed.
bool InputAxisType::ConversionResult::HasSucceeded() const
{
	return fAxisTypeReference.NotNull();
}

/// Determines if conversion failed.
/// @return Returns true if conversion failed, which means that the GetAxisType() function will return NULL.
///         <br>
///         Returns false if conversion was successful.
bool InputAxisType::ConversionResult::HasFailed() const
{
	return fAxisTypeReference.IsNull();
}

/// Fetches the axis type that was created by the conversion function.
/// @return Returns a pointer to the axis type that was created.
///         <br>
///         Returns NULL if conversion failed.
const InputAxisType* InputAxisType::ConversionResult::GetAxisType() const
{
	return &(*fAxisTypeReference);
}


// ----------------------------------------------------------------------------

} // namespace Rtt
