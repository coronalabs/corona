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
#include "Rtt_LinuxRelaunchProjectDialog.h"
#include "Rtt_LinuxPlatform.h"
#include "Rtt_LinuxContext.h"
#include "Rtt_LinuxContainer.h"
#include "wx/app.h"
#include "wx/frame.h"
#include "wx/panel.h"
#include "wx/stattext.h"
#include <string>
#include <chrono>
#include <thread>

// nuklear
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
//#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#include "nuklear_sdl_gl3.h"

enum sdl
{
	OnOpen = SDL_USEREVENT + 1,
	OnNewProject,
	OnBuild,
	OnOpenInEditor,
	OnRelaunch,
	OnOpenFileDialog,
	OnClose,
	OnOpenDocumentation,
	OnOpenSampleProjects,
	OnAbout,
	OnShowProjectFiles,
	OnShowProjectSandbox,
	OnClearProjectSandbox,
	OnRelaunchLastProject,
	OnOpenPreferences,
};

namespace Rtt
{

	struct SolarApp : public ref_counted
	{
		SolarApp(const std::string& resourceDir);
		virtual ~SolarApp();

		virtual bool Initialize();
		void Run();
		bool PollEvents();


		Runtime* GetRuntime() { return fContext->GetRuntime(); }
		LinuxPlatform* GetPlatform() const { return fContext->GetPlatform(); }

		void OnIconized(wxIconizeEvent& event);
		virtual void OnClose(wxCloseEvent& event);
		void ChangeSize(int newWidth, int newHeight);
		SolarAppContext* GetContext() const { return fContext; }
		void ResetWindowSize();

		virtual void GetSavedZoom(int& width, int& height) {}
		virtual bool IsRunningOnSimulator() { return false; }
		const char* GetAppName() const { return fContext->GetAppName(); }
		virtual void DrawMenu() {}

	protected:

		virtual void GuiEvent(SDL_Event& e) {}
		inline void SetTitle(const std::string& name)	{	SDL_SetWindowTitle(fWindow, name.c_str()); }

		SolarAppContext* fContext;
		SDL_Window* fWindow;
		SDL_GLContext fGLcontext;

		std::string fAppPath;
		std::string fProjectPath;
		int fWidth;
		int fHeight;
		nk_context* fNK;		// nuklear context
	};

}

extern wxFrame* solarApp;
extern smart_ptr<Rtt::SolarApp> app;


#endif // Rtt_LINUX_CONTEXT_H
