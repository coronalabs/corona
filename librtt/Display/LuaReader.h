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

#ifndef __Plugin__LuaReader__
#define __Plugin__LuaReader__

#include "CoronaLua.h"
#include "LuaMap.h"

namespace Rtt
{
	class LuaReader
	{
	public:
		
		static const char *GetPathForFileBaseDir(lua_State *L, void *baseDir, const char *file)
		{
			const char *path = NULL;
			int	numParams = 2;
			int topIndex = lua_gettop(L);
			{
				lua_getfield(L, LUA_GLOBALSINDEX, "system");
				lua_getfield(L, -1, "pathForFile");
				
				lua_pushstring( L, file );  // Push argument #1
				lua_pushlightuserdata( L, baseDir ); // Push argument #2
				
				Corona::Lua::DoCall( L, numParams, 1 );
				path = lua_tostring( L, -1 );
			}
			lua_settop(L,topIndex);
			
			return path;
		}
		
		static const char *GetPathForFile(lua_State *L, int index, const char *file)
		{
			
			void *baseDir = lua_touserdata( L, index );
			
			const char *path = NULL;
			int	numParams = 2;
			int topIndex = lua_gettop(L);
			{
				lua_getfield(L, LUA_GLOBALSINDEX, "system");
				lua_getfield(L, -1, "pathForFile");
				
				lua_pushstring( L, file );  // Push argument #1
				lua_pushlightuserdata( L, baseDir ); // Push argument #2
				
				Corona::Lua::DoCall( L, numParams, 1 );
				path = lua_tostring( L, -1 );
			}
			lua_settop(L,topIndex);
			
			return path;
		}
		
		static std::string GetString(lua_State *L, int index)
		{
			const char *val = luaL_checkstring( L, index );
			
			std::string result = std::string(val);
			return result;
			
		};
		
		static LMap GetDataMap(lua_State *L, int index)
		{
			
			LMap map(L,index);
			return map;
		};
		
		static LVector GetVec(lua_State *L, int index)
		{
			
			LVector vec(L,index);
			return vec;
		};
		
	};
	
}

#endif /* defined(__Plugin__LuaReader__) */
