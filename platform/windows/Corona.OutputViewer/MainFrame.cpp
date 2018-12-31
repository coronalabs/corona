//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainFrame.h"
#include "Core\Rtt_Version.h"
#include "Interop\ApplicationServices.h"
#include "Interop\Ticks.h"
#include "LogEntry.h"
#include "MainApp.h"
#include "MFCToolBarLabel.h"
#include "resource.h"
#include "VisualTheme.h"
#include "WinString.h"
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>


#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


#pragma region Static Member Variables
/// <summary>
///  <para>Array of resource "command" IDs to be passed into the statusbar's SetIndicators() method.</para>
///  <para>These define each pane shown in the status bar.</para>
///  <para>
///   WARNING: Each custom ID in this array must have a string defined in the resource file's string table
///   or else the statusbar's SetIndicators() method will assert.
///  </para>
/// </summary>
static UINT kStatusBarIndicatorIdArray[] =
{
	ID_SEPARATOR,
	ID_STATUSBAR_ERROR_COUNT,
	ID_STATUSBAR_WARNING_COUNT,
};

/// <summary>UTF-16 format string used to create an "Hour:Minute:Second.Millisecond" timestamp string.</summary>
static const wchar_t kRtfTimestampFormatString[] = L"%02d:%02d:%02d.%03d  ";

/// <summary>
///  <para>Max TCHAR's allowed in the RTF control.</para>
///  <para>
///   Old log entries at the top of the RTF controls are expected to be removed/rolled-off once this
///   maximum is about to be exceeded.
///  </para>
///  <para>Currently set up to store 4 megabytes worth of characters.</para>
/// </summary>
static const long kRtfMaxCharacters = 4194304 / sizeof(TCHAR);

/// <summary>Name of the registry entry used to store the show/hide state of the "Alert List" pane.</summary>
static const TCHAR kShowAlertListPaneRegistryEntryName[] = _T("ShowAlertListPane");

#pragma endregion


