// ----------------------------------------------------------------------------
// 
// Rtt_WinRTWebViewObject.cpp
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "Rtt_WinRTWebViewObject.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Rtt_Lua.h"
#	include "Rtt_LuaContext.h"
#	include "Rtt_LuaLibSystem.h"
#	include "Rtt_LuaProxy.h"
#	include "Rtt_LuaProxyVTable.h"
#	include "Rtt_Runtime.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region Constructors/Destructors
WinRTWebViewObject::WinRTWebViewObject(const Rect& bounds)
:	Super(bounds)
{
}

WinRTWebViewObject::~WinRTWebViewObject()
{
}

#pragma endregion


#pragma region Public Member Functions
bool WinRTWebViewObject::Initialize()
{
//TODO: Figure out why GetScreenBounds() is triggering an assert.
//	Rect bounds;
//	GetScreenBounds(bounds);
	return true;
}

const LuaProxyVTable& WinRTWebViewObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetWebViewObjectProxyVTable();
}

int WinRTWebViewObject::ValueForKey(lua_State *L, const char key[]) const
{
	Rtt_ASSERT(key);

	int result = 1;

	if (strcmp("request", key) == 0)
	{
		lua_pushcfunction(L, Request);
	}
	else if (strcmp("stop", key) == 0)
	{
		lua_pushcfunction(L, Stop);
	}
	else if (strcmp("back", key) == 0)
	{
		lua_pushcfunction(L, Back);
	}
	else if (strcmp("forward", key) == 0)
	{
		lua_pushcfunction(L, Forward);
	}
	else if (strcmp("reload", key) == 0)
	{
		lua_pushcfunction(L, Reload);
	}
	else if (strcmp("resize", key) == 0)
	{
		lua_pushcfunction(L, Resize);
	}
	else if (strcmp("bounces", key) == 0)
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
		//		Rtt_PRINT( ( "WARNING: Web views do not have bounce behavior on this platform.\n" ) );
	}
	else if (strcmp("canGoBack", key) == 0)
	{
		//lua_pushboolean( L, fCanGoBack ? 1 : 0 );
	}
	else if (strcmp("canGoForward", key) == 0)
	{
		//lua_pushboolean( L, fCanGoForward ? 1 : 0 );
	}
	else if (strcmp("load", key) == 0)
	{
		Rtt_PRINT(("WARNING: Web views do not have load behavior on this platform.\n"));
		lua_pushcfunction(L, Load);
	}
	else
	{
		result = Super::ValueForKey(L, key);
	}

	return result;
}

bool WinRTWebViewObject::SetValueForKey(lua_State *L, const char key[], int valueIndex)
{
	Rtt_ASSERT(key);

	bool result = true;

	if (strcmp("bounces", key) == 0)
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
		//		Rtt_PRINT( ( "WARNING: Web views do not have bounce behavior on this platform.\n" ) );
	}
	else if (strcmp("request", key) == 0
		|| strcmp("stop", key) == 0
		|| strcmp("back", key) == 0
		|| strcmp("forward", key) == 0
		|| strcmp("reload", key) == 0
		|| strcmp("resize", key) == 0)
	{
		// no-op
	}
	else
	{
		result = Super::SetValueForKey(L, key, valueIndex);
	}

	return result;
}

#pragma endregion


#pragma region Protected Static Functions
int WinRTWebViewObject::Load(lua_State *L)
{
	return 0;
}

int WinRTWebViewObject::Request(lua_State *L)
{
	// Fetch the web view from the Lua object.
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	WinRTWebViewObject *view = (WinRTWebViewObject *)luaL_todisplayobject(L, 1, table);
	if (!view)
	{
		return 0;
	}

	// Fetch the URL argument.
	if (lua_type(L, 2) != LUA_TSTRING)
	{
		luaL_error(L, "Function WebView.request() was given an invalid URL argument. Was expecting a string.");
	}
	const char *url = lua_tostring(L, 2);

	// Fetch the base directory, if provided, and then fetch the requested web page.
	if (lua_type(L, 3) == LUA_TSTRING)
	{
		//view->Request(url, lua_tostring(L, 3));
	}
	else if (lua_islightuserdata(L, 3))
	{
		MPlatform::Directory baseDirectory = (MPlatform::Directory)EnumForUserdata(
			LuaLibSystem::Directories(),
			lua_touserdata(L, 3),
			MPlatform::kNumDirs,
			MPlatform::kUnknownDir);
		//view->Request(url, baseDirectory);
	}
	else
	{
		//view->Request(url, NULL);
	}
	return 0;
}

int WinRTWebViewObject::Stop(lua_State *L)
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	WinRTWebViewObject *view = (WinRTWebViewObject *)luaL_todisplayobject(L, 1, table);
	if (view)
	{
		//NativeToJavaBridge::GetInstance()->WebViewRequestStop(view->GetId());
	}
	return 0;
}

int WinRTWebViewObject::Back(lua_State *L)
{
	return 0;
}

int WinRTWebViewObject::Forward(lua_State *L)
{
	return 0;
}

int WinRTWebViewObject::Resize(lua_State *L)
{
	return 0;
}

int WinRTWebViewObject::Reload(lua_State *L)
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	WinRTWebViewObject *view = (WinRTWebViewObject *)luaL_todisplayobject(L, 1, table);
	if (view)
	{
		//NativeToJavaBridge::GetInstance()->WebViewRequestReload(view->GetId());
	}
	return 0;
}

#pragma endregion

} // namespace Rtt
