//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Rtt_LINUX_APP_H
#define Rtt_LINUX_APP_H

#include "Rtt_Event.h"
#include "Core/Rtt_Types.h"
#include "Rtt_Runtime.h"
#include "Core/Rtt_Math.h"
#include "Core/Rtt_Array.h"
#include "shared/Rtt_ProjectSettings.h"
#include "shared/Rtt_NativeWindowMode.h"
#include "Rtt_LinuxInputDeviceManager.h"
#include "Rtt_LinuxSimulatorServices.h"
#include "Rtt_LinuxRuntime.h"
#include "Rtt_LinuxRuntimeDelegate.h"
#include "Rtt_LinuxKeyListener.h"
#include "Rtt_LinuxMouseListener.h"
#include "Rtt_LinuxPlatform.h"
#include "Rtt_LinuxContext.h"
#include "Rtt_LinuxContainer.h"
#include "Rtt_LinuxDialog.h"
#include "Rtt_LinuxUtils.h"
#include "Rtt_LinuxConsoleApp.h"
#include "Rtt_LinuxDisplayObject.h"
#include <sys/inotify.h>

enum sdl
{
	OnOpenProject = SDL_USEREVENT + 1,
	OnNewProject,
	OnCloneProject,
	OnBuild,
	OnOpenInEditor,
	OnRelaunch,
	OnCloseProject,
	onCloseDialog,
	OnOpenDocumentation,
	OnOpenSampleProjects,
	OnAbout,
	OnShowProjectFiles,
	OnShowProjectSandbox,
	OnClearProjectSandbox,
	OnRelaunchLastProject,
	OnOpenPreferences,
	OnFileBrowserSelected,
	OnFileSystemEvent,
	OnBuildLinux,
	OnBuildAndroid,
	OnBuildHTML5,
	OnRotateLeft,
	OnRotateRight,
	OnShake,
	OnZoomIn,
	OnZoomOut,
	OnSetFocusConsole,
	OnStyleColorsLight,
	OnStyleColorsClassic,
	OnStyleColorsDark,
	OnViewAs,
	OnChangeView,
	OnWindowNormal,
	OnWindowMinimized,
	OnWindowMaximized,
	OnWindowFullscreen,
	OnMouseCursorVisible,
	OnSetCursor,
	OnRuntimeError,
	OnPreferencesChanged
};

namespace Rtt
{
	void PushEvent(int evt);

	struct SolarApp : public ref_counted
	{
		SolarApp(const std::string& resourceDir);
		virtual ~SolarApp();

		bool InitSDL();
		virtual bool Init();
		virtual bool LoadApp(const std::string& path);
		void Run();
		bool PollEvents();

		Runtime* GetRuntime() const { return fContext ? fContext->GetRuntime() : NULL; }
		LinuxPlatform* GetPlatform() const { return fContext ? fContext->GetPlatform() : NULL; }

		void OnIconized();
		void SetWindowSize(int newWidth, int newHeight);
		SolarAppContext* GetContext() const { return fContext; }

		virtual bool IsRunningOnSimulator() { return false; }
		bool IsSuspended() const { return GetRuntime()->IsSuspended(); }

		inline bool IsHomeScreen(const std::string& appName) { return appName.compare(HOMESCREEN_ID) == 0; }

		void RenderGUI();
		inline void Pause() { fContext->Pause(); }
		inline void Resume() { fContext->Resume(); }
		inline void SetActivityIndicator(bool visible) { fActivityIndicator = visible; }
		void Log(const char* buf, int len);

		bool IsFullScreen() { return false; }
		bool IsMinimized() { return false; }
		bool IsIconized() { return false; }
		bool IsMaximized() { return false; }

		inline Config& GetConfig() { return fConfig; }
		inline Config& GetPwdStore() { return fPwdStore; }

		void AddDisplayObject(LinuxDisplayObject* obj);
		void RemoveDisplayObject(LinuxDisplayObject* obj);
		NativeAlertRef ShowNativeAlert(const char* title, const char* msg, const char** buttonLabels, U32 numButtons, LuaResource* resource);
		virtual void StartConsole() {}
		virtual void CreateMenu() {}
		const std::string& GetAppPath() const { return fContext->GetAppPath(); }
		const std::string& GetAppName() const { return fContext->GetAppName(); }
		const std::string& GetSaveFolder() const { return fContext->GetSaveFolder(); }
		void GetWindowPosition(int* x, int* y);
		void GetWindowSize(int* w, int* h);
		void SetIcon();

		std::string GetTitle() const { return fContext->GetTitle(); }
		void SetTitle(const std::string& name) { fContext->SetTitle(name); }

		virtual int GetMenuHeight() const { return 0; }

	protected:

		virtual void SolarEvent(const SDL_Event& e) {}
		bool DispathNativeObjectsEvent(const SDL_Event& e);

		smart_ptr<SolarAppContext> fContext;
		SDL_Window* fWindow;
		SDL_GLContext fGLcontext;

		std::string fResourceDir;
		Config fConfig;
		Config fPwdStore;
		smart_ptr<LinuxMouseListener> fMouse;

		// GUI
		ImGuiContext* fImCtx;
		smart_ptr<DlgMenu> fMenu;
		smart_ptr<Window> fDlg;
		bool fActivityIndicator;
		std::vector<LinuxDisplayObject*> fNativeObjects;

		// console
		std::string fLogData;
		smart_ptr<ConsoleWindow> fConsole;
	};

	//
	// FileWatcher
	//
	struct FileWatcher : public ref_counted
	{
		FileWatcher();
		virtual ~FileWatcher();

		bool Start(const std::string& folder);
		void Stop();

		// thread func
		void Watch();

	private:
		smart_ptr<mythread> fThread;
		int m_inotify_fd;
		int m_watch_descriptor;
	};


}

extern smart_ptr<Rtt::SolarApp> app;


#endif // Rtt_LINUX_CONTEXT_H
