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
#include "Rtt_LuaContext.h"
#include "wx/display.h"

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
		EVT_MENU(ID_MENU_OPEN_WELCOME_SCREEN, SolarSimulator::OnOpenWelcome)
		EVT_MENU(ID_MENU_RELAUNCH_PROJECT, SolarSimulator::OnRelaunch)
		EVT_MENU(ID_MENU_SUSPEND, SolarSimulator::OnSuspendOrResume)
		EVT_MENU(ID_MENU_CLOSE_PROJECT, SolarSimulator::OnOpenWelcome)
		EVT_MENU(ID_MENU_ZOOM_IN, SolarSimulator::OnZoomIn)
		EVT_MENU(ID_MENU_ZOOM_OUT, SolarSimulator::OnZoomOut)
		EVT_COMMAND(wxID_ANY, eventOpenProject, SolarSimulator::OnOpen)
		EVT_COMMAND(wxID_ANY, eventRelaunchProject, SolarSimulator::OnRelaunch)
		EVT_COMMAND(wxID_ANY, eventWelcomeProject, SolarSimulator::OnOpenWelcome)
		EVT_ICONIZE(SolarApp::OnIconized)
		EVT_CLOSE(SolarApp::OnClose)
		EVT_TIMER(TIMER_ID, SolarApp::OnTimer)
		wxEND_EVENT_TABLE()

		SolarSimulator::SolarSimulator()
		: fWatcher(NULL)
		, suspendedPanel(NULL)
		, fRelaunchedViaFileEvent(false)
		, fMenuMain(NULL)
		, fViewMenu(NULL)
		, fViewAsAndroidMenu(NULL)
		, fViewAsIOSMenu(NULL)
		, fViewAsTVMenu(NULL)
		, fViewAsDesktopMenu(NULL)
		, fZoomIn(NULL)
		, fZoomOut(NULL)
		, fMenuProject(NULL)
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

		SetMenuBar(NULL);
		delete fMenuMain;
		delete fMenuProject;
	}

	bool SolarSimulator::Start(const std::string& resourcesDir)
	{
		CreateWindow(resourcesDir);

#ifdef Rtt_SIMULATOR
		SetIcon(simulator_xpm);
#endif

		fContext = new SolarAppContext(resourcesDir.c_str());
		fContext->LoadApp(fSolarGLCanvas);
		ResetWindowSize();

		CreateMenus();
		SetMenu(resourcesDir.c_str());

		// restore home screen zoom level
	//	if (IsHomeScreen(appName))
	//	{
	//		fContext->GetRuntimeDelegate()->fContentWidth = LinuxSimulatorView::Config::welcomeScreenZoomedWidth;
	//		fContext->GetRuntimeDelegate()->fContentHeight = LinuxSimulatorView::Config::welcomeScreenZoomedHeight;
	//		ChangeSize(fContext->GetRuntimeDelegate()->fContentWidth, fContext->GetRuntimeDelegate()->fContentHeight);
	//	}

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

	void SolarSimulator::RemoveSuspendedPanel()
	{
		if (suspendedPanel == NULL)
		{
			return;
		}

		suspendedPanel->Destroy();
		suspendedText->Destroy();
		suspendedPanel = NULL;
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

			RemoveSuspendedPanel();

			fContext->GetRuntime()->End();
			delete fContext;
			fContext = new SolarAppContext(fAppPath.c_str());
			fContext->LoadApp(fSolarGLCanvas);
			ResetWindowSize();

			WatchFolder(fContext->GetAppPath(), fContext->GetAppName().c_str());
			SetCursor(wxCURSOR_ARROW);

			wxString newWindowTitle(fContext->GetTitle());

			LinuxSimulatorView::SkinProperties sProperties = LinuxSimulatorView::GetSkinProperties(LinuxSimulatorView::Config::skinID);
			newWindowTitle.append(" - ").append(sProperties.skinTitle.ToStdString());
			LinuxSimulatorView::OnLinuxPluginGet(fContext->GetAppPath(), fContext->GetAppName().c_str(), fContext->GetPlatform());

			SetMenu(fAppPath.c_str());
			SetTitle(newWindowTitle);

			fFileSystemEventTimestamp = wxGetUTCTimeMillis();
		}
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

	void SolarSimulator::SetMenu(const char* appPath)
	{
		const string& appName = GetContext()->GetAppName();
		SetMenuBar(IsHomeScreen(appName) ? fMenuMain : fMenuProject);

		if (!IsHomeScreen(appName) && fViewMenu->FindItem("View As") == -1)
		{
			wxMenu* viewAsMenu = new wxMenu();
			fViewAsAndroidMenu = new wxMenu();
			fViewAsIOSMenu = new wxMenu();
			fViewAsTVMenu = new wxMenu();
			fViewAsDesktopMenu = new wxMenu();
			vector<string>namedAndroidSkins;
			vector<string>genericAndroidSkins;
			vector<string>namedIOSSkins;
			vector<string>genericIOSSkins;
			vector<string>tvSkins;
			vector<string>desktopSkins;
			int currentSkinID = ID_MENU_VIEW_AS;

			const char* startupPath = GetStartupPath(NULL);
			string skinDirPath(startupPath);
			skinDirPath.append("/Resources");
			if (!Rtt_IsDirectory(skinDirPath.c_str()))
			{
				Rtt_LogException("No Resources dir in %s!\n", startupPath);
				return;
			}

			skinDirPath.append("/Skins");
			wxDir skinDir(skinDirPath);
			if (!skinDir.IsOpened())
			{
				Rtt_LogException("Skin directory not found in /Resources!\n");
				return;
			}


			if (!skinDir.IsOpened())
			{
				Rtt_LogException("Skin directory not found in /Resources!\n");
				return;
			}

			wxString filename;
			lua_State* L = GetContext()->GetRuntime()->VMContext().L();
			bool fileExists = skinDir.GetFirst(&filename, wxEmptyString, wxDIR_DEFAULT);

			while (fileExists)
			{
				if (filename.EndsWith(".lua"))
				{
					wxString luaSkinPath(skinDirPath);
					luaSkinPath.append("/").append(filename);

					LinuxSimulatorView::LoadSkin(L, currentSkinID, luaSkinPath.ToStdString());
					LinuxSimulatorView::SkinProperties sProperties = LinuxSimulatorView::GetSkinProperties(currentSkinID);
					wxString skinTitle(sProperties.windowTitleBarName);
					skinTitle.append(wxString::Format(wxT(" (%ix%i)"), sProperties.screenWidth, sProperties.screenHeight));

					if (sProperties.device.Contains("android") && !sProperties.device.Contains("tv"))
					{
						if (sProperties.device.Contains("borderless"))
						{
							genericAndroidSkins.push_back(skinTitle.ToStdString());
						}
						else
						{
							namedAndroidSkins.push_back(skinTitle.ToStdString());
						}
					}
					else if (sProperties.device.Contains("ios"))
					{
						if (sProperties.device.Contains("borderless"))
						{
							genericIOSSkins.push_back(skinTitle.ToStdString());
						}
						else
						{
							namedIOSSkins.push_back(skinTitle.ToStdString());
						}
					}
					else if (sProperties.device.Contains("tv"))
					{
						tvSkins.push_back(skinTitle.ToStdString());
					}
					else if (sProperties.device.Contains("desktop"))
					{
						desktopSkins.push_back(skinTitle.ToStdString());
					}
				}

				currentSkinID++;
				fileExists = skinDir.GetNext(&filename);
			}

			// sort all the skin vectors by name
			sort(namedAndroidSkins.begin(), namedAndroidSkins.end(), SortVectorByName);
			sort(genericAndroidSkins.begin(), genericAndroidSkins.end(), SortVectorByName);
			sort(namedIOSSkins.begin(), namedIOSSkins.end(), SortVectorByName);
			sort(genericIOSSkins.begin(), genericIOSSkins.end(), SortVectorByName);
			sort(tvSkins.begin(), tvSkins.end(), SortVectorByName);
			sort(desktopSkins.begin(), desktopSkins.end(), SortVectorByName);

			// setup the child "view as" menus
			CreateViewAsChildMenu(namedAndroidSkins, fViewAsAndroidMenu);
			fViewAsAndroidMenu->AppendSeparator();
			CreateViewAsChildMenu(genericAndroidSkins, fViewAsAndroidMenu);
			CreateViewAsChildMenu(namedIOSSkins, fViewAsIOSMenu);
			fViewAsIOSMenu->AppendSeparator();
			CreateViewAsChildMenu(genericIOSSkins, fViewAsIOSMenu);
			CreateViewAsChildMenu(tvSkins, fViewAsTVMenu);
			CreateViewAsChildMenu(desktopSkins, fViewAsDesktopMenu);

			viewAsMenu->AppendSubMenu(fViewAsAndroidMenu, _T("&Android"));
			viewAsMenu->AppendSubMenu(fViewAsIOSMenu, _T("&iOS"));
			viewAsMenu->AppendSubMenu(fViewAsTVMenu, _T("&TV"));
			viewAsMenu->AppendSubMenu(fViewAsDesktopMenu, _T("&Desktop"));
			fViewMenu->AppendSubMenu(viewAsMenu, _T("&View As"));
			fViewMenu->AppendSeparator();
			fViewMenu->Append(ID_MENU_OPEN_WELCOME_SCREEN, _T("&Welcome Screen"));
		}
	}

	void SolarSimulator::OnZoomIn(wxCommandEvent& event)
	{
		wxDisplay display(wxDisplay::GetFromWindow(this));
		wxRect screen = display.GetClientArea();
		bool doResize = false;

		int proposedWidth = GetContext()->GetWidth() * LinuxSimulatorView::skinScaleFactor;
		int proposedHeight = GetContext()->GetHeight() * LinuxSimulatorView::skinScaleFactor;

		fZoomOut->Enable(true);

		if (IsHomeScreen(GetContext()->GetAppName()))
		{
			doResize = (proposedWidth < screen.width&& proposedHeight < screen.height);
		}
		else
		{
			if (currentSkinWidth >= proposedWidth && currentSkinHeight >= proposedHeight)
			{
				doResize = (proposedWidth < screen.width&& proposedHeight < screen.height);
			}
		}

		if (doResize)
		{
			GetContext()->SetWidth(proposedWidth);
			GetContext()->SetHeight(proposedHeight);
			ChangeSize(proposedWidth, proposedHeight);
			GetContext()->RestartRenderer();
			GetCanvas()->Refresh(true);

			if (!IsHomeScreen(GetContext()->GetAppName()))
			{
				LinuxSimulatorView::Config::zoomedWidth = proposedWidth;
				LinuxSimulatorView::Config::zoomedHeight = proposedHeight;
				LinuxSimulatorView::Config::Save();

				if (proposedWidth * LinuxSimulatorView::skinScaleFactor > screen.width || proposedHeight * LinuxSimulatorView::skinScaleFactor > screen.height)
				{
					fZoomIn->Enable(false);
				}
			}
			else
			{
				//			LinuxSimulatorView::Config::welcomeScreenZoomedWidth = proposedWidth;
				//			LinuxSimulatorView::Config::welcomeScreenZoomedHeight = proposedHeight;
				LinuxSimulatorView::Config::Save();
			}
		}
	}

	void SolarSimulator::OnZoomOut(wxCommandEvent& event)
	{
		SolarApp* frame = solarApp;
		int proposedWidth = frame->GetContext()->GetWidth() / LinuxSimulatorView::skinScaleFactor;
		int proposedHeight = frame->GetContext()->GetHeight() / LinuxSimulatorView::skinScaleFactor;

		fZoomIn->Enable(true);

		if (proposedWidth >= LinuxSimulatorView::skinMinWidth)
		{
			frame->GetContext()->SetWidth(proposedWidth);
			frame->GetContext()->SetHeight(proposedHeight);
			frame->ChangeSize(proposedWidth, proposedHeight);
			frame->GetContext()->RestartRenderer();
			GetCanvas()->Refresh(true);

			LinuxSimulatorView::Config::zoomedWidth = proposedWidth;
			LinuxSimulatorView::Config::zoomedHeight = proposedHeight;
			LinuxSimulatorView::Config::Save();

			if (proposedWidth / LinuxSimulatorView::skinScaleFactor <= LinuxSimulatorView::skinMinWidth)
			{
				fZoomOut->Enable(false);
			}
		}
	}

	void SolarSimulator::OnViewAsChanged(wxCommandEvent& event)
	{
		int skinID = event.GetId();
		LinuxSimulatorView::SkinProperties sProperties = LinuxSimulatorView::GetSkinProperties(skinID);
		SolarSimulator* thiz = (SolarSimulator*)solarApp;
		wxDisplay display(wxDisplay::GetFromWindow(thiz));
		wxRect screen = display.GetClientArea();
		thiz->currentSkinWidth = sProperties.screenWidth;
		thiz->currentSkinHeight = sProperties.screenHeight;
		int initialWidth = sProperties.screenWidth;
		int initialHeight = sProperties.screenHeight;
		wxString newWindowTitle(thiz->GetContext()->GetTitle());
		newWindowTitle.append(" - ").append(sProperties.skinTitle.ToStdString());
		bool canZoom = sProperties.screenWidth > LinuxSimulatorView::skinMinWidth;

		if (sProperties.selected)
		{
			return;
		}

		thiz->fZoomIn->Enable(canZoom);
		thiz->fZoomOut->Enable(canZoom);

		LinuxSimulatorView::Config::skinID = sProperties.id;
		LinuxSimulatorView::Config::skinWidth = sProperties.screenWidth;
		LinuxSimulatorView::Config::skinHeight = sProperties.screenHeight;
		LinuxSimulatorView::SelectSkin(skinID);
		thiz->ClearMenuCheckboxes(thiz->fViewAsAndroidMenu, sProperties.skinTitle);
		thiz->ClearMenuCheckboxes(thiz->fViewAsIOSMenu, sProperties.skinTitle);
		thiz->ClearMenuCheckboxes(thiz->fViewAsTVMenu, sProperties.skinTitle);
		thiz->ClearMenuCheckboxes(thiz->fViewAsDesktopMenu, sProperties.skinTitle);

		while (initialWidth > screen.width || initialHeight > screen.height)
		{
			initialWidth /= LinuxSimulatorView::skinScaleFactor;
			initialHeight /= LinuxSimulatorView::skinScaleFactor;
		}

		LinuxSimulatorView::Config::zoomedWidth = initialWidth;
		LinuxSimulatorView::Config::zoomedHeight = initialHeight;
		LinuxSimulatorView::Config::Save();

		thiz->GetContext()->SetWidth(initialWidth);
		thiz->GetContext()->SetHeight(initialHeight);
		thiz->ChangeSize(initialWidth, initialHeight);

		wxCommandEvent ev(eventRelaunchProject);
		wxPostEvent(solarApp, ev);
	}

	void SolarSimulator::CreateViewAsChildMenu(vector<string>skin, wxMenu* targetMenu)
	{
		for (int i = 0; i < skin.size(); i++)
		{
			LinuxSimulatorView::SkinProperties sProperties = LinuxSimulatorView::GetSkinProperties(skin[i].c_str());
			wxMenuItem* currentSkin = targetMenu->Append(sProperties.id, skin[i].c_str(), wxEmptyString, wxITEM_CHECK);
			Bind(wxEVT_MENU, &SolarSimulator::OnViewAsChanged, sProperties.id);

			if (sProperties.id == LinuxSimulatorView::Config::skinID)
			{
				wxString newWindowTitle(GetContext()->GetTitle());
				newWindowTitle.append(" - ").append(sProperties.skinTitle.ToStdString());

				LinuxSimulatorView::SelectSkin(sProperties.id);
				currentSkin->Check(true);
				SetTitle(newWindowTitle);
			}
		}
	}

	void SolarSimulator::GetSavedZoom(int& width, int& height)
	{
		if (!IsHomeScreen(fContext->GetAppName()))
		{
			wxDisplay display(wxDisplay::GetFromWindow(solarApp));
			wxRect screen = display.GetClientArea();
			width = LinuxSimulatorView::Config::zoomedWidth;
			height = LinuxSimulatorView::Config::zoomedHeight;

			if (width > screen.width || height > screen.height)
			{
				fZoomIn->Enable(false);
			}

			if (LinuxSimulatorView::Config::skinWidth <= LinuxSimulatorView::skinMinWidth)
			{
				fZoomIn->Enable(false);
				fZoomOut->Enable(false);
			}

			while (width > screen.width || height > screen.height)
			{
				width /= LinuxSimulatorView::skinScaleFactor;
				height /= LinuxSimulatorView::skinScaleFactor;
			}
		}
	}

	void SolarSimulator::OnSuspendOrResume(wxCommandEvent& event)
	{
		if (fContext->GetRuntime()->IsSuspended())
		{
			RemoveSuspendedPanel();
			fHardwareMenu->SetLabel(ID_MENU_SUSPEND, "&Suspend	\tCtrl-Down");
			fContext->Resume();
		}
		else
		{
			CreateSuspendedPanel();
			fHardwareMenu->SetLabel(ID_MENU_SUSPEND, "&Resume	\tCtrl-Down");
			fContext->Pause();
		}
	}

	void SolarSimulator::OnOpenWelcome(wxCommandEvent& event)
	{
		string path(GetStartupPath(NULL));
		path.append("/Resources/homescreen/main.lua");

		wxCommandEvent eventOpen(eventOpenProject);
		eventOpen.SetString(path.c_str());
		wxPostEvent(this, eventOpen);
	}

	void SolarSimulator::OnOpen(wxCommandEvent& event)
	{
		RemoveSuspendedPanel();

		wxString path = event.GetString();
		string fullPath = (const char*)path.c_str();
		path = path.SubString(0, path.size() - 10); // without main.lua

		delete fContext;
		fContext = new SolarAppContext(path.c_str());
		fContext->LoadApp(fSolarGLCanvas);
		ResetWindowSize();

		string appName = fContext->GetAppName();

		WatchFolder(fContext->GetAppPath(), appName.c_str());
		SetCursor(wxCURSOR_ARROW);

		if (!IsHomeScreen(appName))
		{
			fAppPath = fContext->GetAppPath(); // save for relaunch
			UpdateRecentDocs(appName, fullPath);
		}

		wxString newWindowTitle(appName);

		if (!IsHomeScreen(appName))
		{
			LinuxSimulatorView::Config::lastProjectDirectory = fAppPath;
			LinuxSimulatorView::Config::Save();
			LinuxSimulatorView::OnLinuxPluginGet(fContext->GetAppPath(), appName.c_str(), fContext->GetPlatform());
		}
		else
		{
			newWindowTitle = "Solar2D Simulator";
		}

		SetMenu(path.c_str());

		// restore home screen zoom level
	//	if (IsHomeScreen(appName))
	//	{
	//		fContext->GetRuntimeDelegate()->fContentWidth = LinuxSimulatorView::Config::welcomeScreenZoomedWidth;
	//		fContext->GetRuntimeDelegate()->fContentHeight = LinuxSimulatorView::Config::welcomeScreenZoomedHeight;
	//		ChangeSize(fContext->GetRuntimeDelegate()->fContentWidth, fContext->GetRuntimeDelegate()->fContentHeight);
	//	}

		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			if (!IsHomeScreen(appName))
			{
				LinuxSimulatorView::SkinProperties sProperties = LinuxSimulatorView::GetSkinProperties(LinuxSimulatorView::Config::skinID);
				newWindowTitle.append(" - ").append(sProperties.skinTitle.ToStdString());
				fContext->GetPlatform()->SetStatusBarMode(fContext->GetPlatform()->GetStatusBarMode());
				string sandboxPath("~/.Solar2D/Sandbox/");
				sandboxPath.append(fContext->GetTitle());
				sandboxPath.append("_");
				sandboxPath.append(CalculateMD5(fContext->GetTitle().c_str()));

				Rtt_Log("Loading project from: %s\n", fContext->GetAppPath());
				Rtt_Log("Project sandbox folder: %s\n", sandboxPath.c_str());
			}
		}

		SetTitle(newWindowTitle);
	}

}
