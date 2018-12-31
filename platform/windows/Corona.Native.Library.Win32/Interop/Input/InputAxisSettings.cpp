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