#pragma region Message Mappings
IMPLEMENT_DYNAMIC(MainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(MainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_GETMINMAXINFO()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_WM_TIMER()
	ON_WM_COPYDATA()
	ON_NOTIFY(EN_LINK, AFX_IDW_PANE_FIRST, OnLinkNotification)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_PROCESS_START, OnProcessStart)
	ON_UPDATE_COMMAND_UI(ID_PROCESS_START, OnUpdateProcessStart)
	ON_COMMAND(ID_PROCESS_STOP, OnProcessStop)
	ON_UPDATE_COMMAND_UI(ID_PROCESS_STOP, OnUpdateProcessStop)
	ON_COMMAND(ID_EDIT_COPY, OnCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateCopy)
	ON_COMMAND(ID_EDIT_CLEAR_ALL, OnClearAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR_ALL, OnUpdateAlwaysEnabledCommand)
	ON_COMMAND(ID_FIND, OnFind)
	ON_COMMAND(ID_FIND_NEXT, OnFindNext)
	ON_COMMAND(ID_FIND_PREVIOUS, OnFindPrevious)
	ON_UPDATE_COMMAND_UI(ID_FIND_NEXT, OnUpdateAlwaysEnabledCommand)
	ON_UPDATE_COMMAND_UI(ID_FIND_PREVIOUS, OnUpdateAlwaysEnabledCommand)
	ON_UPDATE_COMMAND_UI(ID_FIND_TEXTBOX, OnUpdateAlwaysEnabledCommand)
	ON_UPDATE_COMMAND_UI(ID_FIND_MATCH_CASE, OnUpdateAlwaysEnabledCommand)
	ON_UPDATE_COMMAND_UI(ID_FIND_LOOPING, OnUpdateAlwaysEnabledCommand)
	ON_COMMAND(ID_SETUP_SUBMENU, OnSetupSubmenu)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_UPDATE_COMMAND_UI(ID_APP_ABOUT, OnUpdateAlwaysEnabledCommand)
	ON_COMMAND(ID_HELP, OnHelp)
	ON_UPDATE_COMMAND_UI(ID_HELP, OnUpdateAlwaysEnabledCommand)
	ON_UPDATE_COMMAND_UI(ID_STATUSBAR_ERROR_COUNT, OnUpdateAlwaysEnabledCommand)
	ON_UPDATE_COMMAND_UI(ID_STATUSBAR_WARNING_COUNT, OnUpdateAlwaysEnabledCommand)
	ON_COMMAND_RANGE(ID_THEME_SYSTEM, ID_THEME_BLUE, OnVisualThemeSelectedById)
	ON_UPDATE_COMMAND_UI_RANGE(ID_THEME_SYSTEM, ID_THEME_BLUE, OnUpdateVisualThemeMenuItem)
END_MESSAGE_MAP()

#pragma endregion


#pragma region Constructors/Destructors
MainFrame::MainFrame()
:	fIsProcessingQueuedLogEntries(false),
	fVisualThemeChangedEventHandler(this, &MainFrame::OnVisualThemeChanged),
	fProcessExitedEventHandler(this, &MainFrame::OnProcessExited),
	fPipeClosedEventHandler(this, &MainFrame::OnPipeClosed),
	fPipeReceivedDataEventHandler(this, &MainFrame::OnPipeReceivedData),
	fAlertListRequestingShowSelectedEntryEventHandler(this, &MainFrame::OnAlertListRequestingShowSelectedEntry),
	fAlertListEntryCountChangedEventHandler(this, &MainFrame::OnAlertListEntryCountChanged),
	fAsyncStdInReaderPointer(nullptr),
	fFindEditControlHandle(nullptr)
{
	// Listen for visual theme changes.
	auto applicationPointer = (MainApp*)AfxGetApp();
	applicationPointer->GetVisualThemeChangedEventHandlers().Add(&fVisualThemeChangedEventHandler);
}

MainFrame::~MainFrame()
{
	// Delete the stdin reader if created.
	if (fAsyncStdInReaderPointer)
	{
		delete fAsyncStdInReaderPointer;
		fAsyncStdInReaderPointer = nullptr;
	}

	// Remove event handlers.
	auto applicationPointer = (MainApp*)AfxGetApp();
	applicationPointer->GetVisualThemeChangedEventHandlers().Remove(&fVisualThemeChangedEventHandler);
}

#pragma endregion


#pragma region Event Handlers
int MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Fetch a pointer to the application instance.
	auto appPointer = (MainApp*)::AfxGetApp();

	// Call the base class' OnCreate() method first.
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	// If a custom window name was provided, then apply it.
	if (fCustomWindowName.GetLength() > 0)
	{
		SetWindowTextW(fCustomWindowName);
	}

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);
	
	// Set up the rich edit control.
	fRichEditControl.Create(
			AFX_WS_DEFAULT_VIEW | ES_MULTILINE | ES_NOHIDESEL | WS_VSCROLL, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST);
	fRichEditControl.SetOptions(ECOOP_OR, ECO_AUTOVSCROLL | ECO_SAVESEL | ECO_READONLY);
	fRichEditControl.SetAutoURLDetect(TRUE);
	fRichEditControl.SetEventMask(fRichEditControl.GetEventMask() | ENM_LINK);
	{
		// Set up the control to use a monospaced font that is compatible with OS X.
		CHARFORMAT characterFormat{};
		characterFormat.cbSize = sizeof(characterFormat);
		characterFormat.dwMask = CFM_FACE;
		_tcsncpy_s(characterFormat.szFaceName, LF_FACESIZE, _T("Courier New"), _TRUNCATE);
		fRichEditControl.SetDefaultCharFormat(characterFormat);
	}
	{
		// Set up the control's paragraph formatting to indent subsquent lines in a paragraph past the timestamp string.
		// Note: This only works if the font is monospaced and if the timestamped string has a fixed character count.
		double fractionalTextWidth = 0;
		{
			// Fetch the control's current font settings.
			CHARFORMAT characterFormat{};
			characterFormat.cbSize = sizeof(characterFormat);
			fRichEditControl.GetDefaultCharFormat(characterFormat);
			
			// Convert the control's font settings to an MFC CFont.
			LOGFONT logFont{};
			logFont.lfWeight = (characterFormat.dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL;
			logFont.lfHeight = characterFormat.yHeight * -1;
			logFont.lfCharSet = characterFormat.bCharSet;
			logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
			logFont.lfPitchAndFamily = characterFormat.bPitchAndFamily;
			memcpy(logFont.lfFaceName, characterFormat.szFaceName, LF_FACESIZE);
			CFont rtfFont;
			rtfFont.CreateFontIndirect(&logFont);

			// Have the control's device context temporarily select the above CFont.
			CClientDC deviceContext(&fRichEditControl);
			deviceContext.SetMapMode(MM_TWIPS);
			auto lastSelectedFontPointer = deviceContext.SelectObject(&rtfFont);

			// Measure the timestamp string in TWIPS.
			WinString timestampString;
			timestampString.Format(kRtfTimestampFormatString, 12, 0, 0, 0);
			fractionalTextWidth = (double)deviceContext.GetTextExtent(
					timestampString.GetTCHAR(), timestampString.GetLength()).cx;
			if (fractionalTextWidth < 0)
			{
				fractionalTextWidth *= -1.0;
			}

			// Restore the control's font settings.
			deviceContext.SelectObject(lastSelectedFontPointer);
		}
		PARAFORMAT paragraphFormat{};
		paragraphFormat.cbSize = sizeof(paragraphFormat);
		paragraphFormat.dwMask = PFM_OFFSET;
		paragraphFormat.dxOffset = (LONG)(fractionalTextWidth + 0.5);
		fRichEditControl.SetSel(0, -1);
		fRichEditControl.SetParaFormat(paragraphFormat);
		fRichEditControl.SetSel(0, 0);
	}

	// Set up the toolbar control.
	fToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE);
	fToolBar.LoadToolBar(IDR_MAINFRAME);
	fToolBar.SetWindowText(_T("Toolbar"));
	fToolBar.SetPaneStyle((fToolBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC) & ~(CBRS_GRIPPER | CBRS_SIZE_FIXED));
	fToolBar.SetExclusiveRowMode(TRUE);
	fToolBar.SetRestoredFromRegistry(FALSE);
	fToolBar.SetOwner(this);
	fToolBar.SetRouteCommandsViaFrame(TRUE);
	{
		// Add an unclickable "Find:" label next to the search box.
		MFCToolBarLabel toolbarLabel(ID_FIND_LABEL, _T("Find:"));
		fToolBar.ReplaceButton(ID_FIND_LABEL, toolbarLabel);
	}
	{
		// Add a search textbox to the toolbar.
		CMFCToolBarEditBoxButton toolbarEditBoxButton(ID_FIND_TEXTBOX, 0, ES_AUTOHSCROLL, 250);
		toolbarEditBoxButton.SetFlatMode(FALSE);
		fToolBar.ReplaceButton(ID_FIND_TEXTBOX, toolbarEditBoxButton);
	}
	{
		// Remove the following toolbar buttons.
		// They only exist on the toolbar resource so that we can easily select their images from the image list by ID.
		fToolBar.RemoveButton(fToolBar.CommandToIndex(ID_APP_ABOUT));
		fToolBar.RemoveButton(fToolBar.CommandToIndex(ID_HELP));

		// Add our main tools/setup menu to the end of the toolbar.
		CMenu menu;
		menu.LoadMenu(IDR_TOOLBAR_SETUP_MENU);
		auto themeMenuPointer = menu.GetSubMenu(0)->GetSubMenu(0);
		themeMenuPointer->CheckMenuRadioItem(0, themeMenuPointer->GetMenuItemCount() - 1, 0, MF_BYPOSITION);
		CMFCToolBarMenuButton toolbarMenuButton(
				0, menu.GetSubMenu(0)->GetSafeHmenu(),
				fToolBar.GetButton(fToolBar.CommandToIndex(ID_SETUP_SUBMENU))->GetImage(),
				_T("Tools Menu (Alt+X)"));
		fToolBar.ReplaceButton(ID_SETUP_SUBMENU, toolbarMenuButton);
		menu.Detach();
	}
	fToolBar.SetButtonStyle(fToolBar.CommandToIndex(ID_FIND_MATCH_CASE), TBBS_CHECKBOX);
	fToolBar.SetButtonStyle(fToolBar.CommandToIndex(ID_FIND_LOOPING), TBBS_CHECKBOX | TBBS_CHECKED);
	fToolBar.AdjustLayout();

	// Set up the status bar control.
	fStatusBar.Create(this);
	fStatusBar.SetIndicators(kStatusBarIndicatorIdArray, sizeof(kStatusBarIndicatorIdArray) / sizeof(UINT));
	{
		CClientDC clientDC(&fStatusBar);
		auto dpi = clientDC.GetDeviceCaps(LOGPIXELSX);
		if (dpi > 0)
		{
			int widthInPixels = (int)(100.0 * ((double)dpi / 96.0));
			fStatusBar.SetPaneWidth(fStatusBar.CommandToIndex(ID_STATUSBAR_ERROR_COUNT), widthInPixels);
			fStatusBar.SetPaneWidth(fStatusBar.CommandToIndex(ID_STATUSBAR_WARNING_COUNT), widthInPixels);
		}
	}
	DockPane(&fToolBar, AFX_IDW_DOCKBAR_TOP);
	UpdateStatusBarLogEntryCount();

	// Set up this window's to support docked panels such as the "Alert List".
	CDockingManager::SetDockingMode(DT_SMART);
	EnableAutoHidePanes(CBRS_ALIGN_ANY);
	
	// Set up the "Alert List" pane.
	DWORD windowStyles = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI;
	DWORD controlBarStyles = AFX_CBRS_RESIZE | AFX_CBRS_AUTOHIDE;
	auto wasCreated = fAlertListPane.Create(
			_T("Alert List"), this, CRect(0, 0, 100, 180), TRUE, 0, windowStyles, AFX_CBRS_REGULAR_TABS, controlBarStyles);
	fAlertListPane.EnableDocking(CBRS_NOALIGN);
	DockPane(&fAlertListPane);
	fAlertListPane.GetRequestingShowSelectedEntryEventHandlers().Add(&fAlertListRequestingShowSelectedEntryEventHandler);
	fAlertListPane.GetEntryCountChangedEventHandlers().Add(&fAlertListEntryCountChangedEventHandler);
	{
		// If the user has shown/hidden this pane last time, then restore that setting. (Shown by default.)
		auto intValue = appPointer->GetProfileInt(
				MainApp::kSettingsRegistrySectionName, kShowAlertListPaneRegistryEntryName, 1);
		auto wasPreviouslyShown = (intValue != 0);
		bool isCurrentlyShown = (fAlertListPane.IsAutoHideMode() == FALSE);
		if (wasPreviouslyShown != isCurrentlyShown)
		{
			fAlertListPane.ToggleAutoHide();
		}
	}

	// Special set up when configured to read from stdin only.
	if (appPointer->IsStdInExclusiveModeEnabled())
	{
		// Start reading from the stdin pipe immediately.
		// Note: This pipe reader runs until it breaks. There is no restarting it. The parent process is in control.
		if (!fAsyncStdInReaderPointer)
		{
			auto stdInHandle = ::GetStdHandle(STD_INPUT_HANDLE);
			if (stdInHandle && (stdInHandle != INVALID_HANDLE_VALUE))
			{
				fAsyncStdInReaderPointer = new Interop::Ipc::AsyncPipeReader(stdInHandle);
				fAsyncStdInReaderPointer->GetClosedEventHandlers().Add(&fPipeClosedEventHandler);
				fAsyncStdInReaderPointer->GetReceivedDataEventHandlers().Add(&fPipeReceivedDataEventHandler);
				fAsyncStdInReaderPointer->Start();
			}
		}

		// Disable the window's close [x] button if requested at the command line.
		auto menuPointer = GetSystemMenu(FALSE);
		if (menuPointer)
		{
			UINT enableFlag = MF_ENABLED;
			if (appPointer->IsDisableCloseWhileReadingEnabled())
			{
				if (fAsyncStdInReaderPointer && fAsyncStdInReaderPointer->IsRunning())
				{
					enableFlag = MF_DISABLED | MF_GRAYED;
				}
			}
			menuPointer->EnableMenuItem(SC_CLOSE, enableFlag);
		}

		// Remove toolbar buttons "Open", "Start", and "Stop".
		// These are not supported when reading from the parent process' stdin pipe.
		fToolBar.RemoveButton(fToolBar.CommandToIndex(ID_FILE_OPEN));
		fToolBar.RemoveButton(fToolBar.CommandToIndex(ID_PROCESS_START));
		fToolBar.RemoveButton(fToolBar.CommandToIndex(ID_PROCESS_STOP));
		fToolBar.AdjustLayout();
	}

	// Fetch a handle to the "Find" text box on the toolbar for fast access later.
	{
		auto buttonPointer = fToolBar.GetButton(fToolBar.CommandToIndex(ID_FIND_TEXTBOX));
		auto editBoxButtonPointer = dynamic_cast<CMFCToolBarEditBoxButton*>(buttonPointer);
		if (editBoxButtonPointer)
		{
			auto editBoxPointer = editBoxButtonPointer->GetEditBox();
			if (editBoxPointer)
			{
				fFindEditControlHandle = editBoxPointer->GetSafeHwnd();
			}
		}
	}

	// Apply the application's current theme to this window's UI.
	OnVisualThemeChanged(*appPointer, Interop::EventArgs::kEmpty);

	// Set up a short timer to process all pending/queued log messages.
	// This is an optimization. It allows us to insert multiple log entries in batches.
	SetTimer(0, 10, nullptr);

	// Set up this process to receive WM_COPYDATA messages from other application processes.
	// Note: The ChangeWindowMessageFilter() call is only needed on Windows Vista and newer OS versions.
	HMODULE moduleHandle = ::LoadLibraryW(L"user32");
	if (moduleHandle)
	{
		typedef BOOL(WINAPI *ChangeWindowMessageFilterCallback)(UINT, DWORD);
		auto callback = (ChangeWindowMessageFilterCallback)::GetProcAddress(moduleHandle, "ChangeWindowMessageFilter");
		if (callback)
		{
			callback(WM_COPYDATA, MSGFLT_ADD);
		}
		::FreeLibrary(moduleHandle);
	}

	// Returning zero indicates that we've successfully initialized this window.
	return 0;
}

