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

	class SolarSimulator : public SolarApp
	{
	public:
		SolarSimulator();
		virtual ~SolarSimulator();

		void OnFileSystemEvent(wxFileSystemWatcherEvent& event);
		void OnOpen(wxCommandEvent& event);
		void OnClose(wxCloseEvent& event) override;
		void OnRelaunch(wxCommandEvent& event);
		void OnZoomIn(wxCommandEvent& event);
		void OnZoomOut(wxCommandEvent& event);
		static void OnViewAsChanged(wxCommandEvent& event);
		void OnSuspendOrResume(wxCommandEvent& event);
		void OnOpenWelcome(wxCommandEvent& event);
		void CreateViewAsChildMenu(std::vector<std::string>skin, wxMenu* targetMenu);

		void WatchFolder(const char* path, const char* appName) override;
		bool Start(const std::string& resourcesDir) override;
		void CreateSuspendedPanel() override;
		void RemoveSuspendedPanel() override;
		void CreateMenus() override;
		void SetMenu(const char* appPath) override;
		void GetSavedZoom(int& width, int& height) override;
		bool IsRunningOnSimulator() override { return true; }

		inline bool IsHomeScreen(const std::string& appName) { return appName.compare(HOMESCREEN_ID) == 0; }

		smart_ptr<SimulatorConfig> fSimulatorConfig;

	private:

		wxFileSystemWatcher* fWatcher;
		wxPanel* suspendedPanel;
		bool fRelaunchedViaFileEvent;

		wxMenuBar* fMenuMain;
		wxMenu* fViewMenu;
		wxMenu* fViewAsAndroidMenu;
		wxMenu* fViewAsIOSMenu;
		wxMenu* fViewAsTVMenu;
		wxMenu* fViewAsDesktopMenu;
		wxMenuItem* fZoomIn;
		wxMenuItem* fZoomOut;
		wxMenuBar* fMenuProject;
	};
}

Rtt::SolarSimulator* solarSimulator();


