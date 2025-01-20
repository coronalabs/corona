//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "CoronaLuaLibrary.h"

#include "CoronaLuaLibraryMetadata.h"

#include "CoronaAssert.h"
#include "CoronaLibrary.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Corona
{

// ----------------------------------------------------------------------------

LuaLibrary::LuaLibrary()
:	fMetadata( NULL )
{
}

LuaLibrary::~LuaLibrary()
{
	delete fMetadata;
}

bool
LuaLibrary::Initialize( lua_State *L, void *platformContext )
{
	return true;
}

const char *
LuaLibrary::GetName() const
{
	return fMetadata ? fMetadata->GetName() : NULL;
}

const char *
LuaLibrary::GetPublisherId() const
{
	static const char kPublisherId[] = "com.coronalabs";

	return fMetadata ? fMetadata->GetPublisherId() : kPublisherId;
}

int
LuaLibrary::GetVersion() const
{
	return fMetadata ? fMetadata->GetVersion() : 0;
}

int
LuaLibrary::GetRevision() const
{
	return fMetadata ? fMetadata->GetRevision() : 0;
}

int
LuaLibrary::ValueForKey( lua_State *L )
{
	return 0;
}

lua_CFunction
LuaLibrary::GetFactory() const
{
	return NULL;
}

const luaL_Reg *
LuaLibrary::GetFunctions() const
{
	return NULL;
}

int
LuaLibrary::Open( lua_State *L ) const
{
	int result = 0;

	lua_CFunction factory = GetFactory();
	const luaL_Reg *functions = GetFunctions();

	if ( factory )
	{
		result = CoronaLibraryNewWithFactory(
			L, factory,
			functions, const_cast< Self * >( this ) );

		// Cache metadata values of libraries created with Lua factory
		int index = lua_gettop( L ); CORONA_ASSERT( lua_istable( L, index ) );
		lua_getfield( L, index, "name" );
		const char *name = lua_tostring( L, -1 );

		lua_getfield( L, index, "publisherId" );
		const char *publisherId = lua_tostring( L, -1 );
		CORONA_ASSERT( 0 == strcmp( publisherId, GetPublisherId() ) );
		
		lua_getfield( L, index, "version" );
		int version = (int)lua_tointeger( L, -1 );
		lua_pop( L, 1 );

		lua_getfield( L, index, "revision" );
		int revision = (int)lua_tointeger( L, -1 );
		lua_pop( L, 1 );

		fMetadata = new LuaLibraryMetadata( name, publisherId, version, revision );

		lua_settop( L, index ); // restore stack so table is on top
	}
	else
	{
		const char *name = GetName(); CORONA_ASSERT( name );

		result = CoronaLibraryNew(
			L, name, GetPublisherId(), GetVersion(), GetRevision(),
			functions, const_cast< Self * >( this ) );
	}

	// Trap '__index' metamethod calls and forward to Self::GetProperty
	if ( result > 0 )
	{
		CORONA_ASSERT( 1 == result );

		lua_getfield( L, -1, "usesProviders" );
		bool usesProviders = ! lua_isnil( L, -1 );
		lua_pop( L, 1 );

		// We only set the extension if the library is NOT using providers
		if ( ! usesProviders )
		{
			lua_pushlightuserdata( L, const_cast< Self * >( this ) );
			lua_pushcclosure( L, Self::GetProperty, 1 );
			CoronaLibrarySetExtension( L, -2 );
		}
	}

	return result;
}

static const char kMetatableName[] = __FILE__;

int
LuaLibrary::OpenWrapper( lua_State *L )
{
	int result = 0;

	void *platformContext = CoronaLuaGetContext( L ); // lua_touserdata( L, lua_upvalueindex( 1 ) );
	if ( Initialize( L, platformContext ) )
	{
		result = Open( L );

		// The Lua registry owns 'this', i.e. the library instance.
		// TODO: Add unique prefix to the registry key to avoid collision
		CoronaLuaPushUserdata( L, this, kMetatableName );
		lua_setfield( L, LUA_REGISTRYINDEX, GetName() );
	}

	return result;
}

void
LuaLibrary::InitializeMetatable( lua_State *L )
{
	// Create single metatable used by all LuaLibrary userdata instances
	CoronaLuaInitializeGCMetatable( L, kMetatableName, Finalizer );
}

int
LuaLibrary::Finalizer( lua_State *L )
{
	Self *library = (Self *)CoronaLuaToUserdata( L, 1 );
	delete library;
	return 0;
}

LuaLibrary::Self *
LuaLibrary::GetLibrary( lua_State *L )
{
	// Light userdata is pushed as part of the closure
	Self *library = (Self *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	return library;
}

int
LuaLibrary::GetProperty( lua_State *L )
{
	Self *library = GetLibrary( L );
	return library->ValueForKey( L );
}

// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

