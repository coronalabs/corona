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

#include "Core/Rtt_Build.h"

#include "Rtt_LuaLibBuilder.h"

#include "Rtt_Lua.h"
#include "Rtt_CoronaBuilder.h"
#include "Rtt_Event.h"
#include "Rtt_Runtime.h"

#include <map>
#include <string>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

//
// local result = builder.fetch(url)
//
static int
fetch( lua_State *L )
{
	Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
	CoronaBuilder *builder = (CoronaBuilder *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	const MPlatform& platform = builder->GetPlatform();
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

		if (! platform.HttpDownload(url, result, errorMesg, headers))
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

//
// builder.download(url, filename, headers)
//
static int
download( lua_State *L )
{
	Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
	CoronaBuilder *builder = (CoronaBuilder *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	const MPlatform& platform = builder->GetPlatform();
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
		result = platform.HttpDownloadFile(url, filename, errorMesg, headers);

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
		{ "fetch", fetch },
		{ "download", download },
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

