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

#import "MyCoronaDelegate.h"

#import "CoronaRuntime.h"
#import "CoronaLua.h"
#import "MyLibraryModule.h"

// Sample custom Lua error handler
// Register it via: Corona::Lua::SetErrorHandler()
static int
MyTraceback( lua_State* L )
{
	if (!lua_isstring(L, 1))  // 'message' not a string?
		return 1;  // keep it intact

	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return 1;
	}
	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		return 1;
	}
	lua_pushvalue(L, 1);  // pass error message
	lua_pushinteger(L, 1);  // skip this function and traceback
	lua_call(L, 2, 1);  // call debug.traceback

	if ( ! lua_gethook( L ) ) // Don't interfere with Lua debugger hook
	{
		// Log result of calling debug.traceback()
		NSLog( @"[LUA ERROR]: %s", lua_tostring( L, -1 ) );
	}

	return 1;
}

@implementation MyCoronaDelegate

- (void)willLoadMain:(id<CoronaRuntime>)runtime
{
	// Register modules before execution of main.lua
	const luaL_Reg moduleLoaders[] =
	{
		// Each module is a pair: (name, C-function loader)
		{ MyLibraryModule::Name(), MyLibraryModule::Open },
		
		// Termination
		{ NULL, NULL }
	};

	lua_State *L = runtime.L;

	// Make runtime available to each module
	lua_pushlightuserdata( L, runtime );
	Corona::Lua::RegisterModuleLoaders( L, moduleLoaders, 1 );

	// CUSTOM ERROR HANDLER
	// Uncomment the following line to set MyTraceback as a custom error handler:
	// Corona::Lua::SetErrorHandler( MyTraceback );
}

- (void)didLoadMain:(id<CoronaRuntime>)runtime
{
	lua_State *L = runtime.L;

	// DISPATCH CUSTOM EVENT
	// This does the equivalent of the following Lua code
	// where we have created a special 'delegate' event type
	// The main.lua file registers a listener for 'delegate' event:
	//		local event = { name = "delegate" }
	//		Runtime:dispatchEvent( event )

	// Create 'delegate' event
	const char kNameKey[] = "name";
	const char kValueKey[] = "delegate";
	lua_newtable( L );
	lua_pushstring( L, kValueKey );		// All events are Lua tables
	lua_setfield( L, -2, kNameKey );	// that have a 'name' property

	Corona::Lua::RuntimeDispatchEvent( L, -1 );

	lua_pop( L, 1 ); // pop event
}

@end
