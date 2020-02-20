//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"

#include "Rtt_LuaLibFacebook.h"

#include "Rtt_Lua.h"
#include "Rtt_Event.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaResource.h"
#include "Rtt_MPlatform.h"
#include "Rtt_PlatformFBConnect.h"
#include "Rtt_Runtime.h"
#include "CoronaLua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// facebook.login( appId, listener [, permissions] )
static int
login( lua_State *L )
{
	Runtime *runtime = LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime->Platform();

	PlatformFBConnect *connect = platform.GetFBConnect(); Rtt_ASSERT( connect );

	if ( LUA_TSTRING == lua_type( L, 1 ) )
	{
		const char *appId = lua_tostring( L, 1 );

		LuaResource *resource = NULL;
		if ( Lua::IsListener( L, 2, FBConnectBaseEvent::kName ) )
		{
			resource = Rtt_NEW(
				& platform.GetAllocator(),
				LuaResource( runtime->VMContext().LuaState(), 2 ) );
		}

		const char **permissions = NULL;
		int numPermissions = 0;
		if ( lua_istable( L, 3 ) )
		{
			numPermissions = (int) lua_objlen( L, 3 );
			permissions = (const char **)Rtt_MALLOC( LuaContext::GetAllocator( L ), sizeof( char*) * numPermissions );

			for ( int i = 0; i < numPermissions; i++ )
			{
				// Lua arrays are 1-based, so add 1 to index passed to lua_rawgeti()
				lua_rawgeti( L, 3, i + 1 ); // push permissions[i]

				const char *value = lua_tostring( L, -1 );
				permissions[i] = value;
				lua_pop( L, 1 );
			}
		}

		if ( appId && resource )
		{
			connect->SetListener( resource );
			connect->Login( appId, permissions, numPermissions );
		}

		if ( permissions )
		{
			Rtt_FREE( permissions );
		}
	}
	else
	{
		CoronaLuaError(L, "facebook.login() first argument should be a string");
	}


	return 0;
}

// facebook.logout()
static int
logout( lua_State *L )
{
	Runtime *runtime = LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime->Platform();

	PlatformFBConnect *connect = platform.GetFBConnect(); Rtt_ASSERT( connect );
	connect->Logout();

	return 0;
}

// facebook.request( path [, httpMethod, params] )
static int
request( lua_State *L )
{
	Runtime *runtime = LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime->Platform();

	PlatformFBConnect *connect = platform.GetFBConnect(); Rtt_ASSERT( connect );

	const char *path = luaL_checkstring( L, 1 );
	const char *httpMethod = ( lua_isstring( L, 2 ) ? lua_tostring( L, 2 ) : "GET" );
	connect->Request( L, path, httpMethod, 3 );

	return 0;
}

// facebook.requestOld( method [, httpMethod, params] )
static int
requestOld( lua_State *L )
{
	Runtime *runtime = LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime->Platform();

	PlatformFBConnect *connect = platform.GetFBConnect(); Rtt_ASSERT( connect );
	const char *method = luaL_checkstring( L, 1 );
	const char *httpMethod = ( lua_isstring( L, 2 ) ? lua_tostring( L, 2 ) : "GET" );
	connect->RequestOld( L, method, httpMethod, 3 );

	return 0;
}

// OLD/DEPRECATED: facebook.showDialog( { action=value, params={} } )
// NEW: facebook.showDialog( action [, params] )
static int
showDialog( lua_State *L )
{
	Runtime *runtime = LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime->Platform();

	PlatformFBConnect *connect = platform.GetFBConnect(); Rtt_ASSERT( connect );
	connect->ShowDialog( L, 1 );

	return 0;
}

static int
publishInstall( lua_State *L )
{
	Runtime *runtime = LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime->Platform();

	PlatformFBConnect *connect = platform.GetFBConnect(); Rtt_ASSERT( connect );

	if ( LUA_TSTRING == lua_type( L, 1 ) )
	{
		const char *appId = lua_tostring( L, 1 );
		if ( appId )
		{
			connect->PublishInstall( appId );
		}
	}
	else
	{
		CoronaLuaError(L, "facebook.publicInstall() first argument should be a string");
	}

	return 0;
}
// ----------------------------------------------------------------------------

int
LuaLibFacebook::Open( lua_State *L )
{
	const luaL_Reg kVTable[] =
	{
		{ "login", login },
		{ "logout", logout },
		{ "request", request },
		{ "requestOld", requestOld },
		{ "showDialog", showDialog },
		{ "publishInstall", publishInstall },

		{ NULL, NULL }
	};

	luaL_register( L, "facebook", kVTable ); // leave "facebook" on top of stack

	return 1;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

