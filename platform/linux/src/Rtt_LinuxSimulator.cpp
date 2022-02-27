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
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_LinuxMenuEvents.h"
#include "Rtt_LinuxDialogBuild.h"
#include "Rtt_FileSystem.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LinuxConsoleApp.h"
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netinet/ip.h> 

#if defined(Rtt_SIMULATOR)
#include "resource/simulator.xpm"
#endif

using namespace std;

// global
Rtt::SolarSimulator* solarSimulator = NULL;

void LinuxLog(const char* buf, int len)
{
	if (app)
		app->Log(buf, len);
}

namespace Rtt
{
	//
	// SolarSimulator
	//

	static float skinScaleFactor = 1.5f;

	SolarSimulator::SolarSimulator(const string& resourceDir)
		: SolarApp(resourceDir)
		, fRelaunchedViaFileEvent(false)
		, currentSkinWidth(0)
		, currentSkinHeight(0)
		, currentSkinID(0)
	{
		app = this;
		solarSimulator = this;		// save

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
		int x, y;
		SDL_GetWindowPosition(fWindow, &x, &y);
		fConfig["windowXPos"] = x;
		fConfig["windowYPos"] = y;
		fConfig.Save();
	}

	bool SolarSimulator::LoadApp()
	{
#ifdef Rtt_SIMULATOR
		//		SetIcon(simulator_xpm);
#endif

		const string& lastProjectDirectory = fConfig["lastProjectDirectory"].to_string();
		bool openlastProject = fConfig["openLastProject"].to_bool();
		fContext = new SolarAppContext(fWindow, openlastProject && !lastProjectDirectory.empty() ? lastProjectDirectory : fProjectPath);
		CreateMenu();

		SDL_SetWindowPosition(fWindow, fConfig["windowXPos"].to_int(), fConfig["windowYPos"].to_int());
		if (fContext->LoadApp())
		{
			GetPlatform()->fShowRuntimeErrors = fConfig["showRuntimeErrors"].to_bool();
			Window::SetStyle();

			return fSkins.Load(GetContext()->GetRuntime()->VMContext().L());
		}
		return false;
	}

