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

#if !defined(wxHAS_IMAGES_IN_RESOURCES) && defined(Rtt_SIMULATOR)
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

		fConfigFilePath = GetHomePath();
		fConfigFilePath.append("/.Solar2D/simulator.conf");
		ConfigLoad();
	}

	SolarSimulator::~SolarSimulator()
	{
		int x, y;
		SDL_GetWindowPosition(fWindow, &x, &y);
		ConfigSet("windowXPos", x);
		ConfigSet("windowYPos", y);

		ConfigSave();

		// quit the simulator console
		// ConsoleApp::Quit();
	}

	bool SolarSimulator::LoadApp()
	{
#ifdef Rtt_SIMULATOR
		//		SetIcon(simulator_xpm);
#endif

		const string& lastProjectDirectory = ConfigStr("lastProjectDirectory");
		fContext = new SolarAppContext(fWindow, ConfigInt("ShowWelcome") && !lastProjectDirectory.empty() ? lastProjectDirectory : fProjectPath);
		fMenu = new DlgMenu(fContext->GetAppName());

		SDL_SetWindowPosition(fWindow, ConfigInt("windowXPos"), ConfigInt("windowYPos"));
		if (fContext->LoadApp())
		{
			GetPlatform()->fShowRuntimeErrors = ConfigInt("showRuntimeErrors");

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
				string startPath(solarSimulator->ConfigStr("lastProjectDirectory"));
				fDlg = new DlgOpen("Open Project", 640, 480, startPath);
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
			const string& lastProjectDirectory = solarSimulator->ConfigStr("lastProjectDirectory");
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
				Rtt_Log("OnFileSystemEvent 0x%X: %s", e.user.code, path.c_str());
				const string& s = fConfig["relaunchOnFileChange"];
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
			fDlg = new DlgAndroidBuild("Android Build Setup", 640, 480);
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
			fDlg = new DlgViewAs("View As", 500, 500, &fSkins);
			break;

		case sdl::OnChangeView:
		{
			const SkinProperties* skin = (const SkinProperties*)e.user.data1;
			OnViewAsChanged(skin);
			break;
		}

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

			//LinuxRuntimeErrorDialog* errDialog = fContext->GetPlatform()->GetRuntimeErrorDialog();
			//			if ((errDialog && errDialog->IsShown()) || (fRelaunchProjectDialog && fRelaunchProjectDialog->IsShown()))
			//			{
			//				return;
			//			}

						// workaround for wxFileSystem events firing twice (known wx bug)
		//	if (fFileSystemEventTimestamp >= wxGetUTCTimeMillis() - 250)
		//	{
		//		return;
		//	}

			if (fRelaunchedViaFileEvent)
			{
				const string& relaunchOnFileChange = ConfigStr("relaunchOnFileChange");
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
			fMenu = new DlgMenu(fContext->GetAppName());

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
		/*		wxDisplay display(wxDisplay::GetFromWindow(this));
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
						ConfigSet("zoomedWidth", proposedWidth);
						ConfigSet("zoomedHeight", proposedHeight);
						if (proposedWidth * LinuxSimulatorView::skinScaleFactor > screen.width || proposedHeight * LinuxSimulatorView::skinScaleFactor > screen.height)
						{
							fZoomIn->Enable(false);
						}
					}
					else
					{
						//			fSimulatorConfig->welcomeScreenZoomedWidth = proposedWidth;
						//			fSimulatorConfig->welcomeScreenZoomedHeight = proposedHeight;
					}
					ConfigSave();
				}*/
	}

	void SolarSimulator::OnZoomOut()
	{
		/*		SolarApp* frame = solarApp;
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

					ConfigSet("zoomedWidth", proposedWidth);
					ConfigSet("zoomedHeight", proposedHeight);
					ConfigSave();

					if (proposedWidth / LinuxSimulatorView::skinScaleFactor <= LinuxSimulatorView::skinMinWidth)
					{
						fZoomOut->Enable(false);
					}
				}*/
	}

	void SolarSimulator::OnViewAsChanged(const SkinProperties* skin)
	{
		SDL_DisplayMode screen;
		if (SDL_GetCurrentDisplayMode(0, &screen) != 0)
		{
			return;
		}

		string newWindowTitle(GetContext()->GetTitle());
		newWindowTitle.append(" - ").append(skin->skinTitle);

		int w = skin->screenWidth;
		int h = skin->screenHeight;
		float skinScaleFactor = 1.5f;
		while (w > screen.w || h > screen.h)
		{
			w /= skinScaleFactor;
			h /= skinScaleFactor;
		}

		ConfigSave();

		GetContext()->SetWidth(w);
		GetContext()->SetHeight(h);
		ChangeSize(w, h);
	}

	void SolarSimulator::GetSavedZoom(int& width, int& height)
	{
		/*if (!IsHomeScreen(fContext->GetAppName()))
		{
			wxDisplay display(wxDisplay::GetFromWindow(solarApp));
			wxRect screen = display.GetClientArea();
			width = ConfigInt("zoomedWidth");
			height = ConfigInt("zoomedHeight");

			if (width > screen.width || height > screen.height)
			{
				fZoomIn->Enable(false);
			}

			if (ConfigInt("skinWidth") <= LinuxSimulatorView::skinMinWidth)
			{
				fZoomIn->Enable(false);
				fZoomOut->Enable(false);
			}

			while (width > screen.width || height > screen.height)
			{
				width /= LinuxSimulatorView::skinScaleFactor;
				height /= LinuxSimulatorView::skinScaleFactor;
			}
		}*/
	}

	void SolarSimulator::OnOpen(const string& ppath)
	{
		// sanity check
		if (ppath.size() < 10)
			return;

		ConfigSave();

		string fullPath = ppath;
		string path = ppath.substr(0, ppath.size() - 9); // without main.lua

		fContext = new SolarAppContext(fWindow, path.c_str());
		if (fContext->LoadApp() == false)
		{
			Rtt_LogException("Failed to load app from %s\n", ppath.c_str());
			return;
		}

		string appName = fContext->GetAppName();
		fMenu = new DlgMenu(fContext->GetAppName());

		WatchFolder(fContext->GetAppPath(), appName.c_str());

		if (!IsHomeScreen(appName))
		{
			fAppPath = fContext->GetAppPath(); // save for relaunch
			UpdateRecentDocs(appName, fullPath);
		}

		string newWindowTitle(appName);

		if (!IsHomeScreen(appName))
		{
			ConfigSet("lastProjectDirectory", fAppPath);
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

	// config parser
	void SolarSimulator::ConfigLoad()
	{
		// default values
		fConfig["showRuntimeErrors"] = to_string(true);
		fConfig["openLastProject"] = to_string(false);
		fConfig["relaunchOnFileChange"] = "Always";
		fConfig["windowXPos"] = to_string(10);
		fConfig["windowYPos"] = to_string(10);
		fConfig["skinID"] = to_string(6223);
		fConfig["skinWidth"] = to_string(320);
		fConfig["skinHeight"] = to_string(480);

		FILE* f = fopen(fConfigFilePath.c_str(), "r");
		if (f == NULL)
		{
			Rtt_LogException("Failed to read %s, %s\n", fConfigFilePath.c_str(), strerror(errno));
			return;
		}

		char s[1024];
		while (fgets(s, sizeof(s), f))
		{
			char* comment = strchr(s, '#');
			if (comment)
				*comment = 0;

			char key[sizeof(s)];
			char val[sizeof(s)];
			if (sscanf(s, "%64[^=]=%512[^\n]%*c", key, val) == 2) // Checking scanf read key=val pair
			{
				fConfig[key] = val;
			}
		}
		fclose(f);

		//for (const auto it : fConfig)
		//{
		//	Rtt_Log("%s=%s\n", it.first.c_str(), it.second.c_str());
		//}
	}

	void SolarSimulator::ConfigSave()
	{
		if (IsHomeScreen(GetAppName()))
		{
			FILE* f = fopen(fConfigFilePath.c_str(), "w");
			if (f == NULL)
			{
				Rtt_LogException("Failed to write %s, %s\n", fConfigFilePath.c_str(), strerror(errno));
				return;
			}

			for (const auto& it : fConfig)
			{
				fprintf(f, "%s=%s\n", it.first.c_str(), it.second.c_str());
			}
			fclose(f);
		}
	}

	string& SolarSimulator::ConfigStr(const string& key)
	{
		static string s_empty;
		const auto& it = fConfig.find(key);
		return it != fConfig.end() ? it->second : s_empty;
	}

	int SolarSimulator::ConfigInt(const string& key)
	{
		const auto& it = fConfig.find(key);
		return it != fConfig.end() ? atoi(it->second.c_str()) : 0;
	}

	void SolarSimulator::ConfigSet(const char* key, string& val)
	{
		fConfig[key] = val;
	}

	void SolarSimulator::ConfigSet(const char* key, int val)
	{
		fConfig[key] = ::to_string(val);
	}

}
