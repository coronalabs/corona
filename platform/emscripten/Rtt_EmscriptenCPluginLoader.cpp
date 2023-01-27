//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_EmscriptenCPluginLoader.h"
#include "Corona/CoronaLua.h"
#include "Core/Rtt_String.h"

#if defined(EMSCRIPTEN)

#include "emscripten/emscripten.h"

int Rtt::EmscriptenCPluginLoader::LualoadCInvoker(lua_State *L)
{
	const char* packageName = luaL_checkstring(L, 1);
	packageName = luaL_gsub( L, packageName, ".", "_" );
	lua_remove(L, 1);

	return EM_ASM_INT({
		return Module.ccall('luaopen_'+UTF8ToString($0), 'number', ['number'],[$1])
	}, packageName, L );
}

int Rtt::EmscriptenCPluginLoader::Loader(lua_State *L)
{
	// Fetch the package name that the Lua require() function is trying to load.
	const char* packageName = luaL_checkstring(L, 1);
	packageName = luaL_gsub( L, packageName, ".", "_" );
	lua_remove(L, 1);

	bool hasSymbol = EM_ASM_INT({
		return Module.hasOwnProperty('_luaopen_'+UTF8ToString($0));
	}, packageName);

	if(hasSymbol)
	{
		lua_pushcfunction(L, &EmscriptenCPluginLoader::LualoadCInvoker);
		return 1;
	}
	lua_pushnil(L);
	return 1;
}

#else
int Rtt::EmscriptenCPluginLoader::Loader(lua_State *L) { return 0; }
int Rtt::EmscriptenCPluginLoader::LualoadCInvoker(lua_State *L) { return 0; }
#endif
