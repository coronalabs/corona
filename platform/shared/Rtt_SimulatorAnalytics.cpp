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

#include "Rtt_SimulatorAnalytics.h"

#include <time.h>

#include "Rtt_LuaContext.h"
#include "Rtt_LuaFrameworks.h"
#include "Rtt_LuaLibCrypto.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_Runtime.h"
#include "Rtt_MCrypto.h"
#include "Rtt_LuaLibSocket.h"

#include "CoronaLibrary.h"

#include "AnalyticsProvider.h"

Rtt_EXPORT_BEGIN
#define lua_c

#ifdef Rtt_DEBUGGER
#include "ldo.h"

#endif

Rtt_EXPORT_END

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// TODO: Eliminate dependency on LuaContext:: functions
// Instead, use Lua:: functions.
SimulatorAnalytics::SimulatorAnalytics( const MPlatform& platform, lua_CFunction luaPackageLoaderCallback /*= NULL*/ )
:	fPlatform( platform ),
	fVMContext( LuaContext::New( & platform.GetAllocator(), platform ) ),
	fRef( LUA_NOREF ),
	fIsParticipating( true ),
	fIsSessionStarted( false )
{
#ifdef Rtt_DEBUG
	fIsParticipating = false;
#else
	AnalyticsProviderInit(Rtt_STRING_BUILD);
#endif
}

SimulatorAnalytics::~SimulatorAnalytics()
{
	if ( fIsParticipating )
	{
		AnalyticsProviderFinish();
	}
}

bool
SimulatorAnalytics::Initialize( const char *subscriptionType, int uid )
{
	bool result = true;

	return result;
}

bool
SimulatorAnalytics::Log( const char *eventName )
{
	bool result = true;

	if ( fIsParticipating )
	{
		if (eventName == NULL)
		{
			return false;
		}

		AnalyticsProviderSendEvent(eventName);
	}

	return result;
}

bool
SimulatorAnalytics::Log( const char *eventName, const char *eventDataValue )
{
	bool result = true;

	if ( fIsParticipating )
	{
		Rtt_ASSERT(eventName != NULL);

		if (eventDataValue == NULL)
		{
			// Provides backward compatibility
			AnalyticsProviderSendEvent(eventName);
		}
		else
		{
			AnalyticsProviderSendEvent(eventName, "key", eventDataValue);
		}
	}

	return result;
}

bool
SimulatorAnalytics::Log( const char *eventName, const char *eventDataKey, const char *eventDataValue )
{
	bool result = true;

	if ( fIsParticipating )
	{
		Rtt_ASSERT(eventName != NULL);
		Rtt_ASSERT(eventDataKey != NULL);
		Rtt_ASSERT(eventDataValue != NULL);

		AnalyticsProviderSendEvent(eventName, eventDataKey, eventDataValue);
	}

	return result;
}

bool
SimulatorAnalytics::Log( const char *eventName, const size_t numItems, char **eventDataKeys, char **eventDataValues )
{
	bool result = true;

	if ( fIsParticipating )
	{
		Rtt_ASSERT(eventName != NULL);
		Rtt_ASSERT(numItems > 0);
		Rtt_ASSERT(eventDataKeys != NULL);
		Rtt_ASSERT(eventDataValues != NULL);

		AnalyticsProviderSendEvent(eventName, numItems, eventDataKeys, eventDataValues);
	}

	return result;
}

bool
SimulatorAnalytics::Log( const char *eventName, std::map<std::string, std::string> keyValues )
{
	bool result = true;

	if ( fIsParticipating )
	{
		Rtt_ASSERT(eventName != NULL);
		Rtt_ASSERT(! keyValues.empty());

		AnalyticsProviderSendEvent(eventName, keyValues);
	}

	return result;
}

void
SimulatorAnalytics::BeginSession(int uid)
{
#if 0
	if ( fIsParticipating )
	{
		lua_State *L = fVMContext->L();

		if ( L && LUA_NOREF != fRef )
		{
			// call analytics:beginSession( ) 
			lua_rawgeti( L, LUA_REGISTRYINDEX, fRef );
			int base = lua_gettop( L ); // get index of 'analytics'
			lua_getfield( L, -1, "beginSession" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
			lua_insert( L, base ); // swap 'analytics' and 'analytics.beginSession'
            lua_pushinteger( L, uid );
			(void) Rtt_VERIFY( 0 == LuaContext::DoCall( L, 2, 0 ) );
			fIsSessionStarted = true;
		}
	}
#endif // 0
}

void
SimulatorAnalytics::EndSession()
{
#if 0
	if ( fIsParticipating )
	{
		lua_State *L = fVMContext->L();
        
		if ( L && LUA_NOREF != fRef )
		{
			// call analytics:endSession( )
			lua_rawgeti( L, LUA_REGISTRYINDEX, fRef );
			int base = lua_gettop( L ); // get index of 'analytics'
			lua_getfield( L, -1, "endSession" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
			lua_insert( L, base ); // swap 'analytics' and 'analytics.endSession'
			(void) Rtt_VERIFY( 0 == LuaContext::DoCall( L, 1, 0 ) );
		}
	}
	fIsSessionStarted = false;
#endif // 0
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

