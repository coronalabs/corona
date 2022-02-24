//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_EmscriptenFBConnect.h"
#include "Rtt_Lua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#pragma region Constructors/Destructors
EmscriptenFBConnect::EmscriptenFBConnect()
:	Super()
{
}

#pragma endregion


#pragma region Public Member Functions
void EmscriptenFBConnect::Login(const char *appId, const char *permissions[], int numPermissions) const
{
}

void EmscriptenFBConnect::Logout() const
{
}

void EmscriptenFBConnect::Request(lua_State *L, const char *path, const char *httpMethod, int index) const
{
}

void EmscriptenFBConnect::RequestOld(lua_State *L, const char *method, const char *httpMethod, int index) const
{
}

void EmscriptenFBConnect::ShowDialog(lua_State *L, int index) const
{
	const char *str = NULL;
	if (lua_isstring(L, 1))
	{
		str = lua_tostring(L, 1);
		if (str)
		{
		}
	}
	else if (lua_istable(L, 1))
	{
		lua_getfield(L, index, "action");
		str = lua_tostring(L, -1);
		lua_pop(L, 1);
		Rtt_PRINT(("WARNING: facebook.showDialog( { action= } ) has been deprecated in favor of facebook.showDialog( action [, params] )\n"));

		if (str)
		{
		}
	}
	else
	{
		Rtt_PRINT(("WARNING: Invalid parameters passed to facebook.showDialog( action [, params] )\n"));
	}
}

void EmscriptenFBConnect::PublishInstall(const char *appId) const
{
}

#pragma endregion

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
