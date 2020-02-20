//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_PlatformInputAxis.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include <string.h>


namespace Rtt
{

// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new input axis object.
/// @param descriptor Unique descriptor used to identify this input axis.
PlatformInputAxis::PlatformInputAxis(const InputAxisDescriptor &descriptor)
:	fDescriptor(descriptor),
	fMinValue(Rtt_REAL_NEG_1),
	fMaxValue(Rtt_REAL_1),
	fAccuracy(Rtt_REAL_0),
	fIsAbsolute(true)
{
}

/// Destroys this input axis and its resources.
PlatformInputAxis::~PlatformInputAxis()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Gets the descriptor used to uniquely identify this one axis belonging to one input device.
/// Provides information such as device type, device number, axis number, and its unique descriptor name.
/// @return Returns this axis' descriptor by reference.
InputAxisDescriptor& PlatformInputAxis::GetDescriptor()
{
	return fDescriptor;
}

/// Gets the type of axis such as X, Y, leftTrigger, rightTrigger, etc.
/// @return Returns the type of axis input this is.
InputAxisType PlatformInputAxis::GetType() const
{
	return fAxisType;
}

/// Sets the type of axis such as X or Y for a joystick.
/// @param type The type of axis to set this to. Set to kUnknown if not known.
void PlatformInputAxis::SetType(InputAxisType type)
{
	fAxisType = type;
}

/// Gets the minimum value that this axis input can provide.
/// @return Returns the minimum value that can be provided.
Rtt_Real PlatformInputAxis::GetMinValue() const
{
	return fMinValue;
}

/// Sets the minimum value that this axis input can provide.
/// @param value The minimum value that can be provided. Typically 0 or -1.
void PlatformInputAxis::SetMinValue(Rtt_Real value)
{
	fMinValue = value;
}

/// Gets the maximum value that this axis input can provide.
/// @return Returns the maximum value that can be provided.
Rtt_Real PlatformInputAxis::GetMaxValue() const
{
	return fMaxValue;
}

/// Sets the maximum value that this axis input can provide.
/// @param value The maximum value that can be provided. Typically set to 1.
void PlatformInputAxis::SetMaxValue(Rtt_Real value)
{
	fMaxValue = value;
}

/// Gets the +/- accuracy of the values this axis input provides.
/// @return Returns the accuracy of this axis input's values. Will be greater than or equal to zero.
///         <br>
///         A value of zero indicates perfect accuracy, but more likely indicates that the accuracy
///         of the input device unknown.
Rtt_Real PlatformInputAxis::GetAccuracy() const
{
	return fAccuracy;
}

/// Sets the +/- accuracy of the values this axis input provides.
/// @param value The accuracy of the input. Must be greater than or equal to zero.
///              Zero indicates perfect accuracy.
void PlatformInputAxis::SetAccuracy(Rtt_Real value)
{
	if (value < Rtt_REAL_0)
	{
		value = Rtt_REAL_0;
	}
	fAccuracy = value;
}

/// Determines if this axis input provides absolute values or relative values.
/// @return Returns true if the axis provides absolute values, meaning that it is positional data.
///         This is typically used by joysticks, indicating their currently held position.
///         <br>
///         Returns false if the axis provides relative values, which is typically the distance travelled
///         since the last axis input event. This is typically used by mice and trackpads, indicating
///         how far the input device has moved since it's last input event.
bool PlatformInputAxis::IsAbsolute() const
{
	return fIsAbsolute;
}

/// Sets whether or not this axis input provides absolute values or relative values.
/// @param value Set true if the axis provides absolute values, which is positional data.
///              This is typically used by joysticks, indicating their currently held position.
///              <br>
///              Set false if the axis provides relative values, which is typically the distance travelled
///              since the last received axis input event. This is typically used by mice and trackpads,
///              indicating how far the input device has move since its last input event.
void PlatformInputAxis::SetIsAbsolute(bool value)
{
	fIsAbsolute = value;
}

/// Returns normalized value with regards to minimum and maximum values.
/// @param rawValue Raw value produced by event
///              <br>
///              This function provides mechanism to get abstract [-1;1] or [0;1] input from axes.
Rtt_Real PlatformInputAxis::GetNormalizedValue(Rtt_Real rawValue)
{
	Rtt_Real maxValue = GetMaxValue();
	Rtt_Real minValue = GetMinValue();
	Rtt_Real normalizedValue = Rtt_REAL_0;
	if (maxValue <= minValue)
	{
		normalizedValue = Rtt_REAL_0;
	}
	else if (rawValue >= maxValue)
	{
		normalizedValue = (maxValue > Rtt_REAL_0) ? Rtt_REAL_1 : Rtt_REAL_0;
	}
	else if (rawValue <= minValue)
	{
		normalizedValue = (minValue < Rtt_REAL_0) ? Rtt_REAL_NEG_1 : Rtt_REAL_0;
	}
	else if ((minValue >= Rtt_REAL_0) || (maxValue <= Rtt_REAL_0))
	{
		Rtt_Real range = GetMaxValue() - GetMinValue();
		if (range > Rtt_REAL_0)
		{
			normalizedValue = Rtt_RealDiv(rawValue - GetMinValue(), range);
		}
	}
	else if (rawValue >= Rtt_REAL_0)
	{
		normalizedValue = Rtt_RealDiv(rawValue, maxValue);
	}
	else
	{
		normalizedValue = Rtt_RealDiv(-rawValue, minValue);
	}
	return normalizedValue;
}
	
/// Pushes this object as a Lua table into the given Lua state.
/// @param L The Lua state to push this object into.
void PlatformInputAxis::PushTo(lua_State *L)
{
	// Validate.
	if (NULL == L)
	{
		return;
	}

	// Create the a Lua table for storing the axis input's information.
	lua_newtable(L);

	// Push the axis descriptor name.
	lua_pushstring(L, fDescriptor.GetInvariantName());
	lua_setfield(L, -2, "descriptor");

	// Push the axis number.
	lua_pushinteger(L, fDescriptor.GetAxisNumber());
	lua_setfield(L, -2, "number");

	// Push the axis type name.
	lua_pushstring(L, fAxisType.GetStringId());
	lua_setfield(L, -2, "type");

	// Push the min axis value.
	lua_pushnumber(L, fMinValue);
	lua_setfield(L, -2, "minValue");

	// Push the max axis value.
	lua_pushnumber(L, fMaxValue);
	lua_setfield(L, -2, "maxValue");

	// Push the axis input accuracy.
	lua_pushnumber(L, fAccuracy);
	lua_setfield(L, -2, "accuracy");

//NOTE: Not sure if we should expose this property.
//      This is because we may not be determining this setting reliably on Android.
//	// Push the axis absolute/relative value flag.
//	lua_pushboolean(L, fIsAbsolute ? 1 : 0);
//	lua_setfield(L, -2, "isAbsolute");
}

} // namespace Rtt
