#include "Rtt_LinuxConsole.h"
#include <sys/time.h>
#include <wx/statbmp.h>
#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/fileconf.h>

#ifndef wxHAS_IMAGES_IN_RESOURCES
// ignore future harmless warnings from XPMs
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#include "resource/console.xpm"
#include "resource/save.xpm"
#include "resource/copy.xpm"
#include "resource/erase.xpm"
#include "resource/search-left.xpm"
#include "resource/search-right.xpm"
#include "resource/match-case.xpm"
#include "resource/match-case-on.xpm"
#include "resource/looping-search.xpm"
#include "resource/looping-search-on.xpm"
#include "resource/window-position.xpm"
#include "resource/cog.xpm"
#pragma GCC diagnostic pop
#endif

#define ID_BUTTON_SAVE wxID_HIGHEST + 1
#define ID_BUTTON_COPY wxID_HIGHEST + 2
#define ID_BUTTON_CLEAR wxID_HIGHEST + 3
#define ID_BUTTON_FIND_PREVIOUS wxID_HIGHEST + 4
#define ID_BUTTON_FIND_NEXT wxID_HIGHEST + 5
#define ID_BUTTON_MATCH_CASE wxID_HIGHEST + 6
#define ID_BUTTON_LOOP_SEARCH wxID_HIGHEST + 7
#define ID_BUTTON_THEME wxID_HIGHEST + 8
#define ID_BUTTON_SAVE_WINDOW_POS wxID_HIGHEST + 9
#define ID_LIGHT_THEME_MENU_ITEM wxID_HIGHEST + 10
#define ID_DARK_THEME_MENU_ITEM wxID_HIGHEST + 11
#define ID_DROPDOWN_MENU wxID_HIGHEST + 12
#define ID_INDICATOR_WARNING 8
#define ID_INDICATOR_ERROR 9
#define ID_INDICATOR_WARNING_TEXT 15
#define ID_INDICATOR_ERROR_TEXT 16
#define CONFIG_THEME_ID "/theme"
#define CONFIG_MATCH_CASE_ID "/matchCase"
#define CONFIG_LOOPING_SEARCH_ID "/loopingSearch"
#define CONFIG_WINDOW_X_POSITION "/xPos"
#define CONFIG_WINDOW_Y_POSITION "/yPos"
#define CONFIG_WINDOW_WIDTH "/windowWidth"
#define CONFIG_WINDOW_HEIGHT "/windowHeight"
#define CONFIG_LIGHT_THEME_VALUE "light"
#define CONFIG_DARK_THEME_VALUE "dark"

wxDEFINE_EVENT(eventFindNext, wxCommandEvent);

struct ConsoleLog
{
	int currentPosition = 0;
	int errorCount = 0;
	int warningCount = 0;
	bool buttonMatchCaseOn = false;
	bool buttonLoopingSearchOn = false;
	wxString currentTheme;
	wxColour warningColour = wxColour(255, 255, 0);
	wxColour errorColour = wxColour(192, 0, 0);
	wxColour textColourDarkTheme = wxColour(192, 192, 192);
	wxColour textColourLightTheme = wxColour(0, 0, 0);
	wxColour backgroundColourDarkTheme = wxColour(0, 0, 0);
	wxColour backgroundColourLightTheme = wxColour(255, 255, 255);
	wxColour toolbarBackgroundColor = wxColour(37, 37, 38);
	wxColour themeTextColour;
	wxColour themeBackgroundColour;
	wxString settingsFilePath;
	int windowXPos = 0;
	int windowYPos = 0;
	int windowWidth = 640;
	int windowHeight = 480;
	wxConfig* config;
} consoleLog;

using namespace std;

