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

		void WatchFolder(const std::string& path);
		bool LoadApp(const std::string& path) override;
		void SolarEvent(const SDL_Event& e) override;
		void StartConsole() override;
		void CreateMenu() override;
		bool IsRunningOnSimulator() override { return true; }
		bool Init() override;

		virtual int GetMenuHeight() const { return 19; }		// hack

	private:

		smart_ptr<FileWatcher> fWatcher;
		Skins fSkins;
	};
}

extern Rtt::SolarSimulator* solarSimulator;


