// ----------------------------------------------------------------------------
// 
// CoronaBoxedList.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "CoronaBoxedList.h"
extern "C"
{
#	include "lua.h"
}


namespace CoronaLabs { namespace Corona { namespace WinRT {

#pragma region Consructors/Destructors
CoronaBoxedList::CoronaBoxedList()
{
	fCollection = ref new Platform::Collections::Vector<ICoronaBoxedData^>();
}

#pragma endregion


#pragma region Public Instance Methods/Properties
void CoronaBoxedList::Add(ICoronaBoxedData^ item)
{
	// Validate.
	if (nullptr == item)
	{
		return;
	}

	// Add the given item to the collection.
	fCollection->Append(item);
}

void CoronaBoxedList::Clear()
{
	fCollection->Clear();
}

int CoronaBoxedList::Count::get()
{
	return (int)(fCollection->Size);
}

bool CoronaBoxedList::PushToLua(int64 luaStateMemoryAddress)
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

	// Push the collection's values to Lua.
	lua_createtable(luaStatePointer, count, 0);
	int luaTableStackIndex = lua_gettop(luaStatePointer);
	for (int index = 0; index < count; index++)
	{
		bool wasPushed = fCollection->GetAt(index)->PushToLua(luaStateMemoryAddress);
		if (wasPushed)
		{
			lua_rawseti(luaStatePointer, luaTableStackIndex, index + 1);
		}
	}
	return true;
}

Windows::Foundation::Collections::IIterator<ICoronaBoxedData^>^ CoronaBoxedList::First()
{
	return fCollection->First();
}

#pragma endregion

} } }	// namespace CoronaLabs::Corona::WinRT
