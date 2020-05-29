//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_LuaResource.h"

#include "Rtt_Event.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

//  NOT USED: static const char kNativeAlert[] = "native.Alert";

// ----------------------------------------------------------------------------

LuaResource::LuaResource( const ResourceHandle< lua_State >& handle, int index )
:	fHandle( handle ),
	fRef( LUA_NOREF )
{
	lua_State *L = handle.Dereference();

	Rtt_ASSERT( L );

	if ( L && LUA_NOREF != index )
	{
		lua_pushvalue( L, index ); Rtt_ASSERT( ! lua_isnil( L, -1 ) );
		fRef = luaL_ref( L, LUA_REGISTRYINDEX );
	}
}

LuaResource::~LuaResource()
{
	if ( LUA_NOREF != fRef )
	{
		lua_State *L = fHandle.Dereference();
//		Rtt_ASSERT( L );
		if ( L )
		{
			luaL_unref( L, LUA_REGISTRYINDEX, fRef );
		}
	}
}

int
LuaResource::PushListenerAndEvent( const MEvent& e ) const
{
	int nargs = 0;

	lua_State *L = fHandle.Dereference();
	Rtt_ASSERT( L );

	if ( L )
	{
		nargs = PushListener( L, e.Name() );
		if ( nargs > 0 )
		{
			// PushListener returns number of arguments it pushed on the stack
			// including the function itself. We should subtract one, but since 
			// we're pushing one more arg (below), the net effect is zero.
			e.Push( L );
		}
	}

	return nargs;
}

int
LuaResource::DispatchEvent( const MEvent& e ) const
{
	return DispatchEvent( e, 0 );
}

int
LuaResource::DispatchEvent( const MEvent& e, int nresults ) const
{
	int status = 0;

	int nargs = PushListenerAndEvent( e );
	if ( nargs > 0 )
	{
		lua_State *L = fHandle.Dereference();
		Rtt_ASSERT( L );
		
		RuntimeGuard guard( * LuaContext::GetRuntime( L ) );
		
		status = LuaContext::DoCall( L, nargs, nresults );
	}

	return status;
}

int
LuaResource::Push( lua_State *L ) const
{
	Rtt_ASSERT( L );

	int result = ( LUA_NOREF != fRef );
	if ( result )
	{
		lua_rawgeti( L, LUA_REGISTRYINDEX, fRef );
	}

	return result;
}

int
LuaResource::PushListener( lua_State *L, const char *key ) const
{
	Rtt_ASSERT( L );

	int result = Push( L );

	if ( result )
	{
		if ( lua_istable( L, -1 ) )
		{
			++result; // t is first arg to t.key method
			lua_getfield( L, -1, key ); // t.key
			Rtt_ASSERT( lua_isfunction( L, -1 ) ); // t.key should be a function
			lua_insert( L, -2 ); // move t.key before t
		}
		else
		{
			Rtt_ASSERT( lua_isfunction( L, -1 ) );
		}
	}

	return result;
}

lua_State*
LuaResource::L() const
{
	lua_State * L = fHandle.Dereference();
	Rtt_ASSERT( L );
	return L;
}

void
LuaResource::SetRef(int ref)
{
	fRef = ref;
}

// ----------------------------------------------------------------------------
/*
LuaObject::LuaObject( lua_State *L, int index )
:	fResource( L, index )
{
}

int
LuaObject::GetType() const
{
	
}

bool
LuaObject::IsBoolean() const
{
}

bool
LuaObject::IsCFunction() const
{
}

bool
LuaObject::IsFunction() const
{
}

bool
LuaObject::IsLightuserdata() const
{
}

bool
LuaObject::IsNil() const
{
}

bool
LuaObject::IsNumber() const
{
}

bool
LuaObject::IsString() const
{
}

bool
LuaObject::IsTable() const
{
}

bool
LuaObject::IsUserdata() const
{
}

bool
LuaObject::ToBoolean() const
{
}

lua_CFunction
LuaObject::ToCFunction() const
{
}

lua_Integer
LuaObject::ToInteger() const
{
}

double
LuaObject::ToNumber() const
{
}

const void*
LuaObject::ToPointer() const
{
}

const char*
LuaObject::ToString() const
{
}

void*
LuaObject::ToUserdata() const
{
}

LuaObject
LuaObject::GetField( const char *key ) const
{
}
*/
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

