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
#include "wx/app.h"
#include "wx/frame.h"
#include "wx/panel.h"
#include "wx/stattext.h"
#include <sys/inotify.h>

enum sdl
{
	OnOpenProject = SDL_USEREVENT + 1,
	OnNewProject,
	OnBuild,
	OnOpenInEditor,
	OnRelaunch,
	OnCloseProject,
	onClosePopupModal,
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
	OnConsole
};

namespace Rtt
{
	void PushEvent(int evt);

	struct SolarApp : public ref_counted
	{
		SolarApp(const std::string& resourceDir);
		virtual ~SolarApp();

		bool Init();
		virtual bool LoadApp();
		void Run();
		bool PollEvents();

		Runtime* GetRuntime() { return fContext->GetRuntime(); }
		LinuxPlatform* GetPlatform() const { return fContext->GetPlatform(); }

		void OnIconized(wxIconizeEvent& event);
		void ChangeSize(int newWidth, int newHeight);
		SolarAppContext* GetContext() const { return fContext; }

		virtual void GetSavedZoom(int& width, int& height) {}
		virtual bool IsRunningOnSimulator() { return false; }
		bool IsSuspended() const { return fContext->GetRuntime()->IsSuspended(); }
		virtual void ConfigLoad() {};
		virtual void ConfigSave() {};
		virtual std::map<std::string, std::string>* ConfigGet() { return NULL; }

		const char* GetAppName() const { return fContext->GetAppName(); }
		inline bool IsHomeScreen(const std::string& appName) { return appName.compare(HOMESCREEN_ID) == 0; }
		void SetTitle(const std::string& name);

		void RenderGUI();
		inline void Pause() { fContext->Pause(); }
		inline void Resume() { fContext->Resume(); }
		void SetActivityIndicator(bool visible) { fActivityIndicator = visible; }

	protected:

		virtual void SolarEvent(SDL_Event& e) {}

		SolarAppContext* fContext;
		SDL_Window* fWindow;
		SDL_GLContext fGLcontext;

		std::string fAppPath;
		std::string fProjectPath;
		int fWidth;
		int fHeight;

		// GUI
		ImGuiContext* imctx;
		smart_ptr<Dlg> fMenu;
		smart_ptr<Dlg> fDlg;
		bool fActivityIndicator;
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

		// thread func timer callback
		void Advance();

	private:
		smart_ptr<mythread> fThread;
		int m_inotify_fd;
		int m_watch_descriptor;
	};


}

extern wxFrame* solarApp;
extern smart_ptr<Rtt::SolarApp> app;


#endif // Rtt_LINUX_CONTEXT_H
