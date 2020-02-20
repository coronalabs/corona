//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "CoronaAnalyticsLibrary.h"

#include "CoronaAssert.h"
#include "CoronaLibrary.h"

// ----------------------------------------------------------------------------

CORONA_EXPORT int CoronaPluginLuaLoad_analytics( lua_State * );
CORONA_EXPORT int CoronaPluginLuaLoad_CoronaProvider_analytics( lua_State * );

// ----------------------------------------------------------------------------

const char kProviderName[] = "CoronaProvider.analytics";

CORONA_EXPORT
int luaopen_analytics( lua_State *L )
{
	using namespace Corona;

	Corona::Lua::RegisterModuleLoader(
		L, kProviderName, Corona::Lua::Open< CoronaPluginLuaLoad_CoronaProvider_analytics > );

	lua_CFunction factory = Corona::Lua::Open< CoronaPluginLuaLoad_analytics >;
	int result = CoronaLibraryNewWithFactory( L, factory, NULL, NULL );

	return result;
}

// ----------------------------------------------------------------------------
