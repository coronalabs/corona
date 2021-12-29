//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#define _chdir chdir
#include <string.h>
#include <fstream>
#include "Core/Rtt_Build.h"
#include "Core/Rtt_Time.h"
#include "Rtt_Runtime.h"
#include "Rtt_LuaContext.h"
#include "Core/Rtt_Types.h"
#include "Rtt_LinuxApp.h"
#include "Rtt_LinuxPlatform.h"
#include "Rtt_LinuxRuntimeDelegate.h"
#include "Rtt_LuaFile.h"
#include "Core/Rtt_FileSystem.h"
#include "Rtt_Archive.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Rtt_Freetype.h"
#include "Rtt_LuaLibSimulator.h"
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_LinuxUtils.h"
#include "Rtt_LinuxUtils.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_LinuxMenuEvents.h"
#include "Rtt_ConsoleApp.h"
#include "Rtt_HTTPClient.h"
#include "wx/menu.h"
#include "wx/dcclient.h"
#include "wx/app.h"
#include "wx/display.h"
#include <curl/curl.h>
#include <utility>		// for pairs
#include "lua.h"
#include "lauxlib.h"

#if !defined(wxHAS_IMAGES_IN_RESOURCES) && defined(Rtt_SIMULATOR)
#include "resource/simulator.xpm"
#endif

#define TIMER_ID wxID_HIGHEST + 1

using namespace Rtt;
using namespace std;

wxDEFINE_EVENT(eventOpenProject, wxCommandEvent);
wxDEFINE_EVENT(eventRelaunchProject, wxCommandEvent);
wxDEFINE_EVENT(eventWelcomeProject, wxCommandEvent);

static bool IsHomeScreen(string appName)
{
	return appName.compare(HOMESCREEN_ID) == 0;
}

// for redirecting output to Solar2DConsole
extern "C"
{
	static int print2console(lua_State* L)
	{
		return SolarAppContext::Print(L);
	}
}

// setup frame events
wxBEGIN_EVENT_TABLE(SolarApp, wxFrame)
EVT_MENU(ID_MENU_OPEN_WELCOME_SCREEN, SolarApp::OnOpenWelcome)
EVT_MENU(ID_MENU_RELAUNCH_PROJECT, SolarApp::OnRelaunch)
EVT_MENU(ID_MENU_SUSPEND, SolarApp::OnSuspendOrResume)
EVT_MENU(ID_MENU_CLOSE_PROJECT, SolarApp::OnOpenWelcome)
EVT_MENU(ID_MENU_ZOOM_IN, SolarApp::OnZoomIn)
EVT_MENU(ID_MENU_ZOOM_OUT, SolarApp::OnZoomOut)
EVT_COMMAND(wxID_ANY, eventOpenProject, SolarApp::OnOpen)
EVT_COMMAND(wxID_ANY, eventRelaunchProject, SolarApp::OnRelaunch)
EVT_COMMAND(wxID_ANY, eventWelcomeProject, SolarApp::OnOpenWelcome)
EVT_ICONIZE(SolarApp::OnIconized)
EVT_CLOSE(SolarApp::OnClose)
wxEND_EVENT_TABLE()

