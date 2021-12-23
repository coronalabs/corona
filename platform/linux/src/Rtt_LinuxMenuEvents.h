#ifndef Rtt_Linux_Menu_Events
#define Rtt_Linux_Menu_Events

#include "wx/wx.h"

// menu ids
#define ID_MENU_NEW_PROJECT  wxID_HIGHEST + 20
#define ID_MENU_OPEN_PROJECT  wxID_HIGHEST + 30
#define ID_MENU_OPEN_LAST_PROJECT wxID_HIGHEST + 40
#define ID_MENU_RELAUNCH_PROJECT wxID_HIGHEST + 50
#define ID_MENU_CLOSE_PROJECT wxID_HIGHEST + 60
#define ID_MENU_OPEN_IN_EDITOR wxID_HIGHEST + 70
#define ID_MENU_SHOW_PROJECT_FILES  wxID_HIGHEST + 80
#define ID_MENU_SHOW_PROJECT_SANDBOX wxID_HIGHEST + 90
#define ID_MENU_CLEAR_PROJECT_SANDBOX wxID_HIGHEST + 100
#define ID_MENU_ZOOM_IN wxID_HIGHEST + 101
#define ID_MENU_ZOOM_OUT wxID_HIGHEST + 102
#define ID_MENU_BACK_BUTTON wxID_HIGHEST + 103
#define ID_MENU_BUILD_ANDROID wxID_HIGHEST + 110
#define ID_MENU_BUILD_WEB wxID_HIGHEST + 120
#define ID_MENU_BUILD_LINUX wxID_HIGHEST + 130
#define ID_MENU_SUSPEND wxID_HIGHEST + 140
#define ID_MENU_OPEN_WELCOME_SCREEN wxID_HIGHEST + 150
#define ID_MENU_OPEN_DOCUMENTATION wxID_HIGHEST + 160
#define ID_MENU_OPEN_SAMPLE_CODE wxID_HIGHEST + 170
#define ID_MENU_VIEW_AS wxID_HIGHEST + 180

class LinuxMenuEvents
{
public:
	static void OnCloneProject(wxCommandEvent &event);
	static void OnNewProject(wxCommandEvent &event);
	static void OnOpenFileDialog(wxCommandEvent &event);
	static void OnRelaunchLastProject(wxCommandEvent &event);
	static void OnOpenInEditor(wxCommandEvent &event);
	static void OnShowProjectFiles(wxCommandEvent &event);
	static void OnShowProjectSandbox(wxCommandEvent &event);
	static void OnClearProjectSandbox(wxCommandEvent &event);
	static void OnAndroidBackButton(wxCommandEvent &event);
	static void OnOpenPreferences(wxCommandEvent &event);
	static void OnQuit(wxCommandEvent &WXUNUSED(event));
	static void OnBuildForAndroid(wxCommandEvent &event);
	static void OnBuildForWeb(wxCommandEvent &event);
	static void OnBuildForLinux(wxCommandEvent &event);
	static void OnOpenDocumentation(wxCommandEvent &event);
	static void OnOpenSampleProjects(wxCommandEvent &event);
	static void OnAbout(wxCommandEvent &WXUNUSED(event));
};

#endif // Rtt_Linux_Menu_Events