void MainFrame::OnClose()
{
	// Do not close this window if a child dialog is currently displayed.
	// Note: This prevents the Corona Simulator from remotely closing this window when it exits.
	//       If a child dialog is displayed, then assume ther use is doing something useful such as a SaveAs.
	if (IsWindowEnabled() == FALSE)
	{
		return;
	}

	// Do not close this window if its "Close" [x] button is currently disabled.
	// This way we can honor the /disableClose command line argument, which is what the Corona Simulator uses.
	// Even with the Close button disabled, this OnClose() method can be called for the following reasons:
	// - User presses Alt+F4.
	// - User closes the window via the Windows TaskBar.
	// - A WM_CLOSE message was received from an external application.
	auto menuPointer = GetSystemMenu(FALSE);
	if (menuPointer)
	{
		auto itemState = menuPointer->GetMenuState(SC_CLOSE, MF_BYCOMMAND);
		if (itemState & MF_DISABLED)
		{
			return;
		}
	}

	// If we're currently monitoring the stdout/stderr of a process, then ask the user if its okay to exit it.
	if (fProcessPointer && !fProcessPointer->HasExited())
	{
		const wchar_t message[] =
				L"You are about to close this window.\r\n"
				L"Do you also want to close the application that it is currently running?";
		auto result = MessageBoxW(message, L"Warning", MB_ICONQUESTION | MB_YESNOCANCEL);
		if (IDYES == result)
		{
			OnProcessStop();
		}
		else if (IDCANCEL == result)
		{
			return;
		}
	}

	// Store the "Alert List" pane's current show/hide state to the registry.
	{
		auto intValue = (fAlertListPane.IsAutoHideMode() == FALSE) ? 1 : 0;
		AfxGetApp()->WriteProfileInt(MainApp::kSettingsRegistrySectionName, kShowAlertListPaneRegistryEntryName, intValue);
	}

	// Close this window.
	CFrameWndEx::OnClose();
}

void MainFrame::OnGetMinMaxInfo(MINMAXINFO* minMaxInfoPointer)
{
	// Let the base class handle it first.
	CFrameWndEx::OnGetMinMaxInfo(minMaxInfoPointer);

	// Fetch the system's DPI scale factor.
	double dpiScale = 1.0;
	{
		auto deviceContextPointer = GetDC();
		if (deviceContextPointer)
		{
			auto dpiValue = deviceContextPointer->GetDeviceCaps(LOGPIXELSX);
			if (dpiValue > 0)
			{
				dpiScale = (double)dpiValue / 96.0;
			}
		}
	}

	// Apply a minimum width/height for this window.
	CRect minRectSize(0, 0, 700, 400);
	if (dpiScale != 1.0)
	{
		minRectSize.right = (LONG)((double)minRectSize.right * dpiScale);
		minRectSize.bottom = (LONG)((double)minRectSize.bottom * dpiScale);
	}
	CalcWindowRect(&minRectSize);
	minMaxInfoPointer->ptMinTrackSize.x = minRectSize.Width();
	minMaxInfoPointer->ptMinTrackSize.y = minRectSize.Height();
}

void MainFrame::OnSetFocus(CWnd* lastFocusedWindowPointer)
{
	// Forward the focus to the rich edit control.
	fRichEditControl.SetFocus();
}

BOOL MainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (fRichEditControl.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	{
		return TRUE;
	}

	// otherwise, do default handling
	return CFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void MainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CFrameWndEx::OnSettingChange(uFlags, lpszSection);
}

void MainFrame::OnTimer(UINT_PTR eventId)
{
	ProcessQueuedLogEntries();
}

BOOL MainFrame::OnCopyData(CWnd* windowPointer, COPYDATASTRUCT* dataPointer)
{
	// Validate.
	if (!dataPointer)
	{
		return FALSE;
	}

	// Attempt to handle the given IPC (Inter-Process Communication) command.
	// Note: These commands are expected to come from this application's command line arguments in "MainApp.cpp".
	switch (dataPointer->dwData)
	{
		case (DWORD)IpcCommandId::kIsFileSelected:
		{
			// Validate file paths.
			int characterLength = dataPointer->cbData / sizeof(wchar_t);
			if (fAppFilePath.IsEmpty() || (nullptr == dataPointer->lpData) || (characterLength <= 0))
			{
				return FALSE;
			}

			// Fetch the file path from the given Win32 message.
			std::wstring receivedString((wchar_t*)dataPointer->lpData, characterLength);
			std::wistringstream stringStream(receivedString);
			std::wstring filePath(L"");
			std::getline(stringStream, filePath);
			if (filePath.empty())
			{
				return FALSE;
			}

			// Make sure the given file path is an absolute path.
			// Note: The "fAppFilePath" member variable always stores its path in absolute form.
			MainApp::ConvertToAbsolutPath(filePath);

			// Return true if the given file path matches the path this application window is running.
			return (_wcsicmp(fAppFilePath, filePath.c_str()) == 0) ? TRUE : FALSE;
		}
		case (DWORD)IpcCommandId::kIsRunning:
		{
			// Return true if this window is currently monitoring its referenced application.
			return IsMonitoring() ? TRUE : FALSE;
		}
		case (DWORD)IpcCommandId::kRun:
		{
			// Extract the optional file path and its command line arguments from the given Win32 message.
			// If a file path was not provided, then this window will attempt to run its currently selected app file.
			std::wstring filePath(L"");
			std::wstring commandLineArguments(L"");
			{
				int characterLength = dataPointer->cbData / sizeof(wchar_t);
				if (dataPointer->lpData && (characterLength > 0))
				{
					// Parse the Win32 message's byte array.
					// Note: Both the file path and command line arguments are stored in this one array
					//       and they are expected to be separated by a newline '\n' character.
					std::wstring receivedString((wchar_t*)dataPointer->lpData, characterLength);
					std::wistringstream stringStream(receivedString);
					std::getline(stringStream, filePath);
					if (stringStream)
					{
						std::getline(stringStream, commandLineArguments);
					}

					// Make sure the given file path is in absolute path form.
					MainApp::ConvertToAbsolutPath(filePath);

					// Return an error if the given string is not a valid file path.
					if (!filePath.empty())
					{
						if (::PathFileExistsW(filePath.c_str()) == FALSE)
						{
							return FALSE;
						}
					}
				}
			}

			// Bring this window to the front of the desktop.
			SetForegroundWindow();

			// Start running and monitoring the application.
			if (filePath.empty())
			{
				OnProcessStart();
			}
			else
			{
				StartMonitoringApp(filePath.c_str(), commandLineArguments.c_str());
			}
			return TRUE;
		}
		case (DWORD)IpcCommandId::kStop:
		{
			// Stop the application process this window is monitoring.
			OnProcessStop();
			return TRUE;
		}
	}

	// We were given an unknown command.
	return FALSE;
}

void MainFrame::OnLinkNotification(LPNMHDR notifyHeaderPointer, LRESULT* resultPointer)
{
	// Validate arguments.
	if (!notifyHeaderPointer || !resultPointer)
	{
		return;
	}

	// Handle the URL mouse event.
	auto linkInfoPointer = (ENLINK*)notifyHeaderPointer;
	if (WM_LBUTTONDOWN == linkInfoPointer->msg)
	{
		// The user has clicked on the URL. Display its web page via the system's default web browser.
		try
		{
			CString urlText;
			fRichEditControl.GetTextRange(linkInfoPointer->chrg.cpMin, linkInfoPointer->chrg.cpMax, urlText);
			::ShellExecute(nullptr, L"open", urlText, nullptr, nullptr, SW_SHOWNORMAL);
			*resultPointer = 1;
		}
		catch (...) {}
	}
	else
	{
		// For all other mouse events (such as mouse move and button up events) let the control do its default handling.
		// This allows the control to display the default hand cursor while hover over the URL.
		*resultPointer = 0;
	}
}

void MainFrame::OnFileOpen()
{
	static const TCHAR kRegistryEntryName[] = _T("LastOpenDirectoryPath");

//TODO: In the future, we should add support for selecting Android *.apk files.

	// Do not continue if we're already monitoring an application now.
	// They must stop the current one before selecting a new application to monitor/run.
	if (IsMonitoring())
	{
		return;
	}

	// Fetch the user's previous "Open File" directory path.
	CString directoryPath = AfxGetApp()->GetProfileString(MainApp::kSettingsRegistrySectionName, kRegistryEntryName);

	// Default to the "Documents\Corona Built Apps" directory if a previous path was not found or was invalid.
	if (!directoryPath.IsEmpty() && !::PathIsDirectory(directoryPath))
	{
		directoryPath.Empty();
	}
	if (directoryPath.IsEmpty())
	{
		// First, default to the Windows "Documents" directory.
		::SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, 0, directoryPath.GetBuffer(MAX_PATH));
		directoryPath.ReleaseBuffer();

		// Next, attempt to deafult to the "Corona Built Apps" directory if it exists.
		// If not, then we'll use the above "Documents" directory instead.
		CString coronaBuiltAppsDirectoryPath = directoryPath;
		auto lastCharacter = coronaBuiltAppsDirectoryPath.GetAt(directoryPath.GetLength() - 1);
		if ((lastCharacter != _T('\\')) && (lastCharacter != _T('/')))
		{
			coronaBuiltAppsDirectoryPath.Append(_T("\\"));
		}
		coronaBuiltAppsDirectoryPath.Append(_T("Corona Built Apps"));
		if (::PathIsDirectory(coronaBuiltAppsDirectoryPath))
		{
			directoryPath = coronaBuiltAppsDirectoryPath;
		}
	}

	// Display an "Open File" dialog which asks the user to select an EXE file to run.
	const TCHAR kFileFilter[] = _T("Programs (*.exe)|*.exe||");
	CFileDialog openFileDialog(TRUE, L"exe", nullptr, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, kFileFilter);
	openFileDialog.m_ofn.lpstrTitle = _T("Select an App to Run");
	if (!directoryPath.IsEmpty())
	{
		openFileDialog.m_ofn.lpstrInitialDir = directoryPath;
	}
	auto result = openFileDialog.DoModal();
	if (result != IDOK)
	{
		return;
	}

	// Start running/monitoring the selected application.
	WinString filePath;
	filePath.SetTCHAR(openFileDialog.GetPathName());
	StartMonitoringApp(filePath.GetUTF16(), nullptr);

	// Store the selected file's directory path to the registry.
	// To be used as the default directory the next time the "Open File" button is clicked.
	{
		auto fileNamePointer = ::PathFindFileName(filePath.GetTCHAR());
		if (fileNamePointer)
		{
			directoryPath = filePath.GetTCHAR();
			int fileNameIndex = fileNamePointer - filePath.GetTCHAR();
			directoryPath.Delete(fileNameIndex, _tcsclen(fileNamePointer));
			directoryPath.TrimRight(_T("\\/"));
		}
		if (!directoryPath.IsEmpty())
		{
			AfxGetApp()->WriteProfileString(MainApp::kSettingsRegistrySectionName, kRegistryEntryName, directoryPath);
		}
	}
}

