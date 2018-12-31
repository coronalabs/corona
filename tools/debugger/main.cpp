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

#include "Rtt_Lua.h"
#include "Rtt_LuaLibSocket.h"

#include <stdlib.h>
#include <libgen.h>

#include "Rtt_Lua.h"

typedef struct _LaunchArgs
{
	int argc;
	const char **argv;
	lua_CFunction loader;
	lua_CFunction openlib;
}
LaunchArgs;


int
static LuaOpenModule( lua_State *L, lua_CFunction loader )
{
	int arg = lua_gettop(L);
	(*loader)( L );
	lua_insert(L,1);
	lua_call(L,arg,1);
	return 1;
}

template < int (*F)( lua_State * ) >
static int
LuaOpen( lua_State *L )
{
	return LuaOpenModule( L, F );
}


static int
pmain( lua_State *L )
{
	LaunchArgs* launchArgs = (LaunchArgs*)lua_touserdata(L,1);
	lua_gc(L,LUA_GCSTOP,0);
	luaL_openlibs(L);
	if ( launchArgs->openlib )
	{
		(*launchArgs->openlib)( L );
	}
	lua_gc(L,LUA_GCRESTART,0);

	(*launchArgs->loader)( L );

	const char **argv = launchArgs->argv;
	int i;
	for (i=1; argv[i]; i++) ; /* count */
	lua_createtable(L,i-1,1);
	for (i=0; argv[i]; i++)
	{
		lua_pushstring(L,argv[i]);
		lua_rawseti(L,-2,i);
	}
	lua_setglobal(L,"arg");
	luaL_checkstack(L,i-1,"too many arguments to script");
	for (i=1; argv[i]; i++)
	{
		lua_pushstring(L,argv[i]);
	}
	lua_call(L,i-1,0);
	return 0;
}

static void
fatal( const char* progname, const char* message )
{
	#ifdef _WIN32
		MessageBox(NULL,message,progname,MB_ICONERROR | MB_OK);
	#else
		fprintf(stderr,"%s: %s\n",progname,message);
	#endif
	exit( EXIT_FAILURE );
}

static int
run( int argc, const char *argv[], lua_CFunction loader, lua_CFunction openlib )
{
	lua_State *L = lua_open();
	if ( ! L ) { fatal(argv[0],"not enough memory for state"); }

	LaunchArgs launchArgs = { argc, argv, loader, openlib };
	if ( lua_cpcall( L, pmain, & launchArgs ) ) { fatal(argv[0],lua_tostring(L,-1)); }
	lua_close(L);

	return 0;
}

// ----------------------------------------------------------------------------

#ifdef Rtt_NETWORK

Rtt_EXPORT_BEGIN
	#include "luasocket.h"
Rtt_EXPORT_END

static int
initialize_luasocket( lua_State *L )
{
	using namespace Rtt;

	// Static Modules
	const luaL_Reg luaStaticModules[] =
	{
		{ "socket.core", luaopen_socket_core },
		{ "socket", LuaOpen< luaload_luasocket_socket > },

		{NULL, NULL}
	};

	// Preload modules to eliminate dependency
	lua_getfield( L, LUA_GLOBALSINDEX, "package" );
	lua_getfield( L, -1, "preload" );
	for ( const luaL_Reg *lib = luaStaticModules; lib->func; lib++ )
	{
		lua_pushcfunction( L, lib->func );
		lua_setfield( L, -2, lib->name );
	}
	lua_pop( L, 2 );

	return 0;
}

#endif // Rtt_NETWORK

// ----------------------------------------------------------------------------

namespace Rtt
{

// init.lua is pre-compiled into bytecodes and then placed in a byte array
// constant in init.cpp. The following function defined in init.cpp loads
// the bytecodes via luaL_loadbuffer. The .cpp file is dynamically generated.
int luaload_controller(lua_State *L);

} // Rtt

int main( int argc, const char *argv[] )
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);
    
	int result = EXIT_FAILURE;

    result = run( argc, argv, & Rtt::luaload_controller, & initialize_luasocket );
	
	return result;
}
