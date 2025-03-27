//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_Runtime.h"
#include "Rtt_LuaContext.h"
#include "Core/Rtt_Types.h"
#include "Rtt_EmscriptenContext.h"
#include "Rtt_EmscriptenPlatform.h"
#include "Rtt_EmscriptenRuntimeDelegate.h"
#include "Rtt_LuaFile.h"
#include "Core/Rtt_FileSystem.h"
#include "Rtt_Archive.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Rtt_KeyName.h"

#ifdef WIN32
	#define strncasecmp _strnicmp
	#define strcasecmp stricmp
	#define strtok_r strtok_s
	#define snprintf _snprintf
#else
	#define stricmp strcasecmp
#endif

// #define Rtt_DEBUG_TOUCH 1

#if defined(EMSCRIPTEN)
extern "C"
{
	extern int jsContextInit(int fWidth, int fHeight, int fOrientation);
	extern int jsContextGetWindowWidth();
	extern int jsContextGetWindowHeight();
	extern void jsContextUnlockAudio();
	extern void jsContextSyncFS();
	extern void jsContextResizeNativeObjects();
	extern int jsContextMountFS();
	extern int jsContextGetIntModuleItem(const char* name);
	extern int jsContextLoadFonts(const char* name, void* buf, int size);
	extern void jsContextSetClearColor(int r, int g, int b, int a);
	extern void jsContextConfig(int w, int h);
}
#else
	static int appWidth, appHeight;
	int jsContextInit(int w, int h, int fOrientation) { appWidth = w; appHeight = h; return 0; }
	int jsContextGetWindowWidth() { return appWidth; }
	int jsContextGetWindowHeight() { return appHeight; }
	void jsContextUnlockAudio() {}
	void jsContextSyncFS() {}
	void jsContextResizeNativeObjects() {}
	int jsContextMountFS() { return 1; }
	int jsContextGetIntModuleItem(const char* name) { return 1; }
	int jsContextLoadFonts(const char* name, void* buf, int size)  { return 0; }
	void jsContextSetClearColor(int r, int g, int b, int a) {}
	void jsContextConfig(int w, int h) {}
#endif

