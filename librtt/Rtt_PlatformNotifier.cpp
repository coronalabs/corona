//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_PlatformNotifier.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Event.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

PlatformNotifier::PlatformNotifier( const ResourceHandle<lua_State> & handle, bool isListenerPersistent )
:	fListenerRef( LUA_NOREF ),
	fLuaState( handle ),
	fListenerPersistsAfterCall( isListenerPersistent )
{
}

PlatformNotifier::~PlatformNotifier()
{
	CleanupNotifier();
}

void
PlatformNotifier::ScheduleDispatch( VirtualEvent *e )
{
	if ( Rtt_VERIFY( e ) )
	{
		bool scheduled = false;
		if ( HasListener() )
		{
			lua_State *L = GetLuaState();
			Rtt_ASSERT( L );
			if ( L )
			{
				Runtime *runtime = LuaContext::GetRuntime( L );

				PlatformNotifierTask *task = Rtt_NEW( runtime->Allocator(), PlatformNotifierTask( * this, e ) );

				runtime->GetScheduler().Append( task );

				scheduled = true;
			}
		}

		// In case of error, we need to delete the event
		if ( ! Rtt_VERIFY( scheduled ) )
		{
			Rtt_DELETE( e );
		}
	}
}

bool
PlatformNotifier::HasListener() const
{
	return LUA_NOREF != fListenerRef;
}
	
void
PlatformNotifier::SetListenerRef( int index )
{
	// If a listener is already set, we will clear it and set a new listener.
	// CleanupNotifier already checks for LUA_NOREF != fListenerRef
	CleanupNotifier();

	lua_State * L = fLuaState.Dereference();
	Rtt_ASSERT( L );

	if ( L )
	{
		int t = lua_type( L, index );
		if ( LUA_TFUNCTION == t || LUA_TTABLE == t )
		{
			lua_pushvalue( L, index );
			fListenerRef = luaL_ref( L, LUA_REGISTRYINDEX );
		}
	}
}
	
void
PlatformNotifier::CleanupNotifier()
{
	if ( LUA_NOREF != fListenerRef )
	{
		lua_State *L = fLuaState.Dereference();
		if ( L ) 
		{
			lua_unref( L, fListenerRef );
		}
		fListenerRef = LUA_NOREF;
	}
}

lua_State*
PlatformNotifier::GetLuaState() const
{
	return fLuaState.Dereference();
}

void 
PlatformNotifier::CallListener( const char * eventName, VirtualEvent & e )
{
	if ( LUA_NOREF != fListenerRef )
	{
		lua_State * L = fLuaState.Dereference();
		Rtt_ASSERT( L );
		if ( L )
		{
			lua_rawgeti( L, LUA_REGISTRYINDEX, fListenerRef );

			int t = lua_type( L, -1 );
			int numArgs = 1;

			if ( LUA_TTABLE == t )
			{
				++numArgs;
				lua_getfield( L, -1, eventName ); // get method
				lua_insert( L, -2 ); // swap table and function
			}
			else if ( LUA_TFUNCTION != t )
			{
				numArgs = 0;
			}
			
			if ( ! fListenerPersistsAfterCall )
			{
				// Clean up first, in case the listener recurses
				CleanupNotifier();
			}

			if ( numArgs > 0 )
			{
				// Push event
				e.Push( L );

				LuaContext::DoCall( L, numArgs, 0 );
			}
		}
	}
}

void
PlatformNotifier::RawSetListenerRef(int ref)
{
	fListenerRef = ref;
}

// ----------------------------------------------------------------------------

PlatformNotifierTask::PlatformNotifierTask( PlatformNotifier& notifier, VirtualEvent *e )
:	Super(),
	fNotifier( notifier ),
	fEvent( e )
{
	Rtt_ASSERT( e );
}

PlatformNotifierTask::~PlatformNotifierTask()
{
	Rtt_DELETE( fEvent );
}

void 
PlatformNotifierTask::operator()( Scheduler & sender )
{
	fNotifier.CallListener( fEvent->Name(), *fEvent );
}
	
// ----------------------------------------------------------------------------

} // namespace Rtt
