//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PhysicsContact_H__
#define _Rtt_PhysicsContact_H__

#ifdef Rtt_PHYSICS

#include "Core/Rtt_ResourceHandle.h"

// ----------------------------------------------------------------------------

class b2Contact;

struct lua_State;

namespace Rtt
{

class UserdataWrapper;
	
// ----------------------------------------------------------------------------

class PhysicsContact
{
	public:
		typedef PhysicsContact Self;

	public:
		static const char kMetatableName[];

	public:
		static UserdataWrapper *CreateWrapper( const ResourceHandle< lua_State >& luaStateHandle, b2Contact *contact );

	public:
		static b2Contact* GetContact( lua_State *L, int index );

	public:
		static void Initialize( lua_State *L );
				
	// Metatable methods
	public:
		static int ValueForKey( lua_State *L );
		static int SetValueForKey( lua_State *L );
		static int Finalizer( lua_State *L );
};

// ----------------------------------------------------------------------------
	
} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_PHYSICS	

#endif // _Rtt_PhysicsContact_H__
