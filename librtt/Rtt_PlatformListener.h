//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformListener_H__
#define _Rtt_PlatformListener_H__

// ----------------------------------------------------------------------------

#include "Core/Rtt_Types.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

class PlatformNotifier
{
	public:
		void SetListenerRef( int index );

		void CallListener( const char * eventName );
	
		bool HasListener() const
		{
			return LUA_NOREF != fListenerRef;
		}

		PlatformNotifier( const ResourceHandle<lua_State> & handle )
			: fListenerRef( LUA_NOREF ), fLuaState( handle )
		{
		}
		
		~PlatformNotifier()
		{
			if ( LUA_NOREF != fListenerRef )
			{
				lua_State *L = fLuaState.Dereference();
				Rtt_ASSERT( L );
				lua_unref( L, fListenerRef );
				fListenerRef = LUA_NOREF;
			}
		}

	protected:
		friend class PlatformCallListenerTask;

		ResourceHandle<lua_State> fLuaState;
		int fListenerRef;
};

// ----------------------------------------------------------------------------

class PlatformCallListenerTask : public Task
{
	public:
		// Note that msg must be a static string
		PlatformCallListenerTask( PlatformNotifier * listener, const char * msg ) : fPlayer( player ), fMessage( msg )
		{
		}
		
	public:
		virtual void operator()( Scheduler& sender );
		
		static bool ScheduleCallListener( PlatformNotifier * listener, const char * msg );
		static bool ScheduleCallListenerTask( PlatformCallListenerTask * task );
		
	protected:
		const char * fMessage;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

#endif // _Rtt_PlatformListener_H__


