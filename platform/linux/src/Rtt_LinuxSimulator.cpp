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
#ifdef Rtt_SIMULATOR
		SetIcon(simulator_xpm);
#endif

		if (SolarApp::Start(resourcesDir) == false)
			return false;

		SetTitle("Solar2D Simulator");

		// read from the simulator config file (it'll be created if it doesn't exist)
		LinuxSimulatorView::Config::Load();

		return true;
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
