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
#include "CoronaBoxedData.h"
#include "CoronaBoxedNumber.h"
#include "CoronaBoxedString.h"
#include "CoronaLuaEventProperties.h"
#include "ICoronaBoxedData.h"
#include "ReadOnlyCoronaLuaEventProperties.h"
#include "CoronaLabs\WinRT\NativeStringServices.h"
#include "CoronaLabs\WinRT\OperationResult.h"
#include "CoronaLabs\WinRT\Utf8String.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
extern "C"
{
#	include "lua.h"
}
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace CoronaLabs { namespace Corona { namespace WinRT {

#pragma region Consructors/Destructors
CoronaLuaEventProperties::CoronaLuaEventProperties()
{
	fProperties = ref new Platform::Collections::Map<Platform::String^, ICoronaBoxedData^>();
}

#pragma endregion


#pragma region Public Methods/Properties
void CoronaLuaEventProperties::Set(Platform::String^ name, ICoronaBoxedData^ value)
{
	// Validate property name.
	if (name->IsEmpty())
	{
		throw ref new Platform::NullReferenceException();
	}

	// If value is null, then remove the property from the collection.
	// We do this because Lua tables do not support nil values.
	if (nullptr == value)
	{
		this->Remove(name);
	}

	// Add/update the given property pair.
	fProperties->Insert(name, value);
}

void CoronaLuaEventProperties::Set(Platform::String^ name, bool value)
{
	// Validate property name.
	if (name->IsEmpty())
	{
		throw ref new Platform::NullReferenceException();
	}

	// Add/update the given property pair.
	fProperties->Insert(name, CoronaBoxedBoolean::From(value));
}

void CoronaLuaEventProperties::Set(Platform::String^ name, int value)
{
	// Validate property name.
	if (name->IsEmpty())
	{
		throw ref new Platform::NullReferenceException();
	}

	// Add/update the given property pair.
	fProperties->Insert(name, ref new CoronaBoxedNumber((double)value));
}

void CoronaLuaEventProperties::Set(Platform::String^ name, double value)
{
	// Validate property name.
	if (name->IsEmpty())
	{
		throw ref new Platform::NullReferenceException();
	}

	// Add/update the given property pair.
	fProperties->Insert(name, ref new CoronaBoxedNumber(value));
}

void CoronaLuaEventProperties::Set(Platform::String^ name, Platform::String^ value)
{
	// Validate property name.
	if (name->IsEmpty())
	{
		throw ref new Platform::NullReferenceException();
	}

	// Add/update the given property pair.
	fProperties->Insert(name, CoronaBoxedString::From(value));
}

ICoronaBoxedData^ CoronaLuaEventProperties::Get(Platform::String^ name)
{
	// Return null if the given property name could not be found.
	if (fProperties->HasKey(name) == false)
	{
		return nullptr;
	}

	// Fetch the property value.
	return fProperties->Lookup(name);
}

bool CoronaLuaEventProperties::Contains(Platform::String^ name)
{
	return fProperties->HasKey(name);
}

bool CoronaLuaEventProperties::Remove(Platform::String^ name)
{
	// Do not continue if the given property name could not be found.
	if (fProperties->HasKey(name) == false)
	{
		return false;
	}

	// Remove the property by the given name.
	fProperties->Remove(name);
	return true;
}

void CoronaLuaEventProperties::Clear()
{
	fProperties->Clear();
}

int CoronaLuaEventProperties::Count::get()
{
	return fProperties->Size;
}

Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<Platform::String^, ICoronaBoxedData^>^>^ CoronaLuaEventProperties::First()
{
	return fProperties->First();
}

#pragma endregion


#pragma region Internal Methods
CoronaLabs::WinRT::IOperationResult^ CoronaLuaEventProperties::PushTo(lua_State *luaStatePointer)
{
	// Validate.
	if (nullptr == luaStatePointer)
	{
		return CoronaLabs::WinRT::OperationResult::FailedWith(L"Lua state pointer is null.");
	}

	// Push this object's properties collection into Lua as a table.
	lua_createtable(luaStatePointer, 0, (int)(fProperties->Size));
	int luaTableIndex = lua_gettop(luaStatePointer);
	for (auto&& nextPair : fProperties)
	{
		bool wasPushed = nextPair->Value->PushToLua((int64)luaStatePointer);
		if (wasPushed)
		{
			auto utf8PropertyName = CoronaLabs::WinRT::NativeStringServices::Utf8From(nextPair->Key);
			lua_setfield(luaStatePointer, luaTableIndex, utf8PropertyName->Data);
		}
	}

	// The push was successful.
	return CoronaLabs::WinRT::OperationResult::Succeeded;
}

#pragma endregion


#pragma region Static Functions
CoronaLuaEventProperties^ CoronaLuaEventProperties::CreateWithName(Platform::String^ eventName)
{
	// Validate.
	if (eventName->IsEmpty())
	{
		throw ref new Platform::NullReferenceException();
	}

	// Create and return a new properties collection with the given event name.
	auto properties = ref new CoronaLuaEventProperties();
	properties->Set("name", eventName);
	return properties;
}

CoronaLuaEventProperties^ CoronaLuaEventProperties::From(lua_State *luaStatePointer, int luaEventTableIndex)
{
	// Validate.
	if (nullptr == luaStatePointer)
	{
		return nullptr;
	}

	// Do not continue if not indexing a Lua table.
	if (lua_istable(luaStatePointer, luaEventTableIndex) == false)
	{
		return nullptr;
	}

	// Do not continue if the indexed Lua table is an array, not a hash table.
	// Note: A Lua hash table of key/value pairs will always have a length of zero, even if it is populated.
	auto tableItemCount = lua_objlen(luaStatePointer, luaEventTableIndex);
	if (tableItemCount > 0)
	{
		return nullptr;
	}

	// Copy the Lua event table's key/value pairs to a new properties object.
	auto properties = ref new CoronaLuaEventProperties();
	for (lua_pushnil(luaStatePointer); lua_next(luaStatePointer, luaEventTableIndex) != 0; lua_pop(luaStatePointer, 1))
	{
		// Fetch the entry's key. (It must be a string and non-empty.)
		auto boxedStringKey = dynamic_cast<CoronaBoxedString^>(CoronaBoxedData::FromLua(luaStatePointer, -2));
		if (!boxedStringKey || boxedStringKey->ToUtf16String()->IsEmpty())
		{
			continue;
		}

		// Fetch the entry's value.
		auto boxedValue = CoronaBoxedData::FromLua(luaStatePointer, -1);

		// Add the entry's key/value pair to the properties collection.
		properties->Set(boxedStringKey->ToUtf16String(), boxedValue);
	}

	// Make sure that the event properties we copied contains an event name. This is a required field.
	if (properties->Contains("name") == false)
	{
		return nullptr;
	}

	// Copy was successful. Return the event properties.
	return properties;
}

#pragma endregion

} } }	// namespace CoronaLabs::Corona::WinRT
