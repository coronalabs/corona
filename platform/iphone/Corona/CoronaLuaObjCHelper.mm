//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "CoronaLuaObjCHelper.h"
#include "Rtt_LuaContext.h"
#include "Rtt_ApplePlatform.h"
// ----------------------------------------------------------------------------

CORONA_API NSDictionary *
CoronaLuaCreateDictionary( lua_State *L, int index )
{
	return (NSDictionary*)Rtt::ApplePlatform::CreateDictionary( L, index );
}

CORONA_API int
CoronaLuaPushValue( lua_State *L, id value)
{
	return (int)Rtt::ApplePlatform::Push( L, value );
}
