//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "CoronaLua.h"

class TestAppLibraryModule
{
	public:
		typedef TestAppLibraryModule Self;

	public:
		static const char *Name();
		static int Open( lua_State *L );
};
