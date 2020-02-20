//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "CoronaLibrary.h"

#include "CoronaAssert.h"
#include "CoronaLog.h"
#include "Rtt_Lua.h"

// ----------------------------------------------------------------------------

// Lua bytecodes
static const char kCoronaPrototype[] = "CoronaPrototype";
static const char kCoronaLibrary[] = "CoronaLibrary";
static const char kCoronaProvider[] = "CoronaProvider";

namespace Rtt
{
	int luaload_CoronaPrototype(lua_State* L);
	int luaload_CoronaLibrary(lua_State* L);
	int luaload_CoronaProvider(lua_State* L);
}

// ----------------------------------------------------------------------------

/*
static int
PushSignature( lua_State *L, int index, const char *methodName )
{
	lua_pushvalue( L, index );
	lua_getfield( L, index, "getSignature" );
	lua_insert( L, -2 ); // swap
	lua_pushstring( L, methodName );
	return 0 == Rtt::Lua::DoCall( L, 2, 1 );
}
*/

static bool
IsLibrary( lua_State *L, int index )
{
	bool result = false;

	int base = lua_gettop( L );
	index = Rtt::Lua::Normalize( L, index ); // t is at index

	lua_getfield( L, index, "instanceOf" );
	if ( lua_isfunction( L, -1 ) )
	{
		// instanceOf( t, CoronaLibrary )
		lua_pushvalue( L, index );
		Rtt::Lua::PushModule( L, CoronaLibraryClassName() );

		if ( 0 == Rtt::Lua::DoCall( L, 2, 1 ) )
		{
			result = lua_toboolean( L, -1 );
		}
	}

	lua_settop( L, base );

	return result;
}

/*
static void
PushStubName( lua_State *L, const char *libName )
{
	static const char kStubPrefix[] = "stub_";
	static const char kEmpty[] = "";

	if ( ! libName )
	{
		libName = kEmpty;
	}

	lua_pushfstring( L, "%s%s", kStubPrefix, libName ); // push name
}

static int
PushStubLibrary( lua_State *L, const char *libName )
{
	CORONA_ASSERT( libName );
	PushStubName( L, libName );

	int base = lua_gettop( L );
	const char *stubLibraryName = lua_tostring( L, -1 );
	int result = Rtt::Lua::PushModule( L, stubLibraryName );
	lua_remove( L, base ); // remove name

	CORONA_ASSERT( 0 == result || lua_istable( L, -1 ) );

	return result;
}
*/

// ----------------------------------------------------------------------------

