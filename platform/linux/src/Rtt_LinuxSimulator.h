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
#include "wx/fswatcher.h"

namespace Rtt
{
	class SolarSimulator : public SolarApp
	{
	public:
		SolarSimulator(const std::string& resourceDir);
		virtual ~SolarSimulator();

		void GuiEvent(SDL_Event& e) override;
		void RenderGUI() override;

		void OnFileSystemEvent(wxFileSystemWatcherEvent& event);
		void OnOpen(const std::string& path);
		void OnClose(wxCloseEvent& event) override;
		void OnRelaunch();
		void OnZoomIn(wxCommandEvent& event);
		void OnZoomOut(wxCommandEvent& event);
		void OnViewAsChanged(wxCommandEvent& event);
		void OnSuspendOrResume(wxCommandEvent& event);
		void OnOpenWelcome(wxCommandEvent& event);
		void CreateViewAsChildMenu(std::vector<std::string>skin, wxMenu* targetMenu);

		// menu events
		void OnCloneProject(wxCommandEvent& event);
		void OnNewProject(wxCommandEvent& event);
		void OnOpenFileDialog(wxCommandEvent& event);
		void OnRelaunchLastProject(wxCommandEvent& event);
		void OnOpenInEditor();
		void OnShowProjectFiles(wxCommandEvent& event);
		void OnShowProjectSandbox(wxCommandEvent& event);
		void OnClearProjectSandbox(wxCommandEvent& event);
		void OnAndroidBackButton(wxCommandEvent& event);
		void OnOpenPreferences(wxCommandEvent& event);
		void OnQuit(wxCommandEvent& WXUNUSED(event));
		void OnBuildForAndroid(wxCommandEvent& event);
		void OnBuildForWeb(wxCommandEvent& event);
		void OnBuildForLinux(wxCommandEvent& event);
		void OnOpenSampleProjects();

		void WatchFolder(const char* path, const char* appName);
		virtual bool Initialize() override;
		void CreateSuspendedPanel();
		void RemoveSuspendedPanel();
		void ClearMenuCheckboxes(wxMenu* menu, wxString currentSkinTitle);
		void SetMenu(const char* appPath);
		void GetSavedZoom(int& width, int& height) override;
		bool IsRunningOnSimulator() override { return true; }

		inline bool IsHomeScreen(const std::string& appName) { return appName.compare(HOMESCREEN_ID) == 0; }

		// for simulator settings
		void ConfigLoad();
		void ConfigSave();
		std::string& ConfigStr(const std::string& key);
		int ConfigInt(const std::string& key);
		void ConfigSet(const char* key, std::string& val);
		void ConfigSet(const char* key, int val);

	private:

		std::map<std::string, std::string> fConfig;
		wxFileSystemWatcher* fWatcher;
		wxPanel* suspendedPanel;
		bool fRelaunchedViaFileEvent;
		LinuxRelaunchProjectDialog* fRelaunchProjectDialog;

		wxMenuBar* fMenuMain;
		wxMenu* fViewMenu;
		wxMenu* fViewAsAndroidMenu;
		wxMenu* fViewAsIOSMenu;
		wxMenu* fViewAsTVMenu;
		wxMenu* fViewAsDesktopMenu;
		wxMenu* fHardwareMenu;
		wxMenuItem* fZoomIn;
		wxMenuItem* fZoomOut;
		wxMenuBar* fMenuProject;
		wxLongLong fFileSystemEventTimestamp;
		std::string fConfigFilePath;
		int currentSkinWidth;
		int currentSkinHeight;
	};
}

extern Rtt::SolarSimulator* solarSimulator;


