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

	private:

		smart_ptr<FileWatcher> fWatcher;
		bool fRelaunchedViaFileEvent;

		int currentSkinWidth;
		int currentSkinHeight;
		int currentSkinID;
		Skins fSkins;
	};
}

extern Rtt::SolarSimulator* solarSimulator;


