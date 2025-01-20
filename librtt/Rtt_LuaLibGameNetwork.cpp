//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------

#include "Core/Rtt_Build.h"

#include "Rtt_LuaLibGameNetwork.h"
#include "CoronaLua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static int
openfeint( lua_State *L )
{
	CoronaLuaError( L, "OpenFeint is no longer available");
	return 0;
}

int
LuaLibOpenFeint::Open( lua_State *L )
{
	const luaL_Reg kVTable[] =
	{
		{ "init", openfeint },
		{ "launchDashboard", openfeint },
		{ "unlockAchievement", openfeint },
		{ "launchDashboardWithListLeaderboardsPage", openfeint },
		{ "launchDashboardWithChallengesPage", openfeint },
		{ "launchDashboardWithAchievementsPage", openfeint },
		{ "launchDashboardWithFindFriendsPage", openfeint },
		{ "launchDashboardWithWhosPlayingPage", openfeint },
		{ "launchDashboardWithHighscorePage", openfeint },
		{ "setHighScore", openfeint },
		{ "uploadBlob", openfeint },
		{ "downloadBlob", openfeint },
		
		{ NULL, NULL }
	};
	
	luaL_register( L, "openfeint", kVTable );

	return 1;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

