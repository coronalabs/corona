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

#include "Rtt_LaunchPad.h"

#include <time.h>

#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"
#include "Rtt_MCrypto.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const char kRequireEventName[] = "require";

// Every require call gets logged
int
LaunchPad::Loader( lua_State *L )
{
	if ( lua_isstring( L, 1 ) )
	{
		Runtime *runtime = LuaContext::GetRuntime( L );
		LaunchPad *launchPad = runtime->GetLaunchPad();

		// launchpad is init'd after while reading in config.lu
		// if the user opts out, this will always be NULL, so add a guard
		if ( launchPad )
		{
			const char *name = lua_tostring(L, 1);
			launchPad->Log( kRequireEventName, name );
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------

// launchPad.log( eventName, eventData )
static int
log( lua_State *L )
{
	bool success = false; // did we successfully log?

	const char *eventName = lua_tostring( L, 1 );
	const char *eventData = lua_tostring( L, 2 );

	if ( eventName && eventData )
	{
		LaunchPad *launchPad = (LaunchPad *)lua_touserdata( L, lua_upvalueindex( 1 ) );
		success = launchPad->Log( eventName, eventData );
	}

	lua_pushboolean( L, success );

	return 1;
}

int
LaunchPad::Open( lua_State *L )
{
	const luaL_Reg kVTable[] =
	{
		{ "log", log },

		{ NULL, NULL }
	};

	Runtime *runtime = LuaContext::GetRuntime( L );
	LaunchPad *launchPad = runtime->GetLaunchPad();

	lua_pushlightuserdata( L, launchPad );
	luaI_openlib( L, "launchPad", kVTable, 1 ); // leave library on top of stack
	return 1;
}

// ----------------------------------------------------------------------------

LaunchPad::LaunchPad( const ResourceHandle< lua_State >& handle )
:	fHandle( handle ),
	fRef( LUA_NOREF ),
	fRequireCount( 0 ),
	fIsParticipating( true )
{
}

LaunchPad::~LaunchPad()
{
	lua_State *L = fHandle.Dereference();
	if ( L )
	{
		luaL_unref( L, LUA_REGISTRYINDEX, fRef );
	}
}

// The device implementation actually records information
// The authoring/simulator version is a set of no-ops
#if defined( Rtt_DEVICE_ENV ) && defined( Rtt_NETWORK )

// launchpad.lua is pre-compiled into bytecodes and then placed in a byte array
// constant in launchpad.cpp. The following function defined in launchpad.cpp loads
// the bytecodes via luaL_loadbuffer. The .cpp file is dynamically generated.
int luaload_launchpad(lua_State* L);

static int
digestFunc( lua_State *L )
{
	U8 digest[MCrypto::kMaxDigestSize];

	const MCrypto::Algorithm algorithm = MCrypto::kMD5Algorithm;

	const char *input = lua_tostring( L, 1 );
	if ( !input )
	{
		lua_pushnil( L );
		return 1;
	}

	Rtt::Data<const char> data( input, strlen( input ) );

	const MCrypto& crypto = LuaContext::GetRuntime( L )->Platform().GetCrypto();
	crypto.CalculateDigest( algorithm, data, digest );

	const size_t kDigestLen = 16;
	Rtt_ASSERT( crypto.GetDigestLength( algorithm ) <= kDigestLen );
	char hex[ kDigestLen*2 + 1 ];
	for ( unsigned int i = 0; i < kDigestLen; i++ )
	{
		char *p = hex;
		p += sprintf( hex + 2*i, "%02x", digest[i] );
		Rtt_ASSERT( ((size_t)( p - hex )) < sizeof( hex ) );
	}

	lua_pushstring( L, hex );
	return 1;
}

bool
LaunchPad::Initialize( int index )
{
    bool result = false;
    
    if ( fIsParticipating )
    {
        lua_State *L = fHandle.Dereference();
		Rtt_LUA_STACK_GUARD( L );

        if ( ! Rtt_VERIFY( 0 != index )
             || ( NULL == L ) )
        {
            return false;
        }

        int indexAbs = index;

        if ( indexAbs < 0 )
        {
            // Get positive indices
            indexAbs = lua_gettop( L ) + index + 1;
        }

        // Load launchpad
        luaload_launchpad( L );
        LuaContext::DoCall( L, 0, 0 );

        // newLaunchPad( application.metadata, digestFunc )
        lua_getglobal( L, "newLaunchPad" );

        lua_getfield( L, indexAbs, "metadata" ); // application.metadata from config.lua
        Rtt_ASSERT( lua_istable( L, -1 ) );
        lua_pushcfunction( L, & digestFunc );

		int numArgs = 2;
        result = Rtt_VERIFY( 0 == LuaContext::DoCall( L, numArgs, 1 ) );

        if ( result )
        {
            Rtt_ASSERT( lua_istable( L, -1 ) );
            fRef = luaL_ref( L, LUA_REGISTRYINDEX );
        }
        else
        {
            lua_pop( L, numArgs );
        }
    }

	return result;
}

bool
LaunchPad::ShouldLog( const char *eventName ) const
{
	bool result = true;

    if ( fIsParticipating )
    {
        // Should never use "require" literal. Should use kRequireEventName instead.
        Rtt_ASSERT( kRequireEventName == eventName || 0 != strcmp( eventName, kRequireEventName ) );

        const int kRequireCountMax = 0;  // don't send requires since they are never looked at
        if ( kRequireEventName == eventName )
        {
            ++fRequireCount;
            result = fRequireCount < kRequireCountMax;
        }
    }
    
	return result;
}

bool
LaunchPad::Log( const char *eventName, const char *eventData )
{
	bool result = false;

	if ( fIsParticipating )
	{
		lua_State *L = fHandle.Dereference();
		if ( L && LUA_NOREF != fRef
			 && ShouldLog( eventName ) )
		{
			// call launchPad:log( eventName, eventData ) which is really:
			// launchPad.log( launchPad, eventName, eventData )
			lua_rawgeti( L, LUA_REGISTRYINDEX, fRef );
			int base = lua_gettop( L ); // get index of 'launchPad'
			lua_getfield( L, -1, "log" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
			lua_insert( L, base ); // swap 'launchPad' and 'launchPad.log'
			lua_pushstring( L, eventName );
			lua_pushstring( L, eventData );
			result = Rtt_VERIFY( 0 == LuaContext::DoCall( L, 3, 0 ) );
		}
	}

	return result;
}

#else

bool
LaunchPad::Initialize( int index )
{
	return true;
}

bool
LaunchPad::Log( const char *eventName, const char *eventData )
{
	return true;
}

#endif // Rtt_DEVICE_ENV

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

