//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

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

using namespace Rtt;
using namespace std;

// for redirecting output to Solar2DConsole
extern "C"
{
	static int print2console(lua_State* L)
	{
		return SolarAppContext::Print(L);
	}
}

namespace Rtt
{

	SolarApp::SolarApp()
		: fSolarGLCanvas(NULL)
		, fContext(NULL)
		, fProjectPath("")
		, fTimer(this, TIMER_ID)
	{
#ifdef __WXGTK3__
		setenv("GDK_BACKEND", "x11", 1);
#endif

		curl_global_init(CURL_GLOBAL_ALL);
		wxInitAllImageHandlers();

		const char* homeDir = GetHomePath();
		string basePath(homeDir);
		string sandboxPath(homeDir);
		string pluginPath(homeDir);

		// create default directories if missing

		basePath.append("/.Solar2D");
		sandboxPath.append("/.Solar2D/Sandbox");
		pluginPath.append("/.Solar2D/Plugins");

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
	}

	SolarApp::~SolarApp()
	{
		delete fSolarGLCanvas;
		delete fContext;

		curl_global_cleanup();
	}

	bool SolarApp::Start(const string& resourcesDir)
	{
		// set window

		bool fullScreen = false;
		int windowStyle = wxCAPTION;
		int width = 320;
		int height = 480;
		int minWidth = width;
		int minHeight = height;

		// project settings
		ProjectSettings projectSettings;
		projectSettings.LoadFromDirectory(resourcesDir.c_str());

		// grab the required config settings (we only need width/height at this stage)
		if (projectSettings.HasConfigLua())
		{
			width = projectSettings.GetContentWidth();
			height = projectSettings.GetContentHeight();
		}

		// grab the build settings (we only need width/height at this stage)
		if (projectSettings.HasBuildSettings())
		{
			const Rtt::NativeWindowMode* nativeWindowMode = projectSettings.GetDefaultWindowMode();
			bool isWindowMinimizeButtonEnabled = projectSettings.IsWindowMinimizeButtonEnabled();
			bool isWindowMaximizeButtonEnabled = projectSettings.IsWindowMaximizeButtonEnabled();
			bool isWindowCloseButtonEnabled = projectSettings.IsWindowCloseButtonEnabled();
			bool isWindowResizable = projectSettings.IsWindowResizable();
			width = projectSettings.GetDefaultWindowViewWidth();
			height = projectSettings.GetDefaultWindowViewHeight();
			minWidth = projectSettings.GetMinWindowViewWidth();
			minHeight = projectSettings.GetMinWindowViewHeight();

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

		CreateMenus();

		// create app window
		Create(NULL, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(width, height), windowStyle);

		int vAttrs[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
		fSolarGLCanvas = new SolarGLCanvas(this, vAttrs);

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
			Show(true);
		}

		wxCommandEvent eventOpen(eventOpenProject);

	/*	if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			if (LinuxSimulatorView::Config::openLastProject && !LinuxSimulatorView::Config::lastProjectDirectory.IsEmpty())
			{
				wxString fullPath(LinuxSimulatorView::Config::lastProjectDirectory);
				fullPath.append("/main.lua");
				eventOpen.SetInt(ID_MENU_OPEN_LAST_PROJECT);
				eventOpen.SetString(fullPath);
			}
		}*/

		OnOpen(eventOpen);
		return true;
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

	void SolarApp::ChangeSize(int newWidth, int newHeight)
	{
		SetMinClientSize(wxSize(newWidth, newHeight));
		SetClientSize(wxSize(newWidth, newHeight));
		SetSize(wxSize(newWidth, newHeight));
	}

	void SolarApp::OnOpen(wxCommandEvent& event)
	{
		wxString path = event.GetString();
		string fullPath = (const char*)path.c_str();
		path = path.SubString(0, path.size() - 10); // without main.lua

		delete fContext;
		fContext = new SolarAppContext(path.c_str());
		chdir(fContext->GetAppPath());

		string appName = fContext->GetAppName();

		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			WatchFolder(fContext->GetAppPath(), appName.c_str());
			SetCursor(wxCURSOR_ARROW);
		}

		bool fullScreen = fContext->Init();
		wxString newWindowTitle(appName);

		fContext->LoadApp(fSolarGLCanvas);
		ResetSize();
		fContext->SetCanvas(fSolarGLCanvas);

		fContext->RestartRenderer();
		GetCanvas()->Refresh(true);
		StartTimer(1000.0f / (float)fContext->GetFPS());
		SetTitle(newWindowTitle);
	}

	void SolarApp::StartTimer(float frameDuration)
	{
		fTimer.Start((int)frameDuration);
		fContext->GetRuntime()->BeginRunLoop();
	}

	void SolarApp::OnTimer(wxTimerEvent& event)
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

	//
	// Solar GLCanvas, mouse & key listener
	// 

	wxBEGIN_EVENT_TABLE(SolarGLCanvas, wxGLCanvas)
		EVT_SIZE(SolarGLCanvas::OnSize)
		wxEND_EVENT_TABLE()

		SolarGLCanvas::SolarGLCanvas(SolarApp* parent, const int* vAttrs)
		: wxGLCanvas(parent, wxID_ANY, vAttrs, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
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

	void SolarGLCanvas::Render()
	{
		SetCurrent(*fGLContext);
		SwapBuffers();
	}

	void SolarGLCanvas::OnSize(wxSizeEvent& event)
	{
		// if the window is fully initialized
		if (IsShownOnScreen())
		{
			Refresh(true);
			Render();
		}
	}
}