namespace Rtt
{
	static U64 s_tick = 0;
	void TimerTickShim(void *userdata)
	{
		CoronaAppContext *context = (CoronaAppContext*) userdata;
		float frameDuration = 1.0f / (float) context->getFPS();

		U64 now = Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime());
		if (now - s_tick > frameDuration)		// 60fps ==> 1000/60 = 16.66666 msec
		{
			s_tick = now;
			context->TimerTick();
		}
	}

	MouseListener::MouseListener(Runtime &runtime)
		: fRuntime(runtime)
	{
	}

	//
	//  touch
	//

	void MouseListener::TouchDown(int x, int y, SDL_FingerID fid)
	{
		bool notifyMultitouch = fRuntime.Platform().GetDevice().DoesNotify(MPlatformDevice::kMultitouchEvent);

		// sanity check
		if (fStartPoint.find(fid) != fStartPoint.end() || (notifyMultitouch == false && fStartPoint.size() > 0))
		{
			return;
		}

		fStartPoint[fid] = pt(x, y);

		TouchEvent t((float) x, (float) y, (float) x, (float) y, TouchEvent::kBegan);

		// it must not be ZERO!
		t.SetId((void*) (fid + 1));

#if Rtt_DEBUG_TOUCH
		printf("TouchDown %d(%d, %d)\n", (int) fid, x, y);
#endif

		if (notifyMultitouch)
		{
			MultitouchEvent t2(&t, 1);
			DispatchEvent(t2);
		}
		else
		{
			DispatchEvent(t);
		}
	}

	void		MouseListener::TouchMoved(int x, int y, SDL_FingerID fid)
	{
		bool notifyMultitouch = fRuntime.Platform().GetDevice().DoesNotify(MPlatformDevice::kMultitouchEvent);

#if Rtt_DEBUG_TOUCH
		// Commented out b/c it's very noisy
		//printf("TouchMoved %d(%d, %d)\n", id, x, y);
#endif

		// sanity check
		if (fStartPoint.find(fid) == fStartPoint.end())
		{
			return;
		}

		TouchEvent t((float) x, (float) y, (float) fStartPoint[fid].x, (float) fStartPoint[fid].y, TouchEvent::kMoved);

		// it must not be ZERO!
		t.SetId((void*) (fid + 1));

		if (notifyMultitouch)
		{
			MultitouchEvent t2(&t, 1);
			DispatchEvent(t2);
		}
		else
		{
			DispatchEvent(t);
		}
	}

	void		MouseListener::TouchUp(int x, int y, SDL_FingerID fid)
	{
		bool notifyMultitouch = fRuntime.Platform().GetDevice().DoesNotify(MPlatformDevice::kMultitouchEvent);

		// sanity check
		if (fStartPoint.find(fid) == fStartPoint.end())
		{
			return;
		}

		TouchEvent t((float) x, (float) y, (float) fStartPoint[fid].x, (float) fStartPoint[fid].y, TouchEvent::kEnded);

		// it must not be ZERO!
		t.SetId((void*) (fid + 1));

#if Rtt_DEBUG_TOUCH
		printf( "TouchUp %d(%d, %d)\n", (int) fid, x, y );
#endif

		if (notifyMultitouch)
		{
			MultitouchEvent t2(&t, 1);
			DispatchEvent(t2);
		}
		else
		{
			DispatchEvent(t);
		}

		// Dispatch a "tap" event if mouse button was released in the same position it was pressed in.
		if (fStartPoint[fid].x == x && fStartPoint[fid].y == y)
		{
			Rtt::TapEvent event(x, y, fid + 1);
			DispatchEvent(event);
		}

		fStartPoint.erase(fid);
	}

	void MouseListener::DispatchEvent(const MEvent& e) const
	{
		fRuntime.DispatchEvent(e);
	}

	KeyListener::KeyListener(Runtime &runtime)
		: fRuntime(runtime)
	{
		fKeyName["A"] = KeyName::kA;
		fKeyName["B"] = KeyName::kB;
		fKeyName["C"] = KeyName::kC;
		fKeyName["D"] = KeyName::kD;
		fKeyName["E"] = KeyName::kE;
		fKeyName["F"] = KeyName::kF;
		fKeyName["G"] = KeyName::kG;
		fKeyName["H"] = KeyName::kH;
		fKeyName["I"] = KeyName::kI;
		fKeyName["J"] = KeyName::kJ;
		fKeyName["K"] = KeyName::kK;
		fKeyName["L"] = KeyName::kL;
		fKeyName["M"] = KeyName::kM;
		fKeyName["N"] = KeyName::kN;
		fKeyName["O"] = KeyName::kO;
		fKeyName["P"] = KeyName::kP;
		fKeyName["Q"] = KeyName::kQ;
		fKeyName["R"] = KeyName::kR;
		fKeyName["S"] = KeyName::kS;
		fKeyName["T"] = KeyName::kT;
		fKeyName["U"] = KeyName::kU;
		fKeyName["V"] = KeyName::kV;
		fKeyName["W"] = KeyName::kW;
		fKeyName["X"] = KeyName::kX;
		fKeyName["Y"] = KeyName::kY;
		fKeyName["Z"] = KeyName::kZ;
		fKeyName["0"] = KeyName::k0;
		fKeyName["1"] = KeyName::k1;
		fKeyName["2"] = KeyName::k2;
		fKeyName["3"] = KeyName::k3;
		fKeyName["4"] = KeyName::k4;
		fKeyName["5"] = KeyName::k5;
		fKeyName["6"] = KeyName::k6;
		fKeyName["7"] = KeyName::k7;
		fKeyName["8"] = KeyName::k8;
		fKeyName["9"] = KeyName::k9;
		fKeyName["Keypad 0"] = KeyName::kNumPad0;
		fKeyName["Keypad 1"] = KeyName::kNumPad1;
		fKeyName["Keypad 2"] = KeyName::kNumPad2;
		fKeyName["Keypad 3"] = KeyName::kNumPad3;
		fKeyName["Keypad 4"] = KeyName::kNumPad4;
		fKeyName["Keypad 5"] = KeyName::kNumPad5;
		fKeyName["Keypad 6"] = KeyName::kNumPad6;
		fKeyName["Keypad 7"] = KeyName::kNumPad7;
		fKeyName["Keypad 8"] = KeyName::kNumPad8;
		fKeyName["Keypad 9"] = KeyName::kNumPad9;
		fKeyName["Keypad /"] = KeyName::kNumPadDivide;
		fKeyName["Keypad *"] = KeyName::kNumPadMultiply;
		fKeyName["Keypad -"] = KeyName::kNumPadSubtract;
		fKeyName["Keypad +"] = KeyName::kNumPadAdd;
		fKeyName["Keypad Enter"] = KeyName::kNumPadEnter;
		fKeyName["Keypad ,"] = KeyName::kNumPadComma;
		fKeyName["Keypad ."] = KeyName::kNumPadPeriod;
		fKeyName["Keypad ("] = KeyName::kNumPadLeftParentheses;
		fKeyName["Keypad )"] = KeyName::kNumPadRightParentheses;
		fKeyName["Left Alt"] = KeyName::kLeftAlt;
		fKeyName["Right Alt"] = KeyName::kRightAlt;
		fKeyName["Left Ctrl"] = KeyName::kLeftControl;
		fKeyName["Right Ctrl"] = KeyName::kRightControl;
		fKeyName["Left Shift"] = KeyName::kLeftShift;
		fKeyName["Right Shift"] = KeyName::kRightShift;
		fKeyName["Left Windows"] = KeyName::kLeftCommand;
		fKeyName["Right Windows"] = KeyName::kRightCommand;
		fKeyName["Left GUI"] = KeyName::kLeftCommand;		// web
		fKeyName["Right GUI"] = KeyName::kRightCommand;		// web
		fKeyName["'"] = KeyName::kApostrophe;
		fKeyName["Backspace"] = KeyName::kDeleteBack; //kBack;
		fKeyName["Home"] = KeyName::kHome;
		fKeyName["/"] = KeyName::kForwardSlash;
		fKeyName["\\"] = KeyName::kBackSlash;
		fKeyName["Numlock"] = KeyName::kNumLock;
		fKeyName["ScrollLock"] = KeyName::kScrollLock;
		fKeyName["Pause"] = KeyName::kMediaPause;
		fKeyName["Up"] = KeyName::kUp;
		fKeyName["Down"] = KeyName::kDown;
		fKeyName["Left"] = KeyName::kLeft;
		fKeyName["Right"] = KeyName::kRight;
		fKeyName["End"] = KeyName::kEnd;
		fKeyName["PageUp"] = KeyName::kPageUp;
		fKeyName["PageDown"] = KeyName::kPageDown;
		fKeyName["Insert"] = KeyName::kInsert;
		fKeyName["Delete"] = KeyName::kDeleteForward;
		fKeyName["="] = KeyName::kPlus;
		fKeyName["-"] = KeyName::kMinus;
		fKeyName["F1"] = KeyName::kF1;
		fKeyName["F2"] = KeyName::kF2;
		fKeyName["F3"] = KeyName::kF3;
		fKeyName["F4"] = KeyName::kF4;
		fKeyName["F5"] = KeyName::kF5;
		fKeyName["F6"] = KeyName::kF6;
		fKeyName["F7"] = KeyName::kF7;
		fKeyName["F8"] = KeyName::kF8;
		fKeyName["F9"] = KeyName::kF9;
		fKeyName["F10"] = KeyName::kF10;
		fKeyName["F11"] = KeyName::kF11;
		fKeyName["F12"] = KeyName::kF12;
		fKeyName["F13"] = KeyName::kF13;
		fKeyName["F14"] = KeyName::kF14;
		fKeyName["F15"] = KeyName::kF15;
		fKeyName["F16"] = KeyName::kF16;
		fKeyName["F17"] = KeyName::kF17;
		fKeyName["F18"] = KeyName::kF18;
		fKeyName["F19"] = KeyName::kF19;
		fKeyName["F20"] = KeyName::kF20;
		fKeyName["F21"] = KeyName::kF21;
		fKeyName["F22"] = KeyName::kF22;
		fKeyName["F23"] = KeyName::kF23;
		fKeyName["F24"] = KeyName::kF24;
		fKeyName["`"] = "`";
		fKeyName[";"] = ";";
		fKeyName[","] = ",";
		fKeyName["."] = KeyName::kPeriod;
		fKeyName["["] = KeyName::kLeftBracket;
		fKeyName["]"] = KeyName::kRightBracket;
		fKeyName["Tab"] = KeyName::kTab;
		fKeyName["Return"] = KeyName::kEnter;
		fKeyName["CapsLock"] = KeyName::kCapsLock;
		fKeyName["Escape"] = KeyName::kEscape;
		fKeyName["PrintScreen"] = KeyName::kPrintScreen;
		fKeyName["VolumeUp"] = KeyName::kVolumeUp;
		fKeyName["VolumeDown"] = KeyName::kVolumeDown;
		fKeyName["VolumeMute"] = KeyName::kVolumeMute;
		fKeyName["ZoomIn"] = KeyName::kZoomIn;
		fKeyName["ZoomOut"] = KeyName::kZoomOut;
		fKeyName["Break"] = KeyName::kBreak;
		fKeyName["Menu"] = KeyName::kMenu;
		fKeyName["Application"] = KeyName::kMenu;		// web
		fKeyName["Space"] = KeyName::kSpace;
	}

	void KeyListener::notifyKeyEvent(const SDL_Event& e, bool down)
	{
		Uint16 mod = e.key.keysym.mod;
		bool isNumLockDown = mod & KMOD_NUM ? true : false;
		bool isCapsLockDown = mod & KMOD_CAPS ? true : false;
		bool isShiftDown = mod & KMOD_SHIFT ? true : false;
		bool isCtrlDown = mod & KMOD_CTRL ? true : false;
		bool isAltDown = mod & KMOD_ALT ? true : false;
		bool isCommandDown = false;		//		not supported by SDL

		SDL_Keycode	key = e.key.keysym.sym;
		S32 nativeKeyCode = key;
		PlatformInputDevice *dev = NULL;

		const char* sdlKeyName = SDL_GetKeyName(key);
		auto it = fKeyName.find(sdlKeyName);
		const char* keyName = it == fKeyName.end() ? KeyName::kUnknown : it->second.c_str();

		KeyEvent ke(dev, down ? KeyEvent::kDown : KeyEvent::kUp, keyName, nativeKeyCode, isShiftDown, isAltDown, isCtrlDown, isCommandDown);
		fRuntime.DispatchEvent(ke);
	}

	CoronaAppContext::CoronaAppContext(const char* pathToApp)
		: fRuntime(NULL)
		, fRuntimeDelegate(new EmscriptenRuntimeDelegate())
		, fMouseListener(NULL)
		, fKeyListener(NULL)
		, fPathToApp(pathToApp)
		, fDocumentsDir(pathToApp)
		, fPlatform(NULL)
		, fWidth(320)
		, fHeight(480)
		, fOrientation(DeviceOrientation::kUpright)
		, fAppState(MOUNT_IDBFS)
		, fWindow(NULL)
		, fMode("maximized")
	{
#ifdef EMSCRIPTEN
		fPathToApp = "/";
#endif
		fDocumentsDir += DIR_SEPARATOR;
		fDocumentsDir += "documentsDir";

		// for debugging
#ifdef _WIN32
		_mkdir(fDocumentsDir.c_str());
#endif
	}

	CoronaAppContext::~CoronaAppContext()
	{
		delete fMouseListener;
		delete fKeyListener;
		delete fRuntime;
		delete fRuntimeDelegate;
		delete fPlatform;

		SDL_Quit();
	}

	bool		CoronaAppContext::Initialize()
	{
		// Initialize the SDL library with the Video subsystem
		int rc = SDL_Init(SDL_INIT_VIDEO);
		if (rc != 0)
		{
			Rtt_LogException("SDL_Init: %s(%d)\n", SDL_GetError(), rc);
		}

		SDL_version compiled;
		SDL_version linked;
		SDL_VERSION(&compiled);
		SDL_GetVersion(&linked);

		printf("SDL compiled version %d.%d.%d, ", compiled.major, compiled.minor, compiled.patch);
		printf("linked version %d.%d.%d\n", linked.major, linked.minor, linked.patch);

#if defined(EMSCRIPTEN)
		emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, blurCallback);
		emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, focusCallback);
		emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, false, resizeCallback);
		emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, mouseupCallback);		// for OSX
		emscripten_set_touchend_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, true, touchCallback);		// for iOS
		emscripten_set_beforeunload_callback(this, beforeunloadCallback);

		// resourceDir, documentsDir, temporaryDir,	cachesDir, systemCachesDir 
		fPlatform = new EmscriptenPlatform(fPathToApp.c_str(), fDocumentsDir.c_str(), fPathToApp.c_str(), fPathToApp.c_str(), fPathToApp.c_str());
