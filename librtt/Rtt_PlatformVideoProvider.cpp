//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_PlatformVideoProvider.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_PlatformData.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

void
PlatformVideoProvider::AddProperties( lua_State *L, void* userdata )
{
	VideoProviderResult* result = (VideoProviderResult*)userdata;

	if (result->SelectedVideoFileName)
	{
		lua_pushstring( L, result->SelectedVideoFileName);
		lua_setfield( L, -2, "url" );
	}
	else
	{
		// The file wasn't selected so the "selectedFile" should be nil.
	}

	if ( result->Duration > 0)
	{
		lua_pushnumber( L, result->Duration);
		lua_setfield( L, -2, "duration" );
	}
	else
	{
		// The duration is unknown so don't set it
	}

	if ( result->Size > 0)
	{
		lua_pushnumber( L, result->Size);
		lua_setfield( L, -2, "fileSize" );
	}
	else
	{
		// The size is unknown so don't set it
	}
	
	lua_pushboolean( L, result->SelectedVideoFileName ? 1 : 0 );
	lua_setfield( L, -2, "completed" );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

