//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <math.h>
#include <wx/string.h>
#include <wx/menu.h>
#include <wx/textctrl.h>
#include <sstream>
#include <fstream>
#include "Rtt_LuaContext.h"
#include "Rtt_LuaFile.h"
#include "Rtt_MPlatform.h"
#include "Rtt_PlatformAppPackager.h"
#include "Rtt_PlatformPlayer.h"
#include "Rtt_PlatformSimulator.h"
#include "Rtt_RenderingStream.h"
#include "Rtt_Runtime.h"
#include "Rtt_SimulatorAnalytics.h"
#include "Rtt_LinuxPlatform.h"
#include "Rtt_LinuxSimulatorServices.h"
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_LinuxUtils.h"
#include "Rtt_SimulatorRecents.h"
#include "Rtt_WebAppPackager.h"
#include "Rtt_LinuxAppPackager.h"
#include "Rtt_AndroidAppPackager.h"
#include "Core/Rtt_FileSystem.h"

using namespace std;

namespace Rtt
{
	LinuxPlatformServices::LinuxPlatformServices(MPlatform* platform)
		: fPlatform(platform)
	{
	}

	// static initialise vars
	const float  LinuxSimulatorView::skinScaleFactor = 1.5;
	const int  LinuxSimulatorView::skinMinWidth = 320;
	std::map<int, LinuxSimulatorView::SkinProperties> LinuxSimulatorView::fSkins;

	bool LinuxSimulatorView::LoadSkin(lua_State* L, int skinID, std::string filePath)
	{
		int top = lua_gettop(L);
		int status = Lua::DoFile(L, filePath.c_str(), 0, true);
		lua_pop(L, 1); // remove DoFile result
		lua_getglobal(L, "simulator");

		SkinProperties skin;

		if (lua_type(L, -1) == LUA_TTABLE)
		{
			lua_getfield(L, -1, "device");
			if (lua_type(L, -1) == LUA_TSTRING)
			{
				skin.device = lua_tostring(L, -1);
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "screenOriginX");
			if (lua_type(L, -1) == LUA_TNUMBER)
			{
				skin.screenOriginX = lua_tointeger(L, -1);
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "screenOriginY");
			if (lua_type(L, -1) == LUA_TNUMBER)
			{
				skin.screenOriginY = lua_tointeger(L, -1);
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "screenWidth");
			if (lua_type(L, -1) == LUA_TNUMBER)
			{
				skin.screenWidth = lua_tointeger(L, -1);
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "screenHeight");
			if (lua_type(L, -1) == LUA_TNUMBER)
			{
				skin.screenHeight = lua_tointeger(L, -1);
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "androidDisplayApproximateDpi");
			if (lua_type(L, -1) == LUA_TNUMBER)
			{
				skin.androidDisplayApproximateDpi = lua_tointeger(L, -1);
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "displayManufacturer");
			if (lua_type(L, -1) == LUA_TSTRING)
			{
				skin.displayManufacturer = lua_tostring(L, -1);
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "displayName");
			if (lua_type(L, -1) == LUA_TSTRING)
			{
				skin.displayName = lua_tostring(L, -1);
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "isUprightOrientationPortrait");
			if (lua_type(L, -1) == LUA_TBOOLEAN)
			{
				skin.isUprightOrientationPortrait = lua_toboolean(L, -1);
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "supportsScreenRotation");
			if (lua_type(L, -1) == LUA_TBOOLEAN)
			{
				skin.supportsScreenRotation = lua_toboolean(L, -1);
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "windowTitleBarName");
			if (lua_type(L, -1) == LUA_TSTRING)
			{
				skin.windowTitleBarName = lua_tostring(L, -1);
			}
			lua_pop(L, 1);

			// pop simulator
			lua_pop(L, 1);
			lua_settop(L, top);

			wxString skinTitle(skin.windowTitleBarName);
			skinTitle.append(wxString::Format(wxT(" (%ix%i)"), skin.screenWidth, skin.screenHeight));
			skin.skinTitle = skinTitle;
			skin.id = skinID;
			skin.selected = false;

			fSkins[skinID] = skin;
		}
		else
		{
			printf("Couldn't find 'simulator' table\n");
		}

		return (status == 0);
	}

