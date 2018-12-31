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

#include "Core/Rtt_Build.h"

#include "Rtt_DownloadPluginsMain.h"
#include "Rtt_LuaLibBuilder.h"

// ----------------------------------------------------------------------------
extern "C" {
int luaopen_lfs (lua_State *L);
}

namespace Rtt
{

int luaload_json(lua_State *L);
int luaload_dkjson(lua_State *L);
int luaload_BuilderPluginDownloader(lua_State *L);
int luaload_CoronaPListSupport(lua_State *L);


DownloadPluginsMain::DownloadPluginsMain(lua_State *L)
:	fL( L )
{
	Lua::DoBuffer( L, &luaload_BuilderPluginDownloader, NULL);
}

int DownloadPluginsMain::Run(int argc, const char* args[], const char* usr)
{
	lua_State *L = fL;

	lua_getglobal(L, "DownloadPluginsMain");
	Rtt_ASSERT(lua_type(L, -1) == LUA_TFUNCTION);
	lua_createtable(L, argc, 0);
	for(int i = 0; i<argc; i++)
	{
		lua_pushstring(L, args[i]);
		lua_rawseti(L, -2, i+1);
	}
	lua_pushstring(L, usr);
	lua_pushinteger(L, Rtt_BUILD_YEAR);
	lua_pushinteger(L, Rtt_BUILD_REVISION);

	lua_call(L, 4, 1);
	int ret = -1;
	if(lua_isnumber(L, -1))
	{
		ret = (int)lua_tointeger(L, -1);
	}
	lua_pop(L, 1);

	return ret;
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