SolarApp::SolarApp()
	: fSolarGLCanvas(NULL)
	, fContext(NULL)
	, fMenuMain(NULL)
	, fMenuProject(NULL)
	, fWatcher(NULL)
	,	fProjectPath("")
{
#ifdef Rtt_SIMULATOR
	SetIcon(simulator_xpm);
#endif

	// init curl
	curl_global_init(CURL_GLOBAL_ALL);

	// init image loaders
	wxInitAllImageHandlers();


	const char* homeDir = GetHomePath();
	string basePath(homeDir);
	string sandboxPath(homeDir);
	string pluginPath(homeDir);
	string buildPath(homeDir);
	string projectCreationPath(homeDir);

	basePath.append("/.Solar2D");
	sandboxPath.append("/.Solar2D/Sandbox");
	pluginPath.append("/.Solar2D/Plugins");
	buildPath.append("/Documents/Solar2D Built Apps");
	projectCreationPath.append("/Documents/Solar2D Projects");

#ifdef __WXGTK3__
	setenv("GDK_BACKEND", "x11", 1);
#endif

	// create default directories if missing

	if (!Rtt_IsDirectory(basePath.c_str()))
	{
		Rtt_MakeDirectory(basePath.c_str());
	}

	if (!Rtt_IsDirectory(sandboxPath.c_str()))
	{
		Rtt_MakeDirectory(sandboxPath.c_str());
	}

	if (!Rtt_IsDirectory(pluginPath.c_str()))
	{
		Rtt_MakeDirectory(pluginPath.c_str());
	}

	if (!Rtt_IsDirectory(buildPath.c_str()))
	{
		Rtt_MakeDirectory(buildPath.c_str());
	}

	if (!Rtt_IsDirectory(projectCreationPath.c_str()))
	{
		Rtt_MakeDirectory(projectCreationPath.c_str());
	}

	// start the console immediately
	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
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

	// set window

	bool fullScreen = false;
	int windowStyle = wxCAPTION;
	int width = 320;
	int height = 480;
	int minWidth = width;
	int minHeight = height;
	string projectPath(GetStartupPath(NULL));

	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		projectPath.append("/Resources/homescreen");
	}
	else
	{
		projectPath.append("/Resources");
	}

	// homescreen settings
	ProjectSettings homeScreenProjectSettings;
	homeScreenProjectSettings.LoadFromDirectory(projectPath.c_str());

	// grab the required config settings (we only need width/height at this stage)
	if (homeScreenProjectSettings.HasConfigLua())
	{
		width = homeScreenProjectSettings.GetContentWidth();
		height = homeScreenProjectSettings.GetContentHeight();
	}

	// grab the build settings (we only need width/height at this stage)
	if (homeScreenProjectSettings.HasBuildSettings())
	{
		const Rtt::NativeWindowMode* nativeWindowMode = homeScreenProjectSettings.GetDefaultWindowMode();
		bool isWindowMinimizeButtonEnabled = homeScreenProjectSettings.IsWindowMinimizeButtonEnabled();
		bool isWindowMaximizeButtonEnabled = homeScreenProjectSettings.IsWindowMaximizeButtonEnabled();
		bool isWindowCloseButtonEnabled = homeScreenProjectSettings.IsWindowCloseButtonEnabled();
		bool isWindowResizable = homeScreenProjectSettings.IsWindowResizable();
		width = homeScreenProjectSettings.GetDefaultWindowViewWidth();
		height = homeScreenProjectSettings.GetDefaultWindowViewHeight();
		minWidth = homeScreenProjectSettings.GetMinWindowViewWidth();
		minHeight = homeScreenProjectSettings.GetMinWindowViewHeight();

		if (*nativeWindowMode == Rtt::NativeWindowMode::kNormal)
		{
		}
		else if (*nativeWindowMode == Rtt::NativeWindowMode::kMinimized)
		{
		}
		else if (*nativeWindowMode == Rtt::NativeWindowMode::kMaximized)
		{
		}
		else if (*nativeWindowMode == Rtt::NativeWindowMode::kFullscreen)
		{
			fullScreen = true;
		}

		if (isWindowMinimizeButtonEnabled)
		{
			windowStyle |= wxMINIMIZE_BOX;
		}

		if (isWindowMaximizeButtonEnabled)
		{
			windowStyle |= wxMAXIMIZE_BOX | wxRESIZE_BORDER;
		}

		if (isWindowCloseButtonEnabled)
		{
			windowStyle |= wxCLOSE_BOX;
		}

		if (isWindowResizable)
		{
			windowStyle |= wxRESIZE_BORDER;
		}

		if (fullScreen)
		{
			windowStyle = wxDEFAULT_FRAME_STYLE;
		}
	}

	// sanity checks
	if (width <= 0)
	{
		width = 320;
		minWidth = width;
	}

	if (height <= 0)
	{
		height = 480;
		minHeight = height;
	}

	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		// read from the simulator config file (it'll be created if it doesn't exist)
		LinuxSimulatorView::Config::Load();
	}

	suspendedPanel = NULL;
	fRelaunchedViaFileEvent = false;

	// create app window
	Create(NULL, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(width, height), windowStyle);
	CreateMenus();

	int vAttrs[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
	fSolarGLCanvas = new SolarGLCanvas(this, vAttrs);
	fRelaunchProjectDialog = new LinuxRelaunchProjectDialog(NULL, wxID_ANY, wxEmptyString);
	fProjectPath = string(homeDir);
	fProjectPath.append("/Documents/Solar2D Projects");

	if (!Rtt_IsDirectory(fProjectPath.c_str()))
	{
		Rtt_MakeDirectory(fProjectPath.c_str());
	}

	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		currentSkinWidth = LinuxSimulatorView::Config::skinWidth;
		currentSkinHeight = LinuxSimulatorView::Config::skinHeight;
	}

	if (fullScreen)
	{
		wxDisplay display(wxDisplay::GetFromWindow(this));
		wxRect screen = display.GetClientArea();
		width = screen.width;
		height = screen.height;
	}

	SetClientSize(wxSize(width, height));
	SetSize(wxSize(width, height));
	SetMinClientSize(wxSize(minWidth, minHeight));

	if (fullScreen)
	{
		ShowFullScreen(true);
	}
	else
	{
		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			SetPosition(wxPoint(LinuxSimulatorView::Config::windowXPos, LinuxSimulatorView::Config::windowYPos));
		}
		Show(true);
	}

	wxCommandEvent eventOpen(eventOpenProject);

	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		if (LinuxSimulatorView::Config::openLastProject && !LinuxSimulatorView::Config::lastProjectDirectory.IsEmpty())
		{
			wxString fullPath(LinuxSimulatorView::Config::lastProjectDirectory);
			fullPath.append("/main.lua");
			eventOpen.SetInt(ID_MENU_OPEN_LAST_PROJECT);
			eventOpen.SetString(fullPath);
		}
	}

	OnOpen(eventOpen);

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

}

