//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "CoronaBoxedBoolean.h"
extern "C"
{
#	include "lua.h"
}


namespace CoronaLabs { namespace Corona { namespace WinRT {

#pragma region Consructors/Destructors
CoronaBoxedBoolean::CoronaBoxedBoolean(bool value)
{
	fValue = value;
}

#pragma endregion


#pragma region Public Instance Methods/Properties
bool CoronaBoxedBoolean::Value::get()
{
	return fValue;
}

Platform::String^ CoronaBoxedBoolean::ToString()
{
	return fValue ? L"true" : L"false";
}

bool CoronaBoxedBoolean::Equals(bool value)
{
	return (value == fValue);
}

bool CoronaBoxedBoolean::Equals(Platform::Object^ value)
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
	auto microsoftBoxedValue = dynamic_cast<Platform::IBox<bool>^>(value);
	if (microsoftBoxedValue)
	{
		return Equals(microsoftBoxedValue->Value);
	}
	auto coronaBoxedValue = dynamic_cast<CoronaBoxedBoolean^>(value);
	if (coronaBoxedValue)
	{
		return Equals(coronaBoxedValue->Value);
	}

	// Not equal if given an unknown type.
	return false;
}

int CoronaBoxedBoolean::CompareTo(bool value)
{
	if (fValue && !value)
	{
		return 1;		// True is greater than false.
	}
	else if (!fValue && value)
	{
		return -1;		// False is less than true.
	}
	return 0;			// Both boolean values are equal. (ie: Both are true or both are false.)
}

int CoronaBoxedBoolean::CompareTo(Platform::Object^ value)
{
	// Return "greater" if given a null reference.
	if (nullptr == value)
	{
		return 1;
	}

	// Compare values.
	auto microsoftBoxedBoolean = dynamic_cast<Platform::IBox<bool>^>(value);
	if (microsoftBoxedBoolean)
	{
		return CompareTo(microsoftBoxedBoolean->Value);
	}
	auto coronaBoxedBoolean = dynamic_cast<CoronaBoxedBoolean^>(value);
	if (coronaBoxedBoolean)
	{
		return CompareTo(coronaBoxedBoolean->Value);
	}

	// This object is less than all other objects types.
	return false;
}

int CoronaBoxedBoolean::GetHashCode()
{
	Platform::Object^ boxedValue = fValue;
	return boxedValue->GetHashCode();
}

bool CoronaBoxedBoolean::PushToLua(int64 luaStateMemoryAddress)
{
	// Validate argument.
	if (0 == luaStateMemoryAddress)
	{
		return false;
	}

	// Push this object's value to the top of the Lua stack.
	auto luaStatePointer = (lua_State*)luaStateMemoryAddress;
	lua_pushboolean(luaStatePointer, fValue ? 1 : 0);
	return true;
}

#pragma endregion


#pragma region Public Static Functions/Properties
CoronaBoxedBoolean^ CoronaBoxedBoolean::True::get()
{
	static CoronaBoxedBoolean kTrue(true);
	return %kTrue;
}

CoronaBoxedBoolean^ CoronaBoxedBoolean::False::get()
{
	static CoronaBoxedBoolean kFalse(false);
	return %kFalse;
}

CoronaBoxedBoolean^ CoronaBoxedBoolean::From(bool value)
{
	return value ? CoronaBoxedBoolean::True : CoronaBoxedBoolean::False;
}

#pragma endregion

} } }	// namespace CoronaLabs::Corona::WinRT