Rtt_LinuxConsole::Rtt_LinuxConsole(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) :
	wxFrame(parent, id, title, pos, size, wxDEFAULT_FRAME_STYLE)
{
	struct passwd* pw = getpwuid(getuid());
	const char* homedir = pw->pw_dir;
	consoleLog.settingsFilePath = homedir;

	// ensure working dir
	consoleLog.settingsFilePath.append("/.Solar2D");
	if (wxDirExists(consoleLog.settingsFilePath.c_str()) == false)
	{
		if (wxMkdir(consoleLog.settingsFilePath) == false)
		{
			wxMessageBox(wxT("Failed to create " + consoleLog.settingsFilePath), wxT("Error"), wxOK);
		}
	}

	consoleLog.settingsFilePath.append("/console.conf");
	consoleLog.config = new wxFileConfig("", "", consoleLog.settingsFilePath);
	consoleLog.themeTextColour = consoleLog.textColourDarkTheme;
	consoleLog.themeBackgroundColour = consoleLog.backgroundColourDarkTheme;

	// read from the config file or create it, if it doesn't exist
	if (wxFileExists(consoleLog.settingsFilePath))
	{
		consoleLog.config->Read(wxT(CONFIG_THEME_ID), &consoleLog.currentTheme);
		consoleLog.config->Read(wxT(CONFIG_MATCH_CASE_ID), &consoleLog.buttonMatchCaseOn);
		consoleLog.config->Read(wxT(CONFIG_LOOPING_SEARCH_ID), &consoleLog.buttonLoopingSearchOn);
		consoleLog.config->Read(wxT(CONFIG_WINDOW_X_POSITION), &consoleLog.windowXPos);
		consoleLog.config->Read(wxT(CONFIG_WINDOW_Y_POSITION), &consoleLog.windowYPos);
		consoleLog.config->Read(wxT(CONFIG_WINDOW_WIDTH), &consoleLog.windowWidth);
		consoleLog.config->Read(wxT(CONFIG_WINDOW_HEIGHT), &consoleLog.windowHeight);

		if (consoleLog.currentTheme.IsSameAs(CONFIG_LIGHT_THEME_VALUE))
		{
			consoleLog.themeTextColour = consoleLog.textColourLightTheme;
			consoleLog.themeBackgroundColour = consoleLog.backgroundColourLightTheme;
		}
	}
	else
	{
		consoleLog.config->Write(wxT(CONFIG_WINDOW_X_POSITION), 0);
		consoleLog.config->Write(wxT(CONFIG_WINDOW_Y_POSITION), 0);
		consoleLog.config->Write(wxT(CONFIG_WINDOW_WIDTH), consoleLog.windowWidth);
		consoleLog.config->Write(wxT(CONFIG_WINDOW_HEIGHT), consoleLog.windowHeight);
		consoleLog.config->Write(wxT(CONFIG_THEME_ID), CONFIG_DARK_THEME_VALUE);
		consoleLog.config->Write(wxT(CONFIG_MATCH_CASE_ID), false);
		consoleLog.config->Write(wxT(CONFIG_LOOPING_SEARCH_ID), false);
		consoleLog.config->Flush();
	}

	SetIcon(console_xpm);
	SetSize(wxSize(consoleLog.windowWidth, consoleLog.windowHeight));
	EnableCloseButton(false);
	panelToolBar = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
	statusbar = CreateStatusBar(1);
	bitmapBtnSave = new wxBitmapButton(panelToolBar, ID_BUTTON_SAVE, wxIcon(save_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	bitmapBtnCopy = new wxBitmapButton(panelToolBar, ID_BUTTON_COPY, wxIcon(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	bitmapBtnErase = new wxBitmapButton(panelToolBar, ID_BUTTON_CLEAR, wxIcon(erase_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	bitmapBtnFindPrevious = new wxBitmapButton(panelToolBar, ID_BUTTON_FIND_PREVIOUS, wxIcon(search_left_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	bitmapBtnFindNext = new wxBitmapButton(panelToolBar, ID_BUTTON_FIND_NEXT, wxIcon(search_right_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	bitmapBtnMatchCase = new wxBitmapButton(panelToolBar, ID_BUTTON_MATCH_CASE, wxIcon(match_case_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	bitmapBtnLoopingSearch = new wxBitmapButton(panelToolBar, ID_BUTTON_LOOP_SEARCH, wxIcon(looping_search_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	bitmapBtnSaveWindowPos = new wxBitmapButton(panelToolBar, ID_BUTTON_SAVE_WINDOW_POS, wxIcon(window_position_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	bitmapBtnMenu = new wxBitmapButton(panelToolBar, ID_BUTTON_THEME, wxIcon(cog_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	txtFind = new wxTextCtrl(panelToolBar, wxID_ANY, wxEmptyString);
	txtLog = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTE_MULTILINE);
	linuxIPCServer = new Rtt_LinuxIPCServer();
	dropdownMenu = new DropdownMenu(this, ID_DROPDOWN_MENU, wxPoint(500, 32));
	dropdownMenu->linuxConsole = this;
	SetProperties();
	DoLayout();
}

void Rtt_LinuxConsole::SetProperties()
{
	SetTitle(wxT("Solar2D Console"));
	int statusbarWidths[] = { -1 };
	statusbar->SetStatusWidths(1, statusbarWidths);

	// statusbar fields
	const wxString statusbarFields[] =
	{
		wxT("Errors #0 Alerts #0"),
	};

	for (int i = 0; i < statusbar->GetFieldsCount(); ++i)
	{
		statusbar->SetStatusText(statusbarFields[i], i);
	}

	SetBackgroundColour(consoleLog.toolbarBackgroundColor);
	bitmapBtnSave->SetBackgroundColour(consoleLog.toolbarBackgroundColor);
	bitmapBtnSave->SetSize(bitmapBtnSave->GetBestSize());
	bitmapBtnCopy->SetBackgroundColour(consoleLog.toolbarBackgroundColor);
	bitmapBtnCopy->SetSize(bitmapBtnCopy->GetBestSize());
	bitmapBtnErase->SetBackgroundColour(consoleLog.toolbarBackgroundColor);
	bitmapBtnErase->SetSize(bitmapBtnErase->GetBestSize());
	bitmapBtnFindPrevious->SetBackgroundColour(consoleLog.toolbarBackgroundColor);
	bitmapBtnFindPrevious->SetSize(bitmapBtnFindPrevious->GetBestSize());
	bitmapBtnFindNext->SetBackgroundColour(consoleLog.toolbarBackgroundColor);
	bitmapBtnFindNext->SetSize(bitmapBtnFindNext->GetBestSize());
	bitmapBtnMatchCase->SetBackgroundColour(consoleLog.toolbarBackgroundColor);
	bitmapBtnMatchCase->SetBitmap(consoleLog.buttonMatchCaseOn ? wxIcon(match_case_on_xpm) : wxIcon(match_case_xpm));
	bitmapBtnMatchCase->SetSize(bitmapBtnMatchCase->GetBestSize());
	bitmapBtnLoopingSearch->SetBackgroundColour(consoleLog.toolbarBackgroundColor);
	bitmapBtnLoopingSearch->SetBitmap(consoleLog.buttonLoopingSearchOn ? wxIcon(looping_search_on_xpm) : wxIcon(looping_search_xpm));
	bitmapBtnLoopingSearch->SetSize(bitmapBtnLoopingSearch->GetBestSize());
	bitmapBtnSaveWindowPos->SetBackgroundColour(consoleLog.toolbarBackgroundColor);
	bitmapBtnSaveWindowPos->SetSize(bitmapBtnMenu->GetBestSize());
	bitmapBtnMenu->SetBackgroundColour(consoleLog.toolbarBackgroundColor);
	bitmapBtnMenu->SetSize(bitmapBtnMenu->GetBestSize());
	panelToolBar->SetBackgroundColour(consoleLog.toolbarBackgroundColor);
	statusbar->SetBackgroundColour(consoleLog.toolbarBackgroundColor);
	txtFind->SetMinSize(wxSize(250, 28));
	txtFind->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, 0, wxT("")));
	txtFind->SetBackgroundColour(wxColour(27, 27, 28));
	txtFind->SetForegroundColour(consoleLog.textColourDarkTheme);
	txtFind->Connect(wxEVT_CHAR, wxCharEventHandler(Rtt_LinuxConsole::OnKeyUp));
	txtLog->SetFocus();
	txtLog->SetReadOnly(true);
	txtLog->SetWrapMode(1);
	txtLog->SetMarginOptions(wxSTC_MARGINOPTION_NONE);
	txtLog->StyleSetBackground(wxSTC_STYLE_DEFAULT, consoleLog.themeBackgroundColour);
	txtLog->StyleSetForeground(wxSTC_STYLE_DEFAULT, consoleLog.themeTextColour);
	txtLog->SetCaretForeground(*wxCYAN);
	txtLog->SetSelAlpha(127);
	txtLog->SetSelEOLFilled(false);
	txtLog->SetSelBackground(true, *wxCYAN);

	// remove the left hand margin block
	for (int i = 0; i < 6; i++)
	{
		txtLog->SetMarginWidth(i, 0);
	}

	txtLog->StyleClearAll();

	// Create a new server
	if (!linuxIPCServer->Create(IPC_SERVICE))
	{
		//wxLogMessage("%s server failed to start on %s", kind, IPC_SERVICE);
	}
}

void Rtt_LinuxConsole::DoLayout()
{
	wxBoxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* lblFind = new wxStaticText(panelToolBar, wxID_ANY, wxT("Find:"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	lblFind->SetForegroundColour(consoleLog.textColourDarkTheme);
	sizer2->Add(bitmapBtnSave, 0, wxBOTTOM | wxTOP, 6);
	sizer2->AddSpacer(5);
	sizer2->Add(bitmapBtnCopy, 0, wxBOTTOM | wxTOP, 6);
	sizer2->AddSpacer(5);
	sizer2->Add(bitmapBtnErase, 0, wxBOTTOM | wxTOP, 6);
	sizer2->AddSpacer(5);
	sizer2->Add(lblFind, 0, wxALIGN_CENTER_VERTICAL, 0);
	sizer2->AddSpacer(5);
	sizer2->Add(txtFind, 0, wxALIGN_CENTER_VERTICAL, 0);
	sizer2->AddSpacer(5);
	sizer2->Add(bitmapBtnFindPrevious, 0, wxBOTTOM | wxTOP, 6);
	sizer2->AddSpacer(5);
	sizer2->Add(bitmapBtnFindNext, 0, wxBOTTOM | wxTOP, 6);
	sizer2->AddSpacer(5);
	sizer2->Add(bitmapBtnMatchCase, 0, wxBOTTOM | wxTOP, 6);
	sizer2->AddSpacer(5);
	sizer2->Add(bitmapBtnLoopingSearch, 0, wxBOTTOM | wxTOP, 6);
	sizer2->AddSpacer(5);
	sizer2->Add(bitmapBtnSaveWindowPos, 0, wxBOTTOM | wxTOP, 6);
	sizer2->AddSpacer(5);
	sizer2->Add(bitmapBtnMenu, 0, wxBOTTOM | wxTOP, 6);
	panelToolBar->SetSizer(sizer2);
	sizer1->Add(panelToolBar, 0, wxALL | wxEXPAND, 6);
	sizer1->Add(txtLog, 1, wxEXPAND, 0);
	SetSizer(sizer1);
	Layout();
	SetPosition(wxPoint(consoleLog.windowXPos, consoleLog.windowYPos));
}

BEGIN_EVENT_TABLE(Rtt_LinuxConsole, wxFrame)
EVT_BUTTON(ID_BUTTON_SAVE, Rtt_LinuxConsole::OnBtnSaveClick)
EVT_BUTTON(ID_BUTTON_COPY, Rtt_LinuxConsole::OnBtnCopyClick)
EVT_BUTTON(ID_BUTTON_CLEAR, Rtt_LinuxConsole::OnBtnEraseClick)
EVT_BUTTON(ID_BUTTON_FIND_PREVIOUS, Rtt_LinuxConsole::OnBtnFindPreviousClick)
EVT_BUTTON(ID_BUTTON_FIND_NEXT, Rtt_LinuxConsole::OnBtnFindNextClick)
EVT_BUTTON(ID_BUTTON_MATCH_CASE, Rtt_LinuxConsole::OnBtnMatchCaseClick)
EVT_BUTTON(ID_BUTTON_LOOP_SEARCH, Rtt_LinuxConsole::OnBtnLoopingSearchClick)
EVT_BUTTON(ID_BUTTON_THEME, Rtt_LinuxConsole::OnBtnChangeThemeClick)
EVT_BUTTON(ID_BUTTON_SAVE_WINDOW_POS, Rtt_LinuxConsole::OnBtnSaveWindowPosClick)
EVT_COMMAND(wxID_ANY, eventFindNext, Rtt_LinuxConsole::OnBtnFindNextClick)
END_EVENT_TABLE();

void Rtt_LinuxConsole::OnBtnSaveClick(wxCommandEvent& event)
{
	struct passwd* pw = getpwuid(getuid());
	const char* homedir = pw->pw_dir;
	time_t timeNow = time(NULL);
	char buffer[32];
	char msBuffer[32];
	struct tm* timeInfo;
	struct timeval timeValue;
	gettimeofday(&timeValue, NULL);

	timeInfo = localtime(&timeValue.tv_sec);
	strftime(buffer, 26, "%m-%d-%y-%H.%M.%S", timeInfo);
	string filePath(homedir);
	filePath.append("/.Solar2D/").append("log_").append(buffer).append(".txt");
	bool saved = txtLog->SaveFile(filePath.c_str());

	if (saved)
	{
		string resultMsg("Saved log to: ");
		resultMsg.append(filePath.c_str());

		wxMessageDialog* msgDialog = new wxMessageDialog(this, resultMsg.c_str(), wxT("Save Log"), wxOK);
		msgDialog->ShowModal();
	}
}

void Rtt_LinuxConsole::OnBtnCopyClick(wxCommandEvent& event)
{
	txtLog->SetFocus();
	txtLog->Copy();
}

void Rtt_LinuxConsole::OnBtnEraseClick(wxCommandEvent& event)
{
	ClearLog();
}

void Rtt_LinuxConsole::OnBtnFindPreviousClick(wxCommandEvent& event)
{
	if (!txtFind->IsEmpty())
	{
		int searchFlags = (consoleLog.buttonMatchCaseOn) ? wxSTC_FIND_MATCHCASE : 0;
		wxString searchText = txtFind->GetLineText(0);
		txtLog->SearchAnchor();
		consoleLog.currentPosition = txtLog->SearchPrev(searchFlags, searchText);

		if (consoleLog.currentPosition > 0)
		{
			txtLog->EnsureCaretVisible();
			txtLog->SetCurrentPos(txtLog->GetAnchor() - searchText.Length());
		}
		else
		{
			if (consoleLog.buttonLoopingSearchOn)
			{
				txtLog->SetCurrentPos(txtLog->GetValue().Length());
				txtLog->SetSelection(txtLog->GetValue().Length(), txtLog->GetValue().Length());
				txtLog->SearchAnchor();
			}
		}
	}
}

void Rtt_LinuxConsole::OnBtnFindNextClick(wxCommandEvent& event)
{
	if (!txtFind->IsEmpty())
	{
		int searchFlags = (consoleLog.buttonMatchCaseOn) ? wxSTC_FIND_MATCHCASE : 0;
		wxString searchText = txtFind->GetLineText(0);

		if (consoleLog.currentPosition > 0)
		{
			txtLog->SetCurrentPos(txtLog->GetAnchor() + searchText.Length() + 1);
		}

		txtLog->SearchAnchor();
		consoleLog.currentPosition = txtLog->SearchNext(searchFlags, searchText);

		if (consoleLog.currentPosition > 0)
		{
			txtLog->EnsureCaretVisible();
		}
		else
		{
			if (consoleLog.buttonLoopingSearchOn)
			{
				txtLog->SetCurrentPos(0);
				txtLog->SetSelection(0, 0);
			}
		}
	}
}

void Rtt_LinuxConsole::OnBtnMatchCaseClick(wxCommandEvent& event)
{
	consoleLog.buttonMatchCaseOn = !consoleLog.buttonMatchCaseOn;
	bitmapBtnMatchCase->SetBitmap(consoleLog.buttonMatchCaseOn ? wxIcon(match_case_on_xpm) : wxIcon(match_case_xpm));
	consoleLog.config->Write(wxT(CONFIG_MATCH_CASE_ID), consoleLog.buttonMatchCaseOn);
	consoleLog.config->Flush();
	txtLog->SetFocus();
	ResetSearch();
}

void Rtt_LinuxConsole::OnBtnLoopingSearchClick(wxCommandEvent& event)
{
	consoleLog.buttonLoopingSearchOn = !consoleLog.buttonLoopingSearchOn;
	bitmapBtnLoopingSearch->SetBitmap(consoleLog.buttonLoopingSearchOn ? wxIcon(looping_search_on_xpm) : wxIcon(looping_search_xpm));
	consoleLog.config->Write(wxT(CONFIG_LOOPING_SEARCH_ID), consoleLog.buttonLoopingSearchOn);
	consoleLog.config->Flush();
	txtLog->SetFocus();
	ResetSearch();
}

void Rtt_LinuxConsole::OnBtnSaveWindowPosClick(wxCommandEvent& event)
{
	wxPoint windowPosition = GetPosition();
	wxSize windowSize = GetSize();
	consoleLog.config->Write(wxT(CONFIG_WINDOW_X_POSITION), windowPosition.x);
	consoleLog.config->Write(wxT(CONFIG_WINDOW_Y_POSITION), windowPosition.y);
	consoleLog.config->Write(wxT(CONFIG_WINDOW_WIDTH), windowSize.GetWidth());
	consoleLog.config->Write(wxT(CONFIG_WINDOW_HEIGHT), windowSize.GetHeight());
	consoleLog.config->Flush();

	wxMessageDialog* msgDialog = new wxMessageDialog(this, "Your window preferences have been saved successfully.", wxT("Window Preferences"), wxOK);
	msgDialog->ShowModal();
}

void Rtt_LinuxConsole::OnBtnChangeThemeClick(wxCommandEvent& event)
{
	dropdownMenu->Show(dropdownMenu->IsShown() ? false : true);
}

void Rtt_LinuxConsole::OnKeyUp(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_RETURN)
	{
		wxCommandEvent ev(eventFindNext);
		wxPostEvent(this, ev);
	}

	event.Skip();
}

void Rtt_LinuxConsole::ClearLog()
{
	txtLog->SetFocus();
	txtLog->SetReadOnly(false);
	txtLog->ClearAll();
	txtLog->SetReadOnly(true);

	consoleLog.warningCount = 0;
	consoleLog.errorCount = 0;
	UpdateStatusText();
}

void Rtt_LinuxConsole::UpdateStatusText()
{
	wxString statusbarText;
	statusbarText.append("Errors #").append(to_string(consoleLog.errorCount));
	statusbarText.append(" Alerts #").append(to_string(consoleLog.warningCount));
	statusbar->PushStatusText(statusbarText);
}

void Rtt_LinuxConsole::ResetSearch()
{
	txtLog->SelectNone();
	txtLog->SetCurrentPos(0);
	txtLog->SetSelection(0, 0);
}

void Rtt_LinuxConsole::ChangeTheme()
{
	bool isLightTheme = (consoleLog.currentTheme.IsSameAs(CONFIG_LIGHT_THEME_VALUE));
	consoleLog.themeTextColour = isLightTheme ? consoleLog.textColourLightTheme : consoleLog.textColourDarkTheme;
	consoleLog.themeBackgroundColour = isLightTheme ? consoleLog.backgroundColourLightTheme : consoleLog.backgroundColourDarkTheme;

	txtLog->StyleSetBackground(wxSTC_STYLE_DEFAULT, consoleLog.themeBackgroundColour);
	txtLog->StyleSetForeground(wxSTC_STYLE_DEFAULT, consoleLog.themeTextColour);
	txtLog->StyleClearAll();
	consoleLog.config->Write(wxT(CONFIG_THEME_ID), consoleLog.currentTheme);
	consoleLog.config->Flush();
}

void Rtt_LinuxConsole::HighlightLine(int indicatorNo, wxColour colour)
{
	txtLog->IndicatorSetAlpha(indicatorNo, 255);
	txtLog->IndicatorSetUnder(indicatorNo, true);
	txtLog->IndicatorSetStyle(indicatorNo, wxSTC_INDIC_FULLBOX);
	txtLog->IndicatorSetForeground(indicatorNo, colour);

	bool shouldChangeText = (indicatorNo == ID_INDICATOR_WARNING || indicatorNo == ID_INDICATOR_ERROR);
	int textTargetID = 0;
	wxColour textTargetColour;
	int lineNo = txtLog->GetCurrentLine();
	int startPosition = txtLog->PositionFromLine(lineNo);
	int endPosition = txtLog->GetLineEndPosition(lineNo);
	int length = (endPosition - startPosition);
	txtLog->SetIndicatorCurrent(indicatorNo);
	txtLog->IndicatorFillRange(startPosition, length);

	switch (indicatorNo)
	{
	case ID_INDICATOR_WARNING:
		textTargetID = ID_INDICATOR_WARNING_TEXT;
		textTargetColour = *wxBLACK;
		break;

	case ID_INDICATOR_ERROR:
		textTargetID = ID_INDICATOR_ERROR_TEXT;
		textTargetColour = *wxWHITE;
		break;
	}

	if (shouldChangeText)
	{
		txtLog->IndicatorSetStyle(textTargetID, wxSTC_INDIC_TEXTFORE);
		txtLog->IndicatorSetForeground(textTargetID, textTargetColour);
		txtLog->SetIndicatorCurrent(textTargetID);
		txtLog->IndicatorFillRange(startPosition, length);
	}
}

void Rtt_LinuxConsole::UpdateLog(wxString message)
{
	txtLog->SetReadOnly(false);
	txtLog->SetInsertionPointEnd();
	txtLog->AppendText(message);
	txtLog->SelectNone();
	txtLog->SetReadOnly(true);
	txtLog->ScrollToEnd();
}

void Rtt_LinuxConsole::UpdateLogWarning(wxString message)
{
	txtLog->SetReadOnly(false);
	txtLog->SetInsertionPointEnd();
	txtLog->AppendText(message);
	HighlightLine(ID_INDICATOR_WARNING, consoleLog.warningColour);
	txtLog->SelectNone();
	txtLog->SetReadOnly(true);
	txtLog->ScrollToEnd();

	consoleLog.warningCount++;
	UpdateStatusText();
}

void Rtt_LinuxConsole::UpdateLogError(wxString message)
{
	txtLog->SetReadOnly(false);
	txtLog->SetInsertionPointEnd();
	txtLog->AppendText(message);
	HighlightLine(ID_INDICATOR_ERROR, consoleLog.errorColour);
	txtLog->SelectNone();
	txtLog->SetReadOnly(true);
	txtLog->ScrollToEnd();

	consoleLog.errorCount++;
	UpdateStatusText();
}

DropdownMenu::DropdownMenu(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) :
	wxPanel(parent, id, pos, size, wxBORDER_STATIC)
{
	chkLightTheme = new wxCheckBox(this, ID_LIGHT_THEME_MENU_ITEM, wxEmptyString);
	chkDarkTheme = new wxCheckBox(this, ID_DARK_THEME_MENU_ITEM, wxEmptyString);

	if (consoleLog.currentTheme.IsSameAs(CONFIG_LIGHT_THEME_VALUE))
	{
		chkLightTheme->SetValue(true);
	}
	else
	{
		chkDarkTheme->SetValue(true);
	}

	SetProperties();
	DoLayout();
	Hide();
}

void DropdownMenu::SetProperties()
{
	SetBackgroundColour(*wxBLACK);
	SetForegroundColour(*wxWHITE);
}

void DropdownMenu::DoLayout()
{
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* menu2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* menu1 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* lblMenuThemeLight = new wxStaticText(this, wxID_ANY, wxT("Light Theme"));
	wxStaticText* lblMenuThemeDark = new wxStaticText(this, wxID_ANY, wxT("Dark Theme"));
	lblMenuThemeLight->SetMinSize(wxSize(100, 20));
	lblMenuThemeDark->SetMinSize(wxSize(100, 20));
	menu1->Add(chkLightTheme, 0, wxTOP, 2);
	menu1->Add(lblMenuThemeLight, 0, wxTOP, 2);
	sizer->Add(menu1, 1, wxEXPAND, 0);
	menu2->Add(chkDarkTheme, 0, wxTOP, 2);
	menu2->Add(lblMenuThemeDark, 0, wxTOP, 2);
	sizer->Add(menu2, 1, wxEXPAND, 0);
	SetSizer(sizer);
	sizer->Fit(this);
}

BEGIN_EVENT_TABLE(DropdownMenu, wxPanel)
EVT_CHECKBOX(ID_LIGHT_THEME_MENU_ITEM, DropdownMenu::OnChkLightThemeClicked)
EVT_CHECKBOX(ID_DARK_THEME_MENU_ITEM, DropdownMenu::OnChkDarkThemeClicked)
END_EVENT_TABLE();

void DropdownMenu::OnChkLightThemeClicked(wxCommandEvent& event)
{
	chkDarkTheme->SetValue(false);
	chkLightTheme->SetValue(true);
	consoleLog.currentTheme = CONFIG_LIGHT_THEME_VALUE;
	linuxConsole->ChangeTheme();
	Hide();
}

void DropdownMenu::OnChkDarkThemeClicked(wxCommandEvent& event)
{
	chkLightTheme->SetValue(false);
	chkDarkTheme->SetValue(true);
	consoleLog.currentTheme = CONFIG_DARK_THEME_VALUE;
	linuxConsole->ChangeTheme();
	Hide();
}
