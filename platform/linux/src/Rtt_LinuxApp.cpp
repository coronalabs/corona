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
	{
		curl_global_init(CURL_GLOBAL_ALL);
		//wxInitAllImageHandlers();

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

	bool SolarApp::Initialize()
	{
		SDL_version ver;
		SDL_GetVersion(&ver);
		Rtt_Log("SDL version %d.%d.%d\n", ver.major, ver.minor, ver.patch);

		// Initialize SDL (Note: video is required to start event loop) 
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		{
			Rtt_LogException("Couldn't initialize SDL: %s\n", SDL_GetError());
			return false;
		}

		// init GL

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);

		fWindow = SDL_CreateWindow("Solar2D", 0, 0, 320, 480, SDL_WINDOW_OPENGL); // | SDL_WINDOW_FULLSCREEN);
		if (!fWindow)
		{
			Rtt_LogException("Couldn't create window: %s\n", SDL_GetError());
			return false;
		}

		fGLcontext = SDL_GL_CreateContext(fWindow);
		if (!fGLcontext)
		{
			Rtt_LogException("SDL_GL_CreateContext(): %s\n", SDL_GetError());
			return false;
		}

		//InitGLFuncs();

		// get screen size
		SDL_DisplayMode dm;
		SDL_GetDesktopDisplayMode(0, &dm);
		fWidth = dm.w;
		fHeight = dm.h;

		/*
		//fPlatform = new NintendoPlatform(fPathToApp.c_str(), fDocumentsDir.c_str(), fTmpDir.c_str(), fTmpDir.c_str(), fTmpDir.c_str());
		//fRuntime = new NintendoRuntime(*fPlatform, NULL);
		//fL = fRuntime->VMContext().L();		// save

		//fRuntimeDelegate = new NintendoRuntimeDelegate();
		//fRuntime->SetDelegate(fRuntimeDelegate);

		//fInputDevice = static_cast<NintendoInputDevice*>(GetPlatformInputDevice(0));
		//Rtt_ASSERT(fInputDevice);
		//fInputDevice->SetAxisMinMax(-32767, 32767);

		//fRuntime->SetProperty(Runtime::kEmscriptenMaskSet, true);

		// For debugging, use main.lua if it exists in the app folder
		{
			std::string main_lua = fPathToApp.c_str();
			main_lua += "/main.lua";
			FILE* fi = fopen(main_lua.c_str(), "r");
			if (fi)
			{
				fclose(fi);
				fRuntime->SetProperty(Runtime::kEmscriptenMaskSet | Runtime::kIsApplicationNotArchived | Runtime::kShouldVerifyLicense, true);
			}
		}

		// settings

		std::string orientation = "portrait";		// by default
		std::string title;
		int app_width = 0;
		int app_height = 0;
		fRuntime->readSettings(&app_width, &app_height, &orientation, &title, &fMode);
		if (orientation == "landscapeRight")
		{
			fOrientation = DeviceOrientation::kSidewaysRight;	// bottom of device is to the right
		}
		else if (orientation == "landscapeLeft")
		{
			fOrientation = DeviceOrientation::kSidewaysLeft; 	// bottom of device is to the left
		}
		else if (orientation == "portrait")
		{
			fOrientation = DeviceOrientation::kUpright;	// bottom of device is at the bottom
		}
		else if (orientation == "portraitUpsideDown")
		{
			fOrientation = DeviceOrientation::kUpsideDown;	// bottom of device is at the top
		}
		else
		{
			//Rtt_LogException("Unsupported orientation: '%s'", orientation.c_str());
		}

		// launchOrientation is not Portrait so needs to swap width & height
		fPlatform->setWindow(fWindow, fOrientation, dm.w, dm.h);

		Runtime::LoadParameters parameters;
		parameters.launchOptions = Runtime::kHTML5LaunchOption;
		parameters.orientation = DeviceOrientation::kSidewaysLeft;

		if (fRuntime->LoadApplication(parameters))
		{
			delete fRuntime;
			delete fPlatform;
			return false;
		}

		fRuntime->BeginRunLoop();*/
		return true;
	}

	void SolarApp::Run()
	{
		int fps = 30; //vv getFPS();
		float frameDuration = 1000.0f / fps;

		// main app loop
		while (1)
		{
			U64 start_time = Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime());

			if (!PollEvents())
				break;

			//TimerTick();

			int advance_time = (int)(Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime()) - start_time);
			//			Rtt_Log("advance_time %d\n", advance_time);

			// Don't hog the CPU.
			int sleep_time = Max(frameDuration - advance_time, 1.0f);		// sleep for at least 1ms
			usleep(sleep_time * 1000);
		}
		Rtt_Log("main loop ended\n");
	}

	bool SolarApp::PollEvents()
	{
		// aggregate
		SDL_Event event;
		Uint32 event_number = 0;
		std::map<Uint64, events_t> events;
		while (SDL_PollEvent(&event))
		{
			Uint64 key;
			switch (event.type)
			{
			case SDL_SENSORUPDATE:
			case SDL_SENSORUPDATE + 1:
			case SDL_FINGERMOTION:
			case SDL_MOUSEMOTION:
			{
				// aggregate these events
				key = event.type;
				break;
			}
			default:
			{
				// all other events must be processed
				key = event_number;
				key <<= 32;
				break;
			}
			}

			events[key] = { ++event_number, event };
		}

		// sort on a first come
		std::vector<std::pair<Uint32, events_t>> sorted_events;
		{
			// Copy key-value pair from Map to vector of pairs
			for (auto& it : events)
			{
				sorted_events.push_back(it);
			}

			// sort
			sort(sorted_events.begin(), sorted_events.end(), cmp_events);
		}

		for (auto& it : sorted_events)
		{
			event = it.second.e;
			//SDL_Log("SDL_EVENT %d\n", event.type);

			//U64 start_time = Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime());
			switch (event.type)
			{
			case SDL_APP_WILLENTERFOREGROUND:
			{
				//resume();
				break;
			}
			case SDL_APP_WILLENTERBACKGROUND:
			{
				//pause();
				break;
			}
			case SDL_WINDOWEVENT:
			{
				//SDL_Log("SDL_WINDOWEVENT %d %d,%d", event.window.event, event.window.data1, event.window.data2);
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_SHOWN:
				case SDL_WINDOWEVENT_RESTORED:
					//resume();
					break;
				case SDL_WINDOWEVENT_HIDDEN:
				case SDL_WINDOWEVENT_MINIMIZED:
					//pause();
					break;
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				{
					SDL_DisplayMode dm;
					SDL_GetDesktopDisplayMode(0, &dm);
					if (fWidth != dm.w && fHeight != dm.h)
					{
						//SDL_Log("SDL_WINDOWEVENT_SIZE_CHANGED old %d,%d new %d,%d", fWidth, fHeight, dm.w, dm.h);
						fWidth = dm.w;
						fHeight = dm.h;
						//fPlatform->setWindow(fWindow, fOrientation, dm.w, dm.h);

						//fRuntime->WindowSizeChanged();
						//fRuntime->RestartRenderer(fOrientation);
						//fRuntime->GetDisplay().Invalidate();

						//fRuntime->DispatchEvent(ResizeEvent());

						// refresh native elements
	//					jsContextResizeNativeObjects();
					}
					break;
				}
				default:
					break;
				}
				break;
			}
/*			case SDL_FINGERDOWN:
			{
				SDL_TouchFingerEvent& ef = event.tfinger;
				int w, h;
				SDL_GetWindowSize(fWindow, &w, &h);
				int x = w * ef.x;
				int y = h * ef.y;
				SwapXY(x, y);
				TouchDown(x, y, ef.fingerId);
				break;
			}
			case SDL_FINGERUP:
			{
				SDL_TouchFingerEvent& ef = event.tfinger;
				int w, h;
				SDL_GetWindowSize(fWindow, &w, &h);
				int x = w * ef.x;
				int y = h * ef.y;
				SwapXY(x, y);
				TouchUp(x, y, ef.fingerId);
				break;
			}
			case SDL_FINGERMOTION:
			{
				SDL_TouchFingerEvent& ef = event.tfinger;
				int w, h;
				SDL_GetWindowSize(fWindow, &w, &h);
				int x = w * ef.x;
				int y = h * ef.y;
				SwapXY(x, y);
				TouchMoved(x, y, ef.fingerId);
				break;
			}

			case SDL_MOUSEBUTTONDOWN:
			{
				const SDL_MouseButtonEvent& b = event.button;
				//	if (b.which != SDL_TOUCH_MOUSEID)
				{
					int x = b.x;
					int y = b.y;
					SwapXY(x, y);

#if Rtt_DEBUG_TOUCH
					printf("MOUSE_DOWN(%d, %d)\n", x, y);
#endif

					float scrollWheelDeltaX = 0;
					float scrollWheelDeltaY = 0;

					// Fetch the mouse's current up/down buttons states.
					bool isPrimaryDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);
					bool isSecondaryDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT);
					bool isMiddleDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_MIDDLE);

					// Fetch the current state of the "shift", "alt", and "ctrl" keys.
					const Uint8* key = SDL_GetKeyboardState(NULL);
					bool IsAltDown = key[SDL_SCANCODE_LALT] | key[SDL_SCANCODE_RALT];
					bool IsShiftDown = key[SDL_SCANCODE_LSHIFT] | key[SDL_SCANCODE_RSHIFT];
					bool IsControlDown = key[SDL_SCANCODE_LCTRL] | key[SDL_SCANCODE_RCTRL];
					bool IsCommandDown = key[SDL_SCANCODE_LGUI] | key[SDL_SCANCODE_RGUI];

					Rtt::MouseEvent::MouseEventType eventType = Rtt::MouseEvent::kDown;
					Rtt::MouseEvent mouseEvent(eventType, x, y, Rtt_FloatToReal(scrollWheelDeltaX), Rtt_FloatToReal(scrollWheelDeltaY), 0, isPrimaryDown, isSecondaryDown, isMiddleDown, IsShiftDown, IsAltDown, IsControlDown, IsCommandDown);

					DispatchEvent(mouseEvent);
				}
				break;
			}

			case SDL_MOUSEMOTION:
			{
				const SDL_MouseButtonEvent& b = event.button;
				//	if (b.which != SDL_TOUCH_MOUSEID)
				{
					int x = b.x;
					int y = b.y;
					SwapXY(x, y);

#if Rtt_DEBUG_TOUCH
					printf("MOUSE_MOTION(%d, %d)\n", x, y);
#endif

					float scrollWheelDeltaX = 0;
					float scrollWheelDeltaY = 0;

					// Fetch the mouse's current up/down buttons states.
					bool isPrimaryDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);
					bool isSecondaryDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT);
					bool isMiddleDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_MIDDLE);

					// Fetch the current state of the "shift", "alt", and "ctrl" keys.
					const Uint8* key = SDL_GetKeyboardState(NULL);
					bool IsAltDown = key[SDL_SCANCODE_LALT] | key[SDL_SCANCODE_RALT];
					bool IsShiftDown = key[SDL_SCANCODE_LSHIFT] | key[SDL_SCANCODE_RSHIFT];
					bool IsControlDown = key[SDL_SCANCODE_LCTRL] | key[SDL_SCANCODE_RCTRL];
					bool IsCommandDown = key[SDL_SCANCODE_LGUI] | key[SDL_SCANCODE_RGUI];

					Rtt::MouseEvent::MouseEventType eventType = Rtt::MouseEvent::kMove;

					// Determine if this is a "drag" event.
					if (isPrimaryDown || isSecondaryDown || isMiddleDown)
					{
						eventType = Rtt::MouseEvent::kDrag;
					}

					Rtt::MouseEvent mouseEvent(eventType, x, y, Rtt_FloatToReal(scrollWheelDeltaX), Rtt_FloatToReal(scrollWheelDeltaY), 0,
						isPrimaryDown, isSecondaryDown, isMiddleDown, IsShiftDown, IsAltDown, IsControlDown, IsCommandDown);

					DispatchEvent(mouseEvent);
				}
				break;
			}

			case SDL_MOUSEBUTTONUP:
			{
				const SDL_MouseButtonEvent& b = event.button;
				//	if (b.which != SDL_TOUCH_MOUSEID)
				{
					int x = b.x;
					int y = b.y;
					SwapXY(x, y);

#if Rtt_DEBUG_TOUCH
					printf("MOUSE_UP(%d, %d)\n", x, y);
#endif

					float scrollWheelDeltaX = 0;
					float scrollWheelDeltaY = 0;

					// Fetch the mouse's current up/down buttons states.
					bool isPrimaryDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);
					bool isSecondaryDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT);
					bool isMiddleDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_MIDDLE);

					// Fetch the current state of the "shift", "alt", and "ctrl" keys.
					const Uint8* key = SDL_GetKeyboardState(NULL);
					bool IsAltDown = key[SDL_SCANCODE_LALT] | key[SDL_SCANCODE_RALT];
					bool IsShiftDown = key[SDL_SCANCODE_LSHIFT] | key[SDL_SCANCODE_RSHIFT];
					bool IsControlDown = key[SDL_SCANCODE_LCTRL] | key[SDL_SCANCODE_RCTRL];
					bool IsCommandDown = key[SDL_SCANCODE_LGUI] | key[SDL_SCANCODE_RGUI];

					Rtt::MouseEvent::MouseEventType eventType = Rtt::MouseEvent::kUp;
					Rtt::MouseEvent mouseEvent(eventType, x, y, Rtt_FloatToReal(scrollWheelDeltaX), Rtt_FloatToReal(scrollWheelDeltaY), 0, isPrimaryDown, isSecondaryDown, isMiddleDown, IsShiftDown, IsAltDown, IsControlDown, IsCommandDown);

					DispatchEvent(mouseEvent);
				}
				break;
			}

			case SDL_MOUSEWHEEL:
			{
				const SDL_MouseWheelEvent& w = event.wheel;
				if (w.which != SDL_TOUCH_MOUSEID)
				{
					int scrollWheelDeltaX = w.x;
					int scrollWheelDeltaY = w.y;
					int x = w.x;
					int y = w.y;
					SwapXY(x, y);

#if Rtt_DEBUG_TOUCH
					printf("MOUSE_WHEEL(%d, %d)\n", x, y);
#endif

					// Fetch the mouse's current up/down buttons states.
					bool isPrimaryDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);
					bool isSecondaryDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT);
					bool isMiddleDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_MIDDLE);

					// Fetch the current state of the "shift", "alt", and "ctrl" keys.
					const Uint8* key = SDL_GetKeyboardState(NULL);
					bool IsAltDown = key[SDL_SCANCODE_LALT] | key[SDL_SCANCODE_RALT];
					bool IsShiftDown = key[SDL_SCANCODE_LSHIFT] | key[SDL_SCANCODE_RSHIFT];
					bool IsControlDown = key[SDL_SCANCODE_LCTRL] | key[SDL_SCANCODE_RCTRL];
					bool IsCommandDown = key[SDL_SCANCODE_LGUI] | key[SDL_SCANCODE_RGUI];

					Rtt::MouseEvent::MouseEventType eventType = Rtt::MouseEvent::kScroll;
					Rtt::MouseEvent mouseEvent(eventType, x, y, Rtt_FloatToReal(scrollWheelDeltaX), Rtt_FloatToReal(scrollWheelDeltaY), 0,
						isPrimaryDown, isSecondaryDown, isMiddleDown, IsShiftDown, IsAltDown, IsControlDown, IsCommandDown);

#if Rtt_DEBUG_TOUCH
					//			printf("MouseEvent(%d, %d)\n", b.x, b.y);
#endif

					DispatchEvent(mouseEvent);
				}
				break;
			}
			*/

			case SDL_QUIT:
			case SDLK_ESCAPE:
				return false;

			default:
				break;
			}

			//int advance_time = (int)(Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime()) - start_time);
			//	Rtt_Log("event %x, advance time %d\n", event.type, advance_time);
		}
		return true;
	}

	bool SolarApp::CreateWindow(const string& resourcesDir)
	{
		// set window
		/*
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

		// create app window
		Create(NULL, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(width, height), windowStyle);

		int vAttrs[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };

		Rtt_ASSERT(fSolarGLCanvas == NULL);
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
		}*/
		return true;
	}

	bool SolarApp::Start(const string& resourcesDir)
	{
		CreateWindow(resourcesDir);

		fContext = new SolarAppContext(resourcesDir.c_str());
		fContext->LoadApp(fSolarGLCanvas);
		ResetWindowSize();

		//	SetTitle(fContext->GetAppName());
		return true;
	}

	void SolarApp::ResetWindowSize()
	{
		/*	wxSize clientSize = GetClientSize();

			if (IsFullScreen())
			{
				fContext->SetWidth(clientSize.GetWidth());
				fContext->SetHeight(clientSize.GetHeight());
			}

			ChangeSize(fContext->GetWidth(), fContext->GetHeight());
			GetCanvas()->Refresh(true);*/
	}

	void SolarApp::OnIconized(wxIconizeEvent& event)
	{
		fContext->RestartRenderer();
	}

	void SolarApp::OnClose(wxCloseEvent& event)
	{
		fContext->GetRuntime()->End();
		wxExit();
	}

	void SolarApp::ChangeSize(int newWidth, int newHeight)
	{
		//	SetMinClientSize(wxSize(newWidth, newHeight));
		//	SetClientSize(wxSize(newWidth, newHeight));
		//	SetSize(wxSize(newWidth, newHeight));
	}

	//
	// Solar GLCanvas, mouse & key listener
	// 
	/*
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
	*/
}
