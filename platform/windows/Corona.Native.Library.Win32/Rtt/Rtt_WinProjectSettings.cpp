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
