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

#include "Rtt_LuaLibFlurry.h"

#include "Rtt_LuaContext.h"
#include "Rtt_MPlatform.h"
#include "Rtt_Runtime.h"
#include "CoronaLua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/**
 * Lua to C thunk to initialize Flurry.
 *
 * Lua:
 *		analytics.init( "applicationKey" )
 *
 * These values are obtained from Flurry
 *
 * Should be called as early as possible.
 */
static int
init( lua_State *L )
{
#ifdef Rtt_FLURRY
	const char * applicationKey = luaL_checkstring( L, 1 );

	Runtime *runtime = LuaContext::GetRuntime( L );
	const MPlatform & platform = runtime->Platform();

	platform.FlurryInit( applicationKey );

#else
	CoronaLuaWarning(L, "analytics.init() is not supported in this configuration");
#endif
	
	return 0;
}

// analytics.logEvent( "eventId" )
static int
flurryEvent( lua_State *L )
{
#ifdef Rtt_FLURRY
	const char * eventId = luaL_checkstring( L, 1 );
	const MPlatform & platform = LuaContext::GetRuntime( L )->Platform();

	platform.FlurryEvent( eventId );
#else
	CoronaLuaWarning(L, "analytics.logEvent() is not supported in this configuration");
#endif

	return 0;
}

int
LuaLibFlurry::Open( lua_State *L )
{
	const luaL_Reg kVTable[] =
	{
		{ "init", init },
		{ "logEvent", flurryEvent },
		
		{ NULL, NULL }
	};
	
	luaL_register( L, "analytics", kVTable );

#if !defined( Rtt_FLURRY )
	CoronaLuaWarning(L, "analytics is not supported in this configuration");
#endif

	return 1;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
