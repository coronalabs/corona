//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#ifdef Rtt_SQLITE

#include "Rtt_LuaLibSQLite.h"

#include "Rtt_LuaContext.h"

//#include <sqlite3.h>

// ----------------------------------------------------------------------------

Rtt_EXPORT int luaopen_lsqlite3(lua_State *L);

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------
	
int
LuaLibSQLite::Open( lua_State *L )
{
	int result = luaopen_lsqlite3(L);

	return result;
}

// ----------------------------------------------------------------------------
	
} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_SQLITE