SolarApp::~SolarApp()
{
	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		LinuxSimulatorView::Config::Cleanup();
	}

	delete fWatcher;
	delete fSolarGLCanvas;
	delete fContext;

	SetMenuBar(NULL);
	delete fMenuMain;
	delete fMenuProject;

	curl_global_cleanup();
}

void SolarApp::WatchFolder(const char* path, const char* appName)
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
		Connect(wxEVT_FSWATCHER, wxFileSystemWatcherEventHandler(SolarApp::OnFileSystemEvent));
	}

	wxFileName fn = wxFileName::DirName(path);
	fn.DontFollowLink();
	fWatcher->RemoveAll();
	fWatcher->AddTree(fn);
}

void SolarApp::ResetSize()
{
	wxSize clientSize = GetClientSize();

	if (IsFullScreen())
	{
		fContext->SetWidth(clientSize.GetWidth());
		fContext->SetHeight(clientSize.GetHeight());
	}

	ChangeSize(fContext->GetWidth(), fContext->GetHeight());
	GetCanvas()->Refresh(true);
}

void SolarApp::CreateMenus()
{
	if (LinuxSimulatorView::IsRunningOnSimulator())
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
}

void SolarApp::CreateViewAsChildMenu(vector<string>skin, wxMenu* targetMenu)
{
	for (int i = 0; i < skin.size(); i++)
	{
		LinuxSimulatorView::SkinProperties sProperties = LinuxSimulatorView::GetSkinProperties(skin[i].c_str());
		wxMenuItem* currentSkin = targetMenu->Append(sProperties.id, skin[i].c_str(), wxEmptyString, wxITEM_CHECK);
		Bind(wxEVT_MENU, &SolarApp::OnViewAsChanged, sProperties.id);

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

void SolarApp::ClearMenuCheckboxes(wxMenu* menu, wxString currentSkinTitle)
{
	for (int i = 0; i < menu->GetMenuItemCount(); i++)
	{
		wxMenuItem* currentItem = menu->FindItemByPosition(i);

		if (!currentItem->GetItemLabel().IsSameAs(currentSkinTitle))
		{
			currentItem->Check(false);
		}
	}
}

void SolarApp::OnViewAsChanged(wxCommandEvent& event)
{
	int skinID = event.GetId();
	LinuxSimulatorView::SkinProperties sProperties = LinuxSimulatorView::GetSkinProperties(skinID);
	SolarApp* frame = solarApp;
	wxDisplay display(wxDisplay::GetFromWindow(frame));
	wxRect screen = display.GetClientArea();
	frame->currentSkinWidth = sProperties.screenWidth;
	frame->currentSkinHeight = sProperties.screenHeight;
	int initialWidth = sProperties.screenWidth;
	int initialHeight = sProperties.screenHeight;
	wxString newWindowTitle(frame->GetContext()->GetTitle());
	newWindowTitle.append(" - ").append(sProperties.skinTitle.ToStdString());
	bool canZoom = sProperties.screenWidth > LinuxSimulatorView::skinMinWidth;

	if (sProperties.selected)
	{
		return;
	}

	frame->fZoomIn->Enable(canZoom);
	frame->fZoomOut->Enable(canZoom);

	LinuxSimulatorView::Config::skinID = sProperties.id;
	LinuxSimulatorView::Config::skinWidth = sProperties.screenWidth;
	LinuxSimulatorView::Config::skinHeight = sProperties.screenHeight;
	LinuxSimulatorView::SelectSkin(skinID);
	frame->ClearMenuCheckboxes(solarApp->fViewAsAndroidMenu, sProperties.skinTitle);
	frame->ClearMenuCheckboxes(solarApp->fViewAsIOSMenu, sProperties.skinTitle);
	frame->ClearMenuCheckboxes(solarApp->fViewAsTVMenu, sProperties.skinTitle);
	frame->ClearMenuCheckboxes(solarApp->fViewAsDesktopMenu, sProperties.skinTitle);

	while (initialWidth > screen.width || initialHeight > screen.height)
	{
		initialWidth /= LinuxSimulatorView::skinScaleFactor;
		initialHeight /= LinuxSimulatorView::skinScaleFactor;
	}

	LinuxSimulatorView::Config::zoomedWidth = initialWidth;
	LinuxSimulatorView::Config::zoomedHeight = initialHeight;
	LinuxSimulatorView::Config::Save();

	frame->GetContext()->SetWidth(initialWidth);
	frame->GetContext()->SetHeight(initialHeight);
	frame->ChangeSize(initialWidth, initialHeight);

	wxCommandEvent ev(eventRelaunchProject);
	wxPostEvent(solarApp, ev);
}

void SolarApp::SetMenu(const char* appPath)
{
	if (LinuxSimulatorView::IsRunningOnSimulator())
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
}

void SolarApp::OnIconized(wxIconizeEvent& event)
{
	fContext->RestartRenderer();
}

void SolarApp::OnClose(wxCloseEvent& event)
{
	fContext->GetRuntime()->End();

	// quit the simulator console
	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		LinuxSimulatorView::Config::windowXPos = GetPosition().x;
		LinuxSimulatorView::Config::windowYPos = GetPosition().y;
		LinuxSimulatorView::Config::Save();
		ConsoleApp::Quit();
	}
	wxExit();
}

void SolarApp::OnFileSystemEvent(wxFileSystemWatcherEvent& event)
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

void SolarApp::OnOpenWelcome(wxCommandEvent& event)
{
	string path(GetStartupPath(NULL));
	path.append("/Resources/homescreen/main.lua");

	wxCommandEvent eventOpen(eventOpenProject);
	eventOpen.SetString(path.c_str());
	wxPostEvent(this, eventOpen);
}

void SolarApp::OnRelaunch(wxCommandEvent& event)
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
		_chdir(fContext->GetAppPath());
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
		fSolarGLCanvas->fContext = fContext;
		fContext->SetCanvas(fSolarGLCanvas);
		SetMenu(fAppPath.c_str());
		SetTitle(newWindowTitle);

		fContext->RestartRenderer();
		fSolarGLCanvas->StartTimer(1000.0f / (float)fContext->GetFPS());
		fFileSystemEventTimestamp = wxGetUTCTimeMillis();
	}
}

