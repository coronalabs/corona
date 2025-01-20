//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_BuildParams.h"
#include "Rtt_LuaLibBuilder.h"

#include "Rtt_AppPackagerFactory.h"

extern "C" {
#include "lfs.h"
}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static int
GetParamsTableRef( lua_State *L, const char *path, BuildParams::Format format, int argc, const char *argv[] )
{
	int result = LUA_NOREF;

	switch ( format )
	{
		case BuildParams::kLuaFormat:
		{
			int base = lua_gettop( L );
			for(int i=0; i<argc; i++)
			{
				lua_pushstring(L, argv[i]);
			}
			Lua::DoFile( L, path, argc, false );
			if ( lua_istable( L, base + 1 ) )
			{
				result = luaL_ref( L, LUA_REGISTRYINDEX );
			}
			else
			{
				lua_settop( L, base );
			}
		}
		break;

		case BuildParams::kJsonFormat:
		{
			int base = lua_gettop(L);

			// Call: CoronaBuilderLoadJSONParams(path)
			lua_getglobal( L, "CoronaBuilderLoadJSONParams" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
			lua_pushstring( L, path );


			if (Lua::DoCall( L, 1, 3 ) == 0)
			{
				// Function returns 3 values which have to handled in reverse order so we have to move the table to top of stack
				lua_pushvalue(L, -3);
				result = luaL_ref( L, LUA_REGISTRYINDEX );
			}

			if (result == LUA_REFNIL)
			{
				// Error message is 3rd value on stack
				const char *errorMsg = lua_tostring( L, 3 );
				Rtt_TRACE( ( "ERROR: %s\n", errorMsg ) );
			}

			lua_settop( L, base );
		}
		break;

		default:
		{
			Rtt_ASSERT_NOT_REACHED();
		}
		break;
	}

	return result;
}

// ----------------------------------------------------------------------------

BuildParams::BuildParams( lua_State *L, const char *path, Format format, int argc, const char *argv[] )
:	fL( L ),
	fRef( LUA_NOREF )
{
	fRef = GetParamsTableRef( L, path, format, argc, argv );
}

BuildParams::~BuildParams()
{
	luaL_unref( fL, LUA_REGISTRYINDEX, fRef );
}

bool
BuildParams::IsValid() const
{
	return LUA_NOREF != fRef && LUA_REFNIL != fRef;
}

static const char kPlatformKey[] = "platform";

TargetDevice::Platform
BuildParams::GetTargetPlatform() const
{
	lua_State *L = fL;
	int index = fRef;
	TargetDevice::Platform targetPlatform = TargetDevice::kUnknownPlatform;

	lua_rawgeti( L, LUA_REGISTRYINDEX, index );
	lua_getfield( L, -1, kPlatformKey );
	const char *platform = lua_tostring( L, -1 );

	if (platform != NULL)
	{
		targetPlatform = TargetDevice::PlatformForTag( platform );

		// Try old-style platform names if the platform isn't a new-style platform tag
		if (targetPlatform == TargetDevice::kUnknownPlatform)
		{
			targetPlatform = TargetDevice::PlatformForString( platform );
		}

		if (targetPlatform == TargetDevice::kUnknownPlatform)
		{
			fprintf( stderr, "ERROR: unknown platform '%s'\n", platform );
		}
	}
	else
	{
		fprintf( stderr, "WARNING: missing 'platform' setting; defaulting to 'iOS'\n" );

		targetPlatform = TargetDevice::kIPhonePlatform;
	}

	lua_pop( L, 2 );

	return targetPlatform;
}


AppPackagerParams*
BuildParams::CreatePackagerParams( const AppPackagerFactory& factory, TargetDevice::Platform targetPlatform ) const
{
	lua_State *L = fL;
	int index = fRef;

	lua_rawgeti( L, LUA_REGISTRYINDEX, index );

	AppPackagerParams *result = factory.CreatePackagerParams( L, lua_gettop( L ), targetPlatform );

	return result;
}

PlatformAppPackager*
BuildParams::CreatePackager( const AppPackagerFactory& factory, TargetDevice::Platform targetPlatform ) const
{
	lua_State *L = fL;
	int index = fRef;

	PlatformAppPackager *result = factory.CreatePackager( L, index, targetPlatform );

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

