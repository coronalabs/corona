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

#include "Rtt_LuaUserdataProxy.h"

#include "Core/Rtt_Array.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "CoronaLua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const char kMetatableName[] = __FILE__; // unique identifier for this userdata type

LuaUserdataProxy *
LuaUserdataProxy::ToProxy( lua_State* L, int index )
{
	LuaUserdataProxy *result = NULL;

#ifdef Rtt_DEVICE_ENV
	LuaUserdataProxy **ud = (LuaUserdataProxy **)lua_touserdata( L, index );
#else
	LuaUserdataProxy **ud = (LuaUserdataProxy **)luaL_checkudata( L, index, kMetatableName );
#endif

	if ( ud )
	{
		result = *ud;
	}

	return result;
}

int
LuaUserdataProxy::Index( lua_State *L )
{
    return Index( L, 0 );
}

int
LuaUserdataProxy::Index( lua_State *L, int base )
{
	int result = 0;

	LuaUserdataProxy *proxy = ToProxy( L, base + 1 );

	if ( proxy )
	{
		const char *key = lua_tostring( L, base + 2 );

		if ( key )
		{
			const MLuaUserdataAdapter *adapter = proxy->GetAdapter();

			if ( adapter )
            {
                result = adapter->ValueForKey( * proxy, L, key );

                if (result == 0 && strcmp( key, "_properties" ) == 0)
                {
                    const char **keys = NULL;
                    int numKeys = adapter->GetHash( L )->GetKeys( keys );
                    String resultStr( LuaContext::GetAllocator( L ) );

                    if ( LuaUserdataProxy::PropertiesToJSON( L, proxy, keys, numKeys, resultStr ) > 0 )
                    {
                        lua_pushstring( L, resultStr.GetString() );
                        result = 1;
                    }
                }
            }
        }
    }

	return result;
}

int
LuaUserdataProxy::NewIndex( lua_State *L )
{
	LuaUserdataProxy *proxy = ToProxy( L, 1 );
	if ( proxy )
	{
		const char *key = lua_tostring( L, 2 );
		if ( key )
		{
			const MLuaUserdataAdapter *adapter = proxy->GetAdapter();
			if ( adapter )
			{
				adapter->SetValueForKey( * proxy, L, key, 3 );
			}
		}
	}

	return 0;
}

int
LuaUserdataProxy::GC( lua_State *L )
{
	LuaUserdataProxy **ud = (LuaUserdataProxy **)luaL_checkudata( L, 1, kMetatableName );
	if ( ud )
	{
		LuaUserdataProxy *proxy = ( *ud );
		proxy->ReleaseRef( L );
		Rtt_DELETE( proxy );
	}

	return 0;
}

void
LuaUserdataProxy::Initialize( lua_State *L )
{
	Rtt_LUA_STACK_GUARD( L );

	const luaL_Reg kVTable[] =
	{
		{ "__index", Index },
		{ "__newindex", NewIndex },
		{ "__gc", GC },
		{ NULL, NULL }
	};

	Lua::InitializeMetatable( L, kMetatableName, kVTable );
}

LuaUserdataProxy *
LuaUserdataProxy::New( lua_State *L, void *object )
{
	LuaUserdataProxy *result =
		Rtt_NEW( LuaContext::GetAllocator( L ), LuaUserdataProxy( object ) );
	result->AllocRef( L );

	return result;
}

// ----------------------------------------------------------------------------

LuaUserdataProxy::LuaUserdataProxy( void *object )
:	fObject( object ),
	fAdapter( NULL ),
	fRef( LUA_NOREF )
{
}

LuaUserdataProxy::~LuaUserdataProxy()
{
}

void
LuaUserdataProxy::DetachUserdata()
{
	if ( fObject && fAdapter )
	{
		fAdapter->WillFinalize( * this );
	}

	fObject = NULL;
	fAdapter = NULL;
}

void
LuaUserdataProxy::AllocRef( lua_State *L )
{
	if ( Rtt_VERIFY( LUA_NOREF == fRef ) )
	{
		Rtt_LUA_STACK_GUARD( L );

		Lua::PushUserdata( L, this, kMetatableName );
		Rtt_ASSERT( lua_isuserdata( L, -1 ) );

		fRef = luaL_ref( L, LUA_REGISTRYINDEX );
		Rtt_ASSERT( LUA_NOREF != fRef );
	}
}

void
LuaUserdataProxy::ReleaseRef( lua_State *L )
{
	DetachUserdata();

	if ( LUA_NOREF != fRef )
	{
		luaL_unref( L, LUA_REGISTRYINDEX, fRef );
		fRef = LUA_NOREF;
	}
}

void
LuaUserdataProxy::Push( lua_State *L ) const
{
	Rtt_ASSERT( LUA_NOREF != fRef );

	lua_rawgeti( L, LUA_REGISTRYINDEX, fRef );
}

// ----------------------------------------------------------------------------

// Utility function to translate userdata proxy properties to JSON
int
LuaUserdataProxy::PropertiesToJSON( lua_State *L, const LuaUserdataProxy *proxy, const char **keys, const int numKeys, String& result )
{
    const MLuaUserdataAdapter *adapter = proxy->GetAdapter();
    const int bufLen = 10240;
    char buf[bufLen];
    int numPropertiesAdded = 0;

    for (int k = 0; k < numKeys; k++)
    {
        if (strchr(keys[k], '#'))
        {
            // Deprecated property, skip it
            continue;
        }

        int res = adapter->ValueForKey( *proxy, L, keys[k] );

        if (res > 0)
        {
            buf[0] = '\0';

            CoronaLuaPropertyToJSON(L, -1, keys[k], buf, bufLen, 0);

            if (! result.IsEmpty() && strlen(buf) > 0)
            {
                result.Append(", ");
            }
            
            result.Append(buf);

            ++numPropertiesAdded;

            lua_pop( L, res );
        }
    }
    
    return numPropertiesAdded;
}

} // namespace Rtt

// ----------------------------------------------------------------------------

