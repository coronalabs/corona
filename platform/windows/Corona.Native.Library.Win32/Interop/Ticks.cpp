//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
