// ----------------------------------------------------------------------------
// 
// Rtt_WinRTVideoObject.cpp
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "Rtt_WinRTVideoObject.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Rtt_Lua.h"
#	include "Rtt_LuaContext.h"
#	include "Rtt_LuaLibMedia.h"
#	include "Rtt_LuaProxy.h"
#	include "Rtt_LuaProxyVTable.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region Constructors/Destructors
WinRTVideoObject::WinRTVideoObject(const Rect& bounds)
:	Super( bounds )
{
}

WinRTVideoObject::~WinRTVideoObject()
{
}

#pragma endregion


#pragma region Public Member Functions
bool WinRTVideoObject::Initialize()
{
//TODO: Figure out why GetScreenBounds() is triggering an assert.
//	Rect bounds;
//	GetScreenBounds(bounds);
	return true;
}

const LuaProxyVTable& WinRTVideoObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetVideoObjectProxyVTable();
}

int WinRTVideoObject::ValueForKey(lua_State *L, const char key[]) const
{
	Rtt_ASSERT(key);

	int result = 1;

	if (strcmp("currentTime", key) == 0)
	{
	}
	else if (strcmp("totalTime", key) == 0)
	{
	}
	else if (strcmp("isMuted", key) == 0)
	{
	}
	else if (strcmp("fillMode", key) == 0)
	{
	}
	else if (strcmp("load", key) == 0)
	{
		lua_pushcfunction(L, Load);
	}
	else if (strcmp("play", key) == 0)
	{
		lua_pushcfunction(L, Play);
	}
	else if (strcmp("pause", key) == 0)
	{
		lua_pushcfunction(L, Pause);
	}
	else if (strcmp("seek", key) == 0)
	{
		lua_pushcfunction(L, Seek);
	}
	else if (strcmp("isToggleEnabled", key) == 0)
	{
	}
	else
	{
		result = Super::ValueForKey(L, key);
	}

	return result;
}

bool WinRTVideoObject::SetValueForKey(lua_State *L, const char key[], int valueIndex)
{
	Rtt_ASSERT(key);

	bool result = true;

	if (strcmp("isToggleEnabled", key) == 0)
	{
	}
	else if (strcmp("isMuted", key) == 0)
	{
	}
	else if (strcmp("fillMode", key) == 0)
	{
	}
	else
	{
		result = Super::SetValueForKey(L, key, valueIndex);
	}

	return result;
}

#pragma endregion


#pragma region Protected Static Functions
int WinRTVideoObject::Load(lua_State *L)
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	WinRTVideoObject *o = (WinRTVideoObject *)luaL_todisplayobject( L, 1, table );

	const char * source = NULL;
	if ( lua_isstring( L, 2 ) )
	{
		source = lua_tostring( L, 2 );
	}

	if ( o && source )
	{
//		NativeToJavaBridge::GetInstance()->VideoViewLoad(o->GetId(), source);
	}

	return 0;
}

int WinRTVideoObject::Play( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	WinRTVideoObject *o = (WinRTVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
//		NativeToJavaBridge::GetInstance()->VideoViewPlay(o->GetId());
	}
	return 0;
}

int WinRTVideoObject::Pause( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	WinRTVideoObject *o = (WinRTVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
//		NativeToJavaBridge::GetInstance()->VideoViewPause(o->GetId());
	}
	return 0;
}

int WinRTVideoObject::Seek( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	WinRTVideoObject *o = (WinRTVideoObject *)luaL_todisplayobject( L, 1, table );
	int seekTo = -1;
	if ( lua_isnumber( L, 2 ) )
	{
		seekTo = (int)lua_tonumber( L, 2 );
	}
	if ( o && seekTo > -1)
	{
//		NativeToJavaBridge::GetInstance()->VideoViewSeek(o->GetId(), seekTo);
	}
	return 0;
}

#pragma endregion

} // namespace Rtt
