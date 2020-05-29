//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Rtt_WinVideoObject.h"
#include "Core\Rtt_Build.h"
#include "Interop\UI\Control.h"
#include "Interop\RuntimeEnvironment.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibMedia.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include <string.h>
#ifdef Rtt_DEBUG
#	include "Rtt_PlatformSurface.h"
#	include "Rtt_Runtime.h"
#endif


namespace Rtt
{

WinVideoObject::WinVideoObject(Interop::RuntimeEnvironment& environment, const Rect& bounds)
:	Super(environment, bounds)
{
}

WinVideoObject::~WinVideoObject()
{
}

bool
WinVideoObject::Initialize()
{
	Rtt_ASSERT_NOT_IMPLEMENTED();

	return false;
}

Interop::UI::Control* WinVideoObject::GetControl() const
{
	return nullptr;
}

const LuaProxyVTable&
WinVideoObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetVideoObjectProxyVTable();
}

// view:load( path [, baseSource ] )
int
WinVideoObject::Load( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	WinVideoObject *o = (WinVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
	}

	return 0;
}

// view:play()
int
WinVideoObject::Play( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	WinVideoObject *o = (WinVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
	}

	return 0;
}

// view:pause()
int
WinVideoObject::Pause( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	WinVideoObject *o = (WinVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
	}

	return 0;
}

// view:seek( t )
int
WinVideoObject::Seek( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	WinVideoObject *o = (WinVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
	}

	return 0;
}

int
WinVideoObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;

	if ( strcmp( "currentTime", key ) == 0 )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
		lua_pushnumber( L, 0 );
	}
	else if ( strcmp( "totalTime", key ) == 0 )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
		lua_pushnumber( L, 0 );
	}
	else if ( strcmp( "load", key ) == 0 )
	{
		lua_pushcfunction( L, Load );
	}
	else if ( strcmp( "isMuted", key ) == 0 )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
		lua_pushboolean( L, false );
	}
	else if ( strcmp( "fillMode", key ) == 0 )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
		lua_pushstring( L, "letterbox" );
	}
	else if ( strcmp( "play", key ) == 0 )
	{
		lua_pushcfunction( L, Play );
	}
	else if ( strcmp( "pause", key ) == 0 )
	{
		lua_pushcfunction( L, Pause );
	}
	else if ( strcmp( "seek", key ) == 0 )
	{
		lua_pushcfunction( L, Seek );
	}
	else if ( strcmp( "isToggleEnabled", key ) == 0 )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
		lua_pushboolean( L, false );
	}
	else
	{
		result = Super::ValueForKey( L, key );
	}

	return result;
}

bool
WinVideoObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	if ( strcmp( "isToggleEnabled", key ) == 0 )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
	}
	else if ( strcmp( "isMuted", key ) == 0 )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
	}
	else if ( strcmp( "fillMode", key ) == 0 )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
		Rtt_PRINT( ( "WARNING: video.fillMode is not supported on this platform.\n" ) );
	}
	else
	{
		result = Super::SetValueForKey( L, key, valueIndex );
	}
		
	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