void MainFrame::OnUpdateFileOpen(CCmdUI* commandPointer)
{
	if (commandPointer)
	{
		commandPointer->Enable(IsMonitoring() ? FALSE : TRUE);
	}
}

void MainFrame::OnFileSaveAs()
{
	static const TCHAR kRegistryEntryName[] = _T("LastSaveDirectoryPath");

	// Fetch the user's previous "Save As" destination path.
	CString directoryPath = AfxGetApp()->GetProfileString(MainApp::kSettingsRegistrySectionName, kRegistryEntryName);

	// Default to the "Documents" directory if a previous path was not found or was invalid.
	if (!directoryPath.IsEmpty() && !::PathIsDirectory(directoryPath))
	{
		directoryPath.Empty();
	}
	if (directoryPath.IsEmpty())
	{
		::SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, 0, directoryPath.GetBuffer(MAX_PATH));
		directoryPath.ReleaseBuffer();
	}

	// Display a "Save As" dialog asking the user to select a file name and path.
	const TCHAR kFileFilter[] = _T("Rich Text Format (*.rtf)|*.rtf|Text Document (*.txt)|*.txt||");
	CFileDialog saveAsDialog(FALSE, L"rtf", nullptr, OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, kFileFilter);
	if (!directoryPath.IsEmpty())
	{
		saveAsDialog.m_ofn.lpstrInitialDir = directoryPath;
	}
	auto result = saveAsDialog.DoModal();
	if (result != IDOK)
	{
		return;
	}

	// Fetch the user's entered file name and path.
	CString filePath = saveAsDialog.GetPathName();
	if (filePath.GetLength() <= 0)
	{
		return;
	}

	// Store the selected destination directory to the registry.
	// To be used as the default destination directory the next time the SaveAs button is clicked.
	{
		auto fileNamePointer = ::PathFindFileName(filePath);
		if (fileNamePointer)
		{
			directoryPath = filePath;
			int fileNameIndex = fileNamePointer - (LPCTSTR)filePath;
			directoryPath.Delete(fileNameIndex, _tcsclen(fileNamePointer));
			directoryPath.TrimRight(_T("\\/"));
		}
		if (!directoryPath.IsEmpty())
		{
			AfxGetApp()->WriteProfileString(MainApp::kSettingsRegistrySectionName, kRegistryEntryName, directoryPath);
		}
	}

	// Determine if the user has selected an *.rtf or *.txt file extension.
	bool isRtfFile = false;
	{
		CString lowercaseFileExtension = saveAsDialog.GetFileExt().MakeLower();
		if (_T("rtf") == lowercaseFileExtension)
		{
			isRtfFile = true;
		}
	}

	// Temporarily switch to the system's default text color settings for all non-error/warning messages.
	// This is in case we're currently using a dark visual theme which uses white text.
	// An *.rtf file with white text will appear invisible on a white background, which is the default in WordPad.
	CHARFORMAT2 oldCharacterFormat{};
	oldCharacterFormat.cbSize = sizeof(oldCharacterFormat);
	fRichEditControl.GetDefaultCharFormat(oldCharacterFormat);
	CHARFORMAT2 newCharacterFormat(oldCharacterFormat);
	newCharacterFormat.dwEffects |= CFE_AUTOCOLOR;
	fRichEditControl.SetDefaultCharFormat(newCharacterFormat);

	// Attempt to save the RTF control's text contents to file.
	bool wasSaved = false;
	WinString errorMessage;
	try
	{
		// Create and open the file stream.
		CFile destinationFile(filePath, CFile::modeCreate | CFile::modeWrite);

		// Set up a callback used to receive the RTF control's text to write to file.
		auto streamOutLambda = [](
			DWORD cookie, LPBYTE byteBuffer, LONG bytesToWrite, LONG* bytesWrittenPointer)->DWORD
		{
			// Validate.
			if (!cookie || !byteBuffer)
			{
				return 0;
			}

			// Write the next batch of bytes to file.
			try
			{
				auto filePointer = (CFile*)cookie;
				filePointer->Write(byteBuffer, bytesToWrite);
				if (bytesWrittenPointer)
				{
					*bytesWrittenPointer = bytesToWrite;
				}
			}
			catch (...)
			{
				return 1;
			}
			return 0;
		};

		// Write the RTF control's text to file.
		EDITSTREAM editStream{};
		editStream.dwCookie = (DWORD_PTR)&destinationFile;
		editStream.pfnCallback = streamOutLambda;
		fRichEditControl.StreamOut(isRtfFile ? SF_RTF : SF_TEXT, editStream);

		// Success! We've written the entire RTF control's contents to file.
		wasSaved = true;
	}
	catch (CException* exceptionPointer)
	{
		if (exceptionPointer)
		{
			errorMessage.Expand(1024);
			errorMessage.GetBuffer()[0] = L'\0';
			exceptionPointer->GetErrorMessage(errorMessage.GetBuffer(), errorMessage.GetBufferNumChars());
			exceptionPointer->Delete();
		}
	}
	catch (const std::exception& exception)
	{
		errorMessage.SetUTF8(exception.what());
	}
	catch (...) {}

	// Restore our default text settings before the save occurred. (ie: Re-apply this window's visual theme.)
	fRichEditControl.SetDefaultCharFormat(oldCharacterFormat);

	// Do the following if we've failed to save to file up above.
	if (!wasSaved)
	{
		// Delete the file that was generated above. Odds are it's in a bad/corrupted state.
		::DeleteFile(filePath);

		// Display an error message to the user detailing what went wrong.
		std::wstring utf16Message = L"Failed to write to file. Reason:\r\n";
		if (errorMessage.GetLength() > 0)
		{
			utf16Message += errorMessage.GetUTF16();
		}
		else
		{
			utf16Message += L"Unknown";
		}
		MessageBoxW(utf16Message.c_str(), L"Error", MB_ICONWARNING | MB_OK);
	}
}

void MainFrame::OnUpdateFileSaveAs(CCmdUI* commandPointer)
{
	if (commandPointer)
	{
		commandPointer->Enable((fRichEditControl.GetTextLength() > 0) ? TRUE : FALSE);
	}
}

void MainFrame::OnProcessStart()
{
	StartMonitoringApp(fAppFilePath, fCommandLineArguments);
}

void MainFrame::OnUpdateProcessStart(CCmdUI* commandPointer)
{
	// Validate.
	if (!commandPointer)
	{
		return;
	}

	// Enable/disable the "Start" toolbar button.
	BOOL isEnabled = FALSE;
	if ((fAppFilePath.GetLength() > 0) && !IsMonitoring())
	{
		isEnabled = TRUE;
	}
	commandPointer->Enable(isEnabled);
}

void MainFrame::OnProcessStop()
{
	// Do not continue if a process is not running.
	if (!fProcessPointer)
	{
		return;
	}
	if (fProcessPointer->HasExited())
	{
		return;
	}

	// First, attempt to close the process gracefully by sending it a close message to its main window.
	auto closeResult = fProcessPointer->RequestCloseMainWindow();

	// If we were unable to post a close message to the process' main window, then attempt to force quit it.
	if (closeResult.HasFailed())
	{
		fProcessPointer->RequestForceQuit(-1);
	}
}

void MainFrame::OnUpdateProcessStop(CCmdUI* commandPointer)
{
	// Validate.
	if (!commandPointer)
	{
		return;
	}

	// Enable/disable the "Stop" toolbar button.
	BOOL isEnabled = FALSE;
	if (fProcessPointer && !fProcessPointer->HasExited())
	{
		isEnabled = TRUE;
	}
	commandPointer->Enable(isEnabled);
}

void MainFrame::OnCopy()
{
	// Copy the RTF control's current text selection to the clipboard.
	fRichEditControl.Copy();
}

void MainFrame::OnUpdateCopy(CCmdUI* commandPointer)
{
	// Valiate.
	if (!commandPointer)
	{
		return;
	}

	// Enable/disable the "Copy" toolbar button.
	long startIndex = 0;
	long endIndex = 0;
	fRichEditControl.GetSel(startIndex, endIndex);
	commandPointer->Enable((startIndex != endIndex) ? TRUE : FALSE);
}

