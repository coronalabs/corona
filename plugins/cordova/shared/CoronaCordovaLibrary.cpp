//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "CoronaCordovaLibrary.h"

#include "CoronaLibrary.h"

// ----------------------------------------------------------------------------

CORONA_EXPORT int CoronaPluginLuaLoad_cordova( lua_State * );

namespace Corona
{

// ----------------------------------------------------------------------------

const char CordovaLibrary::kName[] = "cordova";

lua_CFunction
CordovaLibrary::GetFactory() const
{
	return Corona::Lua::Open< CoronaPluginLuaLoad_cordova >;
}

// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

