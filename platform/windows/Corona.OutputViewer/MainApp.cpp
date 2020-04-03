//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainApp.h"
#include "Interop\ApplicationServices.h"
#include "MainFrame.h"
#include "MFCVisualManagerWindowsCustom.h"
#include "resource.h"
#include "VisualTheme.h"
#include "WinString.h"
#include <algorithm>
#include <Psapi.h>
#include <Shlwapi.h>
#include <string>
#include <thread>


#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


#pragma region Static Member Variables
const TCHAR MainApp::kSettingsRegistrySectionName[] = _T("Settings");
const TCHAR MainApp::kVisualThemeRegistryEntryName[] = _T("VisualTheme");
MainApp theApp;

#pragma endregion

VOID CALLBACK ParentProcessExitCallback(
	_In_  PVOID lpParameter,
	_In_  BOOLEAN TimerOrWaitFired
	)
{
#ifdef Rtt_DEBUG
	// This gets rid of the console window when the Simulator is terminated by the debugger
	// in the development environment.  We could also use this mechanism to terminate the
	// console when a Win32 Desktop app finishes if that is desired
	ExitProcess(0);
#endif

	return;
}

#pragma region Constructors/Destructors
MainApp::MainApp()
:	fIsStdInExclusiveModeEnabled(false),
	fIsDisableCloseWhileReadingEnabled(false),
	fVisualThemePointer(nullptr)
{
#ifdef _DEBUG
	if (::GetAsyncKeyState(VK_PAUSE))
	{
		while (!::IsDebuggerPresent())
			::Sleep(100);
	}
#endif
	// Sets the application ID to be used by the Windows jumplist on the taskbar.
	// Note: This application does not use Windows' jumplist feature.
	SetAppID(_T("CoronaLabs.CoronaOutputViewer.NoVersion"));

	// Prevent MFC from saving/restoring the last state of all menus and toolbars.
	m_bSaveState = FALSE;

	// Initialize the UI's visual theme to the window's system default theme.
	SetVisualTheme(VisualTheme::kSystem);
}

#pragma endregion


#pragma region Public Methods
BOOL MainApp::InitInstance()
{
	// Enable this app to use Windows themes and the newest controls provided by WinXP and newer OSes.
	{
		INITCOMMONCONTROLSEX initCommonControls;
		initCommonControls.dwSize = sizeof(initCommonControls);
		initCommonControls.dwICC = ICC_WIN95_CLASSES;
		::InitCommonControlsEx(&initCommonControls);
	}

	// Initialize the base class.
	CWinAppEx::InitInstance();

	// Enable OLE/ActiveX support.
	::AfxOleInit();
	::AfxEnableControlContainer();

	// Enable RTF rich edit control support.
	::AfxInitRichEdit2();

	// Disable Windows jumplist taskbar support.
	EnableTaskbarInteraction(FALSE);

	// Set the registry key that MFC will use to auto-save/restore the last window and panel positions to.
	SetRegistryKey(_T("Corona Labs\\Corona SDK"));

	// Initialize MFC's right-click menu, keyboard, and tooltip managers.
	InitContextMenuManager();
	InitKeyboardManager();
	InitTooltipManager();
	{
		CMFCToolTipInfo toolTipInfo;
		toolTipInfo.m_bVislManagerTheme = TRUE;
		GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &toolTipInfo);
	}

	// Fetch and apply the last selected visual theme from the registry.
	{
		auto stringId = GetProfileString(kSettingsRegistrySectionName, kVisualThemeRegistryEntryName, _T(""));
		auto themePointer = VisualTheme::FromStringId(stringId);
		if (themePointer)
		{
			SetVisualTheme(*themePointer);
		}
	}

	// Defines application command constants matching a "/command:*" command line argument.
	enum class RequestedAppCommand
	{
		kNone,
		kRun,
		kRunInNewWindow,
		kStop,
		kShow,
		kClose
	};

	// Fetch the command line arguments.
	std::wstring commandLineFilePath;
	std::wstring commandLineFileArguments(L"");
	std::wstring customWindowName;
	auto appCommand = RequestedAppCommand::kNone;
	{
		int argumentCount = 0;
		auto argumentArray = ::CommandLineToArgvW(::GetCommandLineW(), &argumentCount);
		if (argumentArray)
		{
			for (int argumentIndex = 1; argumentIndex < argumentCount; argumentIndex++)
			{
				// Fetch the next command line argument.
				auto argumentStringPointer = argumentArray[argumentIndex];
				if (!argumentStringPointer || (L'\0' == argumentStringPointer[0]))
				{
					continue;
				}

				// Check if the next argument is a "path\file" string.
				// Notes:
				// - We only accept 1 file path at the command line. The first one always wins.
				// - The given file path will be converted to an absolute path, if not one already.
				// - If a valid file path was given, ignore the stdin pipe.
				if (commandLineFilePath.empty() && ::PathFileExistsW(argumentStringPointer))
				{
					commandLineFilePath = argumentStringPointer;
					ConvertToAbsolutPath(commandLineFilePath);
					continue;
				}

				// Check if the command line argument matches a known switch.
				// Note: This is not case sensitive and supports arguments starting with a '/' or '-'.
				std::wstring lowercaseArgumentString(argumentStringPointer);
				if (L'-' == lowercaseArgumentString[0])
				{
					lowercaseArgumentString[0] = L'/';
				}
				std::transform(
						lowercaseArgumentString.begin(), lowercaseArgumentString.end(),
						lowercaseArgumentString.begin(), ::towlower);

				// Check if the argument is set to an application command.
				if (RequestedAppCommand::kNone == appCommand)
				{
					if (L"/command:run" == lowercaseArgumentString)
					{
						appCommand = RequestedAppCommand::kRun;
						continue;
					}
					else if (L"/command:runinnewwindow" == lowercaseArgumentString)
					{
						appCommand = RequestedAppCommand::kRunInNewWindow;
						continue;
					}
					else if (L"/command:stop" == lowercaseArgumentString)
					{
						appCommand = RequestedAppCommand::kStop;
						continue;
					}
					else if (L"/command:show" == lowercaseArgumentString)
					{
						appCommand = RequestedAppCommand::kShow;
						continue;
					}
					else if (L"/command:close" == lowercaseArgumentString)
					{
						appCommand = RequestedAppCommand::kClose;
						continue;
					}
				}

				// Was the close button requested to be disabled?
				// Note: The Corona Simulator sets this. The simulator will exit this app when the simulator exits.
				if (L"/disableclose" == lowercaseArgumentString)
				{
					fIsDisableCloseWhileReadingEnabled = true;
					continue;
				}

				// Was a custom window name provided?
				// Note: The Corona Simulator sets this to "Corona Simulator Console".
				const wchar_t kWindowNameSwitch[] = L"/windowname:";
				const size_t kWindowNameSwitchLength = wcslen(kWindowNameSwitch);
				if (lowercaseArgumentString.length() >= kWindowNameSwitchLength)
				{
					if (lowercaseArgumentString.compare(0, kWindowNameSwitchLength, kWindowNameSwitch) == 0)
					{
						customWindowName = argumentStringPointer + kWindowNameSwitchLength;
						if (L'\0' == customWindowName[0])
						{
							customWindowName = L" ";
						}
						continue;
					}
				}

				// Was a parent process id provided?
				const wchar_t kParentProcessSwitch[] = L"/parentprocess:";
				const size_t kParentProcessSwitchLength = wcslen(kParentProcessSwitch);
				if (lowercaseArgumentString.length() >= kParentProcessSwitchLength)
				{
					if (lowercaseArgumentString.compare(0, kParentProcessSwitchLength, kParentProcessSwitch) == 0)
					{
						std::wstring parentProcessStr;

						parentProcessStr = argumentStringPointer + kParentProcessSwitchLength;
						if (L'\0' != parentProcessStr[0])
						{
							unsigned long parentProcessID = (unsigned long)wcstoul(parentProcessStr.c_str(), NULL, 10);
							if (parentProcessID > 0)
							{
								HANDLE hProcHandle = OpenProcess(SYNCHRONIZE, FALSE, parentProcessID);
								HANDLE hNewHandle = NULL;

								// Register to be notified if the parent process terminates
								RegisterWaitForSingleObject(&hNewHandle, hProcHandle, ParentProcessExitCallback, NULL, INFINITE, WT_EXECUTEONLYONCE);
							}
						}
						continue;
					}
				}

				// If an arguments switch was provided, then all proceding command line arguments
				// are to be passed to the EXE file this application will be running.
				if (L"/args" == lowercaseArgumentString)
				{
					const wchar_t kSwitchName[] = L" /args ";
					wchar_t* subStringPointer = nullptr;
					auto fullCommandLine = ::GetCommandLineW();
					if (fullCommandLine)
					{
						subStringPointer = ::StrStrIW(fullCommandLine, kSwitchName);
						if (!subStringPointer)
						{
							std::wstring alternativeSwitchName(kSwitchName);
							std::replace(alternativeSwitchName.begin(), alternativeSwitchName.end(), L'/', L'-');
							subStringPointer = ::StrStrIW(fullCommandLine, alternativeSwitchName.c_str());
						}
					}
					if (subStringPointer)
					{
						subStringPointer += wcslen(kSwitchName);
						if (subStringPointer[0] != L'\0')
						{
							commandLineFileArguments = subStringPointer;
						}
					}
					break;
				}
			}
			::LocalFree(argumentArray);
		}
	}

	// If a custom window name was provided, then load/save this app's last window location to a separate
	// section in the registry. For example, this allows the Corona Simulator's version of this logging window
	// to have a separate window placement than the logging window used to monitor Corona built apps.
	if (customWindowName.length() > 0)
	{
		// We'll use the custom window name as the registry entry name. Modify the name to have valid characters.
		WinString modifiedCustomWindowName(customWindowName.c_str());
		for (int index = modifiedCustomWindowName.GetLength() - 1; index >= 0; index--)
		{
			auto characterPointer = modifiedCustomWindowName.GetBuffer() + index;
			switch (*characterPointer)
			{
				case L'\\':
				case L'/':
				case L':':
				case L'*':
				case L'?':
				case L'\"':
				case L'<':
				case L'>':
				case L'|':
					*characterPointer = L' ';
					break;
			}
		}

		// Append the above modified custom window name to MFC's default "Workspace" registry entry name.
		WinString customWorkspaceRegistryEntryName(GetRegistryBase());
		customWorkspaceRegistryEntryName.Append(L".");
		customWorkspaceRegistryEntryName.Append(modifiedCustomWindowName.GetUTF16());

		// Set up MFC to use this custom registry entry name to save/restore the last window placement.
		SetRegistryBase(customWorkspaceRegistryEntryName.GetTCHAR());
	}

	// If a file path was given and an app command was not provided,
	// then default to running the given file in a new window.
	if (!commandLineFilePath.empty() && (RequestedAppCommand::kNone == appCommand))
	{
		appCommand = RequestedAppCommand::kRunInNewWindow;
	}

	// If we were not given a file path via the command line, then check if the stdin pipe is open.
	// If open, then set up this app to only log from stdin. (This is how the Corona Simulator logs to this app.)
	// If not, then set up this app to log the stdout and sterr from a user selected application file.
	if (commandLineFilePath.empty())
	{
		auto stdInHandle = ::GetStdHandle(STD_INPUT_HANDLE);
		fIsStdInExclusiveModeEnabled = (stdInHandle && (stdInHandle != INVALID_HANDLE_VALUE));
	}

	// Determine if the given command line arguments requires this app to perform IPC (Inter-Process Communications)
	// with existing instances of this application, such as sending a command to existing application window(s).
	bool isIpcRequired = false;
	if (!fIsStdInExclusiveModeEnabled)
	{
		switch (appCommand)
		{
			case RequestedAppCommand::kRun:
			case RequestedAppCommand::kShow:
				if (!commandLineFilePath.empty())
				{
					isIpcRequired = true;
				}
				break;
			case RequestedAppCommand::kClose:
			case RequestedAppCommand::kStop:
				isIpcRequired = true;
				break;
		}
	}

	// Communicate with other instances of this application, if flagged as required above.
	if (isIpcRequired)
	{
		// Set up a context used to store information to be passed to the Win32 EnumWindow() callback.
		struct EnumWindowContext
		{
			bool WasSuccessful;
			RequestedAppCommand AppCommand;
			COPYDATASTRUCT CopyDataStruct;
		};
		std::wstring copyDataString;
		EnumWindowContext enumWindowContext{};
		enumWindowContext.AppCommand = appCommand;
		if (!commandLineFilePath.empty())
		{
			copyDataString = commandLineFilePath;
			if (commandLineFileArguments.empty() == false)
			{
				copyDataString += L'\n';
				copyDataString += commandLineFileArguments;
			}
			enumWindowContext.CopyDataStruct.lpData = (LPVOID)copyDataString.c_str();
			enumWindowContext.CopyDataStruct.cbData = copyDataString.length() * sizeof(wchar_t);
		}

		// Callback to be invoked by the Win32 EnumWindow() function for each window discovered on the desktop.
		auto enumWindowCallback = [](HWND windowHandle, LPARAM lParam)->BOOL
		{
			// Validate.
			if (!windowHandle || !lParam)
			{
				return TRUE;
			}

			// Fetch the given window's process ID.
			DWORD processId = 0;
			::GetWindowThreadProcessId(windowHandle, &processId);
			if (!processId)
			{
				return TRUE;
			}

			// Attempt to open the window's process. Needed to query info about its application.
			auto processHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
			if (!processHandle)
			{
				return TRUE;
			}

			// Fetch the window process' EXE file name and path.
			wchar_t exeFilePath[MAX_PATH];
			exeFilePath[0] = L'\0';
			::GetModuleFileNameExW(processHandle, nullptr, exeFilePath, MAX_PATH);
			::CloseHandle(processHandle);
			if (L'\0' == exeFilePath[0])
			{
				return TRUE;
			}
			wchar_t* exeFileName = exeFilePath;
			::PathStripPathW(exeFileName);

			// Do not continue if the window does not belong to this application.
			if (_wcsicmp(exeFileName, Interop::ApplicationServices::GetExeFileName()) != 0)
			{
				return TRUE;
			}

			// Dereference our context information from LPARAM.
			auto contextPointer = (EnumWindowContext*)lParam;
			auto copyDataStructLParam = (LPARAM)&(contextPointer->CopyDataStruct);

			// If given a file path, ignore app instances that are not referencing that file.
			if (contextPointer->CopyDataStruct.cbData > 0)
			{
				contextPointer->CopyDataStruct.dwData = (DWORD)MainFrame::IpcCommandId::kIsFileSelected;
				auto result = ::SendMessageW(windowHandle, WM_COPYDATA, 0, copyDataStructLParam);
				if (!result)
				{
					// The application window is not referenceing the given file path.
					// Do not send commands to this window and move on to the next window.
					return TRUE;
				}
			}

			// The window belongs to another instance of this application.
			// Send it the app command that was given to us at the command line, such as "/command:run".
			switch (contextPointer->AppCommand)
			{
				case RequestedAppCommand::kRun:
				{
					// Tell the window to start running the application referenced by the given file path.
					contextPointer->CopyDataStruct.dwData = (DWORD)MainFrame::IpcCommandId::kRun;
					auto result = ::SendMessageW(windowHandle, WM_COPYDATA, 0, copyDataStructLParam);
					if (result)
					{
						contextPointer->WasSuccessful = true;
						return FALSE;
					}
					break;
				}
				case RequestedAppCommand::kStop:
				case RequestedAppCommand::kClose:
				{
					// Tell the window to stop monitoring.
					contextPointer->CopyDataStruct.dwData = (DWORD)MainFrame::IpcCommandId::kStop;
					auto result = ::SendMessageW(windowHandle, WM_COPYDATA, 0, copyDataStructLParam);
					if (result)
					{
						// Block this thread until the window is no longer monitoring.
						contextPointer->CopyDataStruct.dwData = (DWORD)MainFrame::IpcCommandId::kIsRunning;
						do
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(10));
							result = ::SendMessageW(windowHandle, WM_COPYDATA, 0, copyDataStructLParam);
						} while (result != 0);

						// If we were requested to close the window, then do so now. Otherwise, we're done.
						if (RequestedAppCommand::kClose == contextPointer->AppCommand)
						{
							auto wasPosted = ::PostMessageW(windowHandle, WM_CLOSE, 0, 0);
							if (wasPosted)
							{
								contextPointer->WasSuccessful = true;
							}
						}
						else
						{
							contextPointer->WasSuccessful = true;
						}
					}
					break;
				}
				case RequestedAppCommand::kShow:
				{
					// Bring the window to the front of the desktop.
					auto wasShown = ::SetForegroundWindow(windowHandle);
					if (wasShown)
					{
						contextPointer->WasSuccessful = true;
					}
					break;
				}
			}

			// Returning true asks the operating system to send us the next window on the desktop.
			return TRUE;
		};

		// Call the above callback for each window discovered on the desktop.
		// This send IPC commands to other instances of this application.
		::EnumWindows(enumWindowCallback, (LPARAM)&enumWindowContext);

		// Determine if this application process should show its desktop window or not
		// based on the results of the above IPC commands.
		switch (appCommand)
		{
			case RequestedAppCommand::kRun:
			case RequestedAppCommand::kShow:
				// If another app instance has ran/shown the given file, then exit out.
				// Otherwise, have this app instance display a window for the given file.
				if (enumWindowContext.WasSuccessful)
				{
					return TRUE;
				}
				break;
			case RequestedAppCommand::kRunInNewWindow:
				// The caller has request this app process to always show a new window.
				break;
			default:
				// For all other app commands, exit out of this app process immediately. (Do not show a window.)
				return TRUE;
		}
	}

	// Create and load the main window.
	auto mainFramePointer = new MainFrame;
	{
		// We must set the base class' member variable to point to this window before we load it.
		// This member variable is returned by MFC's AfxGetMainWnd() function.
		m_pMainWnd = mainFramePointer;
	}
	if (!customWindowName.empty())
	{
		mainFramePointer->SetCustomWindowName(customWindowName.c_str());
	}
	mainFramePointer->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, nullptr, nullptr);
	if (!fIsStdInExclusiveModeEnabled && !commandLineFilePath.empty())
	{
		mainFramePointer->StartMonitoringApp(commandLineFilePath.c_str(), commandLineFileArguments.c_str());
	}
	mainFramePointer->ShowWindow(SW_SHOW);
	mainFramePointer->UpdateWindow();

	// Ask the user to select an application to run/monitor on startup if this application was started without:
	// - An active stdin pipe. (Such as for the Corona Simulator.)
	// - An app file path at the command line.
	// Note: This is the normal path when double clicking on this app's EXE file yourself.
	if (!fIsStdInExclusiveModeEnabled && commandLineFilePath.empty())
	{
		mainFramePointer->PostMessage(WM_COMMAND, ID_FILE_OPEN);
	}

	return TRUE;
}

