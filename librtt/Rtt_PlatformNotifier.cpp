//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

void
PlatformNotifier::ScheduleDispatch( VirtualEvent *e, int ref )
{
	if ( Rtt_VERIFY( e ) )
	{
		bool scheduled = false;
		Rtt_ASSERT( LUA_NOREF != ref && LUA_REFNIL != ref );

		lua_State *L = GetLuaState();
		Rtt_ASSERT( L );
		if ( L )
		{
			Runtime *runtime = LuaContext::GetRuntime( L );

			PlatformNotifierTask *task = Rtt_NEW( runtime->Allocator(), PlatformNotifierTask( * this, e ) );

			task->SetReference( ref );

			runtime->GetScheduler().Append( task );

			scheduled = true;
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
	fEvent( e ),
	fLuaRef( LUA_NOREF )
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
	int oldRef = fNotifier.GetListenerRef();

	if ( LUA_NOREF != fLuaRef )
	{
		fNotifier.RawSetListenerRef( fLuaRef );
	}

	fNotifier.CallListener( fEvent->Name(), *fEvent );

	if ( LUA_NOREF != fLuaRef )
	{
		fNotifier.RawSetListenerRef( oldRef );

		fLuaRef = LUA_NOREF;
	}
}
	
// ----------------------------------------------------------------------------

} // namespace Rtt
