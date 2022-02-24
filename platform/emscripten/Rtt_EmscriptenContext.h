//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <string>
#include <map>
#include "Rtt_Event.h"
#include "Core/Rtt_Types.h"
#include "Rtt_Runtime.h"
#include "Rtt_EmscriptenRuntimeDelegate.h"
#include "Core/Rtt_Math.h"
#include "Core/Rtt_Array.h"

#include <SDL2/SDL.h>

#if defined(EMSCRIPTEN)
#include "emscripten/emscripten.h"
#include "emscripten/html5.h"
#else
#include "Rtt_EmscriptenPlatformWin.h"
#include <GL/glew.h>
#include "Rtt_EmscriptenPlatformWin.h"
#endif


#pragma once

#if defined(EMSCRIPTEN)
	#define DIR_SEPARATOR "/"
#else
	#define DIR_SEPARATOR "\\"
#endif

namespace Rtt
{
	class EmscriptenPlatform;

	class EmscriptenRuntime : public Runtime
	{
	public:
			EmscriptenRuntime( const MPlatform& platform, MCallback *viewCallback = NULL )
				: Runtime(platform, viewCallback)
			{
			}

			void readSettings(int* w, int* h, std::string* orientation, std::string* title, std::string* mode);
			bool readTable(lua_State *L, const char* name, int* w, int* h, std::string* title, std::string* mode) const;
	};

	class KeyListener
	{
	public:
		KeyListener(Runtime &runtime);

		void notifyKeyEvent(const SDL_Event& e, bool down);

	private:
		Runtime& fRuntime;
		std::map<std::string, std::string> fKeyName;	
	};

	class MouseListener
	{
	public:
		MouseListener(Runtime &runtime);

		void TouchDown(int x, int y, SDL_FingerID id);
		void TouchMoved(int x, int y, SDL_FingerID id);
		void TouchUp(int x, int y, SDL_FingerID id);
		void DispatchEvent(const MEvent& e) const;

	private:

		struct pt
		{
			pt() : x(0), y(0) {}
			pt(int xx, int yy) : x(xx), y(yy) {}
			int x;
			int y;
		};

		Runtime& fRuntime;
		std::map<SDL_FingerID, pt> fStartPoint;
	};


	// Immediately broadcast to "Runtime"
	class jsSystemEvent : public VirtualEvent
	{
		public:
			jsSystemEvent(const char* e)
				: fEventName(e)
			{}

			virtual const char* Name() const { return "system"; }
			virtual int Push( lua_State *L ) const;

	private:
		std::string fEventName;
			 
	};

	struct CoronaAppContext
	{
		CoronaAppContext(const char* pathToApp);
		~CoronaAppContext();
			
		enum AppState
		{
			MOUNT_IDBFS,
			WAIT_FOR_IDBFS_MOUNTED,
			RUN_APP,
			COPY_DOCS,
			INIT_APP,
			WAIT_FOR_SYNC,
			LOAD_FONTS,
			WAIT_FOR_FONTS
		};

		bool Initialize();
		bool IsInitialized() const { return NULL != fRuntime; }
		void Start();
		bool ProcessEvent(SDL_Event& event);
		void enumerateFontFiles(const char* dir, std::vector<std::string>& fileList);
		bool TimerTick();

		Runtime* GetRuntime() { return fRuntime; }
		const Runtime *GetRuntime() const { return fRuntime; }

		MouseListener* GetMouseListener() { return fMouseListener; }
		const MouseListener *GetMouseListener() const { return fMouseListener; }
		KeyListener* GetKeyListener() { return fKeyListener; }
		const KeyListener *GetKeyListener() const { return fKeyListener; }
		void pause();
		void resume();
		int getFPS() const	{	return fRuntime ? fRuntime->GetFPS() : 30; }

#if defined(EMSCRIPTEN)
		static int resizeCallback(int eventType, const EmscriptenUiEvent *uiEvent, void *userData);
		static int mouseupCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void * userData);
		static int touchCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
		static int blurCallback(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData);
		static int focusCallback(int eventType, const EmscriptenFocusEvent *focusEvent, void *userData);
		static const char* beforeunloadCallback(int eventType, const void *reserved, void *userData);
#endif

		void requestFullscreen(int scaleMode, int canvasResolutionScaleMode, int filteringMode);
		void enterSoftFullscreen(int scaleMode, int canvasResolutionScaleMode, int filteringMode);

	private:

		EmscriptenRuntime* fRuntime;
		EmscriptenRuntimeDelegate* fRuntimeDelegate;
		MouseListener* fMouseListener;
		KeyListener* fKeyListener;
		std::string fPathToApp;
		std::string fDocumentsDir;

		int fWidth;
		int fHeight;

		EmscriptenPlatform* fPlatform;
		DeviceOrientation::Type fOrientation;
		AppState fAppState;
		SDL_Window* fWindow;
		std::string fMode;
	};

};