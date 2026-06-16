//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Rtt_WinWebViewObject.h"
#include "Core\Rtt_Build.h"
#include "Corona\CoronaLua.h"
#include "Display\Rtt_Display.h"
#include "Interop\UI\Control.h"
#include "Interop\RuntimeEnvironment.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaAux.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_WinPlatform.h"
#include "Rtt_WinWebPopup.h"
#include <algorithm>
#include <string.h>


namespace Rtt
{

#pragma region Constructors/Destructors
WinWebViewObject::WinWebViewObject(Interop::RuntimeEnvironment& environment, const Rect& bounds)
:	Super(environment, bounds),
	fWebBrowserPointer(nullptr),
	fNavigatingEventHandler(this, &WinWebViewObject::OnNavigating),
	fNavigatedEventHandler(this, &WinWebViewObject::OnNavigated),
	fNavigationFailedEventHandler(this, &WinWebViewObject::OnNavigationFailed)
{
}

WinWebViewObject::~WinWebViewObject()
{
	if (fWebBrowserPointer)
	{
		delete fWebBrowserPointer;
		fWebBrowserPointer = nullptr;
	}
}

#pragma endregion


#pragma region Public Methods
bool WinWebViewObject::Initialize()
{
	// Do not continue if this object was already initialized.
	if (fWebBrowserPointer)
	{
		return true;
	}

	// Fetch the bounds of this display object converted from Corona coordinates to native screen coordinates.
	Rect screenBounds;
	GetScreenBounds(screenBounds);

	// Set up a registry path used to store custom Internet Explorer settings exclusive to this app.
	// This allows our WebBrowser control to override default IE settings.
	std::wstring registryPath = GetRuntimeEnvironment().GetRegistryPathWithoutHive();
	registryPath += L"\\corona\\Internet Explorer";

	// Create and configure the native Win32 web browser control this object will manage.
	Interop::UI::WebBrowser::CreationSettings settings{};
	settings.ParentWindowHandle = GetRuntimeEnvironment().GetRenderSurface()->GetWindowHandle();
	settings.Bounds.left = (LONG)Rtt_RealToInt(screenBounds.xMin);
	settings.Bounds.top = (LONG)Rtt_RealToInt(screenBounds.yMin);
	settings.Bounds.right = (LONG)Rtt_RealToInt(screenBounds.xMax);
	settings.Bounds.bottom = (LONG)Rtt_RealToInt(screenBounds.yMax);
	settings.IEOverrideRegistryPath = registryPath.c_str();
	fWebBrowserPointer = new Interop::UI::WebBrowser(settings);
	if (!fWebBrowserPointer)
	{
		return false;
	}

	// Add event handlers to the native web browser control.
	fWebBrowserPointer->GetNavigatingEventHandlers().Add(&fNavigatingEventHandler);
	fWebBrowserPointer->GetNavigatedEventHandlers().Add(&fNavigatedEventHandler);
	fWebBrowserPointer->GetNavigationFailedEventHandlers().Add(&fNavigationFailedEventHandler);

	// Let the base class finish initialization of this object.
	return WinDisplayObject::Initialize();
}

Interop::UI::Control* WinWebViewObject::GetControl() const
{
	return fWebBrowserPointer;
}

const LuaProxyVTable& WinWebViewObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetWebViewObjectProxyVTable();
}

int WinWebViewObject::ValueForKey(lua_State *L, const char key[]) const
{
	// Validate.
	if (!L || !fWebBrowserPointer || Rtt_StringIsEmpty(key))
	{
		Rtt_ASSERT(0);
		return 0;
	}

	// Handle the Lua property/function retrieval.
	int result = 1;
	if (strcmp("request", key) == 0)
	{
		lua_pushcfunction(L, OnRequest);
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
		lua_pushcfunction(L, OnStop);
	}
	else if (strcmp("back", key) == 0)
	{
		lua_pushcfunction(L, OnBack);
	}
	else if (strcmp("forward", key) == 0)
	{
		lua_pushcfunction(L, OnForward);
	}
	else if (strcmp("reload", key) == 0)
	{
		lua_pushcfunction(L, OnReload);
	}
	else if (strcmp("resize", key) == 0)
	{
		lua_pushcfunction(L, OnResize);
	}
	else if (strcmp("deleteCookies", key) == 0)
	{
		lua_pushcfunction(L, OnDeleteCookies);
	}
	else if (strcmp("bounces", key) == 0)
	{
		CoronaLuaWarning(L, "Native WebViews on Windows do not support the \"%s\" property.", key);
	}
	else if (strcmp("canGoBack", key) == 0)
	{
		lua_pushboolean(L, fWebBrowserPointer->CanNavigateBack() ? 1 : 0);
	}
	else if (strcmp("canGoForward", key) == 0)
	{
		lua_pushboolean(L, fWebBrowserPointer->CanNavigateForward() ? 1 : 0);
	}
	else if (strcmp("load", key) == 0)
	{
		lua_pushcfunction(L, OnLoad);
	}
	else
	{
		result = Super::ValueForKey(L, key);
	}
	return result;
}

