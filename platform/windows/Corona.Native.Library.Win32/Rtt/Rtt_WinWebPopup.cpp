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

#include "stdafx.h"
#include "Rtt_WinWebPopup.h"
#include "Corona\CoronaLua.h"
#include "Display\Rtt_Display.h"
#include "Interop\UI\RenderSurfaceControl.h"
#include "Interop\RuntimeEnvironment.h"
#include "Rtt_LuaAux.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_MKeyValueIterable.h"
#include "Rtt_PlatformPlayer.h"
#include "Rtt_Runtime.h"
#include "Rtt_WinPlatform.h"


namespace Rtt
{

#pragma region Constructors/Destructors
WinWebPopup::WinWebPopup(Interop::RuntimeEnvironment& environment)
:	fEnvironment(environment),
	fWebBrowserPointer(nullptr),
	fBaseDirectory(MPlatform::kUnknownDir),
	fWasResizeRequested(false),
	fReservedMessageId(0),
	fRenderSurfaceResizedEventHandler(this, &WinWebPopup::OnRenderSurfaceResized),
	fRuntimeEnteringFrameEventHandler(this, &WinWebPopup::OnRuntimeEnteringFrame),
	fNavigatingEventHandler(this, &WinWebPopup::OnNavigating),
	fNavigationFailedEventHandler(this, &WinWebPopup::OnNavigationFailed),
	fReceivedMessageEventHandler(this, &WinWebPopup::OnReceivedMessage)
{
	// Reserve a unique message ID from the runtime's message-only window and subscribe to its events.
	auto& messageOnlyWindow = fEnvironment.GetMessageOnlyWindow();
	fReservedMessageId = messageOnlyWindow.ReserveMessageId();
	messageOnlyWindow.GetReceivedMessageEventHandlers().Add(&fReceivedMessageEventHandler);
}

WinWebPopup::~WinWebPopup()
{
	// Release the reserved Win32 message ID.
	auto& messageOnlyWindow = fEnvironment.GetMessageOnlyWindow();
	messageOnlyWindow.UnreserveMessageId(fReservedMessageId);
	fReservedMessageId = 0;
	messageOnlyWindow.GetReceivedMessageEventHandlers().Remove(&fReceivedMessageEventHandler);

	// If a popup is currently being displayed, then close it now.
	Close();

	// Delete all web controls that have been queued for destruction.
	// Note: This must be called after the above Close() function call.
	DeleteClosedWebControls();
}

#pragma endregion


#pragma region Public Methods
void WinWebPopup::Show(const MPlatform& platform, const char *url)
{
	// Create a new web browser control for this popup, if not done already.
	if (!fWebBrowserPointer)
	{
		// Fetch the bounds of this display object converted from Corona coordinates to native screen coordinates.
		auto runtimePointer = fEnvironment.GetRuntime();
		if (!runtimePointer)
		{
			return;
		}
		Rtt::Display& display = runtimePointer->GetDisplay();
		Rect screenBounds;
		Preinitialize(display);
		GetScreenBounds(display, screenBounds);

		// Set up a registry path used to store custom Internet Explorer settings exclusive to this app.
		// This allows our WebBrowser control to override default IE settings.
		std::wstring registryPath = fEnvironment.GetRegistryPathWithoutHive();
		registryPath += L"\\corona\\Internet Explorer";

		// Create and configure the native Win32 web browser control this object will manage.
		Interop::UI::WebBrowser::CreationSettings settings{};
		settings.ParentWindowHandle = fEnvironment.GetRenderSurface()->GetWindowHandle();
		settings.Bounds.left = (LONG)Rtt_RealToInt(screenBounds.xMin);
		settings.Bounds.top = (LONG)Rtt_RealToInt(screenBounds.yMin);
		settings.Bounds.right = (LONG)Rtt_RealToInt(screenBounds.xMax);
		settings.Bounds.bottom = (LONG)Rtt_RealToInt(screenBounds.yMax);
		settings.IEOverrideRegistryPath = registryPath.c_str();
		fWebBrowserPointer = new Interop::UI::WebBrowser(settings);
		if (!fWebBrowserPointer)
		{
			return;
		}

		// Add event handlers.
		auto renderSurfacePointer = fEnvironment.GetRenderSurface();
		if (renderSurfacePointer)
		{
			renderSurfacePointer->GetResizedEventHandlers().Add(&fRenderSurfaceResizedEventHandler);
		}
		fEnvironment.GetEnteringFrameEventHandlers().Add(&fRuntimeEnteringFrameEventHandler);
		fWebBrowserPointer->GetNavigatingEventHandlers().Add(&fNavigatingEventHandler);
		fWebBrowserPointer->GetNavigationFailedEventHandlers().Add(&fNavigationFailedEventHandler);
	}

	// Navigate to the given URL.
	WinString updatedUrl;
	if (fBaseDirectory != MPlatform::kUnknownDir)
	{
		Rtt::String filePath(&fEnvironment.GetAllocator());
		fEnvironment.GetPlatform()->PathForFile(url, fBaseDirectory, MPlatform::kDefaultPathFlags, filePath);
		if (!filePath.IsEmpty())
		{
			updatedUrl.SetUTF8(filePath.GetString());
		}
	}
	else if (!fBaseUrl.IsEmpty())
	{
		updatedUrl = fBaseUrl;
		auto length = updatedUrl.GetLength();
		if ((updatedUrl.GetUTF16()[length - 1] != '/') && (updatedUrl.GetUTF16()[length - 1] != '\\'))
		{
			updatedUrl.Append(L'/');
		}
		updatedUrl.Append(url);
	}
	if (updatedUrl.IsEmpty())
	{
		updatedUrl.SetUTF8(url);
	}
	fWebBrowserPointer->NavigateTo(updatedUrl.GetUTF16());
}

bool WinWebPopup::Close()
{
	// Do not continue if there is no web browser to close.
	if (!fWebBrowserPointer)
	{
		return false;
	}

	// Remove event handlers.
	auto renderSurfacePointer = fEnvironment.GetRenderSurface();
	if (renderSurfacePointer)
	{
		renderSurfacePointer->GetResizedEventHandlers().Remove(&fRenderSurfaceResizedEventHandler);
	}
	fEnvironment.GetEnteringFrameEventHandlers().Remove(&fRuntimeEnteringFrameEventHandler);
	fWebBrowserPointer->GetNavigatingEventHandlers().Remove(&fNavigatingEventHandler);
	fWebBrowserPointer->GetNavigationFailedEventHandlers().Remove(&fNavigationFailedEventHandler);

	// Release the popup's assigned Lua listener.
	SetCallback(nullptr);

	// Hide the control since we are not deleting it immediately down below.
	fWebBrowserPointer->SetVisible(false);

	// Add the web browser control to the closed list to be deleted later.
	// Note: We don't want to delete the web browser control immediately in case Close() was called
	//       from one of its event handlers, such as the Lua "urlRequest". Would cause a crash.
	fClosedWebControlCollection.push_back(fWebBrowserPointer);
	fWebBrowserPointer = nullptr;
	fWasResizeRequested = false;

	// Post a message to delete all closed web browser controls later.
	if (fReservedMessageId)
	{
		::PostMessageW(fEnvironment.GetMessageOnlyWindow().GetWindowHandle(), fReservedMessageId, 0, 0);
	}
	return true;
}

void WinWebPopup::Reset()
{
}

void WinWebPopup::SetPostParams(const MKeyValueIterable& params)
{
}

int WinWebPopup::ValueForKey(lua_State *L, const char key[]) const
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
		else if (!fBaseUrl.IsEmpty())
		{
			lua_pushstring(L, fBaseUrl.GetUTF8());
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

bool WinWebPopup::SetValueForKey(lua_State *L, const char key[], int valueIndex)
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
			fBaseUrl.SetUTF8(lua_tostring(L, valueIndex));
			fBaseDirectory = MPlatform::kUnknownDir;
		}
		else if (lua_islightuserdata(L, valueIndex))
		{
			fBaseDirectory = (MPlatform::Directory)EnumForUserdata(
					LuaLibSystem::Directories(), lua_touserdata(L, valueIndex),
					MPlatform::kNumDirs, MPlatform::kUnknownDir);
			fBaseUrl.Clear();
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

#pragma endregion


#pragma region Private Methods
void WinWebPopup::OnRenderSurfaceResized(Interop::UI::Control& sender, const Interop::EventArgs& arguments)
{
	// Flag that the web popup needs to be resized by the next "enterFrame" event.
	// By then, the Corona runtime's content scale/size will be correctly updated for the new window size.
	fWasResizeRequested = true;
}

void WinWebPopup::OnRuntimeEnteringFrame(Interop::RuntimeEnvironment& sender, const Interop::EventArgs& arguments)
{
	// Do not continue unless we need to resize the web browser control.
	if (!fWasResizeRequested)
	{
		return;
	}
	fWasResizeRequested = false;

	// First, update the popup wrapper's cached scale factors.
	auto runtimePointer = fEnvironment.GetRuntime();
	if (!runtimePointer)
	{
		return;
	}
	Preinitialize(runtimePointer->GetDisplay());

	// Update the web browser control's screen bounds.
	if (fWebBrowserPointer)
	{
		Rect coronaBounds{};
		RECT nativeBounds{};
		GetScreenBounds(runtimePointer->GetDisplay(), coronaBounds);
		nativeBounds.left = (LONG)Rtt_RealToInt(coronaBounds.xMin);
		nativeBounds.top = (LONG)Rtt_RealToInt(coronaBounds.yMin);
		nativeBounds.right = (LONG)Rtt_RealToInt(coronaBounds.xMax);
		nativeBounds.bottom = (LONG)Rtt_RealToInt(coronaBounds.yMax);
		fWebBrowserPointer->SetBounds(nativeBounds);
	}
}

void WinWebPopup::OnNavigating(Interop::UI::WebBrowser& sender, Interop::UI::WebBrowserNavigatingEventArgs& arguments)
{
	// Relay this event to the popup's Lua listener.
	bool wasCloseRequested = !ShouldLoadUrl(arguments.GetUrlAsUtf8());
	if (wasCloseRequested)
	{
		Close();
		arguments.SetCanceled();
	}
}

void WinWebPopup::OnNavigationFailed(
	Interop::UI::WebBrowser& sender, const Interop::UI::WebBrowserNavigationFailedEventArgs& arguments)
{
	// Relay this event to the popup's Lua listener.
	bool wasCloseRequested = !DidFailLoadUrl(
			arguments.GetUrlAsUtf8(), arguments.GetErrorMessageAsUtf8(), arguments.GetErrorCode());
	if (wasCloseRequested)
	{
		Close();
	}
}

void WinWebPopup::OnReceivedMessage(Interop::UI::UIComponent& sender, Interop::UI::HandleMessageEventArgs& arguments)
{
	// Do not continue if the received message was not directed to this web popup.
	if (arguments.GetMessageId() != fReservedMessageId)
	{
		return;
	}

	// Delete all closed web browser controls.
	arguments.SetHandled();
	DeleteClosedWebControls();
}

void WinWebPopup::DeleteClosedWebControls()
{
	for (auto&& nextPointer : fClosedWebControlCollection)
	{
		delete nextPointer;
	}
	fClosedWebControlCollection.clear();
}

#pragma endregion

} // namespace Rtt
