//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"
#include "Rtt_AndroidWebViewObject.h"

#include "Core/Rtt_String.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Runtime.h"
#include "NativeToJavaBridge.h"
#include "JavaToNativeBridge.h"


// ----------------------------------------------------------------------------

namespace Rtt
{
// ----------------------------------------------------------------------------

static const char* kCoronaEventPrefix = "JS_";

AndroidWebViewObject::AndroidWebViewObject(
		const Rect& bounds, AndroidDisplayObjectRegistry *displayObjectRegistry, NativeToJavaBridge *ntjb )
:	Super( bounds, displayObjectRegistry, ntjb ),
	fIsPopup( false ),
	fAutoCancelEnabled( false ),
	fCanGoBack( false ),
	fCanGoForward( false )
{
}

AndroidWebViewObject::~AndroidWebViewObject()
{
}

bool
AndroidWebViewObject::Initialize()
{
	Rect bounds;
	GetScreenBounds(bounds);
	fNativeToJavaBridge->WebViewCreate(
			GetId(), bounds.xMin, bounds.yMin, bounds.Width(), bounds.Height(), fIsPopup, fAutoCancelEnabled);
	return true;
}

bool
AndroidWebViewObject::InitializeAsPopup(bool autoCancelEnabled)
{
	fIsPopup = true;
	fAutoCancelEnabled = autoCancelEnabled;
	return Initialize();
}

const LuaProxyVTable&
AndroidWebViewObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetWebViewObjectProxyVTable();
}
int
AndroidWebViewObject::Load( lua_State *L )
{

	return 0;
}

// view:request( url )
int
AndroidWebViewObject::Request( lua_State *L )
{
	// Fetch the web view from the Lua object.
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	AndroidWebViewObject *view = (AndroidWebViewObject *)luaL_todisplayobject(L, 1, table);
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
		view->Request(url, lua_tostring(L, 3));
	}
	else if (lua_islightuserdata(L, 3))
	{
		MPlatform::Directory baseDirectory = (MPlatform::Directory)EnumForUserdata(
												LuaLibSystem::Directories(),
												lua_touserdata( L, 3 ),
												MPlatform::kNumDirs,
												MPlatform::kUnknownDir);
		view->Request(url, baseDirectory);
	}
	else
	{
		view->Request(url, NULL);
	}
	return 0;
}

void
AndroidWebViewObject::Request( const char *url, const MPlatform::Directory baseDirectory )
{
	// Validate.
	if (!url)
	{
		return;
	}
	
	// Request the web page without a base directory if given:
	// 1) An unknown base directory.
	// 2) A URL prefixed with "http" or "https".
	if ((MPlatform::kUnknownDir == baseDirectory) ||
	    Rtt_StringStartsWith(url, "http:") ||
	    Rtt_StringStartsWith(url, "https:"))
	{
		Request(url, NULL);
		return;
	}
	
	// Set up a "base URL" for the given base directory.
	Runtime *runtime = fNativeToJavaBridge->GetRuntime();
	const MPlatform& platform = runtime->Platform();
	String baseUrl(&platform.GetAllocator());
	if (MPlatform::kResourceDir == baseDirectory)
	{
	}
	else
	{
		baseUrl.Set( "file://");
		String baseDirectoryPath(&platform.GetAllocator());
		platform.PathForFile(NULL, baseDirectory, MPlatform::kDefaultPathFlags, baseDirectoryPath );
		baseUrl.Append(baseDirectoryPath.GetString());
		baseUrl.Append("/");
	}
	
	// Request the web page.
	Request(url, baseUrl.GetString());
}

void
AndroidWebViewObject::Request( const char *url, const char *baseUrl )
{
	if (url)
	{
		Runtime *runtime = fNativeToJavaBridge->GetRuntime();
		const MPlatform& platform = runtime->Platform();
		if (baseUrl)
		{
			String path(&platform.GetAllocator(), baseUrl);
			path.Append(url);
			fNativeToJavaBridge->WebViewRequestLoadUrl(GetId(), path.GetString());
		}
		else
		{
			fNativeToJavaBridge->WebViewRequestLoadUrl(GetId(), url);
		}
	}
}

int
AndroidWebViewObject::Stop( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	AndroidWebViewObject *view = (AndroidWebViewObject *)luaL_todisplayobject(L, 1, table);
	if (view)
	{
		NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );
		bridge->WebViewRequestStop(view->GetId());
	}
	return 0;
}

int
AndroidWebViewObject::Back( lua_State *L )
{
	bool canGoBack = false;
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	AndroidWebViewObject *view = (AndroidWebViewObject *)luaL_todisplayobject(L, 1, table);
	if (view && view->CanGoBack())
	{
		NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );
		bridge->WebViewRequestGoBack(view->GetId());
		canGoBack = true;
	}
	lua_pushboolean(L, canGoBack ? 1 : 0);
	return 1;
}