#else
		fPlatform = new EmscriptenPlatformWin(fPathToApp.c_str(), fDocumentsDir.c_str(), fPathToApp.c_str(), fPathToApp.c_str(), fPathToApp.c_str());
#endif

		fRuntime = new EmscriptenRuntime(*fPlatform, NULL);
		fRuntime->SetDelegate(fRuntimeDelegate);
		fRuntime->SetProperty(Runtime::kEmscriptenMaskSet, true);

		// For debugging, use main.lua if it exists in the app folder
		{
			std::string main_lua = fPathToApp.c_str();
			main_lua += DIR_SEPARATOR;
			main_lua += "main.lua";
			FILE* fi = fopen(main_lua.c_str(), "r");
			if (fi)
			{
				fclose(fi);
				fRuntime->SetProperty(Runtime::kEmscriptenMaskSet | Runtime::kIsApplicationNotArchived | Runtime::kShouldVerifyLicense, true);
			}
		}

		// settings

		std::string orientation;
		std::string title;

		int w = 0;
		int h = 0;
		fRuntime->readSettings(&w, &h, &orientation, &title, &fMode);

		// get JS window size
		int jsWindowWidth = jsContextGetWindowWidth();
		int jsWindowHeight = jsContextGetWindowHeight();

		if (orientation == "landscapeRight")
		{
			fOrientation = DeviceOrientation::kSidewaysRight;	// bottom of device is to the right
			if (w > 0 && h > 0)
			{
				fWidth = w;
				fHeight = h;
			}
			else
			{
				// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
				// use swapped default settings 
				Swap(fWidth, fHeight);
			}
		}
		else if (orientation == "landscapeLeft")
		{
			fOrientation = DeviceOrientation::kSidewaysLeft; 	// bottom of device is to the left
			if (w > 0 && h > 0)
			{
				fWidth = w;
				fHeight = h;
			}
			else
			{
				// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
				// use swapped default settings 
				Swap(fWidth, fHeight);
			}
		}
		else if (orientation == "portrait")
		{
			fOrientation = DeviceOrientation::kUpright;	// bottom of device is at the bottom
			if (w > 0 && h > 0)
			{
				fWidth = w;
				fHeight = h;
			}
			else
			{
				// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
				// use default settings 
			}
		}
		else if (orientation == "portraitUpsideDown")
		{
			fOrientation = DeviceOrientation::kUpsideDown;	// bottom of device is at the top
			if (w > 0 && h > 0)
			{
				fWidth = w;
				fHeight = h;
			}
			else
			{
				// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
				// use default settings 
			}
		}
		else
		{
			//Rtt_LogException("Unsupported orientation: '%s'", orientation.c_str());
		}

		jsContextInit(fWidth, fHeight, fOrientation);
		if (fMode == "maximized" || fMode == "fullscreen")
		{
			//Scale double
			float scaleX = (float)(jsWindowWidth * 2) / (float)(fWidth);
			float scaleY = (float)(jsWindowHeight * 2) / (float)(fHeight);
			float scale = fmin(scaleX, scaleY);				// keep ratio
			fWidth *= scale;
			fHeight *= scale;
		}

		Uint32 flags = SDL_WINDOW_OPENGL;
		//flags |= (fMode == "fullscreen") ?  SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_RESIZABLE;
		flags |= SDL_WINDOW_RESIZABLE;
		fWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, fWidth, fHeight, flags);
		SDL_GL_CreateContext(fWindow);
		fPlatform->setWindow(fWindow, fOrientation);

