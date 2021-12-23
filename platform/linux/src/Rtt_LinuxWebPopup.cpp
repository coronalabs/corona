//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_LinuxWebPopup.h"
#include "CoronaLua.h"
//#include "Display\Rtt_Display.h"
//#include "Interop\UI\RenderSurfaceControl.h"
//#include "Interop\RuntimeEnvironment.h"
#include "Rtt_LuaAux.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_MKeyValueIterable.h"
#include "Rtt_PlatformPlayer.h"
#include "Rtt_Runtime.h"
#include "Rtt_LinuxPlatform.h"
#include "Rtt_LinuxContext.h"

#if ( wxUSE_WEBVIEW == 1)

namespace Rtt
{
	LinuxWebPopup::LinuxWebPopup() //Interop::RuntimeEnvironment& environment)
		: fWebBrowserPointer(NULL)
		, fBaseDirectory(MPlatform::kUnknownDir)
		, fWasResizeRequested(false)
	{
	}

	LinuxWebPopup::~LinuxWebPopup()
	{
		// If a popup is currently being displayed, then close it now.
		Close();
	}

	void LinuxWebPopup::onWebPopupLoadedEvent(wxWebViewEvent &e)
	{
	}

	void LinuxWebPopup::onWebPopupNavigatingEvent(wxWebViewEvent &e)
	{
		const char *url = e.GetURL().c_str();
		eventArg *arg = (eventArg*) e.GetEventUserData();

		// Relay this event to the popup's Lua listener.
		bool wasCloseRequested = !arg->fThiz->ShouldLoadUrl(url);

		if (wasCloseRequested)
		{
			arg->fThiz->Close();
		}
	}

	void LinuxWebPopup::onWebPopupNavigatedEvent(wxWebViewEvent &e)
	{
	}

	void LinuxWebPopup::onWebPopupErrorEvent(wxWebViewEvent &e)
	{
		const char *url = e.GetURL().c_str();
		eventArg *arg = (eventArg*) e.GetEventUserData();

		// Relay this event to the popup's Lua listener.
		bool wasCloseRequested = !arg->fThiz->DidFailLoadUrl(url, "", 0);

		if (wasCloseRequested)
		{
			arg->fThiz->Close();
		}
	}

	void LinuxWebPopup::Show(const MPlatform &platform, const char *url)
	{
		// Create a new web browser control for this popup, if not done already.
		if (!fWebBrowserPointer)
		{
			// Fetch the bounds of this display object converted from Corona coordinates to native screen coordinates.
			Runtime *runtimePointer = wxGetApp().GetRuntime();

			if (!runtimePointer)
			{
				return;
			}

			Rtt::Display &display = runtimePointer->GetDisplay();
			Rect screenBounds;
			Preinitialize(display);
			GetScreenBounds(display, screenBounds);

			fWebBrowserPointer = wxWebView::New(wxGetApp().GetParent(), wxID_ANY, "", wxPoint(screenBounds.xMin, screenBounds.yMin), wxSize(screenBounds.Width(), screenBounds.Height()));
			fWebBrowserPointer->Bind(wxEVT_WEBVIEW_NAVIGATING, onWebPopupNavigatingEvent, wxID_ANY, wxID_ANY, new eventArg(this));
			fWebBrowserPointer->Bind(wxEVT_WEBVIEW_NAVIGATED, onWebPopupNavigatedEvent, wxID_ANY, wxID_ANY, new eventArg(this));
			fWebBrowserPointer->Bind(wxEVT_WEBVIEW_LOADED, onWebPopupLoadedEvent, wxID_ANY, wxID_ANY, new eventArg(this));
			fWebBrowserPointer->Bind(wxEVT_WEBVIEW_ERROR, onWebPopupErrorEvent, wxID_ANY, wxID_ANY, new eventArg(this));
		}

		// Navigate to the given URL.
		std::string updatedUrl;

		if (fBaseDirectory != MPlatform::kUnknownDir)
		{
			Rtt::String filePath;
			LinuxPlatform *platform = wxGetApp().GetPlatform();
			platform->PathForFile(url, fBaseDirectory, MPlatform::kDefaultPathFlags, filePath);

			if (!filePath.IsEmpty())
			{
				updatedUrl = std::string("file://") + filePath.GetString();
			}
		}
		else if (fBaseUrl.size() > 0)
		{
			updatedUrl = fBaseUrl;
			auto length = updatedUrl.size();

			if ((updatedUrl[length - 1] != '/') && (updatedUrl[length - 1] != '\\'))
			{
				updatedUrl += '/';
			}

			updatedUrl += url;
		}

		if (updatedUrl.size() == 0)
		{
			updatedUrl = url;
		}

		fWebBrowserPointer->LoadURL(updatedUrl.c_str());
	}

