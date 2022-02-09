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

//#define Rtt_DEBUG_TOUCH 1

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
		, imctx(NULL)
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

		// Cleanup
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		SDL_GL_DeleteContext(fGLcontext);
		SDL_DestroyWindow(fWindow);
		SDL_Quit();
	}

	bool SolarApp::Initialize()
	{
		SDL_version ver;
		SDL_GetVersion(&ver);
		Rtt_Log("SDL version %d.%d.%d\n", ver.major, ver.minor, ver.patch);

		// Initialize SDL (Note: video is required to start event loop) 
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			Rtt_LogException("Couldn't initialize SDL: %s\n", SDL_GetError());
			return false;
		}

		// GL 3.0 + GLSL 130
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE | SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

		uint32_t windowStyle = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI; // | SDL_WINDOW_BORDERLESS;
		windowStyle |= SDL_WINDOW_RESIZABLE;
		fWindow = SDL_CreateWindow("Solar2D", 0, 0, 320, 480, windowStyle);
		Rtt_ASSERT(fWindow);
		fGLcontext = SDL_GL_CreateContext(fWindow);
		Rtt_ASSERT(fGLcontext);
		SDL_GL_MakeCurrent(fWindow, fGLcontext);
		SDL_GL_SetSwapInterval(1); // Enable vsync

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		imctx = ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsClassic();

		// Setup Platform/Renderer backends
		ImGui_ImplSDL2_InitForOpenGL(fWindow, fGLcontext);
		const char* glsl_version = "#version 130";
		ImGui_ImplOpenGL3_Init(glsl_version);

		fContext = new SolarAppContext(fWindow, fProjectPath.c_str());
		fMenu = new ImMenu(fContext->GetAppName());

		return fContext->LoadApp();
	}

	bool SolarApp::PollEvents()
	{
		vector<SDL_Event> events;
		ImGuiIO& io = ImGui::GetIO();

		SDL_Event evt;
		while (SDL_PollEvent(&evt))
		{
			// GUI
			ImGui_ImplSDL2_ProcessEvent(&evt);

			if (evt.type == SDL_QUIT)
				return false;
			if (evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_CLOSE && evt.window.windowID == SDL_GetWindowID(fWindow))
				return false;

			events.push_back(evt);
		}

		for (int i = 0; i < events.size(); i++)
		{
			//SDL_Log("SDL_EVENT %d\n", event.type);
			//U64 start_time = Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime());
			SDL_Event& e = events[i];
			switch (e.type)
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
				//SDL_Log("SDL_WINDOWEVENT %d: %d %d,%d", e.window.windowID, e.window.event, e.window.data1, e.window.data2);
				switch (e.window.event)
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
				case SDL_WINDOWEVENT_CLOSE:
				{
					break;
				}
				default:
					break;
				}
				break;
			}
			case SDL_FINGERDOWN:
			{
				int w, h;
				SDL_GetWindowSize(fWindow, &w, &h);
				SDL_TouchFingerEvent& ef = e.tfinger;
				fContext->GetMouseListener()->TouchDown(w * ef.x, h * ef.y, ef.fingerId);
				break;
			}
			case SDL_FINGERUP:
			{
				int w, h;
				SDL_GetWindowSize(fWindow, &w, &h);
				SDL_TouchFingerEvent& ef = e.tfinger;
				fContext->GetMouseListener()->TouchUp(w * ef.x, h * ef.y, ef.fingerId);
				break;
			}
			case SDL_FINGERMOTION:
			{
				int w, h;
				SDL_GetWindowSize(fWindow, &w, &h);
				SDL_TouchFingerEvent& ef = e.tfinger;
				fContext->GetMouseListener()->TouchMoved(w * ef.x, h * ef.y, ef.fingerId);
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
			{
				const SDL_MouseButtonEvent& b = e.button;
				if (b.which != SDL_TOUCH_MOUSEID)
				{
					// When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
					if (io.WantCaptureMouse)
						break;

					int x = b.x;
					int y = b.y;

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
					fContext->GetMouseListener()->TouchDown(x, y, 0);
					break;
				}
			}

			case SDL_MOUSEMOTION:
			{
				const SDL_MouseButtonEvent& b = e.button;
				if (b.which != SDL_TOUCH_MOUSEID)
				{
					// When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
					if (io.WantCaptureMouse)
						break;

					int x = b.x;
					int y = b.y;

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

#if Rtt_DEBUG_TOUCH
					//			printf("MouseEvent(%d, %d)\n", b.x, b.y);
#endif

					fContext->DispatchEvent(mouseEvent);
					fContext->GetMouseListener()->TouchMoved(x, y, 0);
					break;
				}
			}

			case SDL_MOUSEBUTTONUP:
			{
				const SDL_MouseButtonEvent& b = e.button;
				if (b.which != SDL_TOUCH_MOUSEID)
				{
					// When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
					if (io.WantCaptureMouse)
						break;

					int x = b.x;
					int y = b.y;

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
					fContext->GetMouseListener()->TouchUp(x, y, 0);
					break;
				}
			}

			case SDL_MOUSEWHEEL:
			{
				const SDL_MouseWheelEvent& w = e.wheel;
				if (w.which != SDL_TOUCH_MOUSEID)
				{
					// When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
					if (io.WantCaptureMouse)
						break;

					int scrollWheelDeltaX = w.x;
					int scrollWheelDeltaY = w.y;
					int x = w.x;
					int y = w.y;

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
					break;
				}
			}

			default:
				MenuEvent(e);
				break;
			}

			//int advance_time = (int)(Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime()) - start_time);
			//	Rtt_Log("event %x, advance time %d\n", event.type, advance_time);
		}
		return true;
	}

	void SolarApp::SetTitle(const std::string& name)
	{
		SDL_SetWindowTitle(fWindow, IsHomeScreen(name) ? "Solar2D Simulator" : name.c_str());
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

	void SolarApp::RenderGUI()
	{
		if (imctx == NULL)
			return;

		// draw GUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		fMenu->Draw();
		if (fDlg)
		{
			fDlg->Draw();
		}

		ImGui::EndFrame();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
