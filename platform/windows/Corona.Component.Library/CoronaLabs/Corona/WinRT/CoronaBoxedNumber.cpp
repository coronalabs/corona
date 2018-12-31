// ----------------------------------------------------------------------------
// 
// CoronaBoxedNumber.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "CoronaBoxedNumber.h"
#include "CoronaBoxedBoolean.h"
extern "C"
{
#	include "lua.h"
}


namespace CoronaLabs { namespace Corona { namespace WinRT {

#pragma region Consructors/Destructors
CoronaBoxedNumber::CoronaBoxedNumber(double value)
{
	fValue = value;
}

#pragma endregion


#pragma region Public Instance Methods/Properties
double CoronaBoxedNumber::Value::get()
{
	return fValue;
}

Platform::String^ CoronaBoxedNumber::ToString()
{
	const int MAX_MESSAGE_LENGTH = 64;
	wchar_t message[MAX_MESSAGE_LENGTH];
	_snwprintf_s(message, MAX_MESSAGE_LENGTH, L"%f", fValue);
	return ref new Platform::String(message);
}

bool CoronaBoxedNumber::Equals(double value)
{
	return (value == fValue);
}

bool CoronaBoxedNumber::Equals(Platform::Object^ value)
{
	// Not equal if null.
	if (nullptr == value)
	{
		return false;
	}

	// Equal if the references match.
	if (Platform::Object::ReferenceEquals(this, value))
	{
		return true;
	}

	// Compare values.
	auto microsoftBoxedValue = dynamic_cast<Platform::IBox<double>^>(value);
	if (microsoftBoxedValue)
	{
		return Equals(microsoftBoxedValue->Value);
	}
	auto coronaBoxedValue = dynamic_cast<CoronaBoxedNumber^>(value);
	if (coronaBoxedValue)
	{
		return Equals(coronaBoxedValue->Value);
	}

	// Not equal if given an unknown type.
	return false;
}

int CoronaBoxedNumber::CompareTo(double value)
{
	if (fValue > value)
	{
		return 1;
	}
	else if (fValue < value)
	{
		return -1;
	}
	return 0;
}

int CoronaBoxedNumber::CompareTo(Platform::Object^ value)
{
	// Return "greater" if given a null reference.
	if (nullptr == value)
	{
		return 1;
	}

	// Compare values.
	auto microsoftBoxedValue = dynamic_cast<Platform::IBox<double>^>(value);
	if (microsoftBoxedValue)
	{
		return CompareTo(microsoftBoxedValue->Value);
	}
	auto coronaBoxedValue = dynamic_cast<CoronaBoxedNumber^>(value);
	if (coronaBoxedValue)
	{
		return CompareTo(coronaBoxedValue->Value);
	}
	auto coronaBoxedBoolean = dynamic_cast<CoronaBoxedBoolean^>(value);
	if (coronaBoxedBoolean)
	{
		return 1;
	}

	// This object is less than all other objects types.
	return false;
}

int CoronaBoxedNumber::GetHashCode()
{
	Platform::Object^ boxedValue = fValue;
	return boxedValue->GetHashCode();
}

bool CoronaBoxedNumber::PushToLua(int64 luaStateMemoryAddress)
{
	// Validate argument.
	if (0 == luaStateMemoryAddress)
	{
		return false;
	}

	// Push this object's value to the top of the Lua stack.
	auto luaStatePointer = (lua_State*)luaStateMemoryAddress;
	lua_pushnumber(luaStatePointer, fValue);
	return true;
}

#pragma endregion

} } }	// namespace CoronaLabs::Corona::WinRT
