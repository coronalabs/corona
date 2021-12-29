//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_LinuxSimulator.h"

using namespace std;


SolarSimulator::SolarSimulator(const std::string& resourcesDir)
	: SolarApp(resourcesDir)
	, fWatcher(NULL)

{
	SetTitle("Solar2D Simulator");

}

SolarSimulator::~SolarSimulator()
{
	delete fWatcher;

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