int MainApp::ExitInstance()
{
	// Store the currently selected visual theme to the registry.
	WriteProfileString(kSettingsRegistrySectionName, kVisualThemeRegistryEntryName, fVisualThemePointer->GetStringId());

	// Terminate the OLE/ActiveX system.
	::AfxOleTerm(FALSE);

	// Exit this application.
	return CWinAppEx::ExitInstance();
}

MainApp::VisualThemeChangedEvent::HandlerManager& MainApp::GetVisualThemeChangedEventHandlers()
{
	return fVisualThemeChangedEvent.GetHandlerManager();
}

bool MainApp::IsStdInExclusiveModeEnabled() const
{
	return fIsStdInExclusiveModeEnabled;
}

bool MainApp::IsDisableCloseWhileReadingEnabled() const
{
	return fIsDisableCloseWhileReadingEnabled;
}

const VisualTheme& MainApp::GetVisualTheme() const
{
	return *fVisualThemePointer;
}

void MainApp::SetVisualTheme(const VisualTheme& theme)
{
	// Do not continue if we're already using the given theme.
	if (&theme == fVisualThemePointer)
	{
		return;
	}

	// Store the given theme.
	fVisualThemePointer = &theme;

	// Apply the theme to the global MFC visual manager.
	if (&VisualTheme::kDark == fVisualThemePointer)
	{
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
	}
	else if (&VisualTheme::kBlue == fVisualThemePointer)
	{
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
	}
	else
	{
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(MFCVisualManagerWindowsCustom));
	}

	// Notify the system that the theme has changed.
	fVisualThemeChangedEvent.Raise(*this, Interop::EventArgs::kEmpty);
}

#pragma endregion


#pragma region Public Static Functions
void MainApp::ConvertToAbsolutPath(std::wstring& path)
{
	if (path.empty() == false)
	{
		const size_t kAbsolutePathMaxLength = 1024;
		wchar_t absolutePath[kAbsolutePathMaxLength];
		absolutePath[0] = L'\0';
		::GetFullPathNameW(path.c_str(), kAbsolutePathMaxLength, absolutePath, nullptr);
		if (absolutePath[0] != L'\0')
		{
			path = absolutePath;
		}
	}
}

#pragma endregion