	LinuxSimulatorView::SkinProperties LinuxSimulatorView::GetSkinProperties(int skinID)
	{
		if (fSkins.count(skinID) > 0)
		{
			return fSkins[skinID];
		}

		return fSkins.begin()->second;
	}

	LinuxSimulatorView::SkinProperties LinuxSimulatorView::GetSkinProperties(wxString skinTitle)
	{
		SkinProperties foundSkin;

		for (int i = 0; i < fSkins.size(); i++)
		{
			if (fSkins[i].skinTitle.IsSameAs(skinTitle))
			{
				foundSkin = fSkins[i];
				break;
			}
		}

		return foundSkin;
	}

	void LinuxSimulatorView::DeselectSkins()
	{
		for (int i = 0; i < fSkins.size(); i++)
		{
			fSkins[i].selected = false;
		}
	}

	void LinuxSimulatorView::SelectSkin(int skinID)
	{
		if (fSkins.count(skinID) > 0)
		{
			DeselectSkins();
			fSkins[skinID].selected = true;
		}
	}

	void LinuxSimulatorView::OnLinuxPluginGet(const char* appPath, const char* appName, LinuxPlatform* platform)
	{
		const char* identity = "no-identity";

		// Create the app packager.
		MPlatformServices* service = new LinuxPlatformServices(platform);
		LinuxAppPackager packager(*service);

		// Read the application's "build.settings" file.
		bool wasSuccessful = packager.ReadBuildSettings(appPath);

		if (!wasSuccessful)
		{
			return;
		}

		int targetVersion = Rtt::TargetDevice::kLinux;
		const TargetDevice::Platform targetPlatform(TargetDevice::Platform::kLinuxPlatform);

		// Package build settings parameters.
		bool onlyGetPlugins = true;
		bool runAfterBuild = false;
		bool useWidgetResources = false;
		LinuxAppPackagerParams linuxBuilderParams(
			appName, NULL, identity, NULL,
			appPath, NULL, NULL,
			targetPlatform, targetVersion,
			Rtt::TargetDevice::kLinux, NULL,
			NULL, NULL, false, NULL, useWidgetResources, runAfterBuild, onlyGetPlugins);

		const char kBuildSettings[] = "build.settings";
		Rtt::String buildSettingsPath;
		platform->PathForFile(kBuildSettings, Rtt::MPlatform::kResourceDir, Rtt::MPlatform::kTestFileExists, buildSettingsPath);
		linuxBuilderParams.SetBuildSettingsPath(buildSettingsPath.GetString());
		int rc = packager.Build(&linuxBuilderParams, "/tmp/Solar2D");
	}

	/// Gets a list of recent projects.
	void LinuxSimulatorView::GetRecentDocs(Rtt::LightPtrArray<Rtt::RecentProjectInfo>* listPointer)
	{
		if (listPointer)
		{
			listPointer->Clear();

			vector<pair<string, string>> recentDocs;
			if (ReadRecentDocs(recentDocs))
			{
				for (int i = 0; i < recentDocs.size(); i++)
				{
					// Create a recent project info object.
					RecentProjectInfo* infoPointer = new RecentProjectInfo();
					if (!infoPointer)
					{
						continue;
					}

					const string& appName = recentDocs[i].first;
					const string& appPath = recentDocs[i].second;

					// Copy the project's folder name to the info object.
					String sTitle;
					sTitle.Append(appName.c_str());
					infoPointer->formattedString = sTitle;

					// Copy the project's "main.lua" file path to the info object.
					String sPath;
					sPath.Append(appPath.c_str());
					infoPointer->fullURLString = sPath;

					// Add the info object to the given collection.
					listPointer->Append(infoPointer);
				}
			}
		}
	}

} // namespace Rtt