CORONA_API
const char *CoronaLibraryClassName()
{
	return kCoronaLibrary;
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaLibraryInitialize( lua_State *L )
{
	// TODO: guard against subsequent calls for instance of 'L'

	Rtt::LuaStackGuard guard( L, __FUNCTION__ );

	const luaL_Reg kCoronaClasses[] =
	{
		{ kCoronaPrototype, Rtt::Lua::Open< Rtt::luaload_CoronaPrototype > },
		{ kCoronaLibrary, Rtt::Lua::Open< Rtt::luaload_CoronaLibrary > },
		{ kCoronaProvider, Rtt::Lua::Open< Rtt::luaload_CoronaProvider > },

		{ NULL, NULL }
	};

	Rtt::Lua::RegisterModuleLoaders( L, kCoronaClasses );

	return 0;
}

// ----------------------------------------------------------------------------

static const char kNameKey[] = "name";
static const char kPublisherIdKey[] = "publisherId";
static const char kVersionKey[] = "version";
static const char kRevisionKey[] = "revision";

// Registers 'libFuncs' in table at top of stack
static void
RegisterClosures( lua_State *L, const luaL_Reg libFuncs[], void *context )
{
	if ( libFuncs )
	{
		int nupvalues = 0;
		if ( context )
		{
			nupvalues = 1;
			lua_pushlightuserdata( L, context );
		}
		luaL_openlib( L, NULL, libFuncs, nupvalues );
	}
}

// Create newClass and then call new
// function Prototype:setMetamethod( name, func )
CORONA_API
int CoronaLibraryNew(
	lua_State *L,
	const char *libName, const char *publisherId, int version, int revision,
	const luaL_Reg libFuncs[], void *context )
{
	//	local t =
	//	{
	//		name=libName, publisherId=publisherId, version=version, revision=revision,
	//	}
	lua_createtable( L, 0, 4 );
	
	RegisterClosures( L, libFuncs, context );

	int index = lua_gettop( L );

	if ( ! libName ) { CORONA_LOG_ERROR( "%s must have a 'name' parameter.", __FUNCTION__ ); }
	else
	{
		lua_pushstring( L, libName );
		lua_setfield( L, -2, kNameKey );
	}

	if ( ! publisherId ) { CORONA_LOG_ERROR( "%s must have a 'publisherId' parameter.", __FUNCTION__ ); }
	else
	{
		lua_pushstring( L, publisherId );
		lua_setfield( L, -2, kPublisherIdKey );
	}

	lua_pushinteger( L, version );
	lua_setfield( L, -2, kVersionKey );

	lua_pushinteger( L, revision );
	lua_setfield( L, -2, kRevisionKey );

	// CoronaLibrary.new( CoronaLibrary, t )
	if ( Rtt::Lua::PushModule( L, CoronaLibraryClassName() ) )
	{
		lua_getfield( L, -1, "new" );
		lua_insert( L, -2 ); // move new under library
		lua_pushvalue( L, index );

		CORONA_ASSERT( lua_equal( L, -1, index ) );

		int status = Rtt::Lua::DoCall( L, 2, 1 ); Rtt_UNUSED( status );
		
		CORONA_ASSERT( lua_istable( L, -1 ) );
		CORONA_ASSERT( lua_equal( L, -1, index ) );
		lua_pop( L, 1 );
	}

	Rtt_ASSERT( lua_gettop( L ) == index );

	return 1; // leave t on top
}

CORONA_API
int CoronaLibraryNewWithFactory(
	lua_State *L, lua_CFunction factory,
	const luaL_Reg libFuncs[], void *context )
{
	int result = 0;

	if ( CORONA_VERIFY( factory ) )
	{
		int nres = factory( L );
		if ( nres > 0 )
		{
			if ( IsLibrary( L, -1 ) )
			{
				// TODO: Add error checking
				// * does it have a 'name' field
				// * does it have a 'publisherId' field

				RegisterClosures( L, libFuncs, context );

				result = 1;

				// Pop any extra results
				if ( nres > 1 )
				{
					lua_pop( L, ( nres - 1 ) );
				} 
			}
			else
			{
				lua_pop( L, nres ); // pop all results
			}
		}
	}

	return result;
}

CORONA_API
void CoronaLibrarySetExtension( lua_State *L, int index )
{
	if ( lua_isfunction( L, -1 )
		 && CORONA_VERIFY( IsLibrary( L, index ) ) )
	{
		int extensionIndex = lua_gettop( L );
		index = Rtt::Lua::Normalize( L, index );

		// self:setExtension( __index )
		lua_getfield( L, index, "setExtension" );	// setExtension
		lua_pushvalue( L, index );					// self

		lua_pushvalue( L, extensionIndex );			// extensionFunc

		Rtt::Lua::DoCall( L, 2, 0 );
	}

	// Pop top element (behaves like lua_setfield)
	lua_pop( L, 1 );
}

static int CoronaLibraryProviderLoader( lua_State *L )
{
	int result = 0;

	const char *name = lua_tostring( L, 1 ); CORONA_ASSERT( name );

	// local Provider = require "CoronaProvider"
	lua_getglobal( L, "require" );
	lua_pushstring( L, "CoronaProvider" );
	if ( 0 == CoronaLuaDoCall( L, 1, 1 ) )
	{
		// local Class = Provider:newClass( name )
		lua_getfield( L, -1, "newClass" ); CORONA_ASSERT( lua_isfunction( L, -1 ) );
		lua_insert( L, -2 ); // swap Provider and newClass
		lua_pushstring( L, name );
		if ( 0 == CoronaLuaDoCall( L, 2, 1 ) )
		{
			result = 1;
		}
		else
		{
			CORONA_LOG_ERROR( "Could not define a new provider class (%s).", name );
		}
	}

	return result;
}

CORONA_API
void CoronaLibraryProviderDefine( lua_State *L, const char *libName )
{
	CORONA_ASSERT( libName );

	lua_pushfstring( L, "%s.%s", kCoronaProvider, libName );
	const char *name = lua_tostring( L, -1 );
	CoronaLuaRegisterModuleLoader( L, name, CoronaLibraryProviderLoader, 0 );
	lua_pop( L, 1 ); // pop name
}

CORONA_API
int CoronaLibraryProviderNew( lua_State *L, const char *libName, const char *providerName, const char *publisherId )
{
	int result = 0;
	int base = lua_gettop( L );

	CORONA_ASSERT( libName );
	CORONA_ASSERT( providerName );
	CORONA_ASSERT( publisherId );

	// local modName "CoronaProvider." .. libName
	// local mod = require 'modName'
	lua_getglobal( L, "require" );
	lua_pushfstring( L, "%s.%s", kCoronaProvider, libName );
	if ( 0 == CoronaLuaDoCall( L, 1, 1 ) )
	{
		// Following does the equivalent in Lua:
		//		return mod:new{ name=name, publisherId=publisherId }

		lua_getfield( L, -1, "new" ); // push mod.new
		lua_insert( L, base + 1 ); // swap to setup call: mod.new( mod, ... )

		// local o = { name=name, publisherId=publisherId }
		lua_createtable( L, 0, 2 );

		lua_pushstring( L, providerName );
		lua_setfield( L, -2, "name" );

		lua_pushstring( L, publisherId );
		lua_setfield( L, -2, "publisherId" );

		// mod.new( mod, o )
		result = ( 0 == CoronaLuaDoCall( L, 2, 1 ) );

		if ( ! result )
		{
			CORONA_LOG_ERROR( "Could not create a new provider {name=%s, publisherId=%s} for library (%s). There was an error with the base provider.", providerName, publisherId, libName );
		}
	}
	else
	{
		CORONA_LOG_ERROR( "Could not create a new provider {name=%s, publisherId=%s} for library (%s). The base provider module does not exist.", providerName, publisherId, libName );
	}

	if ( 0 == result )
	{
		lua_settop( L, base );
	}
	else
	{
		CORONA_ASSERT( (base+result) == lua_gettop( L ) );
	}

	return result;
}

CORONA_API
void CoronaLibraryPushProperty( lua_State *L, const char *libName, const char *name )
{
	bool foundLibrary = false;

	if ( CORONA_VERIFY( L ) && libName && name )
	{
		foundLibrary = ( Rtt::Lua::PushModule( L, libName ) );
		if ( foundLibrary )
		{
			lua_getfield( L, -1, name ); // push library[name]
			lua_remove( L, -2 ); // remove library
		}
	}

	if ( ! foundLibrary )
	{
		lua_pushnil( L );
	}
}

/*
static int
CoronaLibraryVCall( lua_State *L, const char *libName, const char *name, const char *sig, bool isMethod, va_list ap )
{
	int nres = 0;

	if ( CORONA_VERIFY( L ) && libName && name )
	{
		if ( Rtt::Lua::PushModule( L, libName ) )
		{
			int base = lua_gettop( L );

			lua_insert( L, base ); // push sig under service
			int serviceIndex = base + 1;

			if ( ! isMethod )
			{
				// push function at top of stack
				lua_getfield( L, serviceIndex, name ); // method = service.name

				// For pure function calls, don't need service
				lua_remove( L, serviceIndex );
			}

			// table object or function should be at top of stack
			// for table objects, we have to specify the method name
			CORONA_ASSERT( lua_istable( L, -1 ) || lua_isfunction( L, -1 ) );
			nres = Rtt::Lua::VCall( L, sig, ap, ( isMethod ? name : NULL ) );

			lua_remove( L, base ); // pop sig
		}
	}

	return nres;
}
*/

CORONA_API
int CoronaLibraryCallFunction( lua_State *L, const char *libName, const char *name, const char *sig, ... )
{
	va_list ap;
	va_start( ap, sig );
	int result = CoronaLibraryCallFunctionV( L, libName, name, sig, ap );
	va_end( ap );

	return result;
}

CORONA_API
int CoronaLibraryCallFunctionV( lua_State *L, const char *libName, const char *name, const char *sig, va_list arguments )
{
	int result = 0;

	if ( Rtt::Lua::PushModule( L, libName ) )
	{
		int base = lua_gettop( L );

		// push function at top of stack
		lua_getfield( L, -1, name ); // method = service.name

		// Remove library
		lua_remove( L, base );

		result = Rtt::Lua::VCall( L, sig, arguments, NULL );
	}
//	int result = CoronaLibraryVCall( L, libName, name, sig, arguments, false, arguments );

	return result;
}

CORONA_API
int CoronaLibraryCallMethod( lua_State *L, const char *libName, const char *name, const char *sig, ... )
{
	va_list ap;
	va_start( ap, sig );
	int result = CoronaLibraryCallMethodV( L, libName, name, sig, ap );
	va_end( ap );

	return result;
}

CORONA_API
int CoronaLibraryCallMethodV( lua_State *L, const char *libName, const char *name, const char *sig, va_list arguments )
{
	int result = 0;

	if ( Rtt::Lua::PushModule( L, libName ) )
	{
		result = Rtt::Lua::VCall( L, sig, arguments, name );
	}
//	int result = CoronaLibraryVCall( L, libName, name, sig, true, arguments );

	return result;
}

// ----------------------------------------------------------------------------