void MainFrame::OnClearAll()
{
	// Ask the user if they're sure they want to clear the log.
	const wchar_t kMessage[] = L"Are you sure you want to remove all logged entries?";
	auto result = MessageBoxW(kMessage, L"Question", MB_ICONQUESTION | MB_YESNO);
	if (result != IDYES)
	{
		return;
	}

	// Display a "wait" mouse cursor.
	CWaitCursor waitCursor;

	// Clear the RTF control.
	fRichEditControl.SetReadOnly(FALSE);
	fRichEditControl.SetSel(0, -1);
	fRichEditControl.Clear();
	fRichEditControl.SetSel(0, 0);
	fRichEditControl.SetReadOnly(TRUE);

	// Clear the "Alert Panel".
	fAlertListPane.RemoveAllLogEntries();

	// Clear the log entry collection.
	fLogEntryCollection.Clear();
}

void MainFrame::OnFind()
{
	// The user has pressed F3 or Ctrl+F to start a search.
	// Set the focus on the find edit control in the toolbar.
	auto buttonPointer = fToolBar.GetButton(fToolBar.CommandToIndex(ID_FIND_TEXTBOX));
	auto editBoxButtonPointer = dynamic_cast<CMFCToolBarEditBoxButton*>(buttonPointer);
	if (editBoxButtonPointer)
	{
		auto editBoxPointer = editBoxButtonPointer->GetEditBox();
		if (editBoxPointer)
		{
			editBoxPointer->SetFocus();
			editBoxPointer->SetSel(0, -1);
		}
	}
}

void MainFrame::OnFindNext()
{
	// If the "Find" text box on the toolbar is empty, then switch the focus to that text box.
	// This allows the user to easily type in search text after pressing F3.
	if (fFindEditControlHandle && (::GetWindowTextLengthW(fFindEditControlHandle) <= 0))
	{
		OnFind();
		return;
	}

	// Search forwards.
	bool isForwardFindRequested = true;
	DoFind(isForwardFindRequested);
}

void MainFrame::OnFindPrevious()
{
	// If the "Find" text box on the toolbar is empty, then switch the focus to that text box.
	// This allows the user to easily type in search text after pressing F3.
	if (fFindEditControlHandle && (::GetWindowTextLengthW(fFindEditControlHandle) <= 0))
	{
		OnFind();
		return;
	}

	// Search backwards.
	bool isForwardFindRequested = false;
	DoFind(isForwardFindRequested);
}

void MainFrame::OnSetupSubmenu()
{
	// Display the setup/tools submenu on the far right on the toolbar.
	auto menuButtonPointer = dynamic_cast<CMFCToolBarMenuButton*>(fToolBar.GetButton(fToolBar.GetCount() - 1));
	if (menuButtonPointer)
	{
		menuButtonPointer->OnClick(menuButtonPointer->GetParentWnd());
	}
}

void MainFrame::OnAppAbout()
{
	// Display an "About" dialog showing version and copyright information.
	WinString message;
	message.Append(L"Corona Console\n\n");
	message.Append(L"Build: ");
	message.Append(Rtt_MACRO_TO_STRING(Rtt_BUILD_REVISION));
	message.Append(L" (");
	message.Append(Rtt_STRING_BUILD_DATE);
	message.Append(L")\r\n\r\n");
	message.Append(Interop::ApplicationServices::GetCopyrightString());
	MessageBoxW(message.GetUTF16(), L"About Corona Console", MB_ICONINFORMATION | MB_OK);
}

void MainFrame::OnHelp()
{
	try
	{
		const wchar_t kUrl[] = L"https://docs.coronalabs.com/guide";
		::ShellExecuteW(nullptr, L"open", kUrl, nullptr, nullptr, SW_SHOWNORMAL);
	}
	catch (...) {}
}

void MainFrame::OnVisualThemeSelectedById(UINT id)
{
	// Fetch the selected theme.
	auto themePointer = VisualTheme::FromResourceIntegerId(id);
	if (!themePointer)
	{
		return;
	}

	// Do not continue if the selected theme is already the current theme.
	auto applicationPointer = (MainApp*)AfxGetApp();
	if (&applicationPointer->GetVisualTheme() == themePointer)
	{
		return;
	}

	// Apply the theme to the application.
	// This will raise an event used to apply the theme to all UI globally.
	applicationPointer->SetVisualTheme(*themePointer);
}

void MainFrame::OnUpdateVisualThemeMenuItem(CCmdUI* commandPointer)
{
	// Validate.
	auto applicationPointer = (MainApp*)AfxGetApp();
	if (!commandPointer || !applicationPointer)
	{
		return;
	}

	// Show a checkmark next to the theme's menu item if it is currently active.
	const VisualTheme& currentTheme = applicationPointer->GetVisualTheme();
	commandPointer->SetRadio((currentTheme.GetResourceIntegerId() == commandPointer->m_nID) ? TRUE : FALSE);
	commandPointer->Enable(TRUE);
}

void MainFrame::OnUpdateAlwaysEnabledCommand(CCmdUI* commandPointer)
{
	if (commandPointer)
	{
		commandPointer->Enable(TRUE);
	}
}

#pragma endregion


#pragma region Public Methods
BOOL MainFrame::PreCreateWindow(CREATESTRUCT& createSettings)
{
	// Destroy the menu on startup. We don't need it. The toolbar is enough.
	if (createSettings.hMenu)
	{
		::DestroyMenu(createSettings.hMenu);
		createSettings.hMenu = nullptr;
	}

	// Let the base class initialize the given settings.
	if (!CFrameWndEx::PreCreateWindow(createSettings))
	{
		return FALSE;
	}

	// Make sure that a client edge style is not applied to the main window.
	createSettings.dwExStyle &= ~WS_EX_CLIENTEDGE;

	// Returning true tells the caller to go ahead and create the window.
	// The OnCreate() method will get called next.
	return TRUE;
}

BOOL MainFrame::PreTranslateMessage(MSG* messagePointer)
{
	// Validate.
	if (!messagePointer)
	{
		return FALSE;
	}

	// Check if we've received a message directed towards the "Find" text box on the toolbar.
	if (messagePointer->hwnd == fFindEditControlHandle)
	{
		if (WM_KEYDOWN == messagePointer->message)
		{
			switch (messagePointer->wParam)
			{
				case VK_F3:
				case VK_RETURN:
				{
					// Search backwards if shift key was held down. Otherwise seach forwards.
					if ((::GetKeyState(VK_LSHIFT) & 0x80) || (::GetKeyState(VK_RSHIFT) & 0x80))
					{
						OnFindPrevious();
					}
					else
					{
						OnFindNext();
					}
					return TRUE;
				}
			}
		}
	}

	// Let the window handle the message.
	return CFrameWndEx::PreTranslateMessage(messagePointer);
}

void MainFrame::StartMonitoringApp(const wchar_t* filePath, const wchar_t* commandLineArguments)
{
	// Validate argument.
	if (!filePath || (L'\0' == filePath[0]))
	{
		return;
	}

	// Do not continue if already started.
	if (IsMonitoring())
	{
		return;
	}

	// Do not continue if the application is set up to listen to stdin only.
	auto appPointer = (MainApp*)::AfxGetApp();
	if (appPointer->IsStdInExclusiveModeEnabled())
	{
		return;
	}

	// Make sure the given file path is an absolute path.
	std::wstring absoluteFilePath(filePath);
	MainApp::ConvertToAbsolutPath(absoluteFilePath);
	filePath = absoluteFilePath.c_str();

	// Do not continue if we're already actively monitoring the given application file.
	if (fProcessPointer && !fProcessPointer->HasExited() && !wcscmp(fProcessPointer->GetFilePath(), filePath))
	{
		return;
	}

	// Release the last monitored process.
	if (fProcessPointer)
	{
		fProcessPointer = nullptr;
	}

	// Store the given application file path and command line arguments.
	fAppFilePath = filePath;
	fCommandLineArguments = commandLineArguments ? commandLineArguments : L"";

	// Launch the selected EXE file and start monitoring it.
	Interop::Ipc::Process::LaunchSettings launchSettings{};
	launchSettings.IsStdOutRedirectionEnabled = true;
	launchSettings.IsStdErrRedirectionEnabled = true;
	launchSettings.FileNamePath = filePath;
	launchSettings.CommandLineArguments = commandLineArguments;
	auto launchResult = Interop::Ipc::Process::LaunchUsing(launchSettings);
	fProcessPointer = launchResult.GetValue();
	if (!fProcessPointer)
	{
		return;
	}

	// Process was successfully launched. Start monitoring it.
	auto stdOutReaderPointer = fProcessPointer->GetStdOutReader();
	if (stdOutReaderPointer)
	{
		stdOutReaderPointer->GetClosedEventHandlers().Add(&fPipeClosedEventHandler);
		stdOutReaderPointer->GetReceivedDataEventHandlers().Add(&fPipeReceivedDataEventHandler);
		auto startResult = stdOutReaderPointer->Start();
	}
	auto stdErrReaderPointer = fProcessPointer->GetStdErrReader();
	if (stdErrReaderPointer)
	{
		stdErrReaderPointer->GetClosedEventHandlers().Add(&fPipeClosedEventHandler);
		stdErrReaderPointer->GetReceivedDataEventHandlers().Add(&fPipeReceivedDataEventHandler);
		auto startResult = stdErrReaderPointer->Start();
	}
	fProcessPointer->GetExitedEventHandlers().Add(&fProcessExitedEventHandler);

	// Prefix the application file's name to this window's title bar, but only if custom window naming is not enabled.
	if (fCustomWindowName.GetLength() <= 0)
	{
		auto fileNamePointer = ::PathFindFileNameW(filePath);
		if (fileNamePointer)
		{
			std::wstring windowTitle(fileNamePointer);
			windowTitle += L" - ";
			windowTitle += Interop::ApplicationServices::GetProductName();
			SetWindowTextW(windowTitle.c_str());
		}
		else
		{
			SetWindowTextW(Interop::ApplicationServices::GetProductName());
		}
	}
}

