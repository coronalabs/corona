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
#include "Rtt_FileSystem.h"

#if !defined(wxHAS_IMAGES_IN_RESOURCES) && defined(Rtt_SIMULATOR)
#include "resource/simulator.xpm"
#endif

using namespace std;

namespace Rtt
{
	wxDEFINE_EVENT(eventOpenProject, wxCommandEvent);
	wxDEFINE_EVENT(eventRelaunchProject, wxCommandEvent);
	wxDEFINE_EVENT(eventWelcomeProject, wxCommandEvent);

	// setup frame events
	wxBEGIN_EVENT_TABLE(SolarApp, wxFrame)
		EVT_MENU(ID_MENU_OPEN_WELCOME_SCREEN, SolarApp::OnOpenWelcome)
		EVT_MENU(ID_MENU_RELAUNCH_PROJECT, SolarSimulator::OnRelaunch)
		EVT_MENU(ID_MENU_SUSPEND, SolarApp::OnSuspendOrResume)
		EVT_MENU(ID_MENU_CLOSE_PROJECT, SolarApp::OnOpenWelcome)
		EVT_MENU(ID_MENU_ZOOM_IN, SolarApp::OnZoomIn)
		EVT_MENU(ID_MENU_ZOOM_OUT, SolarApp::OnZoomOut)
		EVT_COMMAND(wxID_ANY, eventOpenProject, SolarApp::OnOpen)
		EVT_COMMAND(wxID_ANY, eventRelaunchProject, SolarSimulator::OnRelaunch)
		EVT_COMMAND(wxID_ANY, eventWelcomeProject, SolarApp::OnOpenWelcome)
		EVT_ICONIZE(SolarApp::OnIconized)
		EVT_CLOSE(SolarApp::OnClose)
		EVT_TIMER(TIMER_ID, SolarApp::OnTimer)
		wxEND_EVENT_TABLE()

		SolarSimulator::SolarSimulator()
		: fWatcher(NULL)
		,	suspendedPanel(NULL)
		, fRelaunchedViaFileEvent(false)
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

		const char* homeDir = GetHomePath();
		string buildPath(homeDir);
		string projectCreationPath(homeDir);

		// create default directories if missing

