//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_LinuxApp.h"
#include "Rtt_LinuxSimulatorView.h"

namespace Rtt
{
	struct SolarSimulator : public SolarApp
	{
		SolarSimulator(const std::string& resourceDir);
		virtual ~SolarSimulator();

		void OnOpen(const std::string& path);
		void OnRelaunch();
		void OnZoomIn();
		void OnZoomOut();
		void OnViewAsChanged(const SkinProperties* skin);

		void WatchFolder(const char* path, const char* appName);
		virtual bool LoadApp() override;
		void GetSavedZoom(int& width, int& height) override;
		bool IsRunningOnSimulator() override { return true; }
		void SolarEvent(const SDL_Event& e) override;

		// for simulator settings
		void ConfigLoad() override;
		void ConfigSave() override;
		std::string& ConfigStr(const std::string& key);
		int ConfigInt(const std::string& key);
		void ConfigSet(const char* key, std::string& val);
		void ConfigSet(const char* key, int val);
		virtual std::map<std::string, std::string>* ConfigGet() override { return &fConfig; }

	private:

		std::map<std::string, std::string> fConfig;
		smart_ptr<FileWatcher> fWatcher;
		bool fRelaunchedViaFileEvent;
		std::string fConfigFilePath;

		int currentSkinWidth;
		int currentSkinHeight;
		int currentSkinID;
		Skins fSkins;
	};
}

extern Rtt::SolarSimulator* solarSimulator;


