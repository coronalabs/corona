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
#include "SimulatorRuntimeEnvironment.h"
#include "Core\Rtt_Build.h"
#include "Core\Rtt_Allocator.h"
#include "Display\Rtt_Display.h"
#include "Interop\ApplicationServices.h"
#include "Resource.h"
#include "Rtt_AuthorizationTicket.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibSimulator.h"
#include "Rtt_PlatformPlayer.h"
#include "Rtt_Runtime.h"
#include "Rtt_WinPlatform.h"
#include "Rtt_WinSimulatorServices.h"
#include "Simulator.h"
#include "SimulatorView.h"
#include "WinGlobalProperties.h"
#include <exception>


namespace Interop {

#pragma region Constructors/Destructors
SimulatorRuntimeEnvironment::SimulatorRuntimeEnvironment(const SimulatorRuntimeEnvironment::CreationSettings& settings)
:	RuntimeEnvironment(settings),
	fLoadedEventHandler(this, &SimulatorRuntimeEnvironment::OnRuntimeLoaded),
	fTerminatingEventHandler(this, &SimulatorRuntimeEnvironment::OnRuntimeTerminating),
	fLuaMouseEventCallback(this, &SimulatorRuntimeEnvironment::OnLuaMouseEventReceived),
	fDeviceSimulatorServicesPointer(nullptr),
	fCoronaSimulatorServicesPointer(settings.CoronaSimulatorServicesPointer)
{
	// Create a device services object if given a device configuration to simulate.
	if (settings.DeviceConfigPointer)
	{
		fDeviceSimulatorServicesPointer = new DeviceSimulatorServices(this, settings.DeviceConfigPointer);
	}

	// Add event handlers.
	this->GetLoadedEventHandlers().Add(&fLoadedEventHandler);
	this->GetTerminatingEventHandlers().Add(&fTerminatingEventHandler);
}

SimulatorRuntimeEnvironment::~SimulatorRuntimeEnvironment()
{
	// Remove event handlers.
	this->GetLoadedEventHandlers().Remove(&fLoadedEventHandler);
	this->GetTerminatingEventHandlers().Remove(&fTerminatingEventHandler);

	// Delete our device simulator interface.
	if (fDeviceSimulatorServicesPointer)
	{
		delete fDeviceSimulatorServicesPointer;
		fDeviceSimulatorServicesPointer = nullptr;
	}
}

#pragma endregion


#pragma region Public Methods
MDeviceSimulatorServices* SimulatorRuntimeEnvironment::GetDeviceSimulatorServices() const
{
	return fDeviceSimulatorServicesPointer;
}

void SimulatorRuntimeEnvironment::AddMouseCursorRegion(
	Rtt::WinInputDeviceManager::CursorStyle style, const RECT& region)
{
	MouseCursorRegion cursorRegion{};
	cursorRegion.CursorStyle = style;
	cursorRegion.CoronaContentBounds = region;
	fCursorRegionCollection.push_back(cursorRegion);
}

void SimulatorRuntimeEnvironment::RemoveMouseCursorRegion(const RECT& region)
{
	// Do not continue if there are no regions to remove. (This is an optimization.)
	if (fCursorRegionCollection.empty())
	{
		return;
	}

	// Remove all mouse rollover regions from the collection that exactly match the given region.
	for (int index = (int)fCursorRegionCollection.size() - 1; index >= 0; index--)
	{
		auto nextRegion = fCursorRegionCollection.at(index);
		if (::EqualRect(&region, &nextRegion.CoronaContentBounds))
		{
			fCursorRegionCollection.erase(fCursorRegionCollection.begin() + index);
		}
	}

	// If there are no more regions, then set up the stystem to default back to an arrow cursor.
	if (fCursorRegionCollection.empty())
	{
		auto& inputDeviceManager = (Rtt::WinInputDeviceManager&)GetPlatform()->GetDevice().GetInputDeviceManager();
		inputDeviceManager.SetCursor(Rtt::WinInputDeviceManager::CursorStyle::kDefaultArrow);
	}
}

#pragma endregion


#pragma region Public Static Functions
SimulatorRuntimeEnvironment::CreationResult SimulatorRuntimeEnvironment::CreateUsing(
	const SimulatorRuntimeEnvironment::CreationSettings& settings)
{
	// Do not continue if given an invalid device configuration to simulate, if provided.
	if (settings.DeviceConfigPointer)
	{
		if (false == settings.DeviceConfigPointer->configLoaded)
		{
			return CreationResult::FailedWith(L"Failed to load device configuration for the simulator.");
		}
		if ((settings.DeviceConfigPointer->screenWidth <= 0) ||
		    (settings.DeviceConfigPointer->screenHeight <= 0))
		{
			return CreationResult::FailedWith(L"Device configuration to simulate has an invalid screen width and height.");
		}
	}

	/// <summary>Returns the given string if not null. Returns an empty string if given null.</summary>
	/// <param name="text">The string to be returned if not null.</param>
#	define ReturnEmptyWStringIfNull(text) text ? text : L""

	// Fetch the Lua "system.SystemResourceDirectory" path equivalent.
	std::wstring systemResourceDirectoryPath(ReturnEmptyWStringIfNull(settings.SystemResourceDirectoryPath));
	if (systemResourceDirectoryPath.empty())
	{
		auto appProperties = GetWinProperties();
		if (appProperties && appProperties->GetResourcesDir())
		{
			WinString stringConverter;
			stringConverter.SetUTF8(appProperties->GetResourcesDir());
			systemResourceDirectoryPath = stringConverter.GetUTF16();
		}
		if (systemResourceDirectoryPath.empty())
		{
			systemResourceDirectoryPath = ApplicationServices::GetDirectoryPath();
			systemResourceDirectoryPath += L"\\Resources";
		}
	}

	// Fetch the Lua "system.ResourceDirectory" path equivalent.
	std::wstring resourceDirectoryPath(ReturnEmptyWStringIfNull(settings.ResourceDirectoryPath));
	auto index = resourceDirectoryPath.find_last_not_of(L"\\/");
	if (index == std::wstring::npos)
	{
		resourceDirectoryPath.clear();
	}
	else if ((index + 1) < resourceDirectoryPath.length())
	{
		resourceDirectoryPath.erase(index + 1);
	}
	if (resourceDirectoryPath.empty())
	{
		// If a resource directory was not provided, then use the application's directory by default.
		resourceDirectoryPath = ApplicationServices::GetDirectoryPath();
	}

	// Fetch a path to Corona's plugins directory.
	std::wstring pluginsDirectoryPath(ReturnEmptyWStringIfNull(settings.PluginsDirectoryPath));
	if (pluginsDirectoryPath.empty())
	{
		RuntimeEnvironment::CopySimulatorPluginDirectoryPathTo(pluginsDirectoryPath);
	}

	// Fetch a path to the Corona Simulator's installed skins directory.
	std::wstring skinResourceDirectoryPath(ReturnEmptyWStringIfNull(settings.SkinResourceDirectoryPath));
	if (skinResourceDirectoryPath.empty())
	{
		skinResourceDirectoryPath = systemResourceDirectoryPath;
		skinResourceDirectoryPath.append(L"\\Skins");
	}

	// Fetch a path to Corona's user created skins directory.
	std::wstring userSkinsDirectoryPath(ReturnEmptyWStringIfNull(settings.UserSkinsDirectoryPath));
	if (userSkinsDirectoryPath.empty())
	{
		RuntimeEnvironment::CopySimulatorRootAppDataDirectoryPathTo(userSkinsDirectoryPath);
		userSkinsDirectoryPath.append(L"\\Skins");
	}

	// Create a sandbox directory path for the given Corona project folder.
	std::wstring sandboxDirectoryPath;
	RuntimeEnvironment::GenerateSimulatorSandboxPath(resourceDirectoryPath.c_str(), sandboxDirectoryPath);

	// Fetch the Lua "system.DocumentsDirectory" path equivalent.
	std::wstring documentsDirectoryPath(ReturnEmptyWStringIfNull(settings.DocumentsDirectoryPath));
	if (documentsDirectoryPath.empty())
	{
		documentsDirectoryPath = sandboxDirectoryPath;
		documentsDirectoryPath.append(L"\\Documents");
	}

	// Fetch the Lua "system.ApplicationSupportDirectory" path equivalent.
	std::wstring applicationSupportDirectoryPath(ReturnEmptyWStringIfNull(settings.ApplicationSupportDirectoryPath));
	if (applicationSupportDirectoryPath.empty())
	{
		applicationSupportDirectoryPath = sandboxDirectoryPath;
		applicationSupportDirectoryPath.append(L"\\ApplicationSupport");
	}

	// Fetch the Lua "system.TemporaryDirectory" path equivalent.
	std::wstring temporaryDirectoryPath(ReturnEmptyWStringIfNull(settings.TemporaryDirectoryPath));
	if (temporaryDirectoryPath.empty())
	{
		temporaryDirectoryPath = sandboxDirectoryPath;
		temporaryDirectoryPath.append(L"\\TemporaryFiles");
	}

	// Fetch the Lua "system.CachesDirectory" path equivalent.
	std::wstring cachesDirectoryPath(ReturnEmptyWStringIfNull(settings.CachesDirectoryPath));
	if (cachesDirectoryPath.empty())
	{
		cachesDirectoryPath = sandboxDirectoryPath;
		cachesDirectoryPath.append(L"\\CachedFiles");
	}

	// Fetch the Lua "system.SystemCachesDirectory" path equivalent.
	std::wstring systemCachesDirectoryPath(ReturnEmptyWStringIfNull(settings.SystemCachesDirectoryPath));
	if (systemCachesDirectoryPath.empty())
	{
		systemCachesDirectoryPath = sandboxDirectoryPath;
		systemCachesDirectoryPath.append(L"\\.system");
	}

	// Create a copy of the given settings and update it with the simulator's default settings above.
	CreationSettings updatedSettings(settings);
	updatedSettings.ResourceDirectoryPath = resourceDirectoryPath.c_str();
	updatedSettings.DocumentsDirectoryPath = documentsDirectoryPath.c_str();
	updatedSettings.ApplicationSupportDirectoryPath = applicationSupportDirectoryPath.c_str();
	updatedSettings.TemporaryDirectoryPath = temporaryDirectoryPath.c_str();
	updatedSettings.CachesDirectoryPath = cachesDirectoryPath.c_str();
	updatedSettings.SystemCachesDirectoryPath = systemCachesDirectoryPath.c_str();
	updatedSettings.SystemResourceDirectoryPath = systemResourceDirectoryPath.c_str();
	updatedSettings.PluginsDirectoryPath = pluginsDirectoryPath.c_str();
	updatedSettings.SkinResourceDirectoryPath = skinResourceDirectoryPath.c_str();
	updatedSettings.UserSkinsDirectoryPath = userSkinsDirectoryPath.c_str();

	// Attempt to create the Corona runtime environment.
	SimulatorRuntimeEnvironment* environmentPointer = nullptr;
	try
	{
		environmentPointer = new SimulatorRuntimeEnvironment(updatedSettings);
	}
	catch (const std::exception& ex)
	{
		return CreationResult::FailedWith(ex.what());
	}
	catch (...) { }
	if (!environmentPointer)
	{
		// The constructor threw an exception.
		return CreationResult::FailedWith(L"Failed to create the Corona runtime environment.");
	}

	// Load and run a Corona project, if enabled.
	if (settings.IsRuntimeCreationEnabled)
	{
		auto result = environmentPointer->RunUsing(settings);
		if (result.HasFailed())
		{
			return CreationResult::FailedWith(result.GetMessage());
		}
	}

	// Return the newly created Corona runtime environment.
	return CreationResult::SucceededWith(environmentPointer);
}

void SimulatorRuntimeEnvironment::Destroy(SimulatorRuntimeEnvironment* environmentPointer)
{
	if (environmentPointer)
	{
		delete environmentPointer;
	}
}

#pragma endregion


#pragma region Private Methods
void SimulatorRuntimeEnvironment::OnRuntimeLoaded(RuntimeEnvironment& sender, const EventArgs& arguments)
{
	// Fetch all plugin in the "build.settings" file to be downloaded by the simulator's "shell.lua" script.
	sender.GetRuntime()->FindDownloadablePlugins("win32-sim");

	// Validate the Corona project's "build.settings" and "config.lua" files.
	Rtt::PlatformSimulator::ValidateSettings(sender.GetRuntime()->Platform());

	// Register the Corona Simulator's APIs into Lua, if provided.
	// Note: This should only be the case when this runtime environment is running the welcome screen Corona project.
	if (fCoronaSimulatorServicesPointer)
	{
		lua_State *luaStatePointer = sender.GetRuntime()->VMContext().L();
		lua_pushlightuserdata(luaStatePointer, fCoronaSimulatorServicesPointer);
		Rtt::LuaContext::RegisterModuleLoader(
				luaStatePointer, Rtt::LuaLibSimulator::kName, Rtt::LuaLibSimulator::Open, 1);
	}

	// Add Lua event listeners.
	fLuaMouseEventCallback.RegisterTo(sender.GetRuntime()->VMContext().L());
	fLuaMouseEventCallback.AddToRuntimeEventListeners("mouse");
}

void SimulatorRuntimeEnvironment::OnRuntimeTerminating(RuntimeEnvironment& sender, const EventArgs& arguments)
{
	// Remove Lua event listeners.
	fLuaMouseEventCallback.RemoveFromRuntimeEventListeners("mouse");
	fLuaMouseEventCallback.Unregister();
}

int SimulatorRuntimeEnvironment::OnLuaMouseEventReceived(lua_State* luaStatePointer)
{
	// Validate.
	if (!luaStatePointer)
	{
		return 0;
	}

	// Do not continue if there are no mouse cursor rollover region's configured.
	if (fCursorRegionCollection.empty())
	{
		return 0;
	}

	// Fetch the mouse coordinates from the Lua event.
	POINT point;
	point.x = LONG_MIN;
	point.y = LONG_MIN;
	if (lua_type(luaStatePointer, 1) == LUA_TTABLE)
	{
		lua_getfield(luaStatePointer, 1, "x");
		if (lua_type(luaStatePointer, -1) == LUA_TNUMBER)
		{
			point.x = (LONG)lua_tointeger(luaStatePointer, -1);
		}
		lua_pop(luaStatePointer, 1);
		lua_getfield(luaStatePointer, 1, "y");
		if (lua_type(luaStatePointer, -1) == LUA_TNUMBER)
		{
			point.y = (LONG)lua_tointeger(luaStatePointer, -1);
		}
		lua_pop(luaStatePointer, 1);
	}
	if ((LONG_MIN == point.x) || (LONG_MIN == point.y))
	{
		return 0;
	}

	// Determine if the mouse is hovering over a configured mouse cursor rollover region.
	// Note: We search the collection from back-to-front via an STL reverse iterator because the
	//       last region added to the collection is considered to be overlaid on top of the other regions.
	auto cursorStyle = Rtt::WinInputDeviceManager::CursorStyle::kDefaultArrow;
	for (auto iter = fCursorRegionCollection.rbegin(); iter != fCursorRegionCollection.rend(); iter++)
	{
		if (::PtInRect(&iter->CoronaContentBounds, point))
		{
			cursorStyle = iter->CursorStyle;
			break;
		}
	}
	auto& inputDeviceManager = (Rtt::WinInputDeviceManager&)GetPlatform()->GetDevice().GetInputDeviceManager();
	inputDeviceManager.SetCursor(cursorStyle);
	return 0;
}

#pragma endregion


#pragma region DeviceSimulatorServices Class
SimulatorRuntimeEnvironment::DeviceSimulatorServices::DeviceSimulatorServices(
	SimulatorRuntimeEnvironment* environmentPointer,
	const Rtt::PlatformSimulator::Config* deviceConfigPointer)
:	fEnvironmentPointer(environmentPointer),
	fDeviceConfigPointer(deviceConfigPointer),
	fCurrentOrientation(Rtt::DeviceOrientation::kUpright),
	fCurrentStatusBar(Rtt::MPlatform::kTranslucentStatusBar)
{
	if (!fEnvironmentPointer || !fDeviceConfigPointer)
	{
		throw std::invalid_argument(nullptr);
	}
}

const Rtt::PlatformSimulator::Config* SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetDeviceConfig() const
{
	return fDeviceConfigPointer;
}

const char* SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetManufacturerName() const
{
	return fDeviceConfigPointer->displayManufacturer.GetString();
}

const char* SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetModelName() const
{
	return fDeviceConfigPointer->displayName.GetString();
}

double SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetZoomScale() const
{
	// Fetch the surface's current client bounds.
	auto surfacePointer = fEnvironmentPointer->GetRenderSurface();
	if (!surfacePointer)
	{
		return 1.0;
	}
	auto clientBounds = surfacePointer->GetClientBounds();
	auto clientWidth = clientBounds.right - clientBounds.left;
	auto clientHeight = clientBounds.bottom - clientBounds.top;
	if ((clientWidth <= 0) || (clientHeight <= 0))
	{
		return 1.0;
	}

	// Determine the zoom level scale by comparing the surface size with the simulated device screen size.
	// Note: This assumes that the surface width and height is zoomed proportionally.
	double scale = 1.0;
	switch (fCurrentOrientation)
	{
		case Rtt::DeviceOrientation::kSidewaysRight:
		case Rtt::DeviceOrientation::kSidewaysLeft:
			scale = (double)clientHeight / (double)fDeviceConfigPointer->screenWidth;
			break;
		case Rtt::DeviceOrientation::kUpright:
		case Rtt::DeviceOrientation::kUpsideDown:
			scale = (double)clientWidth / (double)fDeviceConfigPointer->screenWidth;
			break;
	}

	// Return the zoom level scale.
	return scale;
}

bool SimulatorRuntimeEnvironment::DeviceSimulatorServices::IsLuaExitAllowed() const
{
	auto applicationPointer = ((CSimulatorApp*)::AfxGetApp());
	if (applicationPointer)
	{
		return applicationPointer->IsLuaExitAllowed();
	}
	return false;
}

bool SimulatorRuntimeEnvironment::DeviceSimulatorServices::IsScreenRotationSupported() const
{
	return fDeviceConfigPointer->supportsScreenRotation;
}

bool SimulatorRuntimeEnvironment::DeviceSimulatorServices::IsMouseSupported() const
{
	return fDeviceConfigPointer->supportsMouse;
}

bool SimulatorRuntimeEnvironment::DeviceSimulatorServices::AreInputDevicesSupported() const
{
	return fDeviceConfigPointer->supportsInputDevices;
}

bool SimulatorRuntimeEnvironment::DeviceSimulatorServices::AreKeyEventsSupported() const
{
	return fDeviceConfigPointer->supportsKeyEvents;
}

bool SimulatorRuntimeEnvironment::DeviceSimulatorServices::AreKeyEventsFromKeyboardSupported() const
{
	return fDeviceConfigPointer->supportsKeyEventsFromKeyboard;
}

bool SimulatorRuntimeEnvironment::DeviceSimulatorServices::IsBackKeySupported() const
{
	return fDeviceConfigPointer->supportsBackKey;
}

bool SimulatorRuntimeEnvironment::DeviceSimulatorServices::AreExitRequestsSupported() const
{
	return fDeviceConfigPointer->supportsExitRequests;
}

bool SimulatorRuntimeEnvironment::DeviceSimulatorServices::AreMultipleAlertsSupported() const
{
	return fDeviceConfigPointer->supportsMultipleAlerts;
}

bool SimulatorRuntimeEnvironment::DeviceSimulatorServices::IsAlertButtonOrderRightToLeft() const
{
	return fDeviceConfigPointer->isAlertButtonOrderRightToLeft;
}

Rtt::DeviceOrientation::Type SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetOrientation() const
{
	return fCurrentOrientation;
}

void SimulatorRuntimeEnvironment::DeviceSimulatorServices::SetOrientation(Rtt::DeviceOrientation::Type value)
{
	fCurrentOrientation = value;
}

POINT SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetSimulatedPointFromClient(const POINT& value)
{
	// Fetch the Corona runtime, if available.
	auto runtimePointer = fEnvironmentPointer->GetRuntime();
	if (!runtimePointer)
	{
		return value;
	}

	// Fetch the render surface's control/window.
	auto surfacePointer = fEnvironmentPointer->GetRenderSurface();
	if (!surfacePointer)
	{
		return value;
	}

	// Convert the given Windows client coordinate to the simulator's current orientation.
	POINT coronaContentCoordinate = value;
	const Rtt::Display& display = runtimePointer->GetDisplay();
	switch (display.GetRelativeOrientation())
	{
		case Rtt::DeviceOrientation::kSidewaysRight:
			coronaContentCoordinate.x = value.y;
			coronaContentCoordinate.y = surfacePointer->GetClientWidth() - value.x;
			break;
		case Rtt::DeviceOrientation::kSidewaysLeft:
			coronaContentCoordinate.x = surfacePointer->GetClientHeight() - value.y;
			coronaContentCoordinate.y = value.x;
			break;
		case Rtt::DeviceOrientation::kUpsideDown:
			coronaContentCoordinate.x = surfacePointer->GetClientWidth() - value.x;
			coronaContentCoordinate.y = surfacePointer->GetClientHeight() - value.y;
			break;
	}

	// Apply the simulator's current zoom level scale.
	auto zoomScale = GetZoomScale();
	if (zoomScale > 0)
	{
		coronaContentCoordinate.x = (LONG)((double)coronaContentCoordinate.x / zoomScale);
		coronaContentCoordinate.y = (LONG)((double)coronaContentCoordinate.y / zoomScale);
	}

	// Return the simulated coordinate.
	return coronaContentCoordinate;
}

double SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetDefaultFontSize() const
{
	return fDeviceConfigPointer->defaultFontSize;
}

int SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetScreenWidthInPixels() const
{
	return (int)(fDeviceConfigPointer->screenWidth + 0.5f);
}

int SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetScreenHeightInPixels() const
{
	return (int)(fDeviceConfigPointer->screenHeight + 0.5f);
}

int SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetAdaptiveScreenWidthInPixels() const
{
	int length = (int)fDeviceConfigPointer->GetAdaptiveWidth();
	if (length <= 0)
	{
		length = GetScreenWidthInPixels();
	}
	return length;
}

int SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetAdaptiveScreenHeightInPixels() const
{
	int length = (int)fDeviceConfigPointer->GetAdaptiveHeight();
	if (length <= 0)
	{
		length = GetScreenHeightInPixels();
	}
	return length;
}

const char* SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetStatusBarImageFilePathFor(
	Rtt::MPlatform::StatusBarMode value) const
{
	switch (value)
	{
		case Rtt::MPlatform::kDefaultStatusBar:
			return fDeviceConfigPointer->statusBarDefaultFile.GetString();
		case Rtt::MPlatform::kTranslucentStatusBar:
			return fDeviceConfigPointer->statusBarTranslucentFile.GetString();
		case Rtt::MPlatform::kDarkStatusBar:
			return fDeviceConfigPointer->statusBarBlackFile.GetString();
		case Rtt::MPlatform::kLightTransparentStatusBar:
			return fDeviceConfigPointer->statusBarLightTransparentFile.GetString();
		case Rtt::MPlatform::kDarkTransparentStatusBar:
			return fDeviceConfigPointer->statusBarDarkTransparentFile.GetString();
		case Rtt::MPlatform::kScreenDressingFile:
			return fDeviceConfigPointer->screenDressingFile.GetString();
	}
	return nullptr;
}

const char* SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetAuthorizationTicketString() const
{
	const char* stringPointer =
			Rtt::AuthorizationTicket::StringForSubscription(Rtt::AuthorizationTicket::kUnknownSubscription);
	auto appProperties = GetWinProperties();
	if (appProperties)
	{
		auto ticket = GetWinProperties()->GetTicket();
		if (ticket)
		{
			stringPointer = Rtt::AuthorizationTicket::StringForSubscription(ticket->GetSubscription());
		}
	}
	return stringPointer;
}

void SimulatorRuntimeEnvironment::DeviceSimulatorServices::RotateClockwise()
{
	auto orientation = (Rtt::DeviceOrientation::Type)(fCurrentOrientation - 1);
	if (Rtt::DeviceOrientation::kUnknown == orientation)
	{
		orientation = Rtt::DeviceOrientation::kSidewaysLeft;
	}
	else if (Rtt::DeviceOrientation::kFaceUp == orientation)
	{
		orientation = Rtt::DeviceOrientation::kUpright;
	}
	fCurrentOrientation = orientation;
}

void SimulatorRuntimeEnvironment::DeviceSimulatorServices::RotateCounterClockwise()
{
	auto orientation = (Rtt::DeviceOrientation::Type)(fCurrentOrientation + 1);
	if (Rtt::DeviceOrientation::kUnknown == orientation)
	{
		orientation = Rtt::DeviceOrientation::kSidewaysLeft;
	}
	else if (Rtt::DeviceOrientation::kFaceUp == orientation)
	{
		orientation = Rtt::DeviceOrientation::kUpright;
	}
	fCurrentOrientation = orientation;
}

void* SimulatorRuntimeEnvironment::DeviceSimulatorServices::ShowNativeAlert(
	const char *title, const char *message, const char **buttonLabels, int buttonCount, Rtt::LuaResource *resource)
{
	// Fetch the Corona Simulator view.
	CSimulatorView *viewPointer = nullptr;
	CFrameWnd *mainWindowPointer = dynamic_cast<CFrameWnd*>(::AfxGetMainWnd());
	if (mainWindowPointer)
	{
		viewPointer = dynamic_cast<CSimulatorView*>(mainWindowPointer->GetActiveView());
	}
	if (nullptr == viewPointer)
	{
		return nullptr;
	}

	// Send a user-defined message to the CSimulatorView, which will show the native alert message box.
	WMU_ALERT_PARAMS WAP{};
	WAP.sTitle = title;
	WAP.sMsg = message;
	WAP.nButtonLabels = buttonCount;
	WAP.psButtonLabels = buttonLabels;
	WAP.pLuaResource = resource;
	WAP.hwnd = nullptr;
	::SendMessage(viewPointer->GetSafeHwnd(), WMU_NATIVEALERT, 1, (LPARAM)&WAP);

	// Return the message box's window handle.
	return WAP.hwnd;
}

void SimulatorRuntimeEnvironment::DeviceSimulatorServices::CancelNativeAlert(void* alertReference)
{
	if (alertReference)
	{
		::SendMessage((HWND)alertReference, WM_COMMAND, IDCANCEL, 0);
	}
}

void SimulatorRuntimeEnvironment::DeviceSimulatorServices::SetActivityIndicatorVisible(bool value)
{
	// Fetch the Corona Simulator view.
	CSimulatorView *viewPointer = nullptr;
	CFrameWnd *mainWindowPointer = dynamic_cast<CFrameWnd*>(::AfxGetMainWnd());
	if (mainWindowPointer)
	{
		viewPointer = dynamic_cast<CSimulatorView*>(mainWindowPointer->GetActiveView());
	}
	if (nullptr == viewPointer)
	{
		return;
	}

	// Send a message to the CSimulatorView to display its custom activity indicator window.
	::SendMessage(viewPointer->GetSafeHwnd(), WMU_ACTIVITYINDICATOR, value ? 1 : 0, 0);
}

void SimulatorRuntimeEnvironment::DeviceSimulatorServices::RequestRestart()
{
	// Fetch the Corona Simulator view.
	CSimulatorView *viewPointer = nullptr;
	CFrameWnd *mainWindowPointer = dynamic_cast<CFrameWnd*>(::AfxGetMainWnd());
	if (mainWindowPointer)
	{
		viewPointer = dynamic_cast<CSimulatorView*>(mainWindowPointer->GetActiveView());
	}
	if (nullptr == viewPointer)
	{
		return;
	}

	// Restart the runtime's Corona project by invoking the "File\Relaunch" menu item in the Corona Simulator window.
	// Note: This assumes that only one Corona project can be simulated at a time.
	::PostMessage(viewPointer->GetSafeHwnd(), WM_COMMAND, ID_FILE_RELAUNCH, 0);
}

void SimulatorRuntimeEnvironment::DeviceSimulatorServices::RequestTerminate()
{
	// Fetch the Corona Simulator view.
	CSimulatorView *viewPointer = nullptr;
	CFrameWnd *mainWindowPointer = dynamic_cast<CFrameWnd*>(::AfxGetMainWnd());
	if (mainWindowPointer)
	{
		viewPointer = dynamic_cast<CSimulatorView*>(mainWindowPointer->GetActiveView());
	}
	if (nullptr == viewPointer)
	{
		return;
	}

	// Terminate the runtime by invoking the "File\Close Project" menu item in the Corona Simulator window.
	// Note: This assumes that only one Corona project can be simulated at a time.
	::PostMessage(viewPointer->GetSafeHwnd(), WM_COMMAND, ID_FILE_CLOSE, 0);
}

void SimulatorRuntimeEnvironment::DeviceSimulatorServices::Shake()
{
	// Fetch the Corona runtime.
	auto runtimePointer = fEnvironmentPointer->GetRuntime();

	// Do not continue if currently suspended or terminated.
	if (!runtimePointer || runtimePointer->IsSuspended())
	{
		return;
	}

	// Generate accelerometer data that simulates a shake event.
	double deltaTime = 0.1;
	double instant[] = { 1.5, 1.5, 1.5 };
	double gravity[] = { 0.0, 0.0, 0.0 };
	switch (fCurrentOrientation)
	{
		case Rtt::DeviceOrientation::kUpright:
			gravity[1] = -1.0;
			break;
		case Rtt::DeviceOrientation::kUpsideDown:
			gravity[1] = 1.0;
			break;
		case Rtt::DeviceOrientation::kSidewaysLeft:
			gravity[0] = 1.0;
			break;
		case Rtt::DeviceOrientation::kSidewaysRight:
			gravity[0] = -1.0;
			break;
	}

	// Dispatch an accelerometer event.
	Rtt::AccelerometerEvent event(gravity, instant, instant, true, deltaTime);
	runtimePointer->DispatchEvent(event);
}

const char* SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetOSName() const
{
	return fDeviceConfigPointer->osName.GetString();
}

void SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetSafeAreaInsetsPixels(Rtt_Real &top, Rtt_Real &left, Rtt_Real &bottom, Rtt_Real &right) const
{
	float multiplier = (GetStatusBar() == Rtt::MPlatform::kHiddenStatusBar) ? 0 : 1;

	if (Rtt::DeviceOrientation::IsSideways(fCurrentOrientation))
	{
		top = fDeviceConfigPointer->safeLandscapeScreenInsetTop;
		left = fDeviceConfigPointer->safeLandscapeScreenInsetLeft;
		bottom = fDeviceConfigPointer->safeLandscapeScreenInsetBottom;
		right = fDeviceConfigPointer->safeLandscapeScreenInsetRight;

		top += multiplier * fDeviceConfigPointer->safeLandscapeScreenInsetStatusBar;
	}
	else
	{
		top = fDeviceConfigPointer->safeScreenInsetTop;
		left = fDeviceConfigPointer->safeScreenInsetLeft;
		bottom = fDeviceConfigPointer->safeScreenInsetBottom;
		right = fDeviceConfigPointer->safeScreenInsetRight;

		top += multiplier * fDeviceConfigPointer->safeScreenInsetStatusBar;
	}
}

Rtt::MPlatform::StatusBarMode SimulatorRuntimeEnvironment::DeviceSimulatorServices::GetStatusBar() const
{
	return fCurrentStatusBar;
}

void SimulatorRuntimeEnvironment::DeviceSimulatorServices::SetStatusBar(Rtt::MPlatform::StatusBarMode newValue)
{
	fCurrentStatusBar = newValue;
}

#pragma endregion

}	// namespace Interop
