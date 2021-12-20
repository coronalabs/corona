//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Rtt_LINUX_CONTEXT_H
#define Rtt_LINUX_CONTEXT_H

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
#include "wx/app.h"
#include "wx/frame.h"
#include "wx/panel.h"
#include "wx/stattext.h"
#include "wx/glcanvas.h"
#include "wx/timer.h"
#include "wx/fswatcher.h"
#include <string>

#define HOMESCREEN_ID "homescreen"

class SolarApp;
class SolarFrame;
class SolarGLCanvas;

wxDECLARE_EVENT(eventOpenProject, wxCommandEvent);
wxDECLARE_EVENT(eventRelaunchProject, wxCommandEvent);
wxDECLARE_EVENT(eventWelcomeProject, wxCommandEvent);

namespace Rtt
{
	class LinuxPlatform;

	struct SolarAppContext
	{
		SolarAppContext(const char *path);
		~SolarAppContext();

		void Close();
		bool Init();
		bool IsInitialized() const { return NULL != fRuntime; }
		void Start();
		void EnumerateFontFiles(const char *dir, std::vector<std::string> &fileList);

		Runtime *GetRuntime() { return fRuntime; }
		LinuxRuntimeDelegate *GetDelegate() { return fRuntimeDelegate; }
		const Runtime *GetRuntime() const { return fRuntime; }
		LinuxMouseListener *GetMouseListener() { return fMouseListener; }
		const LinuxMouseListener *GetMouseListener() const { return fMouseListener; }
		LinuxKeyListener *GetKeyListener() { return fKeyListener; }
		const LinuxKeyListener *GetKeyListener() const { return fKeyListener; }
		void Pause();
		void Resume();
		void RestartRenderer();
		int GetFPS() const { return fRuntime ? fRuntime->GetFPS() : 30; }
		int GetWidth() const;
		void SetWidth(int val);
		int GetHeight() const;
		void SetHeight(int val);
		DeviceOrientation::Type GetOrientation() const { return fRuntimeDelegate->fOrientation; }
		const std::string &GetTitle() const { return fTitle; }
		void Flush();
		bool LoadApp(SolarGLCanvas *canvas);
		SolarGLCanvas *GetCanvas() const { return fCanvas; }
		void SetCanvas(SolarGLCanvas *val) { fCanvas = val; }
		const char *GetAppPath() const { return fPathToApp.c_str(); }
		LinuxPlatform *GetPlatform() const { return fPlatform; }
		const std::string &GetAppName() const { return fAppName; }
		const std::string &GetSaveFolder() const { return fSaveFolder; }
		const LinuxRuntimeDelegate *GetRuntimeDelegate() const { return fRuntimeDelegate; }
		static int Print(lua_State* L);		// re-defined global.print

		bool fIsStarted;

	private:
		std::string fTitle;
		LinuxRuntime *fRuntime;
		LinuxRuntimeDelegate *fRuntimeDelegate;
		LinuxMouseListener *fMouseListener;
		LinuxKeyListener *fKeyListener;
		std::string fPathToApp;
		std::string fAppName;
		LinuxPlatform *fPlatform;
		bool fTouchDeviceExist;
		const char *fMode;
		SolarGLCanvas *fCanvas;
		bool fIsDebApp;
		LinuxSimulatorServices *fSimulator;
		std::string fSaveFolder;
	};
}; // namespace Rtt

//  the canvas window
class SolarGLCanvas : public wxGLCanvas
{
public:
	SolarGLCanvas(SolarFrame *parent, const wxGLAttributes &canvasAttrs);
	~SolarGLCanvas();

	void OnChar(wxKeyEvent &event);
	void OnTimer(wxTimerEvent &event);
	bool IsGLContextAvailable() { return fGLContext != NULL; } // used to know if we must end now because OGL 3.2 isn't available
	void OnPaint(wxPaintEvent &event);
	void OnWindowCreate(wxWindowCreateEvent &event);
	void OnSize(wxSizeEvent &event);
	void StartTimer(float duration);

	SolarFrame *fSolarFrame;
	wxGLContext *fGLContext;
	int fWindowHeight;
	Rtt::SolarAppContext *fContext;
	wxTimer fTimer;
	wxDECLARE_EVENT_TABLE();
};

// the main frame
class SolarFrame : public wxFrame
{
public:
	SolarFrame(int style);
	virtual ~SolarFrame();

	void OnFileSystemEvent(wxFileSystemWatcherEvent &event);
	void OnOpen(wxCommandEvent &event);
	void OnRelaunch(wxCommandEvent &event);
	void OnSuspendOrResume(wxCommandEvent &event);
	void OnOpenWelcome(wxCommandEvent &event);
	void OnZoomIn(wxCommandEvent &event);
	void OnZoomOut(wxCommandEvent &event);
	static void OnViewAsChanged(wxCommandEvent &event);
	void OnIconized(wxIconizeEvent &event);
	void OnClose(wxCloseEvent &event);
	void SetOGLString(const wxString &ogls) { fGLString = ogls; }
	void ChangeSize(int newWidth, int newHeight);
	void CreateSuspendedPanel();
	void RemoveSuspendedPanel();
	SolarGLCanvas *GetCanvas() const { return fSolarGLCanvas; }
	Rtt::SolarAppContext *GetContext() const { return fContext; }
	void ResetSize();
	void SetMenu(const char *appPath);
	void CreateMenus();
	void CreateViewAsChildMenu(std::vector<std::string>skin, wxMenu *targetMenu);
	void ClearMenuCheckboxes(wxMenu *menu, wxString currentSkinTitle);
	void WatchFolder(const char *path, const char *appName);

	bool fRelaunchedViaFileEvent;
	Rtt::LinuxRelaunchProjectDialog *fRelaunchProjectDialog;
	wxLongLong fFileSystemEventTimestamp = 0;
	wxPanel *suspendedPanel;
	wxStaticText *suspendedText;
	wxMenu *fHardwareMenu;
	wxString fGLString;
	SolarGLCanvas *fSolarGLCanvas;
	Rtt::SolarAppContext *fContext;
	wxMenuBar *fMenuMain;
	wxMenu *fViewMenu;
	wxMenu *fViewAsAndroidMenu;
	wxMenu *fViewAsIOSMenu;
	wxMenu *fViewAsTVMenu;
	wxMenu *fViewAsDesktopMenu;
	wxMenuItem *fZoomIn;
	wxMenuItem *fZoomOut;
	wxMenuBar *fMenuProject;
	std::string fAppPath;
	std::string fProjectPath;
	int currentSkinWidth;
	int currentSkinHeight;
	wxFileSystemWatcher *fWatcher;
	wxDECLARE_EVENT_TABLE();
};

wxDECLARE_APP(SolarApp);
class SolarApp : public wxApp
{
public:
	SolarApp();
	~SolarApp();

	bool OnInit() wxOVERRIDE;
	void OnEventLoopEnter(wxEventLoopBase *WXUNUSED(loop));
	SolarFrame *GetFrame() { return fSolarFrame; }
	SolarGLCanvas *GetCanvas() const { return fSolarFrame->GetCanvas(); }
	wxWindow *GetParent();
	Rtt::LinuxPlatform *GetPlatform() const;
	Rtt::Runtime *GetRuntime() { return fSolarFrame->GetContext()->GetRuntime(); };

private:
	SolarFrame *fSolarFrame;
};
#endif // Rtt_LINUX_CONTEXT_H