		buildPath.append("/Documents/Solar2D Built Apps");
		projectCreationPath.append("/Documents/Solar2D Projects");
		if (!Rtt_IsDirectory(buildPath.c_str()))
		{
			Rtt_MakeDirectory(buildPath.c_str());
		}
		if (!Rtt_IsDirectory(projectCreationPath.c_str()))
		{
			Rtt_MakeDirectory(projectCreationPath.c_str());
		}
	}

	SolarSimulator::~SolarSimulator()
	{
		LinuxSimulatorView::Config::Cleanup();
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
		currentSkinWidth = LinuxSimulatorView::Config::skinWidth;
		currentSkinHeight = LinuxSimulatorView::Config::skinHeight;
		
		fRelaunchProjectDialog = new LinuxRelaunchProjectDialog(NULL, wxID_ANY, wxEmptyString);

		SetPosition(wxPoint(LinuxSimulatorView::Config::windowXPos, LinuxSimulatorView::Config::windowYPos));
		SetTitle("Solar2D Simulator");

		Bind(wxEVT_MENU, &LinuxMenuEvents::OnNewProject, ID_MENU_NEW_PROJECT);
		Bind(wxEVT_MENU, &LinuxMenuEvents::OnOpenFileDialog, ID_MENU_OPEN_PROJECT);
		Bind(wxEVT_MENU, &LinuxMenuEvents::OnRelaunchLastProject, ID_MENU_OPEN_LAST_PROJECT);
		Bind(wxEVT_MENU, &LinuxMenuEvents::OnOpenInEditor, ID_MENU_OPEN_IN_EDITOR);
		Bind(wxEVT_MENU, &LinuxMenuEvents::OnShowProjectFiles, ID_MENU_SHOW_PROJECT_FILES);
		Bind(wxEVT_MENU, &LinuxMenuEvents::OnShowProjectSandbox, ID_MENU_SHOW_PROJECT_SANDBOX);
		Bind(wxEVT_MENU, &LinuxMenuEvents::OnClearProjectSandbox, ID_MENU_CLEAR_PROJECT_SANDBOX);
		Bind(wxEVT_MENU, &LinuxMenuEvents::OnAndroidBackButton, ID_MENU_BACK_BUTTON);
		Bind(wxEVT_MENU, &LinuxMenuEvents::OnBuildForAndroid, ID_MENU_BUILD_ANDROID);
		Bind(wxEVT_MENU, &LinuxMenuEvents::OnBuildForWeb, ID_MENU_BUILD_WEB);
		Bind(wxEVT_MENU, &LinuxMenuEvents::OnBuildForLinux, ID_MENU_BUILD_LINUX);
		Bind(wxEVT_MENU, &LinuxMenuEvents::OnOpenPreferences, wxID_PREFERENCES);
		Bind(wxEVT_MENU, &LinuxMenuEvents::OnQuit, wxID_EXIT);
		Bind(wxEVT_MENU, &LinuxMenuEvents::OnOpenDocumentation, ID_MENU_OPEN_DOCUMENTATION);
		Bind(wxEVT_MENU, &LinuxMenuEvents::OnOpenSampleProjects, ID_MENU_OPEN_SAMPLE_CODE);
		Bind(wxEVT_MENU, &LinuxMenuEvents::OnAbout, wxID_ABOUT);

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

	void SolarSimulator::CreateSuspendedPanel()
	{
		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			if (suspendedPanel == NULL)
			{
				suspendedPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(fContext->GetWidth(), fContext->GetHeight()));
				suspendedPanel->SetBackgroundColour(wxColour(*wxBLACK));
				suspendedPanel->SetForegroundColour(wxColour(*wxBLACK));
				suspendedText = new wxStaticText(this, -1, "Suspended", wxDefaultPosition, wxDefaultSize);
				suspendedText->SetForegroundColour(*wxWHITE);
				suspendedText->CenterOnParent();
			}
		}
	}

	void SolarSimulator::RemoveSuspendedPanel()
	{
		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			if (suspendedPanel == NULL)
			{
				return;
			}

			suspendedPanel->Destroy();
			suspendedText->Destroy();
			suspendedPanel = NULL;
		}
	}

	void SolarSimulator::OnRelaunch(wxCommandEvent& event)
	{
		if (fAppPath.size() > 0 && !IsHomeScreen(fContext->GetAppName()))
		{
			bool doRelaunch = !fRelaunchedViaFileEvent;

			if (fContext->GetPlatform()->GetRuntimeErrorDialog()->IsShown() || fRelaunchProjectDialog->IsShown())
			{
				return;
			}

			// workaround for wxFileSystem events firing twice (known wx bug)
			if (fFileSystemEventTimestamp >= wxGetUTCTimeMillis() - 250)
			{
				return;
			}

			if (fRelaunchedViaFileEvent)
			{
				switch (LinuxSimulatorView::Config::relaunchOnFileChange)
				{
				case LinuxPreferencesDialog::RelaunchType::Always:
					doRelaunch = true;
					break;

				case LinuxPreferencesDialog::RelaunchType::Ask:
					if (fRelaunchProjectDialog->ShowModal() == wxID_OK)
					{
						doRelaunch = true;
					}
					break;

				default:
					break;
				}

				fRelaunchedViaFileEvent = false;
			}

			if (!doRelaunch)
			{
				return;
			}

			fContext->GetRuntime()->End();
			delete fContext;
			fContext = new SolarAppContext(fAppPath.c_str());
			chdir(fContext->GetAppPath());
			RemoveSuspendedPanel();

			if (LinuxSimulatorView::IsRunningOnSimulator())
			{
				WatchFolder(fContext->GetAppPath(), fContext->GetAppName().c_str());
				SetCursor(wxCURSOR_ARROW);
			}

			bool fullScreen = fContext->Init();
			wxString newWindowTitle(fContext->GetTitle());

			if (LinuxSimulatorView::IsRunningOnSimulator())
			{
				LinuxSimulatorView::SkinProperties sProperties = LinuxSimulatorView::GetSkinProperties(LinuxSimulatorView::Config::skinID);
				newWindowTitle.append(" - ").append(sProperties.skinTitle.ToStdString());
				LinuxSimulatorView::OnLinuxPluginGet(fContext->GetAppPath(), fContext->GetAppName().c_str(), fContext->GetPlatform());
			}

			fContext->LoadApp(fSolarGLCanvas);
			ResetSize();
			fContext->SetCanvas(fSolarGLCanvas);
			SetMenu(fAppPath.c_str());
			SetTitle(newWindowTitle);

			fContext->RestartRenderer();
			StartTimer(1000.0f / (float)fContext->GetFPS());
			fFileSystemEventTimestamp = wxGetUTCTimeMillis();
		}
	}


}
