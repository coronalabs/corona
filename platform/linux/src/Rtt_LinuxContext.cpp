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
#include "Rtt_LinuxContext.h"
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
#include "Rtt_LinuxApp.h"
#include "Rtt_HTTPClient.h"
#include "wx/menu.h"
#include "wx/dcclient.h"
#include "wx/app.h"
#include "wx/display.h"
#include <curl/curl.h>
#include <utility>		// for pairs
#include "lua.h"
#include "lauxlib.h"

//#define Rtt_DEBUG_TOUCH 1
#define TIMER_ID wxID_HIGHEST + 1

using namespace Rtt;
using namespace std;

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

extern Rtt::SolarApp* solarApp;

namespace Rtt
{

	SolarAppContext::SolarAppContext(const char* path)
		: fRuntime(NULL)
		, fRuntimeDelegate(new LinuxRuntimeDelegate())
		, fMouseListener(NULL)
		, fKeyListener(NULL)
		, fPlatform(NULL)
		, fTouchDeviceExist(false)
		, fMode("normal")
		, fIsDebApp(false)
		, fSimulator(NULL)
		, fProjectSettings(new ProjectSettings())
	{
		string exeFileName;
		const char* homeDir = GetHomePath();
		const char* appPath = GetStartupPath(&exeFileName);

		// override appPath if arg isn't null
		if (path && *path != 0)
		{
			appPath = path;
		}

		// set app name
		if (strcmp(appPath, "/usr/bin") == 0) // deb ?
		{
			// for .deb app the appName is exe file name
			fAppName = exeFileName;
		}
		else
		{
			const char* slash = strrchr(appPath, '/');

			if (slash)
			{
				fAppName = slash + 1;
			}
			else
			{
				slash = strrchr(appPath, '\\');

				if (slash)
				{
					fAppName = slash + 1;
				}
			}
		}

		Rtt_ASSERT(fAppName.size() > 0);
		string startDir(appPath);

		fSaveFolder.append(homeDir);
		fSaveFolder.append("/Documents/Solar2D Built Apps");

		string assetsDir = startDir;
		assetsDir.append("/Resources/resource.car");

		if (Rtt_FileExists(assetsDir.c_str()))
		{
			fPathToApp = startDir;
			chdir(GetAppPath());
			return;
		}

		assetsDir = startDir;
		assetsDir.append("/main.lua");

		if (Rtt_FileExists(assetsDir.c_str()))
		{
			fPathToApp = startDir;
			chdir(GetAppPath());
			return;
		}

		// look for welcomescereen
		startDir = GetStartupPath(NULL);
		startDir.append("/Resources/homescreen");
		assetsDir = startDir;
		assetsDir.append("/main.lua");

		if (Rtt_FileExists(assetsDir.c_str()))
		{
			fAppName = HOMESCREEN_ID;
			fPathToApp = startDir;
			fIsDebApp = false;
			chdir(GetAppPath());
			return;
		}

		Rtt_LogException("Failed to find app\n");
		Rtt_ASSERT(0);
	}

	SolarAppContext::~SolarAppContext()
	{
		delete fMouseListener;
		delete fKeyListener;
		delete fRuntime;
		delete fRuntimeDelegate;
		delete fPlatform;
		delete fSimulator;
		delete fProjectSettings;

		setGlyphProvider(NULL);
	}

