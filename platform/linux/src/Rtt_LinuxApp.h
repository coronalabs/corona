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
#include "wx/app.h"
#include "wx/frame.h"
#include "wx/panel.h"
#include "wx/stattext.h"
#include "wx/glcanvas.h"
#include <string>

namespace Rtt
{
	wxDECLARE_EVENT(eventOpenProject, wxCommandEvent);
	wxDECLARE_EVENT(eventRelaunchProject, wxCommandEvent);
	wxDECLARE_EVENT(eventWelcomeProject, wxCommandEvent);

	class SolarGLCanvas;

	// the main frame
	class SolarApp : public wxFrame
	{
	public:
		SolarApp();
		virtual ~SolarApp();

		Runtime* GetRuntime() { return fContext->GetRuntime(); }
		LinuxPlatform* GetPlatform() const { return fContext->GetPlatform(); }

		void OnIconized(wxIconizeEvent& event);
		virtual void OnClose(wxCloseEvent& event);
		void ChangeSize(int newWidth, int newHeight);
		SolarGLCanvas* GetCanvas() const { return fSolarGLCanvas; }
		SolarAppContext* GetContext() const { return fContext; }
		void ResetWindowSize();
		bool CreateWindow(const std::string& resourcesDir);

		virtual bool Start(const std::string& resourcesDir);
		virtual void GetSavedZoom(int& width, int& height) {}
		virtual bool IsRunningOnSimulator() { return false; }

		wxStaticText* suspendedText;
		SolarGLCanvas* fSolarGLCanvas;
		SolarAppContext* fContext;
		std::string fAppPath;
		std::string fProjectPath;

		wxDECLARE_EVENT_TABLE();
	};

	//  the canvas window
	class SolarGLCanvas : public wxGLCanvas
	{
	public:
		SolarGLCanvas(SolarApp* parent, const int* vAttrs);
		~SolarGLCanvas();

		void OnChar(wxKeyEvent& event);
		void OnSize(wxSizeEvent& event);
		void Render();

	private:
		wxGLContext* fGLContext;
		wxDECLARE_EVENT_TABLE();
	};

}

extern Rtt::SolarApp* solarApp;


#endif // Rtt_LINUX_CONTEXT_H
