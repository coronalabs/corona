//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Rtt_HTTPClient.h"
#include "Rtt_Lua.h"

extern "C" {
int luaopen_lfs (lua_State *L);
int luaopen_socket_core (lua_State *L);
int luaopen_mime_core(lua_State *L);
int luaopen_lpeg (lua_State *L);
}

namespace Rtt {

int luaload_json(lua_State *L);
int luaload_dkjson(lua_State *L);
int luaload_BuilderPluginDownloader(lua_State *L);
int luaload_CoronaPListSupport(lua_State *L);
int luaload_CoronaBuilderPluginCollector(lua_State *L);
int luaload_dkjson(lua_State* L);
int luaload_json(lua_State* L);
extern int luaload_luasocket_socket(lua_State *L);
extern int luaload_luasocket_ftp(lua_State *L);
extern int luaload_luasocket_headers(lua_State *L);
extern int luaload_luasocket_http(lua_State *L);
extern int luaload_luasocket_url(lua_State *L);
extern int luaload_luasocket_mime(lua_State *L);
extern int luaload_luasocket_ltn12(lua_State *L);


int HTTPClient::fetch( lua_State *L )
{
	String result;
	String errorMesg;
	std::map<std::string, std::string> headers;
	if ( lua_istable( L, 2 ) )
	{
		lua_pushnil(L);  /* first key */
		while (lua_next(L, 2) != 0)
		{
			/* uses 'key' (at index -2) and 'value' (at index -1) */
			bool isValidArg = lua_isstring( L, -1 );
			if ( isValidArg )
			{
				std::string name =  lua_tostring( L, -2 );
				std::string value =  lua_tostring( L, -1 );
				headers.insert ( std::map<std::string, std::string>::value_type(name, value) );
			}
			lua_pop( L, 1 );
		}
		
	}
	
	const char *url = NULL;
	
	if ( lua_isstring( L, 1 ) )
	{
		url = lua_tostring( L, 1 );
		
		if (! HTTPClient::HttpDownload(url, result, errorMesg, headers))
		{
			lua_pushnil( L );
			lua_pushstring(L, errorMesg);
		}
		else
		{
			lua_pushstring( L, result );
			lua_pushnil(L);
		}
	}
	else
	{
		luaL_error( L, "ERROR: builder.fetch(url) requires a string as the first argument" );
	}
	
	return 2;
}

int HTTPClient::download( lua_State *L )
{
	bool result = false;
	const char *url = NULL;
	const char *filename = NULL;
	String errorMesg;
	std::map<std::string, std::string> headers;
	
	if (lua_isstring( L, 1 ) && lua_isstring( L, 2 ))
	{
		url = lua_tostring( L, 1 );
		filename = lua_tostring( L, 2 );
	}
	
	if ( lua_istable( L, 3 ) )
	{
		lua_pushnil(L);  /* first key */
		while (lua_next(L, 3) != 0)
		{
			/* uses 'key' (at index -2) and 'value' (at index -1) */
			bool isValidArg = lua_isstring( L, -1 );
			if ( isValidArg )
			{
				std::string name =  lua_tostring( L, -2 );
				std::string value =  lua_tostring( L, -1 );
				headers.insert ( std::pair<std::string, std::string>(name, value) );
			}
			lua_pop( L, 1 );
		}
		
	}
	
	if (url != NULL && filename != NULL)
	{
		result = HTTPClient::HttpDownloadFile(url, filename, errorMesg, headers);
		
		if (! result)
		{
			lua_pushboolean(L, false);
			lua_pushstring(L, errorMesg);
		}
		else
		{
			lua_pushboolean(L, true);
			lua_pushnil(L);
		}
	}
	else
	{
		luaL_error( L, "ERROR: builder.download(url, filename) requires two string arguments" );
	}
	
	return 2;
}



void HTTPClient::registerFetcherModuleLoaders( lua_State *L )
{
	Lua::RegisterModuleLoader( L, "dkjson", Lua::Open< luaload_dkjson > );
	Lua::RegisterModuleLoader( L, "json", Lua::Open< luaload_json > );
	Lua::RegisterModuleLoader( L, "lpeg", luaopen_lpeg );
	Lua::RegisterModuleLoader( L, "lfs", luaopen_lfs );
	Lua::RegisterModuleLoader( L, "socket.core", luaopen_socket_core );
	Lua::RegisterModuleLoader( L, "socket", Lua::Open< luaload_luasocket_socket > );
	Lua::RegisterModuleLoader( L, "socket.ftp", Lua::Open< luaload_luasocket_ftp > );
	Lua::RegisterModuleLoader( L, "socket.headers", Lua::Open< luaload_luasocket_headers > );
	Lua::RegisterModuleLoader( L, "socket.http", Lua::Open< luaload_luasocket_http > );
	Lua::RegisterModuleLoader( L, "socket.url", Lua::Open< luaload_luasocket_url > );
	Lua::RegisterModuleLoader( L, "mime.core", luaopen_mime_core );
	Lua::RegisterModuleLoader( L, "mime", Lua::Open< luaload_luasocket_mime > );
	Lua::RegisterModuleLoader( L, "ltn12", Lua::Open< luaload_luasocket_ltn12 > );
	Lua::RegisterModuleLoader( L, "CoronaBuilderPluginCollector", Lua::Open< luaload_CoronaBuilderPluginCollector >);
	
	lua_pushcfunction(L, &HTTPClient::fetch);
	lua_setglobal ( L, "pluginCollector_fetch");

	lua_pushcfunction(L, &HTTPClient::download);
	lua_setglobal ( L, "pluginCollector_download");
}

}


