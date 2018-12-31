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

#include "Rtt_PlatformModalInteraction.h"

#include "Rtt_Event.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

PlatformModalInteraction::PlatformModalInteraction( const ResourceHandle<lua_State> & handle )
:	PlatformNotifier( handle ),
	fRuntime( NULL ),
	fProperties( 0 )
{
}

PlatformModalInteraction::~PlatformModalInteraction()
{
}

void
PlatformModalInteraction::SetProperty( U32 mask, bool value )
{
	const U32 p = fProperties;
	fProperties = ( value ? p | mask : p & ~mask );
}

void
PlatformModalInteraction::BeginSession( Runtime& runtime )
{
	if ( ! (fProperties & kDoNotSuspend) )
	{
		runtime.Suspend();		
	}

	Rtt_ASSERT( !fRuntime || ( & runtime == fRuntime ) );
	fRuntime = & runtime;
}

void
PlatformModalInteraction::EndSession()
{
	Rtt_ASSERT( fRuntime );
	if ( ! (fProperties & kDoNotSuspend) )
	{
		fRuntime->Resume();
		fRuntime = NULL;
	}
	
	CleanupNotifier();
}

void
PlatformModalInteraction::DidDismiss( AddPropertiesCallback callback, void * userdata )
{
	if ( LUA_NOREF != GetListenerRef() )
	{
		lua_State * L = GetLuaState();
		Rtt_ASSERT( L );

		if ( L )
		{
			lua_rawgeti( L, LUA_REGISTRYINDEX, GetListenerRef() );
			int t = lua_type( L, -1 );
			int numArgs = 1;
			if ( LUA_TTABLE == t )
			{
				++numArgs;
				lua_getfield( L, -1, CompletionEvent::kName ); // get method
				lua_insert( L, -2 ); // swap table and function
			}
			else if ( LUA_TFUNCTION != t )
			{
				numArgs = 0;
			}

			// Cleanup first, in case the listener recurses
			CleanupNotifier();
			
			if ( numArgs > 0 )
			{
				// Push event
				CompletionEvent e;

				e.Push( L );
				
				if ( callback )
				{
					(*callback)( L, userdata );
				}

				LuaContext::DoCall( L, numArgs, 0 );
			}
		}
	}

	EndSession();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

