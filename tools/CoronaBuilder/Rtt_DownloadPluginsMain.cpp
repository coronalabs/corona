//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_DownloadPluginsMain.h"
#include "Rtt_LuaLibBuilder.h"
#include "Rtt_HTTPClient.h"

// ----------------------------------------------------------------------------

namespace Rtt
{
	int luaload_BuilderPluginDownloader(lua_State* L);

DownloadPluginsMain::DownloadPluginsMain(lua_State *L)
:	fL( L )
{
	Lua::DoBuffer( L, &luaload_BuilderPluginDownloader, NULL);
}

int DownloadPluginsMain::Run(int argc, const char* args[])
{
	lua_State *L = fL;
	
	if(argc >= 2 && strcmp("--android-offline-plugins", args[1]) == 0) {
		lua_getglobal(L, "DownloadAndroidOfflinePlugins");
		args++;
		argc--;
	}
	else {
		lua_getglobal(L, "DownloadPluginsMain");
	}
    HTTPClient::registerFetcherModuleLoaders(L);
	
	Rtt_ASSERT(lua_type(L, -1) == LUA_TFUNCTION);
	lua_createtable(L, argc, 0);
	for(int i = 0; i<argc; i++)
	{
		lua_pushstring(L, args[i]);
		lua_rawseti(L, -2, i+1);
	}
	lua_pushnil(L);
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

