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
#include "Ticks.h"


namespace Interop {

#pragma region Constructors/Destructors
Ticks::Ticks()
:	Ticks(0)
{
}

Ticks::Ticks(int32_t rawValue)
:	fValue(rawValue)
{
}

Ticks::~Ticks()
{
}

#pragma endregion


#pragma region Public Methods
Ticks& Ticks::AddMilliseconds(int value)
{
	if (value != 0)
	{
		fValue += (int32_t)value;
	}
	return *this;
}

Ticks& Ticks::AddSeconds(int value)
{
	if (value != 0)
	{
		fValue += (int32_t)(value * 1000);
	}
	return *this;
}

DWORD Ticks::ToRawValue() const
{
	return (DWORD)fValue;
}

bool Ticks::Equals(const Ticks& value) const
{
	return (value.fValue == fValue);
}

bool Ticks::NotEquals(const Ticks& value) const
{
	return (value.fValue != fValue);
}

bool Ticks::operator==(const Ticks& value) const
{
	return Equals(value);
}

bool Ticks::operator!=(const Ticks& value) const
{
	return NotEquals(value);
}

int Ticks::CompareTo(const Ticks& value) const
{
	// First, check for equality.
	if (value.fValue == fValue)
	{
		return 0;
	}

	// Compare the given tick values via subtraction. Overflow for this subtraction operation is okay.
	int32_t rawValue = value.fValue;
	if (INT32_MIN == rawValue)
	{
		// Can't flip sign of largest negative number. So, give it a one millisecond boost.
		rawValue++;
	}
	int32_t deltaTime = fValue - rawValue;
	return (deltaTime < 0) ? -1 : 1;
}

bool Ticks::operator>(const Ticks& value) const
{
	return (CompareTo(value) > 0);
}

bool Ticks::operator>=(const Ticks& value) const
{
	return (CompareTo(value) >= 0);
}

bool Ticks::operator<(const Ticks& value) const
{
	return (CompareTo(value) < 0);
}

bool Ticks::operator<=(const Ticks& value) const
{
	return (CompareTo(value) <= 0);
}

#pragma endregion


#pragma region Public Static Functions
Ticks Ticks::FromCurrentTime()
{
	return Ticks((int32_t)::GetTickCount());
}

Ticks Ticks::FromRawValue(DWORD value)
{
	return Ticks((int32_t)value);
}

#pragma endregion

}	// namespace Interop
