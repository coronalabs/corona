//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "TestAppLibraryModule.h"

#import "CoronaRuntime.h"
#import <UIKit/UIKit.h>

// ----------------------------------------------------------------------------

static int
printRuntime( lua_State *L )
{
	id<CoronaRuntime> runtime = (id<CoronaRuntime>)lua_touserdata( L, lua_upvalueindex( 1 ) );

	NSLog( @"%@", runtime );

	return 0;
}

// ----------------------------------------------------------------------------

const char *
TestAppLibraryModule::Name()
{
	static const char sName[] = "testapp";
	return sName;
}

int
TestAppLibraryModule::Open( lua_State *L )
{
	const luaL_Reg kVTable[] =
	{
		{ "printRuntime", printRuntime },

		{ NULL, NULL }
	};

	// Ensure upvalue is available to library
	void *context = lua_touserdata( L, lua_upvalueindex( 1 ) );
	lua_pushlightuserdata( L, context );

	luaL_openlib( L, Name(), kVTable, 1 ); // leave "mylibrary" on top of stack

	return 1;
}

// ----------------------------------------------------------------------------