#if defined(EMSCRIPTEN)
		// Tell it to use OpenGL version 2.0
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
#endif

#if defined(Rtt_EMSCRIPTEN_ENV) && defined( WIN32 )
		glewExperimental = GL_TRUE;
		glewInit();
#endif

		fMouseListener = new MouseListener(*fRuntime);
		fKeyListener = new KeyListener(*fRuntime);

		if (Runtime::kSuccess != fRuntime->LoadApplication(Runtime::kHTML5LaunchOption, fOrientation)) 
		{
			delete fRuntime;
			delete fPlatform;
			return false;
		}

		// pass config.lua to JS
		if (orientation == "landscapeRight" || orientation == "landscapeLeft")
		{
			Swap(fRuntimeDelegate->fContentWidth, fRuntimeDelegate->fContentHeight);
		}
		jsContextConfig(fRuntimeDelegate->fContentWidth, fRuntimeDelegate->fContentHeight);

		fRuntime->BeginRunLoop();

		DisplayDefaults& defaults = fRuntime->GetDisplay().GetDefaults();
		ColorUnion c;
		c.pixel = defaults.GetClearColor();
		jsContextSetClearColor(c.rgba.r, c.rgba.g, c.rgba.b, c.rgba.a);

		// hack
#ifdef EMSCRIPTEN
		if ((stricmp(fRuntimeDelegate->fScaleMode.c_str(), "zoomStretch") == 0) || (stricmp(fRuntimeDelegate->fScaleMode.c_str(), "zoomEven") == 0))
		{
			EM_ASM_INT({	window.dispatchEvent(new Event('resize')); });
		}

		emscripten_set_element_css_size("canvas", fWidth / 2, fHeight / 2);
