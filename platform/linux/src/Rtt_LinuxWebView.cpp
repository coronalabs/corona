//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_LinuxWebView.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Runtime.h"
#include "Rtt_LinuxApp.h"
#include "Rtt_LinuxPlatform.h"
#include "CoronaLua.h"
#include "Rtt_LuaResource.h"
#include "Rtt_Renderer.h"

namespace Rtt
{
	LinuxWebView::LinuxWebView(const Rect& bounds)
		: Super(bounds)
	{
	}

	LinuxWebView::~LinuxWebView()
	{
		Close();
	}

	bool LinuxWebView::Initialize()
	{
		return Super::Initialize();
	}

	const LuaProxyVTable& LinuxWebView::ProxyVTable() const
	{
		return PlatformDisplayObject::GetWebViewObjectProxyVTable();
	}

	int LinuxWebView::ValueForKey(lua_State* L, const char key[]) const
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
			Rtt_PRINT(("WARNING: Web views do not have bounce behavior on this platform.\n"));
			result = 0;
		}
		else if (strcmp("deleteCookies", key) == 0)
		{
			lua_pushcfunction(L, OnDeleteCookies);
		}
		else if (strcmp("canGoBack", key) == 0)
		{
			lua_pushboolean(L, 0); // false
		}
		else if (strcmp("canGoForward", key) == 0)
		{
			lua_pushboolean(L, 0); // false
		}
		else if (strcmp("load", key) == 0)
		{
			lua_pushcfunction(L, Load);
		}
		else
		{
			result = Super::ValueForKey(L, key);
		}

		return result;
	}

	bool LinuxWebView::SetValueForKey(lua_State* L, const char key[], int valueIndex)
	{
		Rtt_ASSERT(key);

		bool result = true;

		if (strcmp("bounces", key) == 0)
		{
			Rtt_ASSERT_NOT_IMPLEMENTED();
			//	Rtt_PRINT( ( "WARNING: Web views do not have bounce behavior on this platform.\n" ) );
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

	int LinuxWebView::Load(lua_State* L)
	{
		return 0;
	}

	int LinuxWebView::Request(lua_State* L)
	{
		// Fetch the web view from the Lua object.
		const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
		LinuxWebView* view = (LinuxWebView*)luaL_todisplayobject(L, 1, table);

		if (view == NULL)
		{
			return 0;
		}

		// Fetch the URL argument.
		if (lua_type(L, 2) != LUA_TSTRING)
		{
			luaL_error(L, "Function WebView.request() was given an invalid URL argument. Was expecting a string.");
		}

		const char* url = lua_tostring(L, 2);

		// If the optional base directory argument was provided, then update the above URL path with it.
		if (lua_type(L, 3) == LUA_TSTRING)
		{
			std::string baseUrl(lua_tostring(L, 3));

			if (baseUrl.size() > 0)
			{
				auto baseUrlLength = baseUrl.size();

				if ((baseUrl[baseUrlLength - 1] != '/') && (baseUrl[baseUrlLength - 1] != '\\'))
				{
					baseUrl += "/";
				}

				baseUrl += url;

				view->openURL(baseUrl.c_str());
				return 0;
			}
		}
		else if (lua_islightuserdata(L, 3))
		{
			// open local file
			MPlatform::Directory baseDirectory = (MPlatform::Directory)EnumForUserdata(LuaLibSystem::Directories(), lua_touserdata(L, 3), MPlatform::kNumDirs, MPlatform::kUnknownDir);

			if (baseDirectory != MPlatform::kUnknownDir)
			{
				LinuxPlatform* platform = app->GetPlatform();
				String result;
				platform->PathForFile(url, baseDirectory, 0, result);

				String localurl("file://");
				localurl.Append(result.GetString());
				view->openURL(localurl.GetString());
				return 0;
			}
		}

		view->openURL(url);
		return 0;
	}

	bool LinuxWebView::Close()
	{
		// Do not continue if there is no web browser to close.
		fWebView = NULL;
		return true;
	}

	void LinuxWebView::onWebPopupLoadedEvent()
	{
	}

	void LinuxWebView::onWebPopupNavigatingEvent()
	{
		//const char *url = e.GetURL().c_str();
		//eventArg *arg = (eventArg*) e.GetEventUserData();

		//// Relay this event to the popup's Lua listenebounds.
		//Rtt::UrlRequestEvent event(url, Rtt::UrlRequestEvent::kOther);
		//arg->fThiz->DispatchEventWithTarget(event);
	}

	void LinuxWebView::onWebPopupNavigatedEvent()
	{
		//const char *url = e.GetURL().c_str();
		//eventArg *arg = (eventArg*) e.GetEventUserData();

		//Rtt::UrlRequestEvent event(url, Rtt::UrlRequestEvent::kLoaded);
		//arg->fThiz->DispatchEventWithTarget(event);
	}

	void LinuxWebView::onWebPopupErrorEvent()
	{
		//const char *url = e.GetURL().c_str();
		//eventArg *arg = (eventArg*) e.GetEventUserData();

		//// Dispatch a Lua "urlRequest" event indicating that we've failed to load a URL or web page.
		//const char *msg = e.GetString().c_str();
		//Rtt::UrlRequestEvent event(url, msg, 100); // hack, not tested
		//arg->fThiz->DispatchEventWithTarget(event);
	}

	void LinuxWebView::openURL(const char* url)
	{
		Rect bounds = StageBounds();
		fWebView = new WebView(bounds, url);
	}

	void LinuxWebView::Prepare(const Display& display)
	{
	}

	int LinuxWebView::Stop(lua_State* L)
	{
		const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
		LinuxWebView* view = (LinuxWebView*)luaL_todisplayobject(L, 1, table);

		if (view)
		{
			//NativeToJavaBridge::GetInstance()->WebViewRequestStop(view->GetId());
		}
		return 0;
	}

	int LinuxWebView::Back(lua_State* L)
	{
		return 0;
	}

	int LinuxWebView::Forward(lua_State* L)
	{
		return 0;
	}

	int LinuxWebView::Resize(lua_State* L)
	{
		return 0;
	}

	int LinuxWebView::Reload(lua_State* L)
	{
		const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
		LinuxWebView* view = (LinuxWebView*)luaL_todisplayobject(L, 1, table);

		if (view)
		{
			//NativeToJavaBridge::GetInstance()->WebViewRequestReload(view->GetId());
		}
		return 0;
	}

	int LinuxWebView::OnDeleteCookies(lua_State* L)
	{
		CoronaLuaWarning(L, "The native WebView:deleteCookies() function is not supported on Linux.");
		return 0;
	}

	void LinuxWebView::Draw(Renderer& renderer) const
	{
		if (fWebView && fWebView->fData.fFillTexture0)
		{
			renderer.Insert(&fWebView->fData);
		}
	}

	bool LinuxWebView::ProcessEvent(const SDL_Event& evt)
	{
		if (fWebView)
		{
			Rect boundsScreen;
			GetScreenBounds(boundsScreen);
			float k = 1 / GetContentToScreenSx();

			switch (evt.type)
			{
			case SDL_MOUSEBUTTONDOWN:
			{
				const SDL_MouseButtonEvent& b = evt.button;
				if (b.which != SDL_TOUCH_MOUSEID)
				{
					int x = b.x;
					int y = b.y;
					y -= app->GetMenuHeight();
					//printf("%d-%d %d-%d, %dx%d\n", (int)boundsScreen.xMin, (int)boundsScreen.Width(), (int)boundsScreen.yMin, (int)boundsScreen.Height(), x, y);

					if (x >= boundsScreen.xMin && y >= boundsScreen.yMin && x <= boundsScreen.xMax && y <= boundsScreen.yMax)
					{
						x -= boundsScreen.xMin;
						y -= boundsScreen.yMin;
						fWebView->MousePress(x * k, y * k);
						return true;
					}
				}
				break;
			}
			case SDL_MOUSEBUTTONUP:
			{
				const SDL_MouseButtonEvent& b = evt.button;
				if (b.which != SDL_TOUCH_MOUSEID)
				{
					int x = b.x;
					int y = b.y;
					y -= app->GetMenuHeight();
					if (x >= boundsScreen.xMin && y >= boundsScreen.yMin && x <= boundsScreen.xMax && y <= boundsScreen.yMax)
					{
						x -= boundsScreen.xMin;
						y -= boundsScreen.yMin;

						fWebView->MouseRelease(x * k, y * k);
						return true;
					}
				}
				break;
			}
			case SDL_MOUSEMOTION:
			{
				const SDL_MouseButtonEvent& b = evt.button;
				if (b.which != SDL_TOUCH_MOUSEID)
				{
					int x = b.x;
					int y = b.y;
					y -= app->GetMenuHeight();
					if (x >= boundsScreen.xMin && y >= boundsScreen.yMin && x <= boundsScreen.xMax && y <= boundsScreen.yMax)
					{
						x -= boundsScreen.xMin;
						y -= boundsScreen.yMin;
						fWebView->MouseMove(x * k, y * k);
						return true;
					}
				}
				break;
			}
			case SDL_MOUSEWHEEL:
			{
				const SDL_MouseWheelEvent& w = evt.wheel;
				if (w.which != SDL_TOUCH_MOUSEID)
				{
					int x, y;
					SDL_GetMouseState(&x, &y);
					y -= app->GetMenuHeight();

					if (x >= boundsScreen.xMin && y >= boundsScreen.yMin && x <= boundsScreen.xMax && y <= boundsScreen.yMax)
					{
						x -= boundsScreen.xMin;
						y -= boundsScreen.yMin;
						fWebView->MouseWheel(w.x, w.y);
						return true;
					}
				}
				break;
			}
			}
		}
		return false;
	}

}; // namespace Rtt