void SolarApp::ChangeSize(int newWidth, int newHeight)
{
	SetMinClientSize(wxSize(newWidth, newHeight));
	SetClientSize(wxSize(newWidth, newHeight));
	SetSize(wxSize(newWidth, newHeight));
}

void SolarApp::CreateSuspendedPanel()
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

void SolarApp::RemoveSuspendedPanel()
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

void SolarApp::OnZoomIn(wxCommandEvent& event)
{
	SolarApp* frame = solarApp;
	wxDisplay display(wxDisplay::GetFromWindow(frame));
	wxRect screen = display.GetClientArea();
	bool doResize = false;

	int proposedWidth = frame->GetContext()->GetWidth() * LinuxSimulatorView::skinScaleFactor;
	int proposedHeight = frame->GetContext()->GetHeight() * LinuxSimulatorView::skinScaleFactor;

	fZoomOut->Enable(true);

	if (IsHomeScreen(GetContext()->GetAppName()))
	{
		doResize = (proposedWidth < screen.width&& proposedHeight < screen.height);
	}
	else
	{
		if (frame->currentSkinWidth >= proposedWidth && frame->currentSkinHeight >= proposedHeight)
		{
			doResize = (proposedWidth < screen.width&& proposedHeight < screen.height);
		}
	}

	if (doResize)
	{
		frame->GetContext()->SetWidth(proposedWidth);
		frame->GetContext()->SetHeight(proposedHeight);
		frame->ChangeSize(proposedWidth, proposedHeight);
		frame->GetContext()->RestartRenderer();
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

void SolarApp::OnZoomOut(wxCommandEvent& event)
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

void SolarApp::OnSuspendOrResume(wxCommandEvent& event)
{
	if (LinuxSimulatorView::IsRunningOnSimulator())
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
}

void SolarApp::OnOpen(wxCommandEvent& event)
{
	wxString path = event.GetString();
	string fullPath = (const char*)path.c_str();
	path = path.SubString(0, path.size() - 10); // without main.lua

	delete fContext;
	fContext = new SolarAppContext(path.c_str());
	_chdir(fContext->GetAppPath());

	// clear the simulator log
	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		ConsoleApp::Clear();
	}

	string appName = fContext->GetAppName();
	RemoveSuspendedPanel();

	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		WatchFolder(fContext->GetAppPath(), appName.c_str());
		SetCursor(wxCURSOR_ARROW);
	}

	if (!IsHomeScreen(appName))
	{
		fAppPath = fContext->GetAppPath(); // save for relaunch
		UpdateRecentDocs(appName, fullPath);
	}

	bool fullScreen = fContext->Init();
	wxString newWindowTitle(appName);

	if (!IsHomeScreen(appName))
	{
		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			LinuxSimulatorView::Config::lastProjectDirectory = fAppPath;
			LinuxSimulatorView::Config::Save();
			LinuxSimulatorView::OnLinuxPluginGet(fContext->GetAppPath(), appName.c_str(), fContext->GetPlatform());
		}
	}
	else
	{
		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			newWindowTitle = "Solar2D Simulator";
		}
	}

	fContext->LoadApp(fSolarGLCanvas);
	ResetSize();
	fSolarGLCanvas->fContext = fContext;
	fContext->SetCanvas(fSolarGLCanvas);
	SetMenu(path.c_str());

	// Vitaly: for what this ?
	// restore home screen zoom level
