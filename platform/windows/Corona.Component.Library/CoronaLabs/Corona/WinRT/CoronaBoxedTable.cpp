// ----------------------------------------------------------------------------
// 
// CoronaBoxedTable.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "CoronaBoxedTable.h"
extern "C"
{
#	include "lua.h"
}


namespace CoronaLabs { namespace Corona { namespace WinRT {

#pragma region Consructors/Destructors
CoronaBoxedTable::CoronaBoxedTable()
{
	fMap = ref new Platform::Collections::Map<ICoronaBoxedComparableData^, ICoronaBoxedData^, KeyLessThanComparer>();
}

#pragma endregion


#pragma region Public Instance Methods/Properties
void CoronaBoxedTable::Set(ICoronaBoxedComparableData^ key, ICoronaBoxedData^ value)
{
	// Do not continue if given a null key.
	if (nullptr == key)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Remove the key's entry if given a null value.
	// This duplicates Lua's table behavior which does not support nil values in Lua tables.
	if (nullptr == value)
	{
		RemoveByKey(key);
		return;
	}

	// Add or update the key/value pair in the table.
	fMap->Insert(key, value);
}

ICoronaBoxedData^ CoronaBoxedTable::GetValueByKey(ICoronaBoxedComparableData^ key)
{
	// Do not continue if given a null key.
	if (nullptr == key)
	{
		return nullptr;
	}
	
	// Do not continue if key does not exist in the table.
	if (fMap->HasKey(key) == false)
	{
		return nullptr;
	}

	// Fetch the key's value.
	return fMap->Lookup(key);
}

bool CoronaBoxedTable::ContainsKey(ICoronaBoxedComparableData^ key)
{
	if (nullptr == key)
	{
		return false;
	}
	return fMap->HasKey(key);
}

bool CoronaBoxedTable::RemoveByKey(ICoronaBoxedComparableData^ key)
{
	// Validate key.
	if (nullptr == key)
	{
		return false;
	}

	// Do not continue if the key does not exist in the table.
	if (fMap->HasKey(key) == false)
	{
		return false;
	}

	// Remove the key's entry from the table.
	fMap->Remove(key);
	return true;
}

void CoronaBoxedTable::Clear()
{
	fMap->Clear();
}

int CoronaBoxedTable::Count::get()
{
	return (int)(fMap->Size);
}

bool CoronaBoxedTable::PushToLua(int64 luaStateMemoryAddress)
{
	// Validate argument.
	if (0 == luaStateMemoryAddress)
	{
		return false;
	}

	// Get a pointer to the Lua state.
	auto luaStatePointer = (lua_State*)luaStateMemoryAddress;

	// If the collection is empty, then create an empty Lua table and stop here.
	int count = this->Count;
	if (count <= 0)
	{
		lua_newtable(luaStatePointer);
		return true;
	}

	// Push this table's entries into Lua.
	lua_createtable(luaStatePointer, 0, count);
	int luaTableStackIndex = lua_gettop(luaStatePointer);
	for (auto&& entry : fMap)
	{
		if (entry->Key && entry->Value)
		{
			entry->Key->PushToLua(luaStateMemoryAddress);
			entry->Value->PushToLua(luaStateMemoryAddress);
			lua_rawset(luaStatePointer, luaTableStackIndex);
		}
	}
	return true;
}

Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<ICoronaBoxedComparableData^, ICoronaBoxedData^>^>^ CoronaBoxedTable::First()
{
	return fMap->First();
}

#pragma endregion


#pragma region Private KeyComparer Class Functions
CoronaBoxedTable::KeyLessThanComparer::KeyLessThanComparer()
{
}

bool CoronaBoxedTable::KeyLessThanComparer::operator()(
	const ICoronaBoxedComparableData^ x, const ICoronaBoxedComparableData^ y) const
{
	// Do null reference checks first.
	if (!x && !y)
	{
		// Not less-than because both references are null, meaning equal.
		return false;
	}
	else if (x && !y)
	{
		// Not less-than because x is not null and y is null.
		return false;
	}
	else if (!x && y)
	{
		// Is less-than because x is null and y is not.
		return true;
	}

	// If given objects have the same reference, then they are equal. Not less-than.
	auto nonConstantX = const_cast<ICoronaBoxedComparableData^>(x);
	auto nonConstantY = const_cast<ICoronaBoxedComparableData^>(y);
	if (Platform::Object::ReferenceEquals(nonConstantX, nonConstantY))
	{
		return false;
	}

	// Compare the values.
	return (nonConstantX->CompareTo(nonConstantY) < 0);
}

#pragma endregion

} } }	// namespace CoronaLabs::Corona::WinRT