bool WinWebViewObject::SetValueForKey(lua_State *L, const char key[], int valueIndex)
{
	// Validate.
	if (!L || !fWebBrowserPointer || Rtt_StringIsEmpty(key))
	{
		Rtt_ASSERT(0);
		return 0;
	}

	// Handle the Lua value assignment.
	bool result = true;
	if (strcmp("bounces", key) == 0)
	{
		CoronaLuaWarning(L, "Native WebViews on Windows do not support the \"%s\" property.", key);
	}
	else
	{
		result = Super::SetValueForKey(L, key, valueIndex);
	}
	return result;
}

#pragma endregion


#pragma region Private Methods
void WinWebViewObject::OnNavigating(
	Interop::UI::WebBrowser& sender, Interop::UI::WebBrowserNavigatingEventArgs& arguments)
{
	// Dispatch a Lua "urlRequest" event indicating that we're about to load a new URL.
	Rtt::UrlRequestEvent event(arguments.GetUrlAsUtf8(), Rtt::UrlRequestEvent::kOther);
	DispatchEventWithTarget(event);
}

void WinWebViewObject::OnNavigated(
	Interop::UI::WebBrowser& sender, const Interop::UI::WebBrowserNavigatedEventArgs& arguments)
{
	// Dispatch a Lua "urlRequest" event indicating that we've finished loading the main web pgae of a URL.
	Rtt::UrlRequestEvent event(arguments.GetUrlAsUtf8(), Rtt::UrlRequestEvent::kLoaded);
	DispatchEventWithTarget(event);
}

void WinWebViewObject::OnNavigationFailed(
	Interop::UI::WebBrowser& sender, const Interop::UI::WebBrowserNavigationFailedEventArgs& arguments)
{
	// Dispatch a Lua "urlRequest" event indicating that we've failed to load a URL or web page.
	Rtt::UrlRequestEvent event(arguments.GetUrlAsUtf8(), arguments.GetErrorMessageAsUtf8(), arguments.GetErrorCode());
	DispatchEventWithTarget(event);
}

#pragma endregion


#pragma region Private Static Functions
int WinWebViewObject::OnLoad(lua_State *L)
{
	CoronaLuaWarning(L, "The native WebView:load() function is not supported on Windows.");
	return 0;
}

