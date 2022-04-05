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
#include "Rtt_MPlatformServices.h"
#include "Rtt_HTTPClient.h"
#include "Rtt_LinuxKeyListener.h"
#include "Rtt_BitmapUtils.h"
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

	SolarApp::SolarApp(const string& resourceDir)
		: fResourceDir(resourceDir)
		, fWindow(NULL)
		, fImCtx(NULL)
		, fActivityIndicator(false)
	{
		fMouse = new LinuxMouseListener();
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


	bool SolarApp::InitSDL()
	{
		curl_global_init(CURL_GLOBAL_ALL);

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

		uint32_t windowStyle = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
		fWindow = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, windowStyle);
		SetIcon();

		fGLcontext = SDL_GL_CreateContext(fWindow);
		SDL_GL_MakeCurrent(fWindow, fGLcontext);
		SDL_GL_SetSwapInterval(1); // Enable vsync

		// Setup Dear ImGui context

		IMGUI_CHECKVERSION();
		fImCtx = ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls

		// Setup Platform/Renderer backends
		ImGui_ImplSDL2_InitForOpenGL(fWindow, fGLcontext);
		const char* glsl_version = "#version 130";
		ImGui_ImplOpenGL3_Init(glsl_version);

		return true;
	}

	void SolarApp::SetIcon()
	{
		int image_width = 0;
		int image_height = 0;
		string icon_path = GetStartupPath(NULL);
		icon_path.append("/Resources/solar2d.png");

		FILE* f = fopen(icon_path.c_str(), "rb");
		if (f)
		{
			unsigned char* img = bitmapUtil::loadPNG(f, image_width, image_height);
			fclose(f);

			if (img)
			{
				// Set up the pixel format color masks for RGB(A) byte arrays.
				// Only STBI_rgb (3) and STBI_rgb_alpha (4) are supported here!
				Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				int shift = (req_format == STBI_rgb) ? 8 : 0;
				rmask = 0xff000000 >> shift;
				gmask = 0x00ff0000 >> shift;
				bmask = 0x0000ff00 >> shift;
				amask = 0x000000ff >> shift;
#else // little endian, like x86
				rmask = 0x000000ff;
				gmask = 0x0000ff00;
				bmask = 0x00ff0000;
				amask = 0xff000000;
#endif

				int depth = 32;
				int pitch = 4 * image_width;
				SDL_Surface* icon = SDL_CreateRGBSurfaceFrom(img, image_width, image_height, depth, pitch, rmask, gmask, bmask, amask);
				SDL_SetWindowIcon(fWindow, icon);
				SDL_FreeSurface(icon);

				free(img);
			}
		}
	}

	bool SolarApp::Init()
	{
		if (InitSDL())
		{
			return LoadApp(fResourceDir);
		}
		return false;
	}

	void SolarApp::GetWindowPosition(int* x, int* y)
	{
		SDL_GetWindowPosition(fWindow, x, y);
	}

	void SolarApp::GetWindowSize(int* w, int* h)
	{
		SDL_GetWindowSize(fWindow, w, h);
		*h = -GetMenuHeight();
	}

	bool SolarApp::LoadApp(const string& path)
	{
		fContext = new SolarAppContext(fWindow);
		CreateMenu();
		return fContext->LoadApp(fResourceDir);
	}

	bool SolarApp::PollEvents()
	{
		vector<SDL_Event> events;
		SDL_Event evt;
		while (SDL_PollEvent(&evt))
		{
			// GUI
			ImGui_ImplSDL2_ProcessEvent(&evt);

			if (fConsole)
				fConsole->ProcessEvent(evt);

			if (fDlg)
				fDlg->ProcessEvent(evt);

			if (evt.type == SDL_QUIT)
				return false;
			if (evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_CLOSE && evt.window.windowID == SDL_GetWindowID(fWindow))
				return false;

			events.push_back(evt);
		}

		ImGuiIO& io = ImGui::GetIO();
		for (int i = 0; i < events.size(); i++)
		{
			//SDL_Log("SDL_EVENT %d\n", event.type);
			//U64 start_time = Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime());
			const SDL_Event& evt = events[i];
			switch (evt.type)
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

			case sdl::OnSetCursor:
			{
				string cursorName = (const char*)evt.user.data1;
				free(evt.user.data1);
				// todo
				break;
			}
			case sdl::OnMouseCursorVisible:
				SDL_ShowCursor(evt.user.code ? SDL_ENABLE : SDL_DISABLE);
				break;

			case sdl::OnWindowNormal:
				SDL_RestoreWindow(fWindow);
				fContext->Resume();
				break;

			case sdl::OnWindowMaximized:
				SDL_MaximizeWindow(fWindow);
				fContext->Resume();
				break;

			case sdl::OnWindowFullscreen:
				SDL_SetWindowFullscreen(fWindow, SDL_WINDOW_FULLSCREEN);	// SDL_WINDOW_FULLSCREEN_DESKTOP
				fContext->Resume();
				break;

			case sdl::OnWindowMinimized:
				SDL_MinimizeWindow(fWindow);
				fContext->Pause();
				break;

			case SDL_WINDOWEVENT:
			{
				//SDL_Log("SDL_WINDOWEVENT %d: %d %d,%d", e.window.windowID, e.window.event, e.window.data1, e.window.data2);
				switch (evt.window.event)
				{
				case SDL_WINDOWEVENT_MAXIMIZED:
					break;

				case SDL_WINDOWEVENT_SHOWN:
				case SDL_WINDOWEVENT_RESTORED:
					if (evt.window.windowID == SDL_GetWindowID(fWindow))
					{
						fContext->Resume();
					}
					break;
				case SDL_WINDOWEVENT_HIDDEN:
				case SDL_WINDOWEVENT_MINIMIZED:
					if (evt.window.windowID == SDL_GetWindowID(fWindow))
					{
						fContext->Pause();
					}
					break;
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				{
					if (evt.window.windowID == SDL_GetWindowID(fWindow))
					{
						//						SDL_DisplayMode dm;
						//						SDL_GetDesktopDisplayMode(0, &dm);
						int w, h;
						GetWindowSize(&w, &h);
						//		fContext->SetSize(w, h);
					}
					break;
				}
				case SDL_WINDOWEVENT_MOVED:
				{
					if (evt.window.windowID == SDL_GetWindowID(fWindow) && IsHomeScreen(GetAppName()))
					{
						fConfig["x"] = evt.window.data1;
						fConfig["y"] = evt.window.data2;
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
			case SDL_FINGERUP:
			case SDL_FINGERMOTION:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEMOTION:
			case SDL_MOUSEWHEEL:
				// When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
				if (fDlg == NULL && (evt.window.windowID == SDL_GetWindowID(fWindow)) && !io.WantCaptureMouse)
				{
					fMouse->OnEvent(evt, fWindow);
				}
				break;

			case SDL_KEYDOWN:
			{
				// ignore key repeat
				if (evt.key.repeat == 0)
				{
					SDL_Keycode	keycode = evt.key.keysym.sym;
					uint16_t mod = evt.key.keysym.mod;
					bool isNumLockDown = mod & KMOD_NUM ? true : false;
					bool isCapsLockDown = mod & KMOD_CAPS ? true : false;
					bool isShiftDown = mod & KMOD_SHIFT ? true : false;
					bool isCtrlDown = mod & KMOD_CTRL ? true : false;
					bool isAltDown = mod & KMOD_ALT ? true : false;
					bool isCommandDown = mod & KMOD_GUI ? true : false;

					PlatformInputDevice* dev = NULL;
					const char* keyName = GetKeyName(keycode);
					KeyEvent ke(dev, KeyEvent::kDown, keyName, keycode, isShiftDown, isAltDown, isCtrlDown, isCommandDown);
					GetRuntime()->DispatchEvent(ke);
				}
				break;
			}

			case SDL_KEYUP:
			{
				SDL_Keycode	keycode = evt.key.keysym.sym;
				uint16_t mod = evt.key.keysym.mod;
				bool isNumLockDown = mod & KMOD_NUM ? true : false;
				bool isCapsLockDown = mod & KMOD_CAPS ? true : false;
				bool isShiftDown = mod & KMOD_SHIFT ? true : false;
				bool isCtrlDown = mod & KMOD_CTRL ? true : false;
				bool isAltDown = mod & KMOD_ALT ? true : false;
				bool isCommandDown = mod & KMOD_GUI ? true : false;

				PlatformInputDevice* dev = NULL;
				const char* keyName = GetKeyName(keycode);
				KeyEvent ke(dev, KeyEvent::kUp, keyName, keycode, isShiftDown, isAltDown, isCtrlDown, isCommandDown);
				GetRuntime()->DispatchEvent(ke);
				break;
			}

			default:
				SolarEvent(evt);
				break;
			}

			//int advance_time = (int)(Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime()) - start_time);
			//	Rtt_Log("event %x, advance time %d\n", event.type, advance_time);
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

			if (fConsole)
				fConsole->Draw();

			fContext->advance();

			if (fDlg)
				fDlg->Draw();

			int advance_time = (int)(Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime()) - start_time);
			//			Rtt_Log("advance_time %d\n", advance_time);

			// Don't hog the CPU.
			int sleep_time = Max(frameDuration - advance_time, 1.0f);		// sleep for at least 1ms
			this_thread::sleep_for(chrono::milliseconds(sleep_time));
		}
	}

	void SolarApp::Log(const char* buf, int len)
	{
		// truncate
		const int maxsize = 20000;
		if (fLogData.size() > maxsize)
		{
			fLogData.erase(0, (fLogData.size() - maxsize) * 0.9);	// 10%
		}
		fLogData.append(buf, len);
	}

	void SolarApp::RenderGUI()
	{
		if (fImCtx == NULL)
			return;

		SDL_GL_MakeCurrent(fWindow, fGLcontext);
		ImGui::SetCurrentContext(fImCtx);

		// draw GUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		if (IsSuspended())
		{
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			// Always center this window when appearing
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

			if (ImGui::Begin("##Suspended", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs))
			{
				ImGui::Text("Suspended");
				ImGui::End();
			}
		}

		// disable main menu if there is a popup window 
		if (fMenu)
		{
			ImGui::BeginDisabled(fDlg != NULL);
			fMenu->Draw();
			ImGui::EndDisabled();
		}

		for (int i = 0; i < fNativeObjects.size(); i++)
		{
			fNativeObjects[i]->Draw();
		}

		// Activity Indicator
		if (fActivityIndicator)
		{
			DrawActivity();
		}

		ImGui::EndFrame();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void SolarApp::OnIconized()
	{
		fContext->RestartRenderer();
	}

	void SolarApp::SetWindowSize(int w, int h)
	{
		SDL_SetWindowSize(fWindow, w, h + GetMenuHeight());
	}

	void SolarApp::AddDisplayObject(LinuxDisplayObject* obj)
	{
		fNativeObjects.push_back(obj);

		// sanity check
		Rtt_ASSERT(fNativeObjects < 1000);
	}

	void SolarApp::RemoveDisplayObject(LinuxDisplayObject* obj)
	{
		const auto& it = find(fNativeObjects.begin(), fNativeObjects.end(), obj);
		if (it != fNativeObjects.end())
		{
			fNativeObjects.erase(it);
		}
	}

	NativeAlertRef SolarApp::ShowNativeAlert(const char* title, const char* msg, const char** buttonLabels, U32 numButtons, LuaResource* resource)
	{
		fDlg = new DlgAlert(title, msg, buttonLabels, numButtons, resource);
		return NULL;
	}

	//
	// FileWatcher
	//

	FileWatcher::FileWatcher()
		: m_inotify_fd(-1)
		, m_watch_descriptor(-1)
	{
	}

	bool FileWatcher::Start(const string& folder)
	{
		if (m_inotify_fd >= 0)
		{
			// re-entry
			Stop();
		}

		m_inotify_fd = inotify_init();
		if (m_inotify_fd < 0)
		{
			Rtt_LogException("inotify_init failed for %s\n", folder.c_str());
			return false;
		}

		//adding the directory into watch list.
		m_watch_descriptor = inotify_add_watch(m_inotify_fd, folder.c_str(), IN_CREATE | IN_DELETE | IN_MODIFY);
		if (m_watch_descriptor < 0)
		{
			Rtt_LogException("inotify_add_watch failed for %s\n", folder.c_str());
			close(m_inotify_fd);
			m_inotify_fd = -1;
			return false;
		}

		fcntl(m_inotify_fd, F_SETFL, fcntl(m_inotify_fd, F_GETFL) | O_NONBLOCK);

		fThread = new mythread();
		fThread->start([this]() { Watch(); });

		return true;
	}

	void FileWatcher::Stop()
	{
		fThread = NULL;
		if (m_inotify_fd >= 0)
		{
			if (m_watch_descriptor >= 0)
			{
				inotify_rm_watch(m_inotify_fd, m_watch_descriptor);
			}
			close(m_inotify_fd);

			m_inotify_fd = -1;
			m_watch_descriptor = -1;
		}
	}

	FileWatcher::~FileWatcher()
	{
		Stop();
	}

	// thread func
	void FileWatcher::Watch()
	{
		while (fThread && fThread->is_running())
		{
			if (m_inotify_fd >= 0 && m_watch_descriptor >= 0)
			{
				const int EVENT_SIZE = sizeof(struct inotify_event);
				const int EVENT_BUF_LEN = 1024 * (EVENT_SIZE + 16);

				char buffer[EVENT_BUF_LEN];
				int length = read(m_inotify_fd, buffer, EVENT_BUF_LEN);
				if (length < 0)
				{
					int rc = errno;
					switch (rc)
					{
					case EAGAIN:
						length = 0;
						break;
					default:
						Rtt_LogException("failed to read onFileChanged event\n");
						Stop();
						return;
					}
				}

				// actually the read returns the list of change events happens. Here, read the change event one by one and process it accordingly.
				// actually the read returns the list of change events happens. Here, read the change event one by one and process it accordingly.
				int i = 0;
				while (i < length)
				{
					struct inotify_event* event = (struct inotify_event*)&buffer[i];
					if (event->len > 0 && strlen(event->name) > 0)
					{
						SDL_Event e = {};
						e.type = sdl::OnFileSystemEvent;
						e.user.code = event->mask;
						e.user.data1 = strdup(event->name);
						SDL_PushEvent(&e);
					}
					i += EVENT_SIZE + event->len;
				}
			}

			this_thread::sleep_for(chrono::milliseconds(100));
		}
	}

	//
	// Config
	//

	Config::Config()
	{
	}

	Config::Config(const string& path)
	{
		Load(path);
	}

	Config::~Config()
	{
		Save();
	}

	void Config::Load(const string& path)
	{
		fPath = path;		// save
		FILE* f = fopen(path.c_str(), "r");
		if (f)
		{
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
		}
	}

	// load & join & save
	void Config::Save()
	{
		FILE* f = fopen(fPath.c_str(), "w");
		if (f)
		{
			for (const auto& it : fConfig)
			{
				fprintf(f, "%s=%s\n", it.first.c_str(), it.second.c_str());
			}
			fclose(f);
		}
	}

	as_value& Config::operator[](const string& name)
	{
		auto it = fConfig.find(name);
		if (it == fConfig.end())
		{
			fConfig[name] = as_value();
			it = fConfig.find(name);
		}
		return it->second;
	}

	const as_value& Config::operator[](const string& name) const
	{
		static as_value undefined;
		undefined.set_undefined();

		const auto& it = fConfig.find(name);
		if (it == fConfig.end())
			return undefined;
		else
			return it->second;
	}

}	// Rtt