//	if (IsHomeScreen(appName))
//	{
//		fContext->GetRuntimeDelegate()->fContentWidth = LinuxSimulatorView::Config::welcomeScreenZoomedWidth;
//		fContext->GetRuntimeDelegate()->fContentHeight = LinuxSimulatorView::Config::welcomeScreenZoomedHeight;
//		ChangeSize(fContext->GetRuntimeDelegate()->fContentWidth, fContext->GetRuntimeDelegate()->fContentHeight);
//	}

	fContext->RestartRenderer();
	GetCanvas()->Refresh(true);
	fSolarGLCanvas->StartTimer(1000.0f / (float)fContext->GetFPS());

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

// setup glcanvas events
wxBEGIN_EVENT_TABLE(SolarGLCanvas, wxGLCanvas)
EVT_TIMER(TIMER_ID, SolarGLCanvas::OnTimer)
EVT_SIZE(SolarGLCanvas::OnSize)
wxEND_EVENT_TABLE()

SolarGLCanvas::SolarGLCanvas(SolarApp* parent, const int* vAttrs)
	: wxGLCanvas(parent, wxID_ANY, vAttrs, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
	, solarApp(parent)
	, fContext(NULL)
	, fTimer(this, TIMER_ID)
{
	fGLContext = new wxGLContext(this);
	Rtt_ASSERT(fGLContext->IsOK());

	SetSize(parent->GetSize());

	Bind(wxEVT_CHAR, &LinuxKeyListener::OnChar);
	Bind(wxEVT_KEY_DOWN, &LinuxKeyListener::OnKeyDown);
	Bind(wxEVT_KEY_UP, &LinuxKeyListener::OnKeyUp);
	Bind(wxEVT_LEFT_DCLICK, &LinuxMouseListener::OnMouseLeftDoubleClick);
	Bind(wxEVT_LEFT_DOWN, &LinuxMouseListener::OnMouseLeftDown);
	Bind(wxEVT_LEFT_UP, &LinuxMouseListener::OnMouseLeftUp);
	Bind(wxEVT_RIGHT_DCLICK, &LinuxMouseListener::OnMouseRightDoubleClick);
	Bind(wxEVT_RIGHT_DOWN, &LinuxMouseListener::OnMouseRightDown);
	Bind(wxEVT_RIGHT_UP, &LinuxMouseListener::OnMouseRightUp);
	Bind(wxEVT_MIDDLE_DCLICK, &LinuxMouseListener::OnMouseMiddleDoubleClick);
	Bind(wxEVT_MIDDLE_DOWN, &LinuxMouseListener::OnMouseMiddleDown);
	Bind(wxEVT_MIDDLE_UP, &LinuxMouseListener::OnMouseMiddleUp);
	Bind(wxEVT_MOTION, &LinuxMouseListener::OnMouseMove);
	Bind(wxEVT_MOUSEWHEEL, &LinuxMouseListener::OnMouseWheel);
}

SolarGLCanvas::~SolarGLCanvas()
{
	delete fGLContext;
}

void SolarGLCanvas::StartTimer(float frameDuration)
{
	fTimer.Start((int)frameDuration);
	fContext->GetRuntime()->BeginRunLoop();
}

void SolarGLCanvas::OnTimer(wxTimerEvent& event)
{
	Rtt::Runtime* runtime = fContext->GetRuntime();
	if (!runtime->IsSuspended())
	{
		LinuxInputDeviceManager& deviceManager = (LinuxInputDeviceManager&)fContext->GetPlatform()->GetDevice().GetInputDeviceManager();
		deviceManager.dispatchEvents(runtime);

		// advance engine
		(*runtime)();
	}
}

void SolarGLCanvas::Render()
{
	SetCurrent(*fGLContext);
	if (fWindowHeight > 0)
	{
		SwapBuffers();
	}
}

void SolarGLCanvas::OnSize(wxSizeEvent& event)
{
	// if the window is fully initialized
	if (IsShownOnScreen())
	{
		fWindowHeight = event.GetSize().y;
		Refresh(true);
		Render();
	}
}
