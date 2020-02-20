//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __RTT_LUALIBSOCKET__
#define __RTT_LUALIBSOCKET__

#ifdef Rtt_NETWORK

#include "Core/Rtt_Macros.h"
Rtt_EXPORT_BEGIN
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
Rtt_EXPORT_END

#ifndef LUAOPEN_API 
#define LUAOPEN_API 
#endif

namespace Rtt
{

LUAOPEN_API int luaload_luasocket_ftp(lua_State *L);
LUAOPEN_API int luaload_luasocket_headers(lua_State *L);
LUAOPEN_API int luaload_luasocket_http(lua_State *L);
LUAOPEN_API int luaload_luasocket_ltn12(lua_State *L);
LUAOPEN_API int luaload_luasocket_mbox(lua_State *L);
LUAOPEN_API int luaload_luasocket_mime(lua_State *L);
LUAOPEN_API int luaload_luasocket_smtp(lua_State *L);
LUAOPEN_API int luaload_luasocket_socket(lua_State *L);
LUAOPEN_API int luaload_luasocket_tp(lua_State *L);
LUAOPEN_API int luaload_luasocket_url(lua_State *L);
}

#endif // Rtt_NETWORK

#endif // __RTT_LUALIBSOCKET__
