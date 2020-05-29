//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "CoronaAdsLibrary.h"

#include "CoronaAssert.h"
#include "CoronaLibrary.h"

// ----------------------------------------------------------------------------

CORONA_EXPORT int CoronaPluginLuaLoad_ads( lua_State * );
CORONA_EXPORT int CoronaPluginLuaLoad_CoronaProvider_ads( lua_State * );

// ----------------------------------------------------------------------------

static const char kProviderName[] = "CoronaProvider.ads";

CORONA_EXPORT
int luaopen_ads( lua_State *L )
{
	using namespace Corona;

	Corona::Lua::RegisterModuleLoader(
		L, kProviderName, Corona::Lua::Open< CoronaPluginLuaLoad_CoronaProvider_ads > );

	lua_CFunction factory = Corona::Lua::Open< CoronaPluginLuaLoad_ads >;
	int result = CoronaLibraryNewWithFactory( L, factory, NULL, NULL );

	return result;
}

// ----------------------------------------------------------------------------