	void SolarSimulator::SolarEvent(const SDL_Event& e)
	{
		//Rtt_Log("SolarEvent %d\n", e.type);
		switch (e.type)
		{
		case sdl::OnNewProject:
		{
			fDlg = new DlgNewProject("New Project", 640, 350);
			break;
		}

		case sdl::OnOpenProject:
		{
			if (e.user.data1)
			{
				string path = (const char*)e.user.data1;
				free(e.user.data1);
				OnOpen(path);
			}
			else
			{
				// open file dialog
				fDlg = new DlgOpen("Open Project", 640, 480, fConfig["lastProjectDirectory"].to_string());
			}
			break;
		}

		case sdl::OnRelaunch:
			OnRelaunch();
			break;

		case sdl::OnOpenInEditor:
		{
			string path = GetContext()->GetAppPath();
			path.append("/main.lua");
			OpenURL(path);
			break;
		}

		case sdl::OnCloseProject:
		{
			string path(GetStartupPath(NULL));
			path.append("/Resources/homescreen/main.lua");
			OnOpen(path);
			break;
		}

		case sdl::OnOpenDocumentation:
			OpenURL("https://docs.coronalabs.com/api/index.html");
			break;

		case sdl::OnOpenSampleProjects:
		{
			string samplesPath = GetStartupPath(NULL);
			samplesPath.append("/Resources/SampleCode");
			if (!Rtt_IsDirectory(samplesPath.c_str()))
			{
				Rtt_LogException("%s\n not found", samplesPath.c_str());
				break;
			}

			// open file dialog
			fDlg = new DlgOpen("Select Sample Project", 480, 320, samplesPath);
			break;
		}
		case sdl::OnAbout:
			fDlg = new DlgAbout("About Solar2D Simulator", 480, 240);
			break;

		case sdl::OnFileBrowserSelected:
		{
			string path = (const char*)e.user.data1;
			free(e.user.data1);
			OnOpen(path);

			fDlg = NULL;
			break;
		}

		case sdl::OnShowProjectFiles:
			OpenURL(GetContext()->GetAppPath());
			break;

		case sdl::OnShowProjectSandbox:
		{
			const string& appName = GetContext()->GetAppName();
			string path = GetHomePath();
			path.append("/.Solar2D/Sandbox/");
			path.append(appName);
			OpenURL(path);
			break;
		}

		case sdl::OnClearProjectSandbox:
			break;

		case sdl::OnRelaunchLastProject:
		{
			const string& lastProjectDirectory = fConfig["lastProjectDirectory"].to_string();
			if (lastProjectDirectory.size() > 0)
			{
				string path(lastProjectDirectory);
				path.append("/main.lua");
				OnOpen(path);
			}
			break;
		}

		case sdl::OnOpenPreferences:
			fDlg = new DlgPreferences("Solar2D Simulator Preferences", 400, 350);
			break;

		case sdl::onCloseDialog:
			fDlg = NULL;
			break;

		case sdl::OnFileSystemEvent:
		{
			string path = (const char*)e.user.data1;
			free(e.user.data1);

			if (path.size() >= 5 && path.substr(0, 2) != ".#" && path.rfind(".lua") != string::npos)
			{
				//Rtt_Log("OnFileSystemEvent 0x%X: %s\n", e.user.code, path.c_str());
				const string& s = fConfig["relaunchOnFileChange"].to_string();
				if (s == "Always")
				{
					PushEvent(sdl::OnRelaunchLastProject);
				}
				else if (s == "Ask")
				{
					fDlg = new DlgAskRelaunch("Solar2D", 320, 240);
				}
			}
			break;
		}

		case sdl::OnBuildLinux:
			fDlg = new DlgLinuxBuild("Linux Build Setup", 640, 260);
			break;

		case sdl::OnBuildAndroid:
			fDlg = new DlgAndroidBuild("Android Build Setup", 650, 350);
			break;

		case sdl::OnBuildHTML5:
			fDlg = new DlgHTML5Build("HTML5 Build Setup", 640, 260);
			break;

		case sdl::OnStyleColorsLight:
			ImGui::StyleColorsLight();
			break;
		case sdl::OnStyleColorsClassic:
			ImGui::StyleColorsClassic();
			break;
		case sdl::OnStyleColorsDark:
			ImGui::StyleColorsDark();
			break;

		case sdl::OnSetFocusConsole:
			if (fConsole)
			{
				SDL_RaiseWindow(fConsole->GetWindow());
			}
			break;

		case sdl::OnViewAs:
			fDlg = new DlgViewAs("View As", 500, 450, &fSkins);
			break;

		case sdl::OnChangeView:
		{
			const SkinProperties* skin = (const SkinProperties*)e.user.data1;
			OnViewAsChanged(skin);
			break;
		}

		case sdl::OnZoomIn:
			OnZoomIn();
			break;

		case sdl::OnZoomOut:
			OnZoomOut();
			break;

		default:
			break;
		}
	}

	void SolarSimulator::WatchFolder(const char* path, const char* appName)
	{
		fWatcher = NULL;
		if (!IsHomeScreen(appName))
		{
			fWatcher = new FileWatcher();
			fWatcher->Start(path);
		}
	}

	void SolarSimulator::OnRelaunch()
	{
		if (fAppPath.size() > 0 && !IsHomeScreen(fContext->GetAppName()))
		{
			bool doRelaunch = !fRelaunchedViaFileEvent;
			if (fRelaunchedViaFileEvent)
			{
				const string& relaunchOnFileChange = fConfig["relaunchOnFileChange"].to_string();
				if (relaunchOnFileChange == "Always" || relaunchOnFileChange == "Ask")
				{
					doRelaunch = true;
				}
				fRelaunchedViaFileEvent = false;
			}

			if (!doRelaunch)
			{
				return;
			}

			fContext->GetRuntime()->End();
			fContext = new SolarAppContext(fWindow, fAppPath.c_str());
			fContext->LoadApp();
			CreateMenu();

			WatchFolder(fContext->GetAppPath(), fContext->GetAppName());

			string newWindowTitle(fContext->GetTitle());

			//vv			LinuxSimulatorView::SkinProperties sProperties = LinuxSimulatorView::GetSkinProperties(ConfigInt("skinID"));
				//		newWindowTitle.append(" - ").append(sProperties.skinTitle);
			LinuxSimulatorView::OnLinuxPluginGet(fContext->GetAppPath(), fContext->GetAppName(), fContext->GetPlatform());

			SetTitle(newWindowTitle);
		}
	}

	void SolarSimulator::OnZoomIn()
	{
		SDL_DisplayMode screen;
		if (SDL_GetCurrentDisplayMode(0, &screen) == 0)
		{
			int proposedWidth = GetContext()->GetWidth() * skinScaleFactor;
			int proposedHeight = GetContext()->GetHeight() * skinScaleFactor;
			if (proposedWidth <= screen.w && proposedHeight <= screen.h)
			{
				GetContext()->SetWidth(proposedWidth);
				GetContext()->SetHeight(proposedHeight);
				SetWindowSize(proposedWidth, proposedHeight);
				GetContext()->RestartRenderer();
				fConfig.Save();
			}
		}
	}

