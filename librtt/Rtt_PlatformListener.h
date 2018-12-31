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


