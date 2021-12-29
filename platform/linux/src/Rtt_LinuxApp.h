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

	void OnFileSystemEvent(wxFileSystemWatcherEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnRelaunch(wxCommandEvent& event);
	void OnSuspendOrResume(wxCommandEvent& event);
	void OnOpenWelcome(wxCommandEvent& event);
	void OnZoomIn(wxCommandEvent& event);
	void OnZoomOut(wxCommandEvent& event);
	static void OnViewAsChanged(wxCommandEvent& event);
	void OnIconized(wxIconizeEvent& event);
	void OnClose(wxCloseEvent& event);
	void SetOGLString(const wxString& ogls) { fGLString = ogls; }
	void ChangeSize(int newWidth, int newHeight);
	void CreateSuspendedPanel();
	void RemoveSuspendedPanel();
	SolarGLCanvas* GetCanvas() const { return fSolarGLCanvas; }
	Rtt::SolarAppContext* GetContext() const { return fContext; }
	void ResetSize();
	void SetMenu(const char* appPath);
	void CreateMenus();
	void CreateViewAsChildMenu(std::vector<std::string>skin, wxMenu* targetMenu);
	void ClearMenuCheckboxes(wxMenu* menu, wxString currentSkinTitle);
	void WatchFolder(const char* path, const char* appName);

	bool fRelaunchedViaFileEvent;
	Rtt::LinuxRelaunchProjectDialog* fRelaunchProjectDialog;
	wxLongLong fFileSystemEventTimestamp = 0;
	wxPanel* suspendedPanel;
	wxStaticText* suspendedText;
	wxMenu* fHardwareMenu;
	wxString fGLString;
	SolarGLCanvas* fSolarGLCanvas;
	Rtt::SolarAppContext* fContext;
	wxMenuBar* fMenuMain;
	wxMenu* fViewMenu;
	wxMenu* fViewAsAndroidMenu;
	wxMenu* fViewAsIOSMenu;
	wxMenu* fViewAsTVMenu;
	wxMenu* fViewAsDesktopMenu;
	wxMenuItem* fZoomIn;
	wxMenuItem* fZoomOut;
	wxMenuBar* fMenuProject;
	std::string fAppPath;
	std::string fProjectPath;
	int currentSkinWidth;
	int currentSkinHeight;
	wxFileSystemWatcher* fWatcher;
	wxDECLARE_EVENT_TABLE();
};

//  the canvas window
class SolarGLCanvas : public wxGLCanvas
{
public:
	SolarGLCanvas(SolarApp* parent, const int* vAttrs);
	~SolarGLCanvas();

	void OnChar(wxKeyEvent& event);
	void OnTimer(wxTimerEvent& event);
	bool IsGLContextAvailable() { return fGLContext != NULL; } // used to know if we must end now because OGL 3.2 isn't available
	void Render();
	void OnSize(wxSizeEvent& event);
	void StartTimer(float duration);

	SolarApp* solarApp;
	wxGLContext* fGLContext;
	int fWindowHeight;
	Rtt::SolarAppContext* fContext;
	wxTimer fTimer;
	wxDECLARE_EVENT_TABLE();
};

extern SolarApp* solarApp;

#endif // Rtt_LINUX_CONTEXT_H
