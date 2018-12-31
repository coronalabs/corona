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