	void SolarAppContext::Init()
	{
		const char* homeDir = GetHomePath();
		string appDir(homeDir);

		if (solarApp->IsRunningOnSimulator())
		{
			appDir.append("/.Solar2D/Sandbox/");
		}
		else
		{
			appDir.append("/.local/share/");
		}

		if (!IsHomeScreen(fAppName))
		{
			appDir.append(fAppName);

			if (solarApp->IsRunningOnSimulator())
			{
				appDir.append("_");
				appDir.append(CalculateMD5(fAppName));
			}
		}
		else
		{
			if (solarApp->IsRunningOnSimulator())
			{
				appDir.append("Simulator");
			}
		}

		if (!Rtt_IsDirectory(appDir.c_str()))
		{
			Rtt_MakeDirectory(appDir.c_str());
		}

		string documentsDir(appDir);
		string temporaryDir(appDir);
		string cachesDir(appDir);
		string systemCachesDir(appDir);
		string skinDir(GetStartupPath(NULL));

		// setup directory paths
		documentsDir.append("/Documents");
		temporaryDir.append("/TemporaryFiles");
		cachesDir.append("/CachedFiles");
		systemCachesDir.append("/.system");
		skinDir.append("/Resources/Skins");

		if (!Rtt_IsDirectory(documentsDir.c_str()))
		{
			Rtt_MakeDirectory(documentsDir.c_str());
		}

		if (!Rtt_IsDirectory(systemCachesDir.c_str()))
		{
			Rtt_MakeDirectory(systemCachesDir.c_str());
		}

		if (!Rtt_IsDirectory(temporaryDir.c_str()))
		{
			Rtt_MakeDirectory(temporaryDir.c_str());
		}

		if (!Rtt_IsDirectory(cachesDir.c_str()))
		{
			Rtt_MakeDirectory(cachesDir.c_str());
		}

		setGlyphProvider(new glyph_freetype_provider(fPathToApp.c_str()));
		fPlatform = new LinuxPlatform(fPathToApp.c_str(), documentsDir.c_str(), temporaryDir.c_str(), cachesDir.c_str(), systemCachesDir.c_str(), skinDir.c_str(), GetStartupPath(NULL));
		fRuntime = new LinuxRuntime(*fPlatform, NULL);
		fRuntime->SetDelegate(fRuntimeDelegate);

		if (solarApp->IsRunningOnSimulator())
		{
			fRuntime->SetProperty(Runtime::kLinuxMaskSet | Runtime::kIsApplicationNotArchived, true);
		}
		else
		{
			fRuntime->SetProperty(Runtime::kLinuxMaskSet, true);
		}

		bool fullScreen = false;
		int width = 320;
		int height = 480;
		string projectPath(fPathToApp.c_str());

		if (!solarApp->IsRunningOnSimulator())
		{
			projectPath.append("/Resources");
		}

		fProjectSettings->ResetBuildSettings();
		fProjectSettings->ResetConfigLuaSettings();
		fProjectSettings->LoadFromDirectory(projectPath.c_str());

		// read config.lua
		if (fProjectSettings->HasConfigLua())
		{
			if (width <= 0 || height <= 0)
			{
				width = fProjectSettings->GetContentWidth();
				height = fProjectSettings->GetContentHeight();
			}
		}
		else
		{
			// no config.lua, set width & height to default values
			fRuntimeDelegate->SetWidth(width);
			fRuntimeDelegate->SetHeight(height);
		}

		// read build.settings
		if (fProjectSettings->HasBuildSettings())
		{
			int systemLanguage = wxLocale::GetSystemLanguage();

			// fallback to en_us if wx wasn't able to determine the system language
			if (systemLanguage == wxLANGUAGE_UNKNOWN)
			{
				systemLanguage = wxLANGUAGE_ENGLISH_US;
			}

			wxString localeName = wxLocale::GetLanguageInfo(systemLanguage)->CanonicalName.Lower();
			string langCode = localeName.ToStdString().substr(0, 2);
			string countryCode = localeName.ToStdString().substr(3, 5);
			int minWidth = fProjectSettings->GetMinWindowViewWidth();
			int minHeight = fProjectSettings->GetMinWindowViewHeight();
			const char* windowTitle = fProjectSettings->GetWindowTitleTextForLocale(langCode.c_str(), countryCode.c_str());
			const Rtt::NativeWindowMode* nativeWindowMode = fProjectSettings->GetDefaultWindowMode();
			DeviceOrientation::Type orientation = fProjectSettings->GetDefaultOrientation();

			if (windowTitle != NULL)
			{
				fTitle = windowTitle;
			}

			if (*nativeWindowMode == Rtt::NativeWindowMode::kFullscreen)
			{
				fMode = NativeWindowMode::kFullscreen.GetStringId();
			}

			fullScreen = strcmp(fMode, NativeWindowMode::kFullscreen.GetStringId()) == 0;

			if (fullScreen)
			{
				wxDisplay display(wxDisplay::GetFromWindow(solarApp));
				wxRect screen = display.GetClientArea();
				width = screen.width;
				height = screen.height;
			}
			else
			{
				width = fProjectSettings->GetDefaultWindowViewWidth();
				height = fProjectSettings->GetDefaultWindowViewHeight();
				//SetMinClientSize(wxSize(minWidth, minHeight));
			}

			solarApp->GetSavedZoom(width, height);

			switch (orientation)
			{
			case DeviceOrientation::kSidewaysRight:
				fRuntimeDelegate->fOrientation = DeviceOrientation::kSidewaysRight; // bottom of device is to the right

				if (width > 0 && height > 0)
				{
					fRuntimeDelegate->SetWidth(width);
					fRuntimeDelegate->SetHeight(height);
				}
				else
				{
					// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
					// use swapped default settings
					int w = fRuntimeDelegate->GetWidth();
					int h = fRuntimeDelegate->GetHeight();
					fRuntimeDelegate->SetWidth(h);
					fRuntimeDelegate->SetHeight(w);
				}
				break;

			case DeviceOrientation::kSidewaysLeft:
				fRuntimeDelegate->fOrientation = DeviceOrientation::kSidewaysLeft; // bottom of device is to the left

				if (width > 0 && height > 0)
				{
					fRuntimeDelegate->SetWidth(width);
					fRuntimeDelegate->SetHeight(height);
				}
				else
				{
					// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
					// use swapped default settings
					int w = fRuntimeDelegate->GetWidth();
					int h = fRuntimeDelegate->GetHeight();
					fRuntimeDelegate->SetWidth(h);
					fRuntimeDelegate->SetHeight(w);
				}
				break;

			case DeviceOrientation::kUpright:
				fRuntimeDelegate->fOrientation = DeviceOrientation::kUpright; // bottom of device is at the bottom

				if (width > 0 && height > 0)
				{
					fRuntimeDelegate->SetWidth(width);
					fRuntimeDelegate->SetHeight(height);
				}
				else
				{
					// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
					// use default settings
				}
				break;

			case DeviceOrientation::kUpsideDown:
				fRuntimeDelegate->fOrientation = DeviceOrientation::kUpsideDown; // bottom of device is at the top

				if (width > 0 && height > 0)
				{
					fRuntimeDelegate->SetWidth(width);
					fRuntimeDelegate->SetHeight(height);
				}
				else
				{
					// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
					// use default settings
				}
				break;
			}
		}

		if (solarApp->IsRunningOnSimulator())
		{
			fPlatform->fShowRuntimeErrors = true; // LinuxSimulatorView::Config::showRuntimeErrors;
		}

		fPlatform->setWindow(this);
		fMouseListener = new LinuxMouseListener(*fRuntime);
		fKeyListener = new LinuxKeyListener(*fRuntime);

		// Initialize Joystick Support:
		LinuxInputDeviceManager& deviceManager = (LinuxInputDeviceManager&)fPlatform->GetDevice().GetInputDeviceManager();
		deviceManager.init();
	}

