// ----------------------------------------------------------------------------
// 
// CoronaBoxedString.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "CoronaBoxedString.h"
#include "CoronaBoxedBoolean.h"
#include "CoronaBoxedNumber.h"
extern "C"
{
#	include "lua.h"
}


namespace CoronaLabs { namespace Corona { namespace WinRT {

#pragma region Consructors/Destructors
CoronaBoxedString::CoronaBoxedString(Platform::String^ utf16String)
{
	fUtf16String = utf16String;
	fUtf8String = CoronaLabs::WinRT::Utf8String::From(utf16String);
}

CoronaBoxedString::CoronaBoxedString(CoronaLabs::WinRT::Utf8String^ utf8String)
{
	if (nullptr == utf8String)
	{
		throw ref new Platform::NullReferenceException();
	}

	fUtf8String = utf8String;
	fUtf16String = utf8String->ToString();
}

#pragma endregion


#pragma region Public Instance Methods/Properties
CoronaLabs::WinRT::Utf8String^ CoronaBoxedString::ToUtf8String()
{
	return fUtf8String;
}

Platform::String^ CoronaBoxedString::ToUtf16String()
{
	return fUtf16String;
}

Platform::String^ CoronaBoxedString::ToString()
{
	return fUtf16String;
}

bool CoronaBoxedString::Equals(CoronaLabs::WinRT::Utf8String^ value)
{
	return fUtf8String->Equals(value);
}

bool CoronaBoxedString::Equals(Platform::String^ value)
{
	return fUtf16String->Equals(value);
}

bool CoronaBoxedString::Equals(Platform::Object^ value)
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

	// Do a UTF-16 comparison if applicable.
	auto utf16String = dynamic_cast<Platform::String^>(value);
	if (utf16String)
	{
		return Equals(utf16String);
	}

	// Do a UTF-8 comparison if applicable.
	auto utf8String = dynamic_cast<CoronaLabs::WinRT::Utf8String^>(value);
	if (utf8String)
	{
		return Equals(utf8String);
	}

	// The given object type cannot be compared with this class. Not equal.
	return false;
}

int CoronaBoxedString::CompareTo(Platform::Object^ value)
{
	// Return "greater" if given a null reference.
	if (nullptr == value)
	{
		return 1;
	}

	// This object is always greater than the other Corona boxed value types.
	auto coronaBoxedBoolean = dynamic_cast<CoronaBoxedBoolean^>(value);
	if (coronaBoxedBoolean)
	{
		return 1;
	}
	auto coronaBoxedNumber = dynamic_cast<CoronaBoxedNumber^>(value);
	if (coronaBoxedNumber)
	{
		return 1;
	}

	// Compare UTF-16 strings.
	auto coronaBoxedString = dynamic_cast<CoronaBoxedString^>(value);
	if (coronaBoxedString)
	{
		return Platform::String::CompareOrdinal(fUtf16String, coronaBoxedString->fUtf16String);
	}
	return Platform::String::CompareOrdinal(fUtf16String, value->ToString());
}

int CoronaBoxedString::GetHashCode()
{
	return fUtf16String->GetHashCode();
}

bool CoronaBoxedString::PushToLua(int64 luaStateMemoryAddress)
{
	// Validate argument.
	if (0 == luaStateMemoryAddress)
	{
		return false;
	}

	// Push this object's string to the top of the Lua stack.
	auto luaStatePointer = (lua_State*)luaStateMemoryAddress;
	lua_pushstring(luaStatePointer, fUtf8String->Data);
	return true;
}

#pragma endregion


#pragma region Public Static Functions/Properties
CoronaBoxedString^ CoronaBoxedString::Empty::get()
{
	static CoronaBoxedString kEmpty(CoronaLabs::WinRT::Utf8String::Empty);
	return %kEmpty;
}

CoronaBoxedString^ CoronaBoxedString::From(Platform::String^ value)
{
	// If the given string is empty, then return the pre-allocated empty version of this class.
	if (value->IsEmpty())
	{
		return CoronaBoxedString::Empty;
	}

	// Return an object boxing the given string.
	return ref new CoronaBoxedString(value);
}

CoronaBoxedString^ CoronaBoxedString::From(CoronaLabs::WinRT::Utf8String^ value)
{
	// If the given string is null/empty, then return the pre-allocated empty version of this class.
	if (!value || value->IsEmpty)
	{
		return CoronaBoxedString::Empty;
	}

	// Return an object boxing the given string.
	return ref new CoronaBoxedString(value);
}

#pragma endregion

} } }	// namespace CoronaLabs::Corona::WinRT
