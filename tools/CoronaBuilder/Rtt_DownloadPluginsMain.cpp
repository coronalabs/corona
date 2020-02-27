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

// ----------------------------------------------------------------------------
extern "C" {
int luaopen_lfs (lua_State *L);
int luaopen_socket_core (lua_State *L);
int luaopen_mime_core(lua_State *L);
}

namespace Rtt
{

int luaload_json(lua_State *L);
int luaload_dkjson(lua_State *L);
int luaload_BuilderPluginDownloader(lua_State *L);
int luaload_CoronaPListSupport(lua_State *L);
int luaload_CoronaBuilderPluginCollector(lua_State *L);
extern int luaload_luasocket_socket(lua_State *L);
extern int luaload_luasocket_ftp(lua_State *L);
extern int luaload_luasocket_headers(lua_State *L);
extern int luaload_luasocket_http(lua_State *L);
extern int luaload_luasocket_url(lua_State *L);
extern int luaload_luasocket_mime(lua_State *L);
extern int luaload_luasocket_ltn12(lua_State *L);


DownloadPluginsMain::DownloadPluginsMain(lua_State *L)
:	fL( L )
{
	Lua::RegisterModuleLoader( L, "CoronaBuilderPluginCollector", Lua::Open< luaload_CoronaBuilderPluginCollector >);
	Lua::DoBuffer( L, &luaload_BuilderPluginDownloader, NULL);
}

int DownloadPluginsMain::Run(int argc, const char* args[], const char* usr)
{
	lua_State *L = fL;
	
	if(argc >= 2 && strcmp("--android-offline-plugins", args[1]) == 0) {
		lua_getglobal(L, "DownloadAndroidOfflinePlugins");
		args++;
		argc--;
		
		Lua::RegisterModuleLoader( L, "socket.core", luaopen_socket_core );
		Lua::RegisterModuleLoader( L, "socket", Lua::Open< luaload_luasocket_socket > );
		Lua::RegisterModuleLoader( L, "socket.ftp", Lua::Open< luaload_luasocket_ftp > );
		Lua::RegisterModuleLoader( L, "socket.headers", Lua::Open< luaload_luasocket_headers > );
		Lua::RegisterModuleLoader( L, "socket.http", Lua::Open< luaload_luasocket_http > );
		Lua::RegisterModuleLoader( L, "socket.url", Lua::Open< luaload_luasocket_url > );
		Lua::RegisterModuleLoader( L, "mime.core", luaopen_mime_core );
		Lua::RegisterModuleLoader( L, "mime", Lua::Open< luaload_luasocket_mime > );
		Lua::RegisterModuleLoader( L, "ltn12", Lua::Open< luaload_luasocket_ltn12 > );
	}
	else {
		lua_getglobal(L, "DownloadPluginsMain");
	}
	
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

