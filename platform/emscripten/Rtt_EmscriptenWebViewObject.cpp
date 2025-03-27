//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_EmscriptenWebViewObject.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Runtime.h"

#if defined(EMSCRIPTEN)
#include "emscripten/emscripten.h"		// for native alert and etc
extern "C"
{
	extern void jsWebViewOpen(int id, const char* file);
}
#else
	void jsWebViewOpen(int id, const char* file) {}
#endif

namespace Rtt
{

#pragma region Constructors/Destructors
	EmscriptenWebViewObject::EmscriptenWebViewObject(const Rect& bounds)
		: Super(bounds, "iframe")
	{
	}

	EmscriptenWebViewObject::~EmscriptenWebViewObject()
	{
	}

#pragma endregion


#pragma region Public Member Functions
	bool EmscriptenWebViewObject::Initialize()
	{
		return Super::Initialize();
	}

	const LuaProxyVTable& EmscriptenWebViewObject::ProxyVTable() const
	{
		return PlatformDisplayObject::GetWebViewObjectProxyVTable();
	}

	int EmscriptenWebViewObject::ValueForKey(lua_State *L, const char key[]) const
	{
		Rtt_ASSERT(key);

		int result = 1;

		if (strcmp("request", key) == 0)
		{
			lua_pushcfunction(L, Request);
		}
		else if ( strcmp( "injectJS", key ) == 0 )
		{
			lua_pushcfunction( L, InjectJS );
		}
		else if ( strcmp( "registerCallback", key ) == 0 )
		{
			lua_pushcfunction( L, RegisterCallback );
		}
		else if ( strcmp( "on", key ) == 0 )
		{
			lua_pushcfunction( L, On );
		}
		else if ( strcmp( "send", key ) == 0 )
		{
			lua_pushcfunction( L, Send );
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

	bool EmscriptenWebViewObject::SetValueForKey(lua_State *L, const char key[], int valueIndex)
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
	int EmscriptenWebViewObject::Load(lua_State *L)
	{
		return 0;
	}

	int EmscriptenWebViewObject::Request(lua_State *L)
	{
		// Fetch the web view from the Lua object.
		const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
		EmscriptenWebViewObject *view = (EmscriptenWebViewObject *)luaL_todisplayobject(L, 1, table);
		if (view == NULL)
		{
			return 0;
		}

		// Fetch the URL argument.
		if (lua_type(L, 2) != LUA_TSTRING)
		{
			luaL_error(L, "Function WebView.request() was given an invalid URL argument. Was expecting a string.");
		}

		const char *url = lua_tostring(L, 2);
		jsWebViewOpen(view->fElementID, url);
		return 0;
	}

	int EmscriptenWebViewObject::Stop(lua_State *L)
	{
		const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
		EmscriptenWebViewObject *view = (EmscriptenWebViewObject *)luaL_todisplayobject(L, 1, table);
		if (view)
		{
			//NativeToJavaBridge::GetInstance()->WebViewRequestStop(view->GetId());
		}
		return 0;
	}

	int EmscriptenWebViewObject::Back(lua_State *L)
	{
		return 0;
	}

	int EmscriptenWebViewObject::Forward(lua_State *L)
	{
		return 0;
	}

	int EmscriptenWebViewObject::Resize(lua_State *L)
	{
		return 0;
	}

	int EmscriptenWebViewObject::Reload(lua_State *L)
	{
		const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
		EmscriptenWebViewObject *view = (EmscriptenWebViewObject *)luaL_todisplayobject(L, 1, table);
		if (view)
		{
			//NativeToJavaBridge::GetInstance()->WebViewRequestReload(view->GetId());
		}
		return 0;
	}

	int EmscriptenWebViewObject::InjectJS(lua_State *L)
	{
		return 0;
	}

	int EmscriptenWebViewObject::RegisterCallback(lua_State *L)
	{
		return 0;
	}

	int EmscriptenWebViewObject::On(lua_State *L)
	{
		return 0;
	}

	int EmscriptenWebViewObject::Send(lua_State *L)
	{
		return 0;
	}

#pragma endregion

	// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
