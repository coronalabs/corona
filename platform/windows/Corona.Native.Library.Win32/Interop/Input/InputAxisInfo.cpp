//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InputAxisInfo.h"
#include "InputAxisSettings.h"


namespace Interop { namespace Input {

#pragma region Constructors/Destructors
InputAxisInfo::InputAxisInfo(const InputAxisSettings& settings)
:	fSettings(settings)
{
}

InputAxisInfo::~InputAxisInfo()
{
}

#pragma endregion


#pragma region Public Methods
Rtt::InputAxisType InputAxisInfo::GetType() const
{
	return fSettings.GetType();
}

int32_t InputAxisInfo::GetMinValue() const
{
	return fSettings.GetMinValue();
}

int32_t InputAxisInfo::GetMaxValue() const
{
	return fSettings.GetMaxValue();
}

float InputAxisInfo::GetAccuracy() const
{
	return fSettings.GetAccuracy();
}

bool InputAxisInfo::IsAbsolute() const
{
	return fSettings.IsAbsolute();
}

bool InputAxisInfo::Equals(const InputAxisInfo& value) const
{
	// First, check if the 2 object pointers match. (This is an optimization.)
	if (&value == this)
	{
		return true;
	}

	// Last, check if the 2 axis configurations match.
	return fSettings.Equals(value.fSettings);
}

bool InputAxisInfo::Equals(const InputAxisSettings& value) const
{
	return fSettings.Equals(value);
}

bool InputAxisInfo::NotEquals(const InputAxisInfo& value) const
{
	return !Equals(value);
}

bool InputAxisInfo::NotEquals(const InputAxisSettings& value) const
{
	return !Equals(value);
}

bool InputAxisInfo::operator==(const InputAxisInfo& value) const
{
	return Equals(value);
}

bool InputAxisInfo::operator==(const InputAxisSettings& value) const
{
	return Equals(value);
}

bool InputAxisInfo::operator!=(const InputAxisInfo& value) const
{
	return !Equals(value);
}

bool InputAxisInfo::operator!=(const InputAxisSettings& value) const
{
	return !Equals(value);
}

#pragma endregion

} }	// namespace Interop::Input