	// global.print()
	int SolarAppContext::Print(lua_State* L)
	{
		membuf mb;
		int n = lua_gettop(L);  // number of arguments
		lua_getglobal(L, "tostring");
		for (int i = 1; i <= n; i++)
		{
			const char* s;
			lua_pushvalue(L, -1);  // function to be called 
			lua_pushvalue(L, i);   // value to print 
			lua_call(L, 1, 1);
			s = lua_tostring(L, -1);  // get result 
			if (s == NULL)
				return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));

			if (i > 1)
			{
				mb.append('\t');
			}
			mb.append(s);

			lua_pop(L, 1);  // pop result 
		}

		puts(mb.c_str());
		mb.append('\n');
		ConsoleApp::Log(mb.c_str());
		return 0;
	}

	bool SolarAppContext::LoadApp(SolarGLCanvas* canvas)
	{
		fCanvas = canvas;
		Init();

		if (Runtime::kSuccess != fRuntime->LoadApplication(Runtime::kLinuxLaunchOption, fRuntimeDelegate->fOrientation))
		{
			delete fRuntime;
			delete fPlatform;
			return false;
		}

		// add Resources to LUA_PATH
		string luapath(getenv("LUA_PATH"));
		luapath.append(GetStartupPath(NULL));
		luapath.append("/Resources/?.lua;");

		setenv("LUA_PATH", luapath.c_str(), true);

		if (fRuntimeDelegate->fOrientation == DeviceOrientation::kSidewaysRight || fRuntimeDelegate->fOrientation == DeviceOrientation::kSidewaysLeft)
		{
			// Swap(fRuntimeDelegate->fContentWidth, fRuntimeDelegate->fContentHeight);
		}

		if (solarApp->IsRunningOnSimulator())
		{
			fSimulator = new LinuxSimulatorServices();
			lua_State* luaStatePointer = fRuntime->VMContext().L();
			lua_pushlightuserdata(luaStatePointer, fSimulator);
			Rtt::LuaContext::RegisterModuleLoader(luaStatePointer, Rtt::LuaLibSimulator::kName, Rtt::LuaLibSimulator::Open, 1);
		}

		// re-define
		lua_State* L = fRuntime->VMContext().L();
		lua_pushcfunction(L, print2console);
		lua_setglobal(L, "print");

		GetRuntime()->BeginRunLoop();

		// starft timer
		int frameDuration = 1000.0f / (float)GetFPS();
		SetOwner(this);
		Start(frameDuration);

		return true;
	}

	// timer callback
	void SolarAppContext::Notify()
	{
		if (!fRuntime->IsSuspended())
		{
			LinuxInputDeviceManager& deviceManager = (LinuxInputDeviceManager&)GetPlatform()->GetDevice().GetInputDeviceManager();
			deviceManager.dispatchEvents(fRuntime);

			// advance engine
			(*fRuntime)();
		}
	}

	void SolarAppContext::Flush()
	{
		fRuntime->GetDisplay().Invalidate();
		fCanvas->Render();
	}

	void SolarAppContext::Pause()
	{
		if (!fRuntime->IsSuspended())
		{
			fRuntime->Suspend();
		}
	}

	void SolarAppContext::Resume()
	{
		if (GetRuntime()->IsSuspended())
		{
			fRuntime->DispatchEvent(ResizeEvent());
			fRuntime->Resume();
		}
	}

	void SolarAppContext::RestartRenderer()
	{
		fRuntime->GetDisplay().WindowSizeChanged();
		fRuntime->RestartRenderer(fRuntimeDelegate->fOrientation);
		fRuntime->GetDisplay().Invalidate();
		fRuntime->DispatchEvent(ResizeEvent());
	}

	int SolarAppContext::GetWidth() const
	{
		return fRuntimeDelegate->GetWidth();
	}

	void SolarAppContext::SetWidth(int val)
	{
		fRuntimeDelegate->SetWidth(val);
	}

	int SolarAppContext::GetHeight() const
	{
		return fRuntimeDelegate->GetHeight();
	}

	void SolarAppContext::SetHeight(int val)
	{
		fRuntimeDelegate->SetHeight(val);
	}

} // namespace Rtt

