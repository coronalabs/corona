//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_LinuxSimulator.h"
#include "Rtt_LinuxUtils.h"
#include "Rtt_ConsoleApp.h"
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_LinuxMenuEvents.h"

#if !defined(wxHAS_IMAGES_IN_RESOURCES) && defined(Rtt_SIMULATOR)
#include "resource/simulator.xpm"
#endif

using namespace std;

namespace Rtt
{

	SolarSimulator::SolarSimulator()
		: fWatcher(NULL)
	{
		// start the console
		if (ConsoleApp::isStarted())
		{
			ConsoleApp::Clear();
		}
		else
		{
			std::string cmd(GetStartupPath(NULL));
			cmd.append("/Solar2DConsole");
			wxExecute(cmd);
		}
	}

	SolarSimulator::~SolarSimulator()
	{
		delete fWatcher;

	}

	bool SolarSimulator::Start(const std::string& resourcesDir)
	{
		if (SolarApp::Start(resourcesDir) == false)
			return false;

#ifdef Rtt_SIMULATOR
		SetIcon(simulator_xpm);
#endif

		// read from the simulator config file (it'll be created if it doesn't exist)
		LinuxSimulatorView::Config::Load();

		CreateMenus();
		SetTitle("Solar2D Simulator");
		return true;
	}

	void SolarSimulator::CreateMenus()
	{
		{
			fMenuMain = new wxMenuBar();

			// file Menu
			wxMenu* fileMenu = new wxMenu();
			fileMenu->Append(ID_MENU_NEW_PROJECT, _T("&New Project	\tCtrl-N"));
			fileMenu->Append(ID_MENU_OPEN_PROJECT, _T("&Open Project	\tCtrl-O"));
			fileMenu->AppendSeparator();
			fileMenu->Append(ID_MENU_OPEN_LAST_PROJECT, _T("&Relaunch Last Project	\tCtrl-R"));
			fileMenu->AppendSeparator();
			fileMenu->Append(wxID_PREFERENCES, _T("&Preferences..."));
			fileMenu->AppendSeparator();
			fileMenu->Append(wxID_EXIT, _T("&Exit"));
			fMenuMain->Append(fileMenu, _T("&File"));

			// view menu
			//fViewMenu = new wxMenu();
			//fZoomIn = fViewMenu->Append(ID_MENU_ZOOM_IN, _T("&Zoom In \tCtrl-KP_ADD"));
			//fZoomOut = fViewMenu->Append(ID_MENU_ZOOM_OUT, _T("&Zoom Out \tCtrl-KP_Subtract"));
			//fViewMenu->AppendSeparator();
			//fMenuMain->Append(fViewMenu, _T("&View"));

			// about menu
			wxMenu* helpMenu = new wxMenu();
			helpMenu->Append(ID_MENU_OPEN_DOCUMENTATION, _T("&Online Documentation..."));
			helpMenu->Append(ID_MENU_OPEN_SAMPLE_CODE, _T("&Sample projects..."));
			//			helpMenu->Append(ID_MENU_HELP_BUILD_ANDROID, _T("&Building For Android"));
			helpMenu->Append(wxID_ABOUT, _T("&About Simulator..."));
			fMenuMain->Append(helpMenu, _T("&Help"));
		}

		// project's menu
		{
			fMenuProject = new wxMenuBar();

			// file Menu
			wxMenu* fileMenu = new wxMenu();
			fileMenu->Append(ID_MENU_NEW_PROJECT, _T("&New Project	\tCtrl-N"));
			fileMenu->Append(ID_MENU_OPEN_PROJECT, _T("&Open Project	\tCtrl-O"));
			fileMenu->AppendSeparator();

			wxMenu* buildMenu = new wxMenu();
			buildMenu->Append(ID_MENU_BUILD_ANDROID, _T("Android	\tCtrl-B"));
			wxMenuItem* buildForWeb = buildMenu->Append(ID_MENU_BUILD_WEB, _T("HTML5	\tCtrl-Shift-Alt-B"));
			wxMenu* buildForLinuxMenu = new wxMenu();
			buildForLinuxMenu->Append(ID_MENU_BUILD_LINUX, _T("x64	\tCtrl-Alt-B"));
			wxMenuItem* buildForARM = buildForLinuxMenu->Append(ID_MENU_BUILD_LINUX, _T("ARM	\tCtrl-Alt-A"));
			buildMenu->AppendSubMenu(buildForLinuxMenu, _T("&Linux"));
			fileMenu->AppendSubMenu(buildMenu, _T("&Build"));
			buildForARM->Enable(false);

			fileMenu->Append(ID_MENU_OPEN_IN_EDITOR, _T("&Open In Editor	\tCtrl-Shift-O"));
			fileMenu->Append(ID_MENU_SHOW_PROJECT_FILES, _T("&Show Project Files"));
			fileMenu->Append(ID_MENU_SHOW_PROJECT_SANDBOX, _T("&Show Project Sandbox"));
			fileMenu->AppendSeparator();
			fileMenu->Append(ID_MENU_CLEAR_PROJECT_SANDBOX, _T("&Clear Project Sandbox"));
			fileMenu->AppendSeparator();
			fileMenu->Append(ID_MENU_RELAUNCH_PROJECT, _T("Relaunch	\tCtrl-R"));
			fileMenu->Append(ID_MENU_CLOSE_PROJECT, _T("Close Project	\tCtrl-W"));
			fileMenu->AppendSeparator();
			fileMenu->Append(wxID_PREFERENCES, _T("&Preferences..."));
			fileMenu->AppendSeparator();
			fileMenu->Append(wxID_EXIT, _T("&Exit"));
			fMenuProject->Append(fileMenu, _T("&File"));

			// hardware menu
			fHardwareMenu = new wxMenu();
			wxMenuItem* rotateLeft = fHardwareMenu->Append(wxID_HELP_CONTENTS, _T("&Rotate Left"));
			wxMenuItem* rotateRight = fHardwareMenu->Append(wxID_HELP_INDEX, _T("&Rotate Right"));
			//fHardwareMenu->Append(wxID_ABOUT, _T("&Shake"));
			fHardwareMenu->AppendSeparator();
			wxMenuItem* back = fHardwareMenu->Append(ID_MENU_BACK_BUTTON, _T("&Back"));
			fHardwareMenu->AppendSeparator();
			fHardwareMenu->Append(ID_MENU_SUSPEND, _T("&Suspend	\tCtrl-Down"));
			fMenuProject->Append(fHardwareMenu, _T("&Hardware"));
			rotateLeft->Enable(false);
			rotateRight->Enable(false);

			// view menu
			fViewMenu = new wxMenu();
			fZoomIn = fViewMenu->Append(ID_MENU_ZOOM_IN, _T("&Zoom In \tCtrl-KP_ADD"));
			fZoomOut = fViewMenu->Append(ID_MENU_ZOOM_OUT, _T("&Zoom Out \tCtrl-KP_Subtract"));
			fViewMenu->AppendSeparator();
			fMenuProject->Append(fViewMenu, _T("&View"));

			// about menu
			wxMenu* helpMenu = new wxMenu();
			helpMenu->Append(ID_MENU_OPEN_DOCUMENTATION, _T("&Online Documentation..."));
			helpMenu->Append(ID_MENU_OPEN_SAMPLE_CODE, _T("&Sample projects..."));
			//			helpMenu->Append(ID_MENU_HELP_BUILD_ANDROID, _T("&Building For Android"));
			helpMenu->Append(wxID_ABOUT, _T("&About Simulator..."));
			fMenuProject->Append(helpMenu, _T("&Help"));
		}
	}

