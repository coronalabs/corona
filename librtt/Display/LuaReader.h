//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
