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
	struct ConsoleClient : public ref_counted
	{
		ConsoleClient();
		virtual ~ConsoleClient();

		bool Connect(int attempts);
		int Log(const char* buf, int len);

	private:
		int fSocket;
	};

	struct SolarSimulator : public SolarApp
	{
		SolarSimulator(const std::string& resourceDir);
		virtual ~SolarSimulator();

		void OnOpen(const std::string& path);
		void OnRelaunch();
		void OnZoomIn(wxCommandEvent& event);
		void OnZoomOut(wxCommandEvent& event);
		void OnViewAsChanged(wxCommandEvent& event);
		void CreateViewAsChildMenu(std::vector<std::string>skin, wxMenu* targetMenu);

		// menu events
		void OnCloneProject(wxCommandEvent& event);
		void OnClearProjectSandbox(wxCommandEvent& event);
		void OnAndroidBackButton(wxCommandEvent& event);
		void OnBuildForAndroid(wxCommandEvent& event);
		void OnBuildForWeb(wxCommandEvent& event);

		void WatchFolder(const char* path, const char* appName);
		virtual bool LoadApp() override;
		void ClearMenuCheckboxes(wxMenu* menu, wxString currentSkinTitle);
		void GetSavedZoom(int& width, int& height) override;
		bool IsRunningOnSimulator() override { return true; }
		void SolarEvent(SDL_Event& e) override;

		// for simulator settings
		void ConfigLoad() override;
		void ConfigSave() override;
		std::string& ConfigStr(const std::string& key);
		int ConfigInt(const std::string& key);
		void ConfigSet(const char* key, std::string& val);
		void ConfigSet(const char* key, int val);
		virtual std::map<std::string, std::string>* ConfigGet() override { return &fConfig; }
		virtual void Log(const char* buf, int len) override;


	private:

		std::map<std::string, std::string> fConfig;
		smart_ptr<FileWatcher> fWatcher;
		bool fRelaunchedViaFileEvent;
		wxLongLong fFileSystemEventTimestamp;
		std::string fConfigFilePath;
		int currentSkinWidth;
		int currentSkinHeight;
		smart_ptr<ConsoleClient> fConsole;
	};
}

extern Rtt::SolarSimulator* solarSimulator;