bool MainFrame::IsMonitoring()
{
	if (fAsyncStdInReaderPointer && fAsyncStdInReaderPointer->IsRunning())
	{
		return true;
	}
	if (fProcessPointer && !fProcessPointer->HasExited())
	{
		return true;
	}
	return false;
}

const wchar_t* MainFrame::GetMonitoredAppFilePath() const
{
	return fAppFilePath;
}

const wchar_t* MainFrame::GetCustomWindowName() const
{
	return fCustomWindowName;
}

void MainFrame::SetCustomWindowName(const wchar_t* name)
{
	// Store the custom name.
	fCustomWindowName = name ? name : L"";

	// Apply the name to the window's title bar, if visible.
	if (GetSafeHwnd())
	{
		if (fCustomWindowName.GetLength() > 0)
		{
			SetWindowTextW(fCustomWindowName);
		}
		else
		{
			CStringW defaultWindowName;
			defaultWindowName.LoadStringW(IDR_MAINFRAME);
			SetWindowTextW(defaultWindowName);
		}
	}
}

#pragma endregion


#pragma region Private Methods
void MainFrame::OnVisualThemeChanged(MainApp& sender, const Interop::EventArgs& arguments)
{
	// Fetch the currently selected theme.
	const VisualTheme* themePointer = &sender.GetVisualTheme();

	// Apply the theme to the RTF control.
	CHARFORMAT characterFormat{};
	characterFormat.cbSize = sizeof(characterFormat);
	fRichEditControl.GetDefaultCharFormat(characterFormat);
	characterFormat.dwMask |= CFM_COLOR;
	if (&VisualTheme::kDark == themePointer)
	{
		characterFormat.dwEffects &= ~CFE_AUTOCOLOR;
		characterFormat.crTextColor = RGB(192, 192, 192);
		fRichEditControl.SetBackgroundColor(FALSE, RGB(0, 0, 0));
	}
	else
	{
		characterFormat.dwEffects |= CFE_AUTOCOLOR;
		fRichEditControl.SetBackgroundColor(TRUE, RGB(255, 255, 255));
	}
	fRichEditControl.SetDefaultCharFormat(characterFormat);
	fRichEditControl.Invalidate();

	// Force the docking areas containing the toolbar and bottom pane to redraw themeselves.
	AdjustDockingLayout();
}

void MainFrame::OnProcessExited(Interop::Ipc::Process& sender, const Interop::EventArgs& arguments)
{
	// Log that the application this window is monitoring has exited/closed.
	CStringA utf8Message;
	auto exitCodeResult = sender.GetExitCode();
	if (exitCodeResult.HasSucceeded())
	{
		utf8Message.Format("Application closed with exit code: %d\r\n", exitCodeResult.GetValue());
	}
	else
	{
		utf8Message = "Application closed. Exit code is unknown.\r\n";
	}
	QueueLogEntry(Interop::DateTime::FromCurrentLocal(), utf8Message);

	// Give this window the focus on the desktop.
	ActivateFrame();
}

void MainFrame::OnPipeClosed(Interop::Ipc::AsyncPipeReader& sender, const Interop::EventArgs& arguments)
{
	if (&sender == fAsyncStdInReaderPointer)
	{
		QueueLogEntry(Interop::DateTime::FromCurrentLocal(), "The stdin connection has closed.\r\n");

		auto menuPointer = GetSystemMenu(FALSE);
		if (menuPointer)
		{
			menuPointer->EnableMenuItem(SC_CLOSE, MF_ENABLED);
		}
	}
}

void MainFrame::OnPipeReceivedData(
	Interop::Ipc::AsyncPipeReader& sender, const Interop::Ipc::IODataEventArgs& arguments)
{
	// Log the received message from the stdin/stdout/stderr pipe.
	QueueLogEntry(arguments.GetTimestamp(), arguments.GetText());
}

void MainFrame::OnAlertListRequestingShowSelectedEntry(AlertListPane& sender, const Interop::EventArgs& arguments)
{
	// Fetch the unique integer ID of the log entry that the user double clicked on in the "Alert List" pane.
	auto selectedLogEntryId = sender.GetSelectedLogEntryId();
	if (!selectedLogEntryId)
	{
		return;
	}

	// Do not continue if the received log entry ID no longer exists in the collection.
	if (!fLogEntryCollection.ContainsId(selectedLogEntryId))
	{
		return;
	}

	// Calculate where in the RTF control the double clicked log entry starts and ends.
	long rtfStartIndex = 0;
	long rtfEndIndex = 0;
	const int logEntryCount = fLogEntryCollection.GetCount();
	for (int logEntryIndex = 0; logEntryIndex < logEntryCount; logEntryIndex++)
	{
		auto nextLogEntryPointer = fLogEntryCollection.GetByIndex(logEntryIndex);
		if (!nextLogEntryPointer)
		{
			continue;
		}
		if (nextLogEntryPointer->GetIntegerId() == selectedLogEntryId)
		{
			rtfEndIndex = rtfStartIndex + ((long)nextLogEntryPointer->GetRtfCharacterCount() - 1);
			break;
		}
		rtfStartIndex += (long)nextLogEntryPointer->GetRtfCharacterCount();
	}

	// Make sure the start/end indexes fetched above are valid.
	auto rtfTextLength = fRichEditControl.GetTextLength();
	if (rtfStartIndex > rtfTextLength)
	{
		rtfStartIndex = rtfTextLength;
	}
	if (rtfEndIndex < rtfStartIndex)
	{
		rtfEndIndex = rtfStartIndex;
	}
	else if (rtfEndIndex > rtfTextLength)
	{
		rtfEndIndex = rtfTextLength;
	}

	// Highlight the requested log entry in the RTF control.
	fRichEditControl.SetSel(rtfStartIndex, rtfEndIndex);
	fRichEditControl.SetFocus();
}

void MainFrame::OnAlertListEntryCountChanged(AlertListPane& sender, const Interop::EventArgs& arguments)
{
	// Log entries have been added or removed from the "Alert List" panel.
	// Update the status bar's error and warning counts to reflect this change.
	UpdateStatusBarLogEntryCount();
}

void MainFrame::UpdateStatusBarLogEntryCount()
{
	WinString message;
	COLORREF backColor;
	COLORREF textColor;
	int paneIndex;

	// Update the status bar's error count.
	paneIndex = fStatusBar.CommandToIndex(ID_STATUSBAR_ERROR_COUNT);
	if (paneIndex >= 0)
	{
		int entryCount = fAlertListPane.GetErrorLogEntryCount();
		message.Format(L"Errors: %d", entryCount);
		fStatusBar.SetPaneText(paneIndex, message.GetTCHAR(), FALSE);
		backColor = (entryCount > 0) ? RGB(192, 0, 0) : -1;
		textColor = (entryCount > 0) ? RGB(255, 255, 255) : -1;
		fStatusBar.SetPaneBackgroundColor(paneIndex, backColor, FALSE);
		fStatusBar.SetPaneTextColor(paneIndex, textColor, FALSE);
	}

	// Update the status bar's warning count.
	paneIndex = fStatusBar.CommandToIndex(ID_STATUSBAR_WARNING_COUNT);
	if (paneIndex >= 0)
	{
		int entryCount = fAlertListPane.GetWarningLogEntryCount();
		message.Format(L"Warnings: %d", entryCount);
		fStatusBar.SetPaneText(paneIndex, message.GetTCHAR(), FALSE);
		backColor = (entryCount > 0) ? RGB(255, 255, 0) : -1;
		textColor = (entryCount > 0) ? RGB(0, 0, 0) : -1;
		fStatusBar.SetPaneBackgroundColor(paneIndex, backColor, FALSE);
		fStatusBar.SetPaneTextColor(paneIndex, textColor, FALSE);
	}

	// Notify the status bar to draw the above changes.
	fStatusBar.Invalidate();
}

