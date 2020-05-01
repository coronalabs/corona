//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_LuaLibBuilder.h"

#include "Rtt_Lua.h"
#include "Rtt_CoronaBuilder.h"
#include "Rtt_Event.h"
#include "Rtt_Runtime.h"
#include "Rtt_HTTPClient.h"

#include <map>
#include <string>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------


#if 0
static int
getPreference( lua_State *L )
{
    const char *prefName = NULL;

    if ( lua_isstring( L, 1 ) )
    {
        prefName = lua_tostring( L, 1 );
    }
    else
    {
        luaL_error( L, "ERROR: builder.getPreference(name) requires a string as the first argument" );
    }

    const char *prefValue = builder->GetPreference(prefName);
    
    lua_pushstring( L, prefValue );
    
    return 1;
}

static int
setPreference( lua_State *L )
{
    const char *prefName = lua_tostring( L, 1);
    const char *prefValue = NULL;
    
    if ( lua_isstring( L, 1 ) )
    {
        prefName = lua_tostring( L, 1 );
    }
    else
    {
        luaL_error( L, "ERROR: builder.setPreference(name, value) requires a string as the first argument" );
    }
    
    if ( lua_isstring( L, 2 ) )
    {
        prefValue = lua_tostring( L, 2 );
    }
    else
    {
        luaL_error( L, "ERROR: builder.setPreference(name, value) requires a string as the second argument" );
    }
    
    builder->SetPreference(prefName, prefValue);
    
    return 0;
}
#endif

// ----------------------------------------------------------------------------

const char LuaLibBuilder::kName[] = "builder";

int
LuaLibBuilder::Open( lua_State *L )
{
	const luaL_Reg kVTable[] =
	{
		{ "fetch", &HTTPClient::fetch },
		{ "download", &HTTPClient::download },
#if 0
		{ "getPreference", getPreference },
		{ "setPreference", setPreference },
#endif

		{ NULL, NULL }
	};

	// The controlling instance of CoronaBuilder is an upvalue
	Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
	void *context = lua_touserdata( L, lua_upvalueindex( 1 ) );
	lua_pushlightuserdata( L, context );

	luaL_openlib( L, kName, kVTable, 1 ); // leave "builder" on top of stack

	return 1;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

