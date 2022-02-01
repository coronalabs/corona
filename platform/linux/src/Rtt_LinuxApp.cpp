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

#define Rtt_DEBUG_TOUCH 1

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

	SolarApp::SolarApp(const std::string& resourceDir)
		: fProjectPath(resourceDir)
		, fWindow(NULL)
		, fWidth(0)
		, fHeight(0)
	{
		curl_global_init(CURL_GLOBAL_ALL);

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
		fContext = NULL;
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

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);

		CreateWindow(fProjectPath);

		fContext = new SolarAppContext(fWindow, fProjectPath.c_str());
		return fContext->LoadApp();
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
				fContext->Resume();
				break;
			}
			case SDL_APP_WILLENTERBACKGROUND:
			{
				fContext->Pause();
				break;
			}
			case SDL_WINDOWEVENT:
			{
				//SDL_Log("SDL_WINDOWEVENT %d %d,%d", event.window.event, event.window.data1, event.window.data2);
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_SHOWN:
				case SDL_WINDOWEVENT_RESTORED:
					fContext->Resume();
					break;
				case SDL_WINDOWEVENT_HIDDEN:
				case SDL_WINDOWEVENT_MINIMIZED:
					fContext->Pause();
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
						*/
			case SDL_MOUSEBUTTONDOWN:
			{
				const SDL_MouseButtonEvent& b = event.button;
				//	if (b.which != SDL_TOUCH_MOUSEID)
				{
					int x = b.x;
					int y = b.y;
					//SwapXY(x, y);

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

					fContext->DispatchEvent(mouseEvent);
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
					//SwapXY(x, y);

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

					fContext->DispatchEvent(mouseEvent);
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
					//SwapXY(x, y);

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

					fContext->DispatchEvent(mouseEvent);
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
					//SwapXY(x, y);

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

					fContext->DispatchEvent(mouseEvent);
				}
				break;
			}

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
		bool fullScreen = false;
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

		uint32_t windowStyle = SDL_WINDOW_OPENGL;

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
				//				windowStyle |= wxMINIMIZE_BOX;
			}

			if (isWindowMaximizeButtonEnabled)
			{
				windowStyle |= SDL_WINDOW_RESIZABLE;
			}

			if (isWindowCloseButtonEnabled)
			{
				//				windowStyle |= wxCLOSE_BOX;
			}

			if (isWindowResizable)
			{
				//				windowStyle |= wxRESIZE_BORDER;
			}

			if (fullScreen)
			{
				windowStyle |= SDL_WINDOW_FULLSCREEN;
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

		fWindow = SDL_CreateWindow("Solar2D", 0, 0, width, height, windowStyle);
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
		return true;
	}

	void SolarApp::Run()
	{
		int fps = fContext->GetFPS();
		float frameDuration = 1000.0f / fps;

		// main app loop
		while (1)
		{
			U64 start_time = Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime());

			if (!PollEvents())
				break;

			fContext->advance();

			int advance_time = (int)(Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime()) - start_time);
			//			Rtt_Log("advance_time %d\n", advance_time);

			// Don't hog the CPU.
			int sleep_time = Max(frameDuration - advance_time, 1.0f);		// sleep for at least 1ms
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
		}
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

}
