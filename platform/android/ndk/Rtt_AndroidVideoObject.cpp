//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"

#include "Rtt_AndroidVideoObject.h"
#include "Rtt_AndroidPlatform.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibMedia.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Runtime.h"
#include "Rtt_MPlatform.h"
#include "NativeToJavaBridge.h"
//#include "Rtt_Rendering.h"

#ifdef Rtt_DEBUG
	// Used in asserts in Initialize()
	#include "Rtt_PlatformSurface.h"
#endif
#include <android/log.h>
// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AndroidVideoObject::AndroidVideoObject(
		const Rect& bounds, AndroidDisplayObjectRegistry *displayObjectRegistry, NativeToJavaBridge *ntjb )
:	Super( bounds, displayObjectRegistry, ntjb )
{
}

AndroidVideoObject::~AndroidVideoObject()
{
}

bool
AndroidVideoObject::Initialize()
{
	Rect bounds;
	GetScreenBounds(bounds);
	fNativeToJavaBridge->VideoViewCreate(
			GetId(), bounds.xMin, bounds.yMin, bounds.Width(), bounds.Height() );
	return true;
}

const LuaProxyVTable&
AndroidVideoObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetVideoObjectProxyVTable();
}

// view:load( path [, baseSource ] )
int
AndroidVideoObject::Load( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	AndroidVideoObject *o = (AndroidVideoObject *)luaL_todisplayobject( L, 1, table );

	NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );
	String sourceWithPath(bridge->GetRuntime()->Allocator());
		
	bool isRemote = false;
	int nextArg = 2;
	
	// Get the absolute path to this file and pass it to Java.
	const char * source = LuaLibMedia::GetLocalOrRemotePath( L, nextArg, sourceWithPath, isRemote );

	if ( o && source )
	{
		bridge->VideoViewLoad(o->GetId(), source);
	}

	return 0;
}

// view:play()
int
AndroidVideoObject::Play( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	AndroidVideoObject *o = (AndroidVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );
		bridge->VideoViewPlay(o->GetId());
	}

	return 0;
}

// view:pause()
int
AndroidVideoObject::Pause( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	AndroidVideoObject *o = (AndroidVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );
		bridge->VideoViewPause(o->GetId());
	}

	return 0;
}

// view:seek( t )
int
AndroidVideoObject::Seek( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	AndroidVideoObject *o = (AndroidVideoObject *)luaL_todisplayobject( L, 1, table );
	int seekTo = -1;
	if ( lua_isnumber( L, 2 ) )
	{
		seekTo = (int)lua_tonumber( L, 2 );
	}
	if ( o && seekTo > -1)
	{
		NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );
		bridge->VideoViewSeek(o->GetId(), seekTo);
	}

	return 0;
}

int
AndroidVideoObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;

	if ( strcmp( "currentTime", key ) == 0 )
	{
		int result = fNativeToJavaBridge->VideoViewGetCurrentTime(GetId());
		lua_pushnumber( L, result );
	}
	else if ( strcmp( "totalTime", key ) == 0 )
	{
		int result = fNativeToJavaBridge->VideoViewGetTotalTime(GetId());
		lua_pushnumber( L, result );
	}
	else if ( strcmp( "isMuted", key ) == 0 )
	{
		bool result = fNativeToJavaBridge->VideoViewGetIsMuted(GetId());
		lua_pushboolean( L, result );
	}
	else if ( strcmp( "fillMode", key ) == 0 )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
		lua_pushstring( L, "letterbox" );
	}
	else if ( strcmp( "load", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, Load, 1 );
	}
	else if ( strcmp( "play", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, Play, 1 );
	}
	else if ( strcmp( "pause", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, Pause, 1 );
	}
	else if ( strcmp( "seek", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, Seek, 1 );
	}
	else if ( strcmp( "isToggleEnabled", key ) == 0 )
	{
		bool result = false;
		result = fNativeToJavaBridge->VideoViewGetIsTouchTogglesPlay(GetId());
		lua_pushboolean( L, result );
	}
	else if ( strcmp( "isPaused", key ) == 0 )
	{
		bool result = (fNativeToJavaBridge->VideoViewGetIsPlaying(GetId()) == false);
		lua_pushboolean( L, result ? 1 : 0 );
	}
	else
	{
		result = Super::ValueForKey( L, key );
	}

	return result;
}

bool
AndroidVideoObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	if ( strcmp( "isToggleEnabled", key ) == 0 )
	{
		bool toggle = false;
		if ( lua_isboolean( L, valueIndex ) )
		{
			toggle = lua_toboolean( L, valueIndex );
		}
		fNativeToJavaBridge->VideoViewTouchTogglesPlay(GetId(), toggle);
	}
	else if ( strcmp( "isMuted", key ) == 0 )
	{
		bool mute = false;
		if ( lua_isboolean( L, valueIndex ) )
		{
			mute = lua_toboolean( L, valueIndex );
		}
		fNativeToJavaBridge->VideoViewMute(GetId(), mute);
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