	void SolarSimulator::OnZoomOut()
	{
		SDL_DisplayMode screen;
		if (SDL_GetCurrentDisplayMode(0, &screen) == 0)
		{
			int proposedWidth = GetContext()->GetWidth() / skinScaleFactor;
			int proposedHeight = GetContext()->GetHeight() / skinScaleFactor;
			if (proposedWidth >= 320) //skinMinWidth)
			{
				GetContext()->SetWidth(proposedWidth);
				GetContext()->SetHeight(proposedHeight);
				SetWindowSize(proposedWidth, proposedHeight);
				GetContext()->RestartRenderer();
				fConfig.Save();
			}
		}
	}

	void SolarSimulator::OnViewAsChanged(const SkinProperties* skin)
	{
		SDL_DisplayMode screen;
		if (SDL_GetCurrentDisplayMode(0, &screen) == 0)
		{
			string newWindowTitle(GetContext()->GetTitle());
			newWindowTitle.append(" - ").append(skin->skinTitle);

			int w = skin->screenWidth;
			int h = skin->screenHeight;
			while (w > screen.w || h > screen.h)
			{
				w /= skinScaleFactor;
				h /= skinScaleFactor;
			}

			GetContext()->SetWidth(w);
			GetContext()->SetHeight(h);
			SetWindowSize(w, h);
			GetContext()->RestartRenderer();
		}
	}

	void SolarSimulator::OnOpen(const string& ppath)
	{
		// sanity check
		if (ppath.size() < 10)
			return;

		fConfig.Save();

		string fullPath = ppath;
		string path = ppath.substr(0, ppath.size() - 9); // without main.lua

		fContext = new SolarAppContext(fWindow, path.c_str());
		if (fContext->LoadApp() == false)
		{
			Rtt_LogException("Failed to load app from %s\n", ppath.c_str());
			return;
		}

		string appName = fContext->GetAppName();
		CreateMenu();

		WatchFolder(fContext->GetAppPath(), appName.c_str());

		if (!IsHomeScreen(appName))
		{
			fAppPath = fContext->GetAppPath(); // save for relaunch
			UpdateRecentDocs(appName, fullPath);
		}

		string newWindowTitle(appName);

		if (!IsHomeScreen(appName))
		{
			fConfig["lastProjectDirectory"] = fAppPath;
			fConfig.Save();

			LinuxSimulatorView::OnLinuxPluginGet(fContext->GetAppPath(), appName.c_str(), fContext->GetPlatform());
		}
		else
		{
			newWindowTitle = "Solar2D Simulator";
		}

		// restore home screen zoom level
	//	if (IsHomeScreen(appName))
	//	{
	//		fContext->GetRuntimeDelegate()->fContentWidth = fSimulatorConfig->welcomeScreenZoomedWidth;
	//		fContext->GetRuntimeDelegate()->fContentHeight = fSimulatorConfig->welcomeScreenZoomedHeight;
	//		ChangeSize(fContext->GetRuntimeDelegate()->fContentWidth, fContext->GetRuntimeDelegate()->fContentHeight);
	//	}

		if (!IsHomeScreen(appName))
		{
			//vv LinuxSimulatorView::SkinProperties sProperties = LinuxSimulatorView::GetSkinProperties(ConfigInt("skinID"));
			//vv newWindowTitle += " - " + sProperties.skinTitle;
			fContext->GetPlatform()->SetStatusBarMode(fContext->GetPlatform()->GetStatusBarMode());
			string sandboxPath("~/.Solar2D/Sandbox/");
			sandboxPath.append(fContext->GetTitle());
			sandboxPath.append("_");
			sandboxPath.append(CalculateMD5(fContext->GetTitle()));

			Rtt_Log("Loading project from: %s\n", fContext->GetAppPath());
			Rtt_Log("Project sandbox folder: %s\n", sandboxPath.c_str());
		}

		SetTitle(newWindowTitle);
	}

	void SolarSimulator::StartConsole()
	{
		fConsole = new ConsoleWindow("Solar2D Simulator Console", 640, 480, &fLogData);
	}

	void SolarSimulator::CreateMenu()
	{
		fMenu = new DlgMenu(fContext->GetAppName());
	}


}
