//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Rtt_WinProjectSettings.h"
extern "C"
{
#	include "lua.h"
}


namespace Rtt
{

#pragma region Constructors/Destructors
WinProjectSettings::WinProjectSettings()
{
	ResetBuildSettings();
}

WinProjectSettings::~WinProjectSettings()
{
}

#pragma endregion


#pragma region Public Methods
void WinProjectSettings::ResetBuildSettings()
{
	ProjectSettings::ResetBuildSettings();
	fPreferenceStorageType = PreferenceStorageType::kSQLite;
	fIsSingleInstanceWindowEnabled = true;
}

WinProjectSettings::PreferenceStorageType WinProjectSettings::GetWin32PreferenceStorageType() const
{
	return fPreferenceStorageType;
}

bool WinProjectSettings::IsWin32SingleInstanceWindowEnabled() const
{
	return fIsSingleInstanceWindowEnabled;
}

#pragma endregion


#pragma region Protected Methods
void WinProjectSettings::OnLoadedFrom(lua_State* luaStatePointer)
{
	// Validate.
	if (!luaStatePointer)
	{
		return;
	}

	// Let the base class handle this first.
	ProjectSettings::OnLoadedFrom(luaStatePointer);

	// Fetch Win32 specific build settings.
	// Note: The give Lua state contains the "build.settings" file's global "settings" table.
	lua_getglobal(luaStatePointer, "settings");
	if (lua_istable(luaStatePointer, -1))
	{
		lua_getfield(luaStatePointer, -1, "win32");
		if (lua_istable(luaStatePointer, -1))
		{
			// Fetch the preference storage type.
			lua_getfield(luaStatePointer, -1, "preferenceStorage");
			if (lua_type(luaStatePointer, -1) == LUA_TSTRING)
			{
				auto stringPointer = lua_tostring(luaStatePointer, -1);
				if (stringPointer)
				{
					if (_stricmp(stringPointer, "sqlite") == 0)
					{
						fPreferenceStorageType = PreferenceStorageType::kSQLite;
					}
					else if (_stricmp(stringPointer, "registry") == 0)
					{
						fPreferenceStorageType = PreferenceStorageType::kRegistry;
					}
				}
			}
			lua_pop(luaStatePointer, 1);

			// Fetch the single instance mode enable setting.
			lua_getfield(luaStatePointer, -1, "singleInstance");
			if (lua_type(luaStatePointer, -1) == LUA_TBOOLEAN)
			{
				fIsSingleInstanceWindowEnabled = lua_toboolean(luaStatePointer, -1) ? true : false;
			}
			lua_pop(luaStatePointer, 1);
		}
		lua_pop(luaStatePointer, 1);
	}
	lua_pop(luaStatePointer, 1);
}

#pragma endregion

} // namespace Rtt
