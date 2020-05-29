//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_AndroidSupportTools.h"
#include "Core/Rtt_Assert.h"

// Although this is a .c file, Windows still needs the 'extern "C"'
#ifdef __cplusplus
extern "C"
{
#endif
	#include "lauxlib.h"
	#include "lualib.h"
	#include "lfs.h"
#ifdef __cplusplus
}
#endif


// Although this is a .c file, Windows still needs the 'extern "C"'
#ifdef __cplusplus
extern "C"
{
#endif
// Not declared in lpeg.h, forward declare it here so we can use it.
extern int luaopen_lpeg (lua_State *L);

// Defined in librtt/Rtt_Lua.cpp to bridge .c to .cpp
extern int CoronaLuaLoad_re (lua_State *L);
#ifdef __cplusplus
}
#endif


lua_State* Rtt_AndroidSupportTools_NewLuaState(const char* filepath)
{
	int ret_val;
	lua_State* L = luaL_newstate();
	if(NULL == L)
	{
		return NULL;
	}
	
	luaL_openlibs(L);
	
	// Open LPeg
	lua_pushcfunction(L, luaopen_lpeg);
	lua_pushstring(L, "lpeg");
	lua_call(L, 1, 0);
    
	
	// Open re
    CoronaLuaLoad_re( L );
    
	
	// Open LuaFileSystem
	lua_pushcfunction(L, luaopen_lfs);
	lua_pushstring(L, "lfs");
	lua_call(L, 1, 0);
	
	
	// Open support file
	ret_val = luaL_loadfile(L, filepath);
	if(0 != ret_val)
	{
		lua_pop(L, 1);
		lua_close(L);
		return NULL;
	}
	
	ret_val = lua_pcall(L,0,0,0);
	if(0 != ret_val)
	{
		Rtt_TRACE_SIM( ( "Error loading %s: %s\n", filepath, lua_tostring(L, -1) ) );
		lua_pop(L, 1);
		lua_close(L);
		return NULL;
	}

	return L;
}

void Rtt_AndroidSupportTools_CloseLuaState(lua_State* L)
{
	if(NULL != L)
	{
		lua_close(L);
	}
}

bool Rtt_AndroidSupportTools_IsAndroidPackageName(lua_State* L, const char* packagename)
{
	int ret_val;
	bool isvalid;

	Rtt_ASSERT(NULL != L);

	lua_getglobal(L, "ValidateAndroidPackageName");
	lua_pushstring(L, packagename);
	ret_val = lua_pcall(L, 1, 1, 0);
	if(0 != ret_val)
	{
		Rtt_TRACE_SIM( ( "Error calling ValidateAndroidPackageName: %s\n", lua_tostring(L, -1) ) );
		lua_pop(L, 1);
		lua_close(L);
		return false;
	}
	
	isvalid = lua_toboolean(L, -1);
	lua_pop(L, 1);
	
	return isvalid;
}

bool Rtt_AndroidSupportTools_ValidateFilesForJavaKeywordsInDirectory(lua_State* L, const char* rootdir)
{
	// TODO: Callers expect 2 extra args on the stack.
	// Fix callers???
	lua_pushnil( L );
	lua_pushnil( L );
	
	return true;
}

bool Rtt_AndroidSupportTools_ValidateFilesForDuplicateBaseNamesInDirectory(lua_State* L, const char* rootdir)
{
	// TODO: Callers expect 2 extra args on the stack.
	// Fix callers???
	lua_pushnil( L );
	lua_pushnil( L );
	
	return true;
}


bool Rtt_AndroidSupportTools_ValidateResFilesForForbiddenSubdirectories(lua_State* L, const char* rootdir)
{
	// TODO: Callers expect 2 extra args on the stack.
	// Fix callers???
	lua_pushnil( L );
	lua_pushnil( L );
	
	return true;
}

bool Rtt_CommonSupportTools_ValidateLuaFilesForForbiddenSubdirectories(lua_State* L, const char* rootdir)
{
	// TODO: Callers expect 2 extra args on the stack.
	// Fix callers???
	lua_pushnil( L );
	lua_pushnil( L );

	return true;
}

bool Rtt_CommonSupportTools_ValidateNameForRestrictedASCIICharacters(lua_State* L, const char* name)
{
	int ret_val;
	bool isvalid;
	
	Rtt_ASSERT(NULL != L);
	
	lua_getglobal(L, "ValidateRestrictedASCIIName");
	lua_pushstring(L, name);
	ret_val = lua_pcall(L, 1, 1, 0);
	if(0 != ret_val)
	{
		Rtt_TRACE_SIM( ( "Error calling ValidateRestrictedASCIIName: %s\n", lua_tostring(L, -1) ) );
		lua_pop(L, 1);
		lua_close(L);
		return false;
	}
	
	isvalid = lua_toboolean(L, -1);
	lua_pop(L, 1);
	
	return isvalid;
}
