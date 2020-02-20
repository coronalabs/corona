//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_MPlatform.h"

#include "Display/Rtt_DisplayObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

DisplayObject*
luaL_todisplayobject( lua_State *L, int index, const LuaProxyVTable& table )
{
	DisplayObject *result = NULL;

	DisplayObject *o = (DisplayObject*)LuaProxy::GetProxyableObject( L, index );
	if ( o && & o->ProxyVTable() == & table )
	{
		result = o;
	}

	return result;
}


void
setProperty( lua_State *L, const char key[], size_t keyLen, Coordinate value )
{
	Rtt_ASSERT( lua_istable( L, -1 ) );

	lua_pushlstring( L, key, keyLen );
	lua_pushnumber( L, Rtt_RealToFloat( value ) );
	lua_settable( L, -3 );
}

#ifdef Rtt_DEBUG

bool
luaL_verify( lua_State *L, bool cond, const char *reason )
{
	if ( ! Rtt_VERIFY( cond ) )
	{
		lua_getfield( L, LUA_GLOBALSINDEX, "print" );
		Rtt_ASSERT( lua_isfunction( L, -1 ) );

		lua_getfield(L, LUA_GLOBALSINDEX, "debug");
		if ( ! Rtt_VERIFY( lua_istable( L, -1 ) ) )
		{
			lua_pop( L, 2 );
			goto exit_gracefully;
		}

		lua_getfield(L, -1, "traceback");
		if ( ! Rtt_VERIFY( lua_isfunction( L, -1 ) ) )
		{
			lua_pop( L, 3 );
			goto exit_gracefully;
		}

		lua_remove( L, -2 ); // pop debug

		const char kNull[] = "";
		if ( ! reason ) { reason = kNull; }

		lua_pushstring( L, reason );
		lua_call(L, 1, 1); // call debug.traceback

		Rtt_ASSERT( lua_isfunction( L, -2 ) );
		lua_call( L, 1, 0 ); // call print
	}

exit_gracefully:
	return Rtt_VERIFY( cond );
}

#endif

void*
UserdataForEnum( const char buffer[], U32 index )
{
	return const_cast< char* >( & buffer[index] );
}

bool
EnumExistsForUserdata( const char buffer[], void *p, S32 numIndices )
{
	Rtt_ASSERT( numIndices > 0 );

	long offset = (const char*)p - buffer;
	return offset >= 0 && offset < numIndices;
}

S32
EnumForUserdata( const char buffer[], void* p, S32 numIndices, S32 defaultIndex )
{
	long offset = (const char*)p - buffer;
	return offset >= 0 && offset < numIndices ? (S32) offset : defaultIndex;
}

// Creates path based on the parameter at index (with optional baseDir at index+1)
// Returns index position of next param (index+1 if no baseDir, index+2 if baseDir)
int
luaL_initpath( lua_State *L, const MPlatform& platform, int index, String& outPath  )
{
	int nextArg = index + 1;

	MPlatform::Directory baseDir = MPlatform::kResourceDir;
	const char *filename = lua_tostring( L, index );
	if ( lua_islightuserdata( L, nextArg ) )
	{
		void *p = lua_touserdata( L, nextArg );
		baseDir = (MPlatform::Directory)EnumForUserdata(
			 LuaLibSystem::Directories(),
			 p,
			 MPlatform::kNumDirs,
			 MPlatform::kResourceDir );

		++nextArg;
	}

	platform.PathForFile( filename, baseDir, MPlatform::kDefaultPathFlags, outPath );

	return nextArg;
}

// ----------------------------------------------------------------------------

void*
UserdataWrapper::GetFinalizedValue()
{
	static int sValue = 0;
	return & sValue;
} 

static
int PushUserdataWrapperTable( lua_State *L )
{
	const char kUserdataWrapper[] = "userdataWrapper";
	lua_getfield( L, LUA_REGISTRYINDEX, kUserdataWrapper );

	if ( lua_isnil( L, -1 ) )
	{
		lua_newtable( L );
		{
			// metatable for weak references to values
			lua_newtable( L ); // create mt
			lua_pushstring( L, "v" );
			lua_setfield( L, -2, "__mode" );
			lua_setmetatable( L, -2 );
		}

		lua_pushvalue( L, -1 ); // push again b/c at end of function, we need it at top of stack
		lua_setfield( L, LUA_REGISTRYINDEX, kUserdataWrapper );
	}
	return 1;
}

UserdataWrapper::UserdataWrapper( const ResourceHandle< lua_State >& handle, void *ud, const char *mtName )
:	fHandle( handle ),
	fData( ud )
{
	lua_State *L = handle.Dereference(); Rtt_ASSERT( L );

	// Store userdata in a weak table so we can look it up later to Push()
	PushUserdataWrapperTable( L );

	// Push key
	lua_pushlightuserdata( L, this );

	// Push value to let Lua own "this" by putting inside a userdata.
	Lua::PushUserdata( L, this, mtName );

	lua_settable( L, -3 );

	lua_pop( L, 1 ); // pop userdataWrapper table
}

UserdataWrapper::~UserdataWrapper()
{
	lua_State *L = fHandle.Dereference();
	if ( L )
	{
		PushUserdataWrapperTable( L );
		lua_pushlightuserdata( L, this );
		lua_pushnil( L );
		lua_settable( L, -3 );

		lua_pop( L, 1 ); // pop userdataWrapper table
	}
}

int
UserdataWrapper::Push() const
{
	int result = 0; // Number of args pushed on stack

	lua_State *L = fHandle.Dereference();
	if ( L )
	{
		PushUserdataWrapperTable( L );
		lua_pushlightuserdata( L, const_cast< Self * >( this ) );
		lua_gettable( L, -2 );

		result = 1; // doesn't matter if it's nil or non-nil

		lua_remove( L, -2 ); // remove userdataWrapper table
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

