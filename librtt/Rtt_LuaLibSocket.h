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