	void SolarSimulator::WatchFolder(const char* path, const char* appName)
	{
		if (IsHomeScreen(string(appName)))
		{
			// do not watch main screen folder
			return;
		}

		// wxFileSystemWatcher
		if (fWatcher == NULL)
		{
			fWatcher = new wxFileSystemWatcher();
			fWatcher->SetOwner(this);
			Connect(wxEVT_FSWATCHER, wxFileSystemWatcherEventHandler(SolarSimulator::OnFileSystemEvent));
		}

		wxFileName fn = wxFileName::DirName(path);
		fn.DontFollowLink();
		fWatcher->RemoveAll();
		fWatcher->AddTree(fn);
	}

	void SolarSimulator::OnFileSystemEvent(wxFileSystemWatcherEvent& event)
	{
		if (fContext->GetRuntime()->IsSuspended())
		{
			return;
		}

		int type = event.GetChangeType();
		const wxFileName& f = event.GetPath();
		wxString fn = f.GetFullName();
		wxString fp = f.GetFullPath();
		wxString ext = f.GetExt();

		switch (type)
		{
		case wxFSW_EVENT_CREATE:
		case wxFSW_EVENT_DELETE:
		case wxFSW_EVENT_RENAME:
		case wxFSW_EVENT_MODIFY:
		{
			if (ext.IsSameAs("lua"))
			{
				fRelaunchedViaFileEvent = true;
				wxCommandEvent ev(eventRelaunchProject);
				wxPostEvent(solarApp, ev);
			}
			break;
		}

		default:
			break;
		}
	}
}