int WinWebViewObject::OnRequest(lua_State *L)
{
	// Fetch the WinWebView object.
	auto displayObjectPointer = (WinWebViewObject*)LuaProxy::GetProxyableObject(L, 1);
	if (&displayObjectPointer->ProxyVTable() != &PlatformDisplayObject::GetWebViewObjectProxyVTable())
	{
		return 0;
	}

	// Fetch the URL argument.
	if (lua_type(L, 2) != LUA_TSTRING)
	{
		luaL_error(L, "Function WebView.request() was given an invalid URL argument. Was expecting a string.");
	}
	WinString url(lua_tostring(L, 2));

	// If the optional base directory argument was provided, then update the above URL path with it.
	if (lua_type(L, 3) == LUA_TSTRING)
	{
		WinString baseUrl(lua_tostring(L, 3));
		if (!baseUrl.IsEmpty())
		{
			auto baseUrlLength = baseUrl.GetLength();
			if ((baseUrl.GetUTF16()[baseUrlLength - 1] != '/') && (baseUrl.GetUTF16()[baseUrlLength - 1] != '\\'))
			{
				baseUrl.Append(L'/');
			}
			baseUrl.Append(url);
			url.SetUTF16(baseUrl.GetUTF16());
		}
	}
	else if (lua_islightuserdata(L, 3))
	{
		MPlatform::Directory baseDirectory = (MPlatform::Directory)EnumForUserdata(
				LuaLibSystem::Directories(), lua_touserdata(L, 3), MPlatform::kNumDirs, MPlatform::kUnknownDir);
		if (baseDirectory != MPlatform::kUnknownDir)
		{
			Interop::RuntimeEnvironment& runtimeEnvironment = displayObjectPointer->GetRuntimeEnvironment();
			Rtt::String filePath(&runtimeEnvironment.GetAllocator());
			runtimeEnvironment.GetPlatform()->PathForFile(
					url.GetUTF8(), baseDirectory, MPlatform::kDefaultPathFlags, filePath);
			if (!filePath.IsEmpty())
			{
				url.SetUTF8(filePath.GetString());
			}
		}
	}

	// Navigate to the given URL.
	auto webBrowserPointer = displayObjectPointer->fWebBrowserPointer;
	if (webBrowserPointer)
	{
		webBrowserPointer->NavigateTo(url.GetUTF16());
	}
	return 0;
}

int WinWebViewObject::OnStop(lua_State *L)
{
	auto displayObjectPointer = (WinWebViewObject*)LuaProxy::GetProxyableObject(L, 1);
	if (&displayObjectPointer->ProxyVTable() == &PlatformDisplayObject::GetWebViewObjectProxyVTable())
	{
		if (displayObjectPointer->fWebBrowserPointer)
		{
			displayObjectPointer->fWebBrowserPointer->StopLoading();
		}
	}
	return 0;
}

int WinWebViewObject::OnBack(lua_State *L)
{
	auto displayObjectPointer = (WinWebViewObject*)LuaProxy::GetProxyableObject(L, 1);
	if (&displayObjectPointer->ProxyVTable() == &PlatformDisplayObject::GetWebViewObjectProxyVTable())
	{
		if (displayObjectPointer->fWebBrowserPointer)
		{
			displayObjectPointer->fWebBrowserPointer->NavigateBack();
		}
	}
	return 0;
}

int WinWebViewObject::OnForward(lua_State *L)
{
	auto displayObjectPointer = (WinWebViewObject*)LuaProxy::GetProxyableObject(L, 1);
	if (&displayObjectPointer->ProxyVTable() == &PlatformDisplayObject::GetWebViewObjectProxyVTable())
	{
		if (displayObjectPointer->fWebBrowserPointer)
		{
			displayObjectPointer->fWebBrowserPointer->NavigateForward();
		}
	}
	return 0;
}

int WinWebViewObject::OnResize(lua_State *L)
{
	CoronaLuaWarning(L, "The native WebView:resize() function is not supported on Windows.");
	return 0;
}

int WinWebViewObject::OnReload(lua_State *L)
{
	auto displayObjectPointer = (WinWebViewObject*)LuaProxy::GetProxyableObject(L, 1);
	if (&displayObjectPointer->ProxyVTable() == &PlatformDisplayObject::GetWebViewObjectProxyVTable())
	{
		if (displayObjectPointer->fWebBrowserPointer)
		{
			displayObjectPointer->fWebBrowserPointer->Reload();
		}
	}
	return 0;
}

int WinWebViewObject::OnDeleteCookies(lua_State *L)
{
	CoronaLuaWarning(L, "The native WebView:deleteCookies() function is not supported on Windows.");
	return 0;
}

int WinWebViewObject::InjectJS(lua_State *L)
{
	CoronaLuaWarning(L, "The native WebView:injectJS() function is not supported on Windows.");
	return 0;
}

int WinWebViewObject::RegisterCallback(lua_State *L)
{
	CoronaLuaWarning(L, "The native WebView:registerCallback() function is not supported on Windows.");
	return 0;
}

int WinWebViewObject::On(lua_State *L)
{
	CoronaLuaWarning(L, "The native WebView:on() function is not supported on Windows.");
	return 0;
}

int WinWebViewObject::Send(lua_State *L)
{
	CoronaLuaWarning(L, "The native WebView:send() function is not supported on Windows.");
	return 0;
}

#pragma endregion

} // namespace Rtt