int
AndroidWebViewObject::Forward( lua_State *L )
{
	bool canGoForward = false;
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	AndroidWebViewObject *view = (AndroidWebViewObject *)luaL_todisplayobject(L, 1, table);
	if (view && view->CanGoForward())
	{
		NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );
		bridge->WebViewRequestGoForward(view->GetId());
		canGoForward = true;
	}
	lua_pushboolean(L, canGoForward ? 1 : 0);
	return 1;
}

int
AndroidWebViewObject::Resize( lua_State *L )
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
	return 0;
}

int
AndroidWebViewObject::Reload( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	AndroidWebViewObject *view = (AndroidWebViewObject *)luaL_todisplayobject(L, 1, table);
	if (view)
	{
		NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );
		bridge->WebViewRequestReload(view->GetId());
	}
	return 0;
}

int
AndroidWebViewObject::DeleteCookies( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	AndroidWebViewObject *view = (AndroidWebViewObject *)luaL_todisplayobject(L, 1, table);
	if (view)
	{
		NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );
		bridge->WebViewRequestDeleteCookies(view->GetId());
	}
	return 0;
}

int
AndroidWebViewObject::InjectJS( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	AndroidWebViewObject *view = (AndroidWebViewObject *)luaL_todisplayobject(L, 1, table);
	if ( view )
	{
		const char *jsCode = lua_tostring( L, 2 );
		view->InjectJSCode( jsCode );
	}

	return 0;
}

int
AndroidWebViewObject::RegisterCallback( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	AndroidWebViewObject *view = (AndroidWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( view )
	{
		const char *eventName = lua_tostring( L, 2 );
		String jsEventName(kCoronaEventPrefix);
		jsEventName.Append( eventName );
		view->AddEventListener( L, 3, jsEventName.GetString() );
	}

	return 0;
}

int
AndroidWebViewObject::On( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	AndroidWebViewObject *view = (AndroidWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( view )
	{
		const char *eventName = lua_tostring( L, 2 );
		String jsEventName(kCoronaEventPrefix);
		jsEventName.Append( eventName );
		view->AddEventListener( L, 3, jsEventName.GetString() );
	}

	return 0;
}

int
AndroidWebViewObject::Send( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	AndroidWebViewObject *view = (AndroidWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( view )
	{
		const char* eventName = lua_tostring( L, 2 );
		const char* jsonContent = "{}";
		if ( 0 == LuaContext::JsonEncode( L, 3 ) )
		{
			jsonContent = lua_tostring( L, -1 );
		}

		String s( "window.dispatchEvent(new CustomEvent('" );
		s.Append( kCoronaEventPrefix );
		s.Append( eventName );
		s.Append( "', {detail: " );
		s.Append( jsonContent );
		s.Append( "}));" );

		view->InjectJSCode( s.GetString() );
	}

	return 0;
}

void
AndroidWebViewObject::InjectJSCode( const char *jsCode )
{
	fNativeToJavaBridge->WebViewRequestInjectJS( GetId(), jsCode );
}

int
AndroidWebViewObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;
	
	if ( strcmp( "request", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, Request, 1 );
	}
	else if ( strcmp( "injectJS", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, InjectJS, 1 );
	}
	else if ( strcmp( "registerCallback", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, RegisterCallback, 1 );
	}
	else if ( strcmp( "on", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, On, 1 );
	}
	else if ( strcmp( "send", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, Send, 1 );
	}
	else if ( strcmp( "stop", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, Stop, 1 );
	}
	else if ( strcmp( "back", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, Back, 1 );
	}
	else if ( strcmp( "forward", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, Forward, 1 );
	}
	else if ( strcmp( "reload", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, Reload, 1 );
	}
	else if ( strcmp( "resize", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, Resize, 1 );
	}
	else if ( strcmp( "deleteCookies", key ) == 0 )
	{
		lua_pushlightuserdata( L, fNativeToJavaBridge );
		lua_pushcclosure( L, DeleteCookies, 1 );
	}
	else if ( strcmp( "bounces", key ) == 0 )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
//		Rtt_PRINT( ( "WARNING: Web views do not have bounce behavior on this platform.\n" ) );
	}
	else if ( strcmp( "canGoBack", key ) == 0 )
	{
		lua_pushboolean( L, fCanGoBack ? 1 : 0 );
	}
	else if ( strcmp( "canGoForward", key ) == 0 )
	{
		lua_pushboolean( L, fCanGoForward ? 1 : 0 );
	}
	else if ( strcmp( "load", key ) == 0 )
	{
		Rtt_PRINT( ( "WARNING: Web views do not have load behavior on this platform.\n" ) );
		lua_pushcfunction( L, Load );
	}
	else
	{
		result = Super::ValueForKey( L, key );
	}

	return result;
}

bool
AndroidWebViewObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	if ( strcmp( "bounces", key ) == 0 )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
//		Rtt_PRINT( ( "WARNING: Web views do not have bounce behavior on this platform.\n" ) );
	}
	else if ( strcmp( "request", key ) == 0
		 || strcmp( "stop", key ) == 0
		 || strcmp( "back", key ) == 0
		 || strcmp( "forward", key ) == 0
		 || strcmp( "reload", key ) == 0
		 || strcmp( "resize", key ) == 0 )
	{
		// no-op
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

