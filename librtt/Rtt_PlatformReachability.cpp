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

#include "Rtt_PlatformReachability.h"

#include "Rtt_Event.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------
	
const char PlatformReachability::kReachabilityListenerEvent[] = "networkStatus";

// ----------------------------------------------------------------------------

PlatformReachability::PlatformReachability( const ResourceHandle<lua_State> & handle, PlatformReachabilityType type, const char* address  )
:	fLuaState( handle )
{
	
}

PlatformReachability::~PlatformReachability()
{
	
}
	
bool
PlatformReachability::IsValid() const
{
	return false;
}

bool
PlatformReachability::IsReachable() const
{
	return false;
}

bool
PlatformReachability::IsConnectionRequired() const
{
	return false;
}

bool
PlatformReachability::IsConnectionOnDemand() const
{
	return false;
}

bool
PlatformReachability::IsInteractionRequired() const
{
	return false;
}
bool
PlatformReachability::IsReachableViaCellular() const
{
	return false;
}
bool
PlatformReachability::IsReachableViaWiFi() const
{
	return false;
}
/*
bool
PlatformReachability::HasReachability() const
{
	return false;
}
*/

// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------

const char ReachabilityChangeEvent::kName[] = "networkStatus";

ReachabilityChangeEvent::ReachabilityChangeEvent( const PlatformReachability& reachability, const char *address )
:	fNetworkReachability( reachability ),
	fAddress( address )
{
}

	/*
ReachabilityChangeEvent::~ReachabilityChangeEvent()
{
}

	 */
const char*
ReachabilityChangeEvent::Name() const
{
	return Self::kName;
}

int
ReachabilityChangeEvent::Push( lua_State *L ) const
{
	// Check for additional calls
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		// event.address, event.isReachable, etc
		lua_pushstring( L, fAddress );
		lua_setfield( L, -2, "address" );

		lua_pushboolean( L, fNetworkReachability.IsReachable() );
		lua_setfield( L, -2, "isReachable" );

		lua_pushboolean( L, fNetworkReachability.IsConnectionRequired() );
		lua_setfield( L, -2, "isConnectionRequired" );

		lua_pushboolean( L, fNetworkReachability.IsConnectionOnDemand() );
		lua_setfield( L, -2, "isConnectionOnDemand" );
		
		lua_pushboolean( L, fNetworkReachability.IsInteractionRequired() );
		lua_setfield( L, -2, "isInteractionRequired" );
		
		lua_pushboolean( L, fNetworkReachability.IsReachableViaCellular() );
		lua_setfield( L, -2, "isReachableViaCellular" );
		
		lua_pushboolean( L, fNetworkReachability.IsReachableViaWiFi() );
		lua_setfield( L, -2, "isReachableViaWiFi" );
		
	}

	return 1;
}

void
ReachabilityChangeEvent::Dispatch( lua_State *L, Runtime& runtime ) const
{
	// Invoke Lua code: "network._dispatchStatus( address, event )"
	lua_getglobal( L, "network" );
	// This will call a private implementation in init.lua
	lua_getfield( L, -1, "_dispatchStatus" );
	lua_pushstring( L, fAddress );
	int nargs = 1 + Push( L );
	LuaContext::DoCall( L, nargs, 0 );
}


// ----------------------------------------------------------------------------	
	
} // namespace Rtt

// ----------------------------------------------------------------------------
