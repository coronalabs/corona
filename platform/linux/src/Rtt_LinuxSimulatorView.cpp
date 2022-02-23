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

	void LinuxSimulatorView::OnLinuxPluginGet(const char* appPath, const char* appName, LinuxPlatform* platform)
	{
		const char* identity = "no-identity";

		// Create the app packager.
		LinuxPlatformServices service(platform);
		LinuxAppPackager packager(service);

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

	//
	// Skins
	//

	const map<string, SkinProperties>* Skins::GetSkins(const string& tabname) const
	{
		const auto& it = fSkins.find(tabname);
		return it == fSkins.end() ? NULL : &it->second;
	}

	bool Skins::Load(lua_State* L)
	{
		const char* startupPath = GetStartupPath(NULL);
		string skinDirPath(startupPath);
		skinDirPath.append("/Resources");
		if (!Rtt_IsDirectory(skinDirPath.c_str()))
		{
			Rtt_LogException("No Resources dir in %s!\n", startupPath);
			return false;
		}

		skinDirPath.append("/Skins");
		if (!Rtt_IsDirectory(skinDirPath.c_str()))
		{
			Rtt_LogException("Skin directory not found in /Resources!\n");
			return false;
		}

		vector<string> skins = Rtt_ListFiles(skinDirPath.c_str());
		for (int i = 0; i < skins.size(); i++)
		{
			const string& filename = skins[i];
			SkinProperties sProperties;
			if (filename.rfind(".lua") != string::npos && Load(L, filename, sProperties))
			{
				string skinTitle(sProperties.windowTitleBarName);
				skinTitle += to_string(sProperties.screenWidth) + "x" + to_string(sProperties.screenHeight);
				sProperties.skinTitle = skinTitle;

				if (sProperties.device.find("android") != string::npos && !sProperties.device.find("tv") != string::npos)
				{
					if (sProperties.device.find("borderless") != string::npos)
					{
						fSkins["genericAndroid"][skinTitle] = sProperties;
					}
					else
					{
						fSkins["namedAndroid"][skinTitle] = sProperties;
					}
				}
				else if (sProperties.device.find("ios") != string::npos)
				{
					if (sProperties.device.find("borderless") != string::npos)
					{
						fSkins["genericIOS"][skinTitle] = sProperties;
					}
					else
					{
						fSkins["namedIOS"][skinTitle] = sProperties;
					}
				}
				else if (sProperties.device.find("tv") != string::npos)
				{
					fSkins["tv"][skinTitle] = sProperties;
				}
				else if (sProperties.device.find("desktop") != string::npos)
				{
					fSkins["desktop"][skinTitle] = sProperties;
				}
			}
		}
		return true;
	}


	bool Skins::Load(lua_State* L, const string& filePath, SkinProperties& skin)
	{
		int top = lua_gettop(L);
		int status = Lua::DoFile(L, filePath.c_str(), 0, true);
		lua_pop(L, 1); // remove DoFile result
		lua_getglobal(L, "simulator");

		skin = {};
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

			string skinTitle(skin.windowTitleBarName);
			skinTitle += to_string(skin.screenWidth) + "x" + to_string(skin.screenHeight);
			skin.skinTitle = skinTitle;
			skin.selected = false;
		}
		else
		{
			printf("Couldn't find 'simulator' table\n");
		}
		return (status == 0);
	}


} // namespace Rtt
