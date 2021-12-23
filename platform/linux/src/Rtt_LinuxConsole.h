#ifndef LINUX_CONSOLE_H
#define LINUX_CONSOLE_H

#include <pwd.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <wx/wx.h>
#include <wx/image.h>
#include <wx/stc/stc.h>
#include <wx/icon.h>
#include "Rtt_LinuxIPCServer.h"
#include "Rtt_LinuxIPCServerConnection.h"

using namespace std;
class DropdownMenu;

class Rtt_LinuxConsole: public wxFrame
{
public:
	Rtt_LinuxConsole(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE);

private:
	void SetProperties();
	void DoLayout();

protected:
	wxStatusBar *statusbar;
	wxBitmapButton *bitmapBtnSave;
	wxBitmapButton *bitmapBtnCopy;
	wxBitmapButton *bitmapBtnErase;
	wxBitmapButton *bitmapBtnFindPrevious;
	wxBitmapButton *bitmapBtnFindNext;
	wxBitmapButton *bitmapBtnMatchCase;
	wxBitmapButton *bitmapBtnLoopingSearch;
	wxBitmapButton *bitmapBtnSaveWindowPos;
	wxBitmapButton *bitmapBtnMenu;
	wxPanel *panelToolBar;
	wxTextCtrl *txtFind;
	wxStyledTextCtrl *txtLog;
	Rtt_LinuxIPCServer *linuxIPCServer;
	DropdownMenu *dropdownMenu;

	DECLARE_EVENT_TABLE();

public:
	void OnBtnSaveClick(wxCommandEvent &event);
	void OnBtnCopyClick(wxCommandEvent &event);
	void OnBtnEraseClick(wxCommandEvent &event);
	void OnBtnFindPreviousClick(wxCommandEvent &event);
	void OnBtnFindNextClick(wxCommandEvent &event);
	void OnBtnMatchCaseClick(wxCommandEvent &event);
	void OnBtnLoopingSearchClick(wxCommandEvent &event);
	void OnBtnSaveWindowPosClick(wxCommandEvent &event);
	void OnBtnChangeThemeClick(wxCommandEvent &event);
	void OnKeyUp(wxKeyEvent &event);
	void ClearLog();
	void UpdateStatusText();
	void HighlightLine(int indicatorNo, wxColour colour);
	void ResetSearch();
	void ChangeTheme();
	void UpdateLog(wxString message);
	void UpdateLogWarning(wxString message);
	void UpdateLogError(wxString message);
};

class DropdownMenu: public wxPanel
{
public:
	DropdownMenu(wxWindow *parent, wxWindowID id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = 0);

private:
	void SetProperties();
	void DoLayout();

protected:
	wxCheckBox *chkLightTheme;
	wxCheckBox *chkDarkTheme;

	DECLARE_EVENT_TABLE();

public:
	Rtt_LinuxConsole *linuxConsole;
	void OnChkLightThemeClicked(wxCommandEvent &event);
	void OnChkDarkThemeClicked(wxCommandEvent &event);
	void ShowDropDownMenu();
};

#endif // LINUX_CONSOLE_H
