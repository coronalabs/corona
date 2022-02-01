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

namespace Rtt
{

	class SolarApp : public ref_counted
	{

		// for event aggerating
		struct events_t
		{
			Uint32 n;
			SDL_Event e;
		};
		static bool cmp_events(std::pair<Uint32, events_t>& a, std::pair<Uint32, events_t>& b) { return a.second.n < b.second.n; }

	public:
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
		bool CreateWindow(const std::string& resourcesDir);

		virtual void GetSavedZoom(int& width, int& height) {}
		virtual bool IsRunningOnSimulator() { return false; }
		const char* GetAppName() const { return fContext->GetAppName(); }

		wxStaticText* suspendedText;
		SolarAppContext* fContext;
		std::string fAppPath;
		std::string fProjectPath;

		SDL_Window* fWindow;
		SDL_GLContext fGLcontext;
		int fWidth;
		int fHeight;
	};

}

extern wxFrame* solarApp;
extern smart_ptr<Rtt::SolarApp> app;


#endif // Rtt_LINUX_CONTEXT_H
