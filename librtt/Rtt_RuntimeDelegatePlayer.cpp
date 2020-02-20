//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_RuntimeDelegatePlayer.h"

#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibSocket.h"
#include "Rtt_Runtime.h"

Rtt_EXPORT_BEGIN
	#define lua_c

	#ifdef Rtt_LUA_LFS
		#include "lfs.h"
	#endif

Rtt_EXPORT_END


#include "CoronaLuaLibrary.h"

extern "C" {
// #include "luasocket.h"
	int luaopen_socket_core(lua_State *L);
// #include "mime.h"
	int luaopen_mime_core(lua_State *L);
}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

RuntimeDelegatePlayer::~RuntimeDelegatePlayer()
{
}

void
RuntimeDelegatePlayer::DidInitLuaLibraries( const Runtime& sender ) const
{
	PreloadLibraries( sender );
}

void
RuntimeDelegatePlayer::PreloadLibraries( const Runtime& sender )
{
	// Static Modules
	// NOTE: This is where OPTIONAL modules should go that need to be preloaded.
	const luaL_Reg luaStaticModules[] =
	{
#ifdef Rtt_LUA_LFS
		{ "lfs", luaopen_lfs },
#endif
#ifdef Rtt_NETWORK
		{ "socket.core", luaopen_socket_core },
		{ "socket", Lua::Open< luaload_luasocket_socket >  },
		{ "socket.ftp", Lua::Open< luaload_luasocket_ftp > },
		{ "socket.headers", Lua::Open< luaload_luasocket_headers > },
		{ "socket.http", Lua::Open< luaload_luasocket_http > },
		{ "socket.mbox", Lua::Open< luaload_luasocket_mbox > },
		{ "socket.smtp", Lua::Open< luaload_luasocket_smtp > },
		{ "socket.tp", Lua::Open< luaload_luasocket_tp > },
		{ "socket.url", Lua::Open< luaload_luasocket_url > },
		{ "mime.core", luaopen_mime_core },
		{ "mime", Lua::Open< luaload_luasocket_mime > },
		{ "ltn12", Lua::Open< luaload_luasocket_ltn12 > },
#endif

		{NULL, NULL}
	};

	lua_State *L = sender.VMContext().L();
	Rtt_LUA_STACK_GUARD( L );

	// Preload modules to eliminate dependency
	lua_getfield( L, LUA_GLOBALSINDEX, "package" );
	lua_getfield( L, -1, "preload" );
	for ( const luaL_Reg *lib = luaStaticModules; lib->func; lib++ )
	{
		lua_pushcfunction( L, lib->func );
		lua_setfield( L, -2, lib->name );
	}

	lua_pop( L, 2 );

	Corona::LuaLibrary::InitializeMetatable( L );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