	bool LinuxWebPopup::Close()
	{
		// Do not continue if there is no web browser to close.
		wxApp *app = &wxGetApp();

		if (fWebBrowserPointer && app != NULL)
		{
			// Remove event handlers.
			fWebBrowserPointer->Unbind(wxEVT_WEBVIEW_NAVIGATING, onWebPopupNavigatingEvent);
			fWebBrowserPointer->Unbind(wxEVT_WEBVIEW_NAVIGATED, onWebPopupNavigatedEvent);
			fWebBrowserPointer->Unbind(wxEVT_WEBVIEW_LOADED, onWebPopupLoadedEvent);
			fWebBrowserPointer->Unbind(wxEVT_WEBVIEW_ERROR, onWebPopupErrorEvent);

			// Release the popup's assigned Lua listener.
			SetCallback(nullptr);

			// Hide the control since we are not deleting it immediately down below.
			delete fWebBrowserPointer;
			fWebBrowserPointer = nullptr;
			fWasResizeRequested = false;
			return true;
		}
		return false;
	}

	void LinuxWebPopup::Reset()
	{
	}

	void LinuxWebPopup::SetPostParams(const MKeyValueIterable &params)
	{
	}

	int LinuxWebPopup::ValueForKey(lua_State *L, const char key[]) const
	{
		// Validate.
		if (!L || Rtt_StringIsEmpty(key))
		{
			return 0;
		}

		int result = 1;

		if (Rtt_StringCompare("baseUrl", key) == 0)
		{
			if (fBaseDirectory != MPlatform::kUnknownDir)
			{
				LuaLibSystem::PushDirectory(L, fBaseDirectory);
			}
			else if (fBaseUrl.size() > 0)
			{
				lua_pushstring(L, fBaseUrl.c_str());
			}
			else
			{
				lua_pushnil(L);
			}
		}
		else if (Rtt_StringCompare("hasBackground", key) == 0)
		{
			lua_pushboolean(L, 1);
		}
		else if (strcmp("autoCancel", key) == 0)
		{
			lua_pushboolean(L, 0);
		}
		else
		{
			result = 0;
		}

		return result;
	}

	bool LinuxWebPopup::SetValueForKey(lua_State *L, const char key[], int valueIndex)
	{
		// Validate.
		if (!L || Rtt_StringIsEmpty(key) || !valueIndex)
		{
			return false;
		}

		bool result = true;

		if (Rtt_StringCompare("baseUrl", key) == 0)
		{
			if (lua_isstring(L, valueIndex))
			{
				fBaseUrl = lua_tostring(L, valueIndex);
				fBaseDirectory = MPlatform::kUnknownDir;
			}
			else if (lua_islightuserdata(L, valueIndex))
			{
				fBaseDirectory = (MPlatform::Directory)EnumForUserdata(LuaLibSystem::Directories(), lua_touserdata(L, valueIndex), MPlatform::kNumDirs, MPlatform::kUnknownDir);
				fBaseUrl = "";
			}
		}
		else if (!Rtt_StringCompare("hasBackground", key) || !Rtt_StringCompare("autoCancel", key))
		{
			CoronaLuaWarning(L, "Native WebPopups on Windows do not support the \"%s\" option.", key);
		}
		else
		{
			result = false;
		}
		return result;
	}
}; // namespace Rtt

#endif
