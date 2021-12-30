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
#include "wx/timer.h"
#include "wx/fswatcher.h"
#include <string>

#define TIMER_ID wxID_HIGHEST + 1

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

		Rtt::Runtime* GetRuntime() { return fContext->GetRuntime(); }
		Rtt::LinuxPlatform* GetPlatform() const { return fContext->GetPlatform(); }

		virtual void OnOpen(wxCommandEvent& event);
		void OnIconized(wxIconizeEvent& event);
		void OnClose(wxCloseEvent& event);
		void SetOGLString(const wxString& ogls) { fGLString = ogls; }
		void ChangeSize(int newWidth, int newHeight);
		virtual void CreateSuspendedPanel() {}
		virtual void RemoveSuspendedPanel() {}
		SolarGLCanvas* GetCanvas() const { return fSolarGLCanvas; }
		Rtt::SolarAppContext* GetContext() const { return fContext; }
		void ResetSize();
		void ClearMenuCheckboxes(wxMenu* menu, wxString currentSkinTitle);
		void StartTimer(float duration);
		void OnTimer(wxTimerEvent& event);

		virtual void WatchFolder(const char* path, const char* appName) {};
		virtual bool Start(const std::string& resourcesDir);
		virtual void CreateMenus() {};
		virtual void SetMenu(const char* appPath) {};
		virtual void GetSavedZoom(int& width, int& height) {}

		Rtt::LinuxRelaunchProjectDialog* fRelaunchProjectDialog;
		wxLongLong fFileSystemEventTimestamp = 0;
		wxStaticText* suspendedText;
		wxMenu* fHardwareMenu;
		wxString fGLString;
		SolarGLCanvas* fSolarGLCanvas;
		Rtt::SolarAppContext* fContext;
		std::string fAppPath;
		std::string fProjectPath;
		int currentSkinWidth;
		int currentSkinHeight;
		wxTimer fTimer;

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