#endif

		return true;
	}

#if defined(EMSCRIPTEN)

	// iOS Web Audio Unlocker
	int CoronaAppContext::mouseupCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void * userData) 
	{
		CoronaAppContext* ctx = (CoronaAppContext*) userData;
		jsContextUnlockAudio();

		// request fullScreen from mouse gesture
		static bool isFirstTime = true;
		if (ctx->fMode == "fullscreen" && isFirstTime)
		{
			//printf("requestFullscreen: mode=%s, ctx=%p\n", ctx->fMode.c_str(), ctx);
			isFirstTime = false;
			ctx->requestFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT); 
		}
	  return 0;
	}

	int CoronaAppContext::touchCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
	{
		return mouseupCallback(0, 0, 0);
	}

	int CoronaAppContext::blurCallback(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData)
	{
		// check event target, ingnore all events except #window
		if (*focusEvent->id == 0)		// event from #window ?
		{
			CoronaAppContext* ctx = (CoronaAppContext*) userData;
			ctx->pause();
		}
	  return 0;
	}

	int CoronaAppContext::focusCallback(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData)
	{
		// check event target, ingnore all events except #window
		if (*focusEvent->id == 0)		// event from #window ?
		{
			CoronaAppContext* ctx = (CoronaAppContext*) userData;
			ctx->resume();
		}
		return 0;
	}

	int CoronaAppContext::resizeCallback(int eventType, const EmscriptenUiEvent *uiEvent, void *userData)
	{
		SDL_Event sdlevent;
		sdlevent.type = SDL_WINDOWEVENT;
		sdlevent.window.data1 = uiEvent->windowInnerWidth;
		sdlevent.window.data2 = uiEvent->windowInnerHeight;
		sdlevent.window.windowID = 0;
		sdlevent.window.event = SDL_WINDOWEVENT_RESIZED;
		SDL_PushEvent(&sdlevent);
		return 0;
	}

	const char* CoronaAppContext::beforeunloadCallback(int eventType, const void *reserved, void *userData)
	{
		jsContextSyncFS();

		CoronaAppContext* ctx = (CoronaAppContext*) userData;
		Rtt::jsSystemEvent ev("applicationExit");
		ctx->GetRuntime()->DispatchEvent(ev);
		return NULL;
	}

