//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InputAxisSettings.h"
#include <cmath>


namespace Interop { namespace Input {

#pragma region Constructors/Destructors
InputAxisSettings::InputAxisSettings()
:	fType(Rtt::InputAxisType::kUnknown),
	fMinValue((int32_t)MININT16),
	fMaxValue((int32_t)MAXINT16),
	fAccuracy(0)
{
}

InputAxisSettings::~InputAxisSettings()
{
}

#pragma endregion


#pragma region Public Methods
Rtt::InputAxisType InputAxisSettings::GetType() const
{
	return fType;
}

void InputAxisSettings::SetType(const Rtt::InputAxisType& value)
{
	fType = value;
}

int32_t InputAxisSettings::GetMinValue() const
{
	return fMinValue;
}

void InputAxisSettings::SetMinValue(int32_t value)
{
	fMinValue = value;
}

int32_t InputAxisSettings::GetMaxValue() const
{
	return fMaxValue;
}

void InputAxisSettings::SetMaxValue(int32_t value)
{
	fMaxValue = value;
}

float InputAxisSettings::GetAccuracy() const
{
	return fAccuracy;
}

void InputAxisSettings::SetAccuracy(float value)
{
	if (value < 0)
	{
		value = 0;
	}
	fAccuracy = value;
}

bool InputAxisSettings::IsAbsolute() const
{
	return fIsAbsolute;
}

void InputAxisSettings::SetIsAbsolute(bool value)
{
	fIsAbsolute = value;
}

bool InputAxisSettings::Equals(const InputAxisSettings& value) const
{
	// First, check if the 2 object pointers match. (This is an optimization.)
	if (&value == this)
	{
		return true;
	}

	// Do not continue if the member variables don't match.
	if (fType != value.fType)
	{
		return false;
	}
	if (fMinValue != value.fMinValue)
	{
		return false;
	}
	if (fMaxValue != value.fMaxValue)
	{
		return false;
	}
	if (std::abs(fAccuracy - value.fAccuracy) > FLT_EPSILON)
	{
		return false;
	}
	if (fIsAbsolute != value.fIsAbsolute)
	{
		return false;
	}

	// The given settings equals this object's settings.
	return true;
}

bool InputAxisSettings::NotEquals(const InputAxisSettings& value) const
{
	return !Equals(value);
}

bool InputAxisSettings::operator==(const InputAxisSettings& value) const
{
	return Equals(value);
}

bool InputAxisSettings::operator!=(const InputAxisSettings& value) const
{
	return !Equals(value);
}

#pragma endregion

} }	// namespace Interop::Input
