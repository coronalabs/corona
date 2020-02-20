//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "CoronaBoxedData.h"
#include "CoronaBoxedBoolean.h"
#include "CoronaBoxedNumber.h"
#include "CoronaBoxedString.h"
#include "CoronaBoxedList.h"
#include "CoronaBoxedTable.h"
#include "ICoronaBoxedComparableData.h"
#include "CoronaLabs\WinRT\NativeStringServices.h"
#include "CoronaLabs\WinRT\Utf8String.h"
extern "C"
{
#	include "lua.h"
}


namespace CoronaLabs { namespace Corona { namespace WinRT {

#pragma region Consructors/Destructors
CoronaBoxedData::CoronaBoxedData()
{
}

#pragma endregion


#pragma region Public Static Functions
ICoronaBoxedData^ CoronaBoxedData::FromLua(int64 luaStateMemoryAddress, int luaStackIndex)
{
	// Validate.
	if (0 == luaStateMemoryAddress)
	{
		return nullptr;
	}

	// Create a boxed data object storing a copy of the value(s) indexed in Lua.
	return CoronaBoxedData::FromLua((lua_State*)luaStateMemoryAddress, luaStackIndex);
}

#pragma endregion


#pragma region Internal Static Functions
ICoronaBoxedData^ CoronaBoxedData::FromLua(lua_State *luaStatePointer, int luaStackIndex)
{
	// Validate.
	if (nullptr == luaStatePointer)
	{
		return nullptr;
	}

	// Create a boxed data object storing a copy of the value(s) indexed in Lua.
	ICoronaBoxedData^ boxedData = nullptr;
	auto luaType = lua_type(luaStatePointer, luaStackIndex);
	switch (luaType)
	{
		case LUA_TBOOLEAN:
		{
			boxedData = CoronaBoxedBoolean::From(lua_toboolean(luaStatePointer, luaStackIndex) ? true : false);
			break;
		}
		case LUA_TNUMBER:
		{
			boxedData = ref new CoronaBoxedNumber(lua_tonumber(luaStatePointer, luaStackIndex));
			break;
		}
		case LUA_TSTRING:
		{
			auto utf8String = ref new CoronaLabs::WinRT::Utf8String(lua_tostring(luaStatePointer, luaStackIndex));
			boxedData = CoronaBoxedString::From(utf8String);
			break;
		}
		case LUA_TTABLE:
		{
			// Convert relative index to an absolute index.
			if (luaStackIndex < 0)
			{
				luaStackIndex = lua_gettop(luaStatePointer) + (luaStackIndex + 1);
			}

			// Determine if the Lua table is an array or a hash table.
			auto tableItemCount = lua_objlen(luaStatePointer, luaStackIndex);
			if (tableItemCount > 0)
			{
				// Copy the Lua array's values to a boxed list.
				auto boxedList = ref new CoronaBoxedList();
				for (lua_pushnil(luaStatePointer); lua_next(luaStatePointer, luaStackIndex) != 0; lua_pop(luaStatePointer, 1))
				{
					boxedList->Add(CoronaBoxedData::FromLua(luaStatePointer, -1));
				}
				boxedData = boxedList;
			}
			else
			{
				// Copy the Lua hash table's key/value pairs to a boxed table.
				// Note: Lua hash table's alway return zero for item count, even when populated. Only supported by Lua arrays.
				auto boxedTable = ref new CoronaBoxedTable();
				for (lua_pushnil(luaStatePointer); lua_next(luaStatePointer, luaStackIndex) != 0; lua_pop(luaStatePointer, 1))
				{
					auto boxedKey = dynamic_cast<ICoronaBoxedComparableData^>(CoronaBoxedData::FromLua(luaStatePointer, -2));
					if (nullptr == boxedKey)
					{
						continue;
					}
					auto boxedValue = CoronaBoxedData::FromLua(luaStatePointer, -1);
					boxedTable->Set(boxedKey, boxedValue);
				}
				boxedData = boxedTable;
			}
			break;
		}
	}
	return boxedData;
}

#pragma endregion

} } }	// namespace CoronaLabs::Corona::WinRT