#endif

	void CoronaAppContext::Start()
	{
#if defined(EMSCRIPTEN)
		emscripten_set_main_loop_arg(&TimerTickShim, this, 0, 1); // Never returns
#else
		bool closeApp = false;
		while (closeApp == false)
		{
			closeApp = TimerTick();
			SDL_Delay(30);		// hack, 30FPS for debugging
		}
#endif
	}

	void CoronaAppContext::pause()
	{
		if (fRuntime->IsSuspended() == false)
		{
			fRuntime->Suspend();
		}
	}

	void CoronaAppContext::resume()
	{
		if (GetRuntime()->IsSuspended())
		{
			fRuntime->DispatchEvent(ResizeEvent());
			fRuntime->Resume();
		}
	}

	void CoronaAppContext::requestFullscreen(int scaleMode, int canvasResolutionScaleMode, int filteringMode)
	{
#ifdef EMSCRIPTEN
		EmscriptenFullscreenStrategy s;
		memset(&s, 0, sizeof(s));
		s.scaleMode = scaleMode;
		s.canvasResolutionScaleMode = canvasResolutionScaleMode;
		s.filteringMode = filteringMode;
		EMSCRIPTEN_RESULT ret = emscripten_request_fullscreen_strategy("emscripten_border", 1, &s);
#endif
	}

	void CoronaAppContext::enterSoftFullscreen(int scaleMode, int canvasResolutionScaleMode, int filteringMode)
	{
#ifdef EMSCRIPTEN
		EmscriptenFullscreenStrategy s;
		memset(&s, 0, sizeof(s));
		s.scaleMode = scaleMode;
		s.canvasResolutionScaleMode = canvasResolutionScaleMode;
		s.filteringMode = filteringMode;
		EMSCRIPTEN_RESULT ret = emscripten_enter_soft_fullscreen("emscripten_border", &s);
#endif
	}

	bool 	CoronaAppContext::ProcessEvent(SDL_Event& event)
	{
		//printf("sdl event %X, %s\n", event.type);
		switch (event.type)
		{
		case SDL_FINGERDOWN:
		{
			int w, h;
			SDL_GetWindowSize(fWindow, &w, &h);
			SDL_TouchFingerEvent &ef = event.tfinger;
			GetMouseListener()->TouchDown(w * ef.x, h * ef.y, ef.fingerId);
			break;
		}
		case SDL_FINGERUP:
		{
			int w, h;
			SDL_GetWindowSize(fWindow, &w, &h);
			SDL_TouchFingerEvent &ef = event.tfinger;
			GetMouseListener()->TouchUp(w * ef.x, h * ef.y, ef.fingerId);
			break;
		}
		case SDL_FINGERMOTION:
		{
			int w, h;
			SDL_GetWindowSize(fWindow, &w, &h);
			SDL_TouchFingerEvent &ef = event.tfinger;
			GetMouseListener()->TouchMoved(w * ef.x, h * ef.y, ef.fingerId);
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			const SDL_MouseButtonEvent& b = event.button;
			if (b.which != SDL_TOUCH_MOUSEID)
			{
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

				fRuntime->DispatchEvent(mouseEvent);
				GetMouseListener()->TouchDown(x, y, 0);
				break;
			}
		}

		case SDL_MOUSEMOTION:
		{
			const SDL_MouseButtonEvent& b = event.button;
			if (b.which != SDL_TOUCH_MOUSEID)
			{
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

				fRuntime->DispatchEvent(mouseEvent);
				GetMouseListener()->TouchMoved(x, y, 0);
				break;
			}
		}

		case SDL_MOUSEBUTTONUP:
		{
			const SDL_MouseButtonEvent& b = event.button;
			if (b.which != SDL_TOUCH_MOUSEID)
			{
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

				fRuntime->DispatchEvent(mouseEvent);
				GetMouseListener()->TouchUp(x, y, 0);
				break;
			}
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

				fRuntime->DispatchEvent(mouseEvent);
				break;
			}
		}
		case SDL_KEYDOWN:
		{
			SDL_Keycode	key = event.key.keysym.sym;
			if (key == SDLK_ESCAPE)
			{
				event.type = SDL_QUIT;
				return true;		// close app
			}
			
			// ignore key repeat
			if (event.key.repeat == 0)
			{
				GetKeyListener()->notifyKeyEvent(event, true);
			}
			break;
		}
		case SDL_KEYUP:
		{
			// ignore key repeat
			if (event.key.repeat == 0)
			{
				GetKeyListener()->notifyKeyEvent(event, false);
			}
			break;
		}

		case SDL_TEXTINPUT:
		{
			// ignore key repeat
			if (event.key.repeat == 0)
			{
				const char* a = event.text.text;
			}
			break;
		}

		case SDL_WINDOWEVENT:
			//SDL_Log("SDL_WINDOWEVENT %x %x", event.window.event);
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_SHOWN:
				//SDL_Log("Window %d shown", event.window.windowID);
				resume();
				break;
			case SDL_WINDOWEVENT_HIDDEN:
			{
				//SDL_Log("Window %d hidden", event.window.windowID);
				pause();
				break;
			}
			case SDL_WINDOWEVENT_EXPOSED:
				//SDL_Log("Window %d exposed", event.window.windowID);
				break;
			case SDL_WINDOWEVENT_MOVED:
				//SDL_Log("Window %d moved to %d,%d", event.window.windowID, event.window.data1, event.window.data2);
				break;
			case SDL_WINDOWEVENT_RESIZED:
			{
				bool fullScreen = false;
#ifdef EMSCRIPTEN
				fullScreen = EM_ASM_INT({
					var fullscreenElement = document.fullscreenElement || document.mozFullScreenElement || document.webkitFullscreenElement || document.msFullscreenElement;
					return fullscreenElement != null ? true: false;
				});
#endif
				//SDL_Log("Window %d resized to %dx%d", event.window.windowID, event.window.data1, event.window.data2);
				// resize only for 'maximized' to fill fit browers's window
				if (fullScreen == false && (fMode == "maximized" || fMode == "fullscreen"))
//				if (fullScreen == false && fMode == "maximized")
				{
					int w = event.window.data1;
					int h = event.window.data2;

					//Fix error zoom
					if (w == 0 || h == 0) 
					{
						w = jsContextGetWindowWidth();
						h = jsContextGetWindowHeight();
					}

					// keep ratio
					float scaleX = (w * 2) / (float)fWidth;
					float scaleY = (h * 2) / (float)fHeight;

					float scale = fmin(scaleX, scaleY);
					if ((stricmp(fRuntimeDelegate->fScaleMode.c_str(), "zoomStretch") == 0) || (stricmp(fRuntimeDelegate->fScaleMode.c_str(), "zoomEven") == 0))
					{
						w = fWidth * scaleX;
						h = fHeight * scaleY;
					}
					else
					{
						w = fWidth * scale;
						h = fHeight * scale;
					}

					SDL_SetWindowSize(fWindow, w, h);

					fRuntime->WindowSizeChanged();
					fRuntime->RestartRenderer(fOrientation);
					fRuntime->GetDisplay().Invalidate();

					fRuntime->DispatchEvent(ResizeEvent());
				
#ifdef EMSCRIPTEN
					
					emscripten_set_element_css_size("canvas", w / 2, h / 2);			
#endif
				}
				else 
				{
#ifdef EMSCRIPTEN
					emscripten_set_element_css_size("canvas", fWidth / 2, fHeight / 2);
#endif
				}

				// refresh native elements
				jsContextResizeNativeObjects();
				break;
			}
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				//SDL_Log("Window %d size changed to %dx%d", event.window.windowID, event.window.data1, event.window.data2);
				break;
			case SDL_WINDOWEVENT_MINIMIZED:
			{
				//SDL_Log("Window %d minimized", event.window.windowID);
				pause();
				break;
			}
			case SDL_WINDOWEVENT_MAXIMIZED:
				//SDL_Log("Window %d maximized", event.window.windowID);
				break;
			case SDL_WINDOWEVENT_RESTORED:
			{
				// when this occurs, openGL is reinitialised, and all of the textures that were loaded on program start are cleared and re-loaded.
				//SDL_Log("Window %d restored", event.window.windowID);
				resume();
				break;
			}
			case SDL_WINDOWEVENT_ENTER:
				//SDL_Log("Mouse entered window %d", event.window.windowID);
				break;
			case SDL_WINDOWEVENT_LEAVE:
				//SDL_Log("Mouse left window %d", event.window.windowID);
				break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				//SDL_Log("Window %d gained keyboard focus", event.window.windowID);
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				//SDL_Log("Window %d lost keyboard focus", event.window.windowID);
				break;
			case SDL_WINDOWEVENT_CLOSE:
				//SDL_Log("Window %d closed", event.window.windowID);
				break;
			case SDL_WINDOWEVENT_TAKE_FOCUS:
				//SDL_Log("Window %d is offered a focus", event.window.windowID);
				break;
			case SDL_WINDOWEVENT_HIT_TEST:
				//SDL_Log("Window %d has a special hit test", event.window.windowID);
				break;
			default:
				//SDL_Log("Window %d got unknown event %d", event.window.windowID, event.window.event);
				break;
			}
			break;

		default:
			break;
		}
		return false;
	}

	void CoronaAppContext::enumerateFontFiles(const char* dir, std::vector<std::string>& files)
	{
		std::vector<std::string> fileList = Rtt_ListFiles(dir);
		for (int i = 0; i < fileList.size(); i++)
		{
			const std::string& name = fileList[i];
			if (Rtt_IsDirectory(name.c_str()) && name != "//proc" && name != "//dev" && name != "//tmp" && name != "//home")		// hack: proc, dev, tmp, home
			{
				enumerateFontFiles(name.c_str(), files);
			}
			else
			{
				int len = name.size();
				if (len > 4 && (strcmp(name.c_str() + len - 4, ".ttf") == 0) || (strcmp(name.c_str() + len - 4, ".otf") == 0))
				{
					files.push_back(name);
				}
			}
		}
	}

	bool CoronaAppContext::TimerTick()
	{
		switch (fAppState)
		{
		case MOUNT_IDBFS:
		{
			// mount IDBFS and sync it
			int rc = jsContextMountFS();
			fAppState = (rc == 1) ? WAIT_FOR_IDBFS_MOUNTED : LOAD_FONTS;
			break;
		}

		case WAIT_FOR_IDBFS_MOUNTED:
		{
			// wait for documentsDir mounted
			int rc = jsContextGetIntModuleItem("documentsDirLoaded");
			if (rc == 1)
			{
				fAppState = COPY_DOCS;
			}
			break;
		}

		case COPY_DOCS:
		{
			// check for first start
			std::string installed = fDocumentsDir;
			installed += DIR_SEPARATOR;
			installed += ".installed";
			if (Rtt_FileExists(installed.c_str()))
			{
				// app already installed
				fAppState = LOAD_FONTS;
			}
			else
			{
				//Rtt_Log("First time start\n");
				FILE* f = fopen(installed.c_str(), "w");
				if (f)
				{
					// create .installed file, it serves as marker
					fclose(f);

					// copy databases
					std::vector<std::string> files = Rtt_ListFiles(fPathToApp.c_str());
					//Rtt_Log("Total file count in the App: %d\n", files.size());

					int copiedFiles = 0;
					for (unsigned int i = 0; i < files.size(); i++)
					{
						int k = files[i].find(".db");
						if (k > 0 && k == files[i].size() - 3)
						{
							int begin = files[i].rfind(DIR_SEPARATOR);
							if (begin < 0)
							{
								// take whole string
								begin = 0;
							}
							std::string fileName = files[i].c_str() + begin;

							// copy to permanent memory
							std::string dst = fDocumentsDir;
							dst += fileName;
							Rtt_CopyFile(files[i].c_str(), dst.c_str());
							copiedFiles++;

							//Rtt_Log("Creating sandbox: %s to %s\n", files[i].c_str(), dst.c_str());
						}
					}
				
					if (copiedFiles > 0)
					{
						jsContextSyncFS();
						fAppState = WAIT_FOR_SYNC;
					}
					else
					{
						// no database files, so nothing to sync, just goto run app
						//Rtt_Log("Nothing to sync\n");
						fAppState = LOAD_FONTS;
					}
				}
				else
				{
					Rtt_LogException("Failed to create  %s file\n", installed.c_str());
				}
			}
			break;
		}

		case WAIT_FOR_SYNC:
		{
			// wait for documentsDir mounted
			int rc = jsContextGetIntModuleItem("idbfsSynced");
			if (rc == 1)
			{
				//Rtt_Log("Syncing ended\n");
				fAppState = LOAD_FONTS;
			}
			break;
		}

		case LOAD_FONTS:
		{
			int loadingFonts = 0;

			// Enumerate .ttf font files
			std::vector<std::string> fileList;
			enumerateFontFiles(fPathToApp.c_str(), fileList);

			for (int i = 0; i < fileList.size(); i++)
			{
				const std::string& name = fileList[i];
				FILE* fi = fopen(name.c_str(), "rb");
				if (fi)
				{
					fseek(fi, 0, SEEK_END);
					int size = ftell(fi);
					fseek(fi, 0, SEEK_SET);
					void* buf = malloc(size);
					fread(buf, 1, size, fi);
					fclose(fi);

					loadingFonts += jsContextLoadFonts(name.c_str(), buf, size);

					free(buf);
				}
			}
			fAppState = loadingFonts > 0 ? WAIT_FOR_FONTS : INIT_APP;
			break;
		}
		case WAIT_FOR_FONTS:
		{
#ifndef WIN32
			int rc = jsContextGetIntModuleItem("loadingFonts");
			if (rc == 0)
			{
				fAppState = INIT_APP;
			}
#else
				fAppState = INIT_APP;
#endif
			break;
		}
		case INIT_APP:
			Initialize();
			fAppState = RUN_APP;
			break;

		case RUN_APP:
		{
			// main loop
			SDL_Event event;
			bool closeApp = false;
			while (SDL_PollEvent(&event) && closeApp == false)
			{
				closeApp = ProcessEvent(event);
			}

			if (fRuntime->IsSuspended() == false)
			{
				(*fRuntime)();
			}

			return closeApp;
		}
		default:
			Rtt_ASSERT(0);
		}
		return false;
	}

	bool EmscriptenRuntime::readTable(lua_State *L, const char* table, int* w, int* h, std::string* title, std::string* mode) const
	{
		bool rc = false;
		int top = lua_gettop(L);

		lua_getfield(L, -1, table);
		if (lua_istable(L, -1))
		{
			rc = true;
			lua_getfield(L, -1, "defaultViewWidth");
			if ((!lua_isnil(L, -1)) && (lua_isnumber(L, -1)))
			{
				*w = lua_tointeger(L, -1);
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "defaultViewHeight");
			if ((!lua_isnil(L, -1)) && (lua_isnumber(L, -1)))
			{
				*h = lua_tointeger(L, -1);
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "defaultMode");
			if ((!lua_isnil(L, -1)) && (lua_isstring(L, -1)))
			{
				*mode = lua_tostring(L, -1);
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "titleText");
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "default");
				if ((!lua_isnil(L, -1)) && (lua_isstring(L, -1)))
				{
					*title = lua_tostring(L, -1);
				}
				lua_pop(L, 1);		// remove default
			}
			lua_pop(L, 1);		// remove titleText
		}

		lua_settop(L, top);
		return rc;
	}


	void EmscriptenRuntime::readSettings(int* w, int* h, std::string* orientation, std::string* title, std::string* mode)
	{
		Rtt_ASSERT(w != NULL && h != NULL);

		lua_State *L = VMContext().L();
		int status = 0;
		const MPlatform& p = Platform();
		int top = lua_gettop(L);

		//
		// read build.settings
		//
		const char kBuildSettings[] = "build.settings";
		String filePath(&p.GetAllocator());
		p.PathForFile(kBuildSettings, MPlatform::kResourceDir, MPlatform::kTestFileExists, filePath);
		const char *path = filePath.GetString();
		if (path)
		{
			status = VMContext().DoFile(path, 0, true);
		}
		else
		{
			// other cases assume a non-zero status means there's an error msg on the stack
			// so push a "fake" error msg on the stack so we are consistent with those cases
			lua_pushnil(L);
		}
		lua_pop(L, 1);		// remove DoFile result

		if (status == 0)
		{
			lua_getglobal(L, "settings"); // settings
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "orientation"); // settings.orientation
				if (lua_istable(L, -1))
				{
					lua_getfield(L, -1, "default");
					if ((!lua_isnil(L, -1)) && (lua_isstring(L, -1)))
					{
						*orientation = lua_tostring(L, -1);
					}
					lua_pop(L, 1);
				}
				lua_pop(L, 1);		// remove orientation

				// first try settings from 'web' table
				if (readTable(L, "web", w, h, title, mode) == false)
				{
					// next try settings from 'html5' table
					if (readTable(L, "html5", w, h, title, mode) == false)
					{
						// next try settings from 'window' table
						readTable(L, "window", w, h, title, mode);
					}
				}
			}
			lua_pop(L, 1);		// remove settings
		}
		else
		{
			Rtt_LogException("No build.settings file\n");
		}
		lua_settop(L, top);
	}

	int jsSystemEvent::Push( lua_State *L ) const
	{
		if ( Rtt_VERIFY( VirtualEvent::Push( L ) ) )
		{
			lua_pushstring( L, fEventName.c_str() );
			lua_setfield( L, -2, kTypeKey );
		}
		return 1;
	}

}

