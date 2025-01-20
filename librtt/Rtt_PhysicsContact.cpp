//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#ifdef Rtt_PHYSICS	

#include "Rtt_PhysicsContact.h"

// TODO: see which of these are actually needed here
#include "Rtt_Lua.h"
#include "Rtt_LuaLibPhysics.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_Runtime.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_LuaContext.h"

#include "Box2D/Box2D.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const char PhysicsContact::kMetatableName[] = "physics.contact"; // unique identifier for this userdata type

UserdataWrapper *
PhysicsContact::CreateWrapper( const ResourceHandle< lua_State >& luaStateHandle, b2Contact *contact )
{
	// Lua owns wrapper (which has a weak pointer to joint)
	UserdataWrapper *result = Rtt_NEW(
		runtime.Allocator(),
		UserdataWrapper( luaStateHandle, contact, PhysicsContact::kMetatableName ) );

	return result;
}

b2Contact*
PhysicsContact::GetContact( lua_State *L, int index )
{
	b2Contact *result = NULL;

	UserdataWrapper **ud = (UserdataWrapper **)luaL_checkudata( L, index, Self::kMetatableName );
	if ( ud )
	{
		UserdataWrapper *wrapper = *ud;

		result = (b2Contact*)wrapper->Dereference();
	}

	return result;
}

int
PhysicsContact::ValueForKey( lua_State *L )
{
	int result = 0;    // number of args pushed on the stack
	
	b2Contact *contact = GetContact( L, 1 );
	
	if ( contact )
	{
		const char *key = luaL_checkstring( L, 2 );		
		result = 1;

		if ( 0 == strcmp( "isTouching", key ) )
		{
			lua_pushboolean( L, contact->IsTouching() );
		}
		else if ( 0 == strcmp( "isEnabled", key ) )
		{
			lua_pushboolean( L, contact->IsEnabled() );
		}
		else if ( 0 == strcmp( "friction", key ) )
		{
			lua_pushnumber( L, contact->GetFriction() );
		}
		else if ( 0 == strcmp( "bounce", key ) )
		{
			lua_pushnumber( L, contact->GetRestitution() );
		}
		else if ( 0 == strcmp( "tangentSpeed", key ) )
		{
			lua_pushnumber( L, contact->GetTangentSpeed() );
		}
		else
		{
			result = 0;
		}
	}
	
	return result;
}

int
PhysicsContact::SetValueForKey( lua_State *L )
{
	b2Contact *contact = GetContact( L, 1 );
	
	if ( contact )
	{		
		const char *key = luaL_checkstring( L, 2 );

		if ( 0 == strcmp( "isEnabled", key ) )
		{
			contact->SetEnabled( lua_toboolean( L, 3 ) );
		}
		else if ( 0 == strcmp( "friction", key ) )
		{
			contact->SetFriction( lua_tonumber( L, 3 ) );
		}
		else if ( 0 == strcmp( "bounce", key ) )
		{
			contact->SetRestitution( lua_tonumber( L, 3 ) );
		}
		else if ( 0 == strcmp( "tangentSpeed", key ) )
		{
			contact->SetTangentSpeed( lua_tonumber( L, 3 ) );
		}
	}

	return 0;
}


int
PhysicsContact::Finalizer( lua_State *L )
{
	UserdataWrapper **ud = (UserdataWrapper **)luaL_checkudata( L, 1, Self::kMetatableName );
	if ( ud )
	{
		UserdataWrapper *wrapper = *ud;

		Rtt_DELETE( wrapper );
	}

	return 0;
}

// Call this to init metatable
void
PhysicsContact::Initialize( lua_State *L )
{
	Rtt_LUA_STACK_GUARD( L );

	const luaL_Reg kVTable[] =
	{
		{ "__index", Self::ValueForKey },
		{ "__newindex", Self::SetValueForKey },
		{ "__gc", Self::Finalizer },
		{ NULL, NULL }
	};
		
	Lua::InitializeMetatable( L, Self::kMetatableName, kVTable );
}

// ----------------------------------------------------------------------------
	
} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_PHYSICS	