void MainFrame::DoFind(bool isForwardFindRequested)
{
	// Fetch the text to search for from the toolbar's edit control.
	CStringW text;
	{
		auto buttonPointer = fToolBar.GetButton(fToolBar.CommandToIndex(ID_FIND_TEXTBOX));
		auto editBoxButtonPointer = dynamic_cast<CMFCToolBarEditBoxButton*>(buttonPointer);
		if (editBoxButtonPointer)
		{
			auto editBoxPointer = editBoxButtonPointer->GetEditBox();
			if (editBoxPointer && (editBoxPointer->GetWindowTextLengthW() > 0))
			{
				editBoxPointer->GetWindowTextW(text);
			}
		}
	}
	if (text.IsEmpty())
	{
		return;
	}

	// Do not continue if the RTF control is empty. (No text to search.)
	auto rtfTextLength = fRichEditControl.GetTextLength();
	if (rtfTextLength <= 0)
	{
		MessageBeep(MB_ICONWARNING);
		return;
	}

	// Fetch the state of the "Match Case" toolbar toggle button.
	bool isCaseSensitive = (fToolBar.GetButtonStyle(fToolBar.CommandToIndex(ID_FIND_MATCH_CASE)) & TBBS_CHECKED) != 0;

	// Fetch the current selection in the RTF control.
	long selectionStartIndex = 0;
	long selectionEndIndex = 0;
	fRichEditControl.GetSel(selectionStartIndex, selectionEndIndex);
	if (selectionStartIndex < 0)
	{
		selectionStartIndex = 0;
	}
	if (selectionEndIndex < 0)
	{
		selectionEndIndex = rtfTextLength;
	}
	
	// Search for the requested text in the RTF control.
	DWORD findFlags = isForwardFindRequested ? FR_DOWN : 0;
	if (isCaseSensitive)
	{
		findFlags |= FR_MATCHCASE;
	}
	FINDTEXTEXW findParameters{};
	findParameters.lpstrText = text;
	if (isForwardFindRequested)
	{
		// Search forwards.
		if (selectionEndIndex < rtfTextLength)
		{
			findParameters.chrg.cpMin = selectionEndIndex;
		}
		else
		{
			findParameters.chrg.cpMin = 0;
		}
		findParameters.chrg.cpMax = -1;
	}
	else
	{
		// Search backwards.
		if (selectionStartIndex > 0)
		{
			findParameters.chrg.cpMin = selectionStartIndex;
		}
		else
		{
			findParameters.chrg.cpMin = rtfTextLength;
		}
		findParameters.chrg.cpMax = 0;
	}
	auto matchIndex = fRichEditControl.FindTextW(findFlags, &findParameters);
	if (matchIndex >= 0)
	{
		// Text found. Highlight it in the RTF control and exit this function.
		fRichEditControl.SetSel(findParameters.chrgText);
		return;
	}

	// Do a looping search if enabled on the toolbar.
	if (fToolBar.GetButtonStyle(fToolBar.CommandToIndex(ID_FIND_LOOPING)) & TBBS_CHECKED)
	{
		if (isForwardFindRequested)
		{
			findParameters.chrg.cpMin = 0;
			findParameters.chrg.cpMax = -1;
		}
		else
		{
			findParameters.chrg.cpMin = rtfTextLength;
			findParameters.chrg.cpMax = 0;
		}
		matchIndex = fRichEditControl.FindTextW(findFlags, &findParameters);
		if (matchIndex >= 0)
		{
			// Text found. Highlight it in the RTF control and exit this function.
			fRichEditControl.SetSel(findParameters.chrgText);
			return;
		}
	}

	// Search text not found. Let the user know by making a system sound.
	MessageBeep(MB_ICONWARNING);
}

void MainFrame::QueueLogEntry(const Interop::DateTime& timestamp, const char* text)
{
	if (text)
	{
		QueueLogEntry(timestamp, std::make_shared<const std::string>(text));
	}
}

void MainFrame::QueueLogEntry(const Interop::DateTime& timestamp, const wchar_t* text)
{
	if (text)
	{
		WinString stringTranscoder(text);
		QueueLogEntry(timestamp, stringTranscoder.GetUTF8());
	}
}

void MainFrame::QueueLogEntry(const Interop::DateTime& timestamp, std::shared_ptr<const std::string> text)
{
	if (text.get())
	{
		PendingLogEntry pendingLogEntry;
		pendingLogEntry.Timestamp = timestamp;
		pendingLogEntry.Text = text;
		fPendingLogEntryQueue.push_back(pendingLogEntry);
	}
}

