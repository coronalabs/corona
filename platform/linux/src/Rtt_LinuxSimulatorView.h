//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Array.h"
#include "Rtt_PlatformSimulator.h"
#include "Rtt_TargetDevice.h"
#include "Rtt_LinuxSimulatorServices.h"
#include "Rtt_LinuxContext.h"
#include "Rtt_LinuxContainer.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_MPlatform.h"

namespace Rtt
{
	class LinuxPlatformServices : public MPlatformServices
	{
	public:
		LinuxPlatformServices(MPlatform* platform);

		// MPlatformServices
		virtual const MPlatform& Platform() const override { return *fPlatform; }

		virtual void GetPreference(const char* key, String* value) const override;

		// Pass NULL for value to remove pref
		virtual void SetPreference(const char* key, const char* value) const override
		{
			assert(0&&"todo: SetPreference");
			//fPlatform->SetPreference(key, value);
		}

		virtual void GetSecurePreference(const char* key, String* value) const override { return GetPreference(key, value); }
		virtual bool SetSecurePreference(const char* key, const char* value) const
		{
			SetPreference(key, value);
			return true;
		}

		virtual void GetLibraryPreference(const char* key, String* value) const override { return GetPreference(key, value); }
		virtual void SetLibraryPreference(const char* key, const char* value) const override { SetPreference(key, value); }
		virtual bool IsInternetAvailable() const override { return false; };
		virtual bool IsLocalWifiAvailable() const override { return false; };
		virtual void Terminate() const override {}
		virtual void Sleep(int milliseconds) const override {};

	private:
		MPlatform* fPlatform;
	};

	class LinuxSimulatorView
	{
	public:
		LinuxSimulatorView();
		virtual ~LinuxSimulatorView();

		static void OnLinuxPluginGet(const char* appPath, const char* appName, LinuxPlatform* platform);
		static void GetRecentDocs(LightPtrArray<RecentProjectInfo>* listPointer);

	};

	//
	// Skins
	//

	struct SkinProperties
	{
		std::string device;
		int screenOriginX;
		int screenOriginY;
		int screenWidth;
		int screenHeight;
		int androidDisplayApproximateDpi;
		std::string displayManufacturer;
		std::string displayName;
		bool isUprightOrientationPortrait;
		bool supportsScreenRotation;
		std::string windowTitleBarName;
		std::string skinTitle;
		bool selected;
	};

	struct Skins
	{
		bool Load(lua_State* L);
		const std::map<std::string, SkinProperties>* GetSkins(const std::string& tabname) const;
		int Count() const { return fSkins.size(); }

	private:

		bool Load(lua_State* L, const std::string& filePath, SkinProperties& skin);
		std::map<std::string, std::map<std::string, SkinProperties>> fSkins;	// <tab, <filename, properties>>
	};

}; // namespace Rtt