void MainFrame::ProcessQueuedLogEntries()
{
	// Do not continue if there are no pending log entries in the queue.
	if (fPendingLogEntryQueue.empty())
	{
		return;
	}
	
	// Do not continue if we're already processed the queued log entries. (ie: Do not allow recursive calls.)
	if (fIsProcessingQueuedLogEntries)
	{
		return;
	}

	// Process the queued/pending log entries. This does the following:
	// - Displays them in the RTF control and Alert List panel.
	// - Stores them to our main in-memory log entry collection for quick lookup.
	fIsProcessingQueuedLogEntries = true;
	try
	{
		// Set up a timeout to abort processing queued log entries in case it's taking too long.
		// This can happen if we're queuing log entries faster than we can display them onscreen.
		// This timeout prevents the UI from "locking-up", allowing the user to interact with the UI.
		const auto kTimeoutTimeInTicks = Interop::Ticks::FromCurrentTime().AddMilliseconds(500);

		// Process queued log entries until the queue is empty or until we timeout.
		while (!fPendingLogEntryQueue.empty() && (Interop::Ticks::FromCurrentTime() < kTimeoutTimeInTicks))
		{
			// Process the next batch of pending log entries from our queue.
			// Note: For best performance, we need to minimize the number of inserts into the RTF control by appending
			//       multiple log entries to a single "consolidated" string. But we also need to limit the size of this
			//       consolidated string to a max size (ie: our batch of log entries) in case we receive a large amount
			//       of log entries so that the window appears active (ie: don't block for a long time doing huge inserts).
			WinString consolidatedRtfLogText;
			int maxEntriesToProcess = 0;
			{
				// Calculate the number of entries we need to pop in the next batch.
				int estimatedConsolidatedStringSize = 0;
				for (auto&& pendingLogEntry : fPendingLogEntryQueue)
				{
					if (pendingLogEntry.Text.get())
					{
						estimatedConsolidatedStringSize += pendingLogEntry.Text->length() + 2;
						maxEntriesToProcess++;
						if (estimatedConsolidatedStringSize >= 2048)
						{
							break;
						}
					}
				}

				// Optimization: Allocate the total size of the consolidated string now to avoid allocations when
				//               appending log entries strings to it down below. (And add 1 for the null character.)
				if (estimatedConsolidatedStringSize > 0)
				{
					consolidatedRtfLogText.Expand(estimatedConsolidatedStringSize + 1);
				}

				// Make sure we pop off at least 1 log entry.
				if (maxEntriesToProcess <= 0)
				{
					maxEntriesToProcess = 1;
				}
			}
			int processedEntryCount = 0;
			int firstLogEntryIndex = -1;
			int lastLogEntryIndex = -1;
			bool hasNonNormalEntryType = false;
			for (auto&& pendingLogEntry : fPendingLogEntryQueue)
			{
				// Keep count of the number of "pending" log entries we'll be popping off of the queue.
				processedEntryCount++;

				// Validate the next pending log entry.
				if (!pendingLogEntry.Text.get())
				{
					continue;
				}

				// Create a new log entry in the main collection and copy the given data to it.
				auto logEntryPointer = fLogEntryCollection.Add();
				if (!logEntryPointer)
				{
					continue;
				}
				logEntryPointer->SetDateTime(pendingLogEntry.Timestamp);
				logEntryPointer->SetText(pendingLogEntry.Text);

				// Track the index range of the new log entries in our in-memory collection.
				if (firstLogEntryIndex < 0)
				{
					firstLogEntryIndex = fLogEntryCollection.GetCount() - 1;
					lastLogEntryIndex = firstLogEntryIndex;
				}
				else
				{
					lastLogEntryIndex++;
				}

				// Classify the entry type.
				// We do this by checking if it is prefixed with an error or warning string. (This is not case sensitive.)
				static const char kLogErrorPrefixName[] = "ERROR:";
				static const char kLogWarningPrefixName[] = "WARNING:";
				if (!_strnicmp(logEntryPointer->GetText(), kLogErrorPrefixName, strlen(kLogErrorPrefixName)))
				{
					logEntryPointer->SetType(LogEntry::Type::kError);
				}
				else if (!_strnicmp(logEntryPointer->GetText(), kLogWarningPrefixName, strlen(kLogWarningPrefixName)))
				{
					logEntryPointer->SetType(LogEntry::Type::kWarning);
				}

				// Do the following special handling for error/warning messages.
				if (logEntryPointer->GetType() != LogEntry::Type::kNormal)
				{
					// Flag that at least 1 log entry in the text we're about to insert is an error/warning message.
					// This is an optimization. The RTF insertion code below only highlights text if this flag is set.
					hasNonNormalEntryType = true;

					// Add the received log entry to the "Alert List" panel.
					fAlertListPane.AddLogEntry(*logEntryPointer);
				}

				// Generate a log message to be added to the RTF control. Formatted as follows:
				// - Prefixed with a timestamp string.
				// - Ensures line endings are always formatted as \r\n.
				// - Ensures that the message ends with a \r\n.
				WinString rtfLogText;
				rtfLogText.Expand(pendingLogEntry.Text->length() + 1);
				auto systemTime = logEntryPointer->GetDateTime().ToSystemTime();
				rtfLogText.Format(
						kRtfTimestampFormatString,
						systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);
				rtfLogText.Append(logEntryPointer->GetText());
				rtfLogText.Replace("\r\n", "\n");
				rtfLogText.Replace("\r", "[\\r]");
//TODO: Add support for embeded nulls. They currently truncate the log message.
//				rtfLogText.Replace("\0", "[\\0]");
				rtfLogText.Replace("\n", "\r\n");
				auto rtfLogTextLength = rtfLogText.GetLength();
				if (rtfLogText.GetUTF16()[rtfLogTextLength - 1] != L'\n')
				{
					rtfLogText.Append(L"\r\n");
					rtfLogTextLength += 2;
				}

				// Update the log message's RTF character length in our in-memory collection.
				// Used to quickly find messages in the RTF control when double clicking an item in the "Alert List" panel.
				// Note: Subtract 1 to exclude the "carriage return" character. RTF text selection ignores them.
				logEntryPointer->SetRtfCharacterCount(rtfLogTextLength - 1);

				// Append the RTF log text to our consolidated string.
				// This improves performance by inserting all log entries in one shot into the RTF control later.
				consolidatedRtfLogText.Append(rtfLogText.GetUTF16());

				// Stop processing log entries if we've hit the maximum batch size.
				if (processedEntryCount >= (maxEntriesToProcess - 1))
				{
					break;
				}
			}

			// Pop off the log entries processed up above from the queue.
			if (processedEntryCount >= 2)
			{
				auto iterator = fPendingLogEntryQueue.begin();
				fPendingLogEntryQueue.erase(iterator, iterator + processedEntryCount);
			}
			else
			{
				fPendingLogEntryQueue.erase(fPendingLogEntryQueue.begin());
			}

			// Insert all of the queued log entry text into the RTF control.
			fRichEditControl.SetRedraw(FALSE);
			{
				// Fetch the user's current cursor and text selection in the control.
				CHARRANGE lastSelectionRange{};
				fRichEditControl.GetSel(lastSelectionRange);
				bool wasEndOfRtfSelected = false;
				if (fRichEditControl.LineFromChar(lastSelectionRange.cpMin) >= (fRichEditControl.GetLineCount() - 1))
				{
					wasEndOfRtfSelected = true;
				}

				// Fetch the user's current vertical scroll position in the control.
				int previousFirstVisibleLineIndex = fRichEditControl.GetFirstVisibleLine();
				bool wasScrollBarAtBottom = true;
				{
					SCROLLINFO scrollInfo{};
					scrollInfo.cbSize = sizeof(scrollInfo);
					fRichEditControl.GetScrollInfo(SB_VERT, &scrollInfo);
					if (((int)scrollInfo.nPage > 0) &&
					    (scrollInfo.nTrackPos < (scrollInfo.nMax - scrollInfo.nMin) - (int)scrollInfo.nPage))
					{
						wasScrollBarAtBottom = false;
					}
				}

				// If the text to be inserted exceeds our max character limit for the RTF control,
				// then delete (aka: roll-off) the older log entries to make room for the new log entry text.
				long predictedRtfTextLength = fRichEditControl.GetTextLength() + consolidatedRtfLogText.GetLength();
				{
					// Do not count the "carriage returns" in this total predicted text length.
					// The RTF conrol's text selection APIs ignores these characters.
					predictedRtfTextLength -= fLogEntryCollection.GetCount();
				}
				if (predictedRtfTextLength > kRtfMaxCharacters)
				{
					// Determine how many old log entries we need to remove to fit-in the newest log entries.
					// Note: Never delete the oldest log entry, because that's the newest entry we're trying to insert.
					//       We need to insert at least 1 entry, even if it exceeds our maximum character limit.
					const int kMaxEntryIndex = fLogEntryCollection.GetCount() - 2;
					int entryRemovalCount = 0;
					long rtfCharacterRemovalCount = 0;
					for (int entryIndex = 0; entryIndex <= kMaxEntryIndex; entryIndex++)
					{
						// Add this old entry to our removal count.
						entryRemovalCount++;

						// Fetch the next oldest log entry.
						auto logEntryPointer = fLogEntryCollection.GetByIndex(entryIndex);
						if (!logEntryPointer)
						{
							continue;
						}

						// Remove this old log entry from the "Alert List" panel.
						fAlertListPane.RemoveLogEntry(*logEntryPointer);

						// Check if we're done removing old log entries.
						rtfCharacterRemovalCount += logEntryPointer->GetRtfCharacterCount();
						if ((predictedRtfTextLength - rtfCharacterRemovalCount) <= kRtfMaxCharacters)
						{
							break;
						}
					}

					// Delete the oldest log entries.
					if (entryRemovalCount > 0)
					{
						// Remove the oldest log entries from our in-memory collection.
						fLogEntryCollection.RemoveByIndexRange(0, entryRemovalCount);

						// Adjust the indexes to our new log entries in the in-memory collection.
						firstLogEntryIndex -= entryRemovalCount;
						lastLogEntryIndex -= entryRemovalCount;

						// Check if we've removed the new log entries that we're about to insert into the RTF control.
						// If we have, then we need to remove their text from the front of the consolidated text string.
						if (firstLogEntryIndex < 0)
						{
							// Make the indexes valid.
							firstLogEntryIndex = 0;
							if (lastLogEntryIndex < 0)
							{
								lastLogEntryIndex = 0;
							}

							// Delete the removed log entries from the front of the string.
							unsigned int rtfCharacterInsertCount = 0;
							for (int entryIndex = 0; entryIndex <= lastLogEntryIndex; entryIndex++)
							{
								auto logEntryPointer = fLogEntryCollection.GetByIndex(entryIndex);
								if (logEntryPointer)
								{
									rtfCharacterInsertCount += logEntryPointer->GetRtfCharacterCount() + 1;
								}
							}
							consolidatedRtfLogText.DeleteRange(
									0, consolidatedRtfLogText.GetLength() - rtfCharacterInsertCount);
						}

						// Remove the oldest log entries from the RTF control.
						if (rtfCharacterRemovalCount > 0)
						{
							// Adjust the RTF control's current cursor position and selection range.
							// This way they'll appear in the same spot after removing the text at the top of the control.
							lastSelectionRange.cpMin -= rtfCharacterRemovalCount;
							lastSelectionRange.cpMax -= rtfCharacterRemovalCount;
							if (lastSelectionRange.cpMin < 0)
							{
								lastSelectionRange.cpMin = 0;
							}
							if (lastSelectionRange.cpMax < 0)
							{
								lastSelectionRange.cpMax = 0;
							}

							// Offset the previously viewed line index by the number of lines being removed.
							int entryLineCount = (int)fRichEditControl.LineFromChar(rtfCharacterRemovalCount - 1) + 1;
							previousFirstVisibleLineIndex -= entryLineCount;
							if (previousFirstVisibleLineIndex < 0)
							{
								previousFirstVisibleLineIndex = 0;
							}

							// Remove the oldest log entries from the top of the RTF control.
							fRichEditControl.SetSel(0, rtfCharacterRemovalCount);
							fRichEditControl.ReplaceSel(_T(""));
						}
					}
				}

				// Add the batch of log messages to the end of the RTF control.
				auto lastRtfTextLength = fRichEditControl.GetTextLength();
				fRichEditControl.SetSel(lastRtfTextLength, -1);
				fRichEditControl.ReplaceSel(consolidatedRtfLogText.GetTCHAR());

				// If we've inserted any error/warning messages, then highlight them.
				if (hasNonNormalEntryType)
				{
					// Fetch the default character format settings.
					CHARFORMAT2 characterFormat{};
					characterFormat.cbSize = sizeof(characterFormat);
					fRichEditControl.GetDefaultCharFormat(characterFormat);

					// Calculate the first inserted log entry's index in the RTF control.
					// This index must exclude the "carriage returns" inserted into the RTF control.
					// Note: The RTF control's GetLength() includes the '\r' characters. So, we must subtract them off.
					auto nextRtfEntryIndex = lastRtfTextLength - firstLogEntryIndex;

					// Traverse the newly inserted log entries to be highlighted.
					for (int entryIndex = firstLogEntryIndex; entryIndex <= lastLogEntryIndex; entryIndex++)
					{
						auto logEntryPointer = fLogEntryCollection.GetByIndex(entryIndex);
						if (logEntryPointer)
						{
							auto logEntryType = logEntryPointer->GetType();
							if (logEntryType != LogEntry::Type::kNormal)
							{
								characterFormat.dwMask |= CFM_COLOR | CFM_BACKCOLOR;
								characterFormat.dwEffects &= ~(CFE_AUTOCOLOR | CFE_AUTOBACKCOLOR);
								if (logEntryPointer->GetType() == LogEntry::Type::kWarning)
								{
									characterFormat.crTextColor = RGB(0, 0, 0);
									characterFormat.crBackColor = RGB(255, 255, 0);
								}
								else if (logEntryPointer->GetType() == LogEntry::Type::kError)
								{
									characterFormat.crTextColor = RGB(255, 255, 255);
									characterFormat.crBackColor = RGB(192, 0, 0);
								}
								fRichEditControl.SetSel(
										nextRtfEntryIndex, nextRtfEntryIndex + logEntryPointer->GetRtfCharacterCount());
								fRichEditControl.SetSelectionCharFormat(characterFormat);
							}
							nextRtfEntryIndex += logEntryPointer->GetRtfCharacterCount();
						}
					}
				}

				// Restore the user's previous cursor position or selection.
				// If the cursor was at the bottom of the control, then move the cursor under the newly logged line of text.
				if (wasEndOfRtfSelected)
				{
					fRichEditControl.SetSel(fRichEditControl.GetTextLength(), -1);
				}
				else
				{
					fRichEditControl.SetSel(lastSelectionRange);
				}

				// Restore the user's previous vertical scroll bar position.
				// If the scroll bar was at the bottom before, then lock it to the bottom when new messages are received.
				if (wasScrollBarAtBottom)
				{
					fRichEditControl.SendMessageW(WM_VSCROLL, SB_BOTTOM);
				}
				else
				{
					auto currentFirstVisibleLineIndex = fRichEditControl.GetFirstVisibleLine();
					auto lineScrollCount = previousFirstVisibleLineIndex - currentFirstVisibleLineIndex;
					if (lineScrollCount)
					{
						fRichEditControl.LineScroll(lineScrollCount);
					}
				}
			}
			fRichEditControl.SetRedraw(TRUE);
			fRichEditControl.Invalidate();
			fRichEditControl.UpdateWindow();
		}
	}
	catch (...) {}
	fIsProcessingQueuedLogEntries = false;
}

#pragma endregion
