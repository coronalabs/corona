//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Winver.h>
#include <gdiplus.h>
#include <io.h>
#include <ios>
#include <Fcntl.h>

#include "Simulator.h"
#include "MainFrm.h"

#include "SimulatorDoc.h"
#include "SimulatorDocTemplate.h"
#include "SimulatorView.h"

#include "resource.h"
#include "WinString.h"
#include "ProgressWnd.h"
#include "CoronaInterface.h"
#include "WinGlobalProperties.h"  // set the properties
#include "CoronaInterface.h"  // player interface, appDeinit()

#include "Core/Rtt_Build.h"
#include "Interop\Ipc\AsyncPipeReader.h"
#include "Interop\Storage\RegistryStoredPreferences.h"
#include "Rtt_Version.h"    // Rtt_STRING_BUILD and Rtt_STRING_BUILD_DATE
#include "Rtt_SimulatorAnalytics.h"
#include "Rtt_JavaHost.h"   // GetJavaHome()


#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Single Instance Control
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Name of the semaphore used to control the number of instances running
LPCTSTR SINGLE_INSTANCE_OBJECT_NAME = _T("com.coronalabs.windows.single_instance.v1");
 
// This data is shared by all instances, so that when we
// detect that we have another instance running, we can use
// this to send messages to the window of that first instance:
#pragma data_seg (".SingleInstance")
LONG  SimFirstInstanceHwnd = 0;
// If you want to add anything here the crucial thing to remember is that 
// any shared data items must be initialized to some value
#pragma data_seg ()

// Tell the linker about our shared data segment
#pragma comment(linker, "/section:.SingleInstance,RWS")


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Static Member Variables
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>The one and only CSimulatorApp object.</summary>
CSimulatorApp theApp;

// CSimulatorApp initialization
// Copied from appcore.cpp
AFX_STATIC_DATA const TCHAR _afxFileSection[] = _T("Recent File List");
AFX_STATIC_DATA const TCHAR _afxFileEntry[] = _T("File%d");
AFX_STATIC_DATA const TCHAR _afxPreviewSection[] = _T("Settings");
AFX_STATIC_DATA const TCHAR _afxPreviewEntry[] = _T("PreviewPages");


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CSimulatorApp
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSimulatorApp::CSimulatorApp()
{
	// Place all significant initialization in InitInstance
	m_zoom = 0;  // 0 is normal, -1 is zoomed out once, 2 is zoomed in twice
	m_rotation = 0;  // 0, 90, 180, 270
	m_isDebugModeEnabled = false;
	m_isLuaExitAllowed = false;
	m_isConsoleEnabled = true;
	m_isStopBuildRequested = false;
}

// InitInstance - initialize the application
BOOL CSimulatorApp::InitInstance()
{
	// Load the simulator version of the Corona library, which is only used by plugins to link against by name.
	// This is a thin proxy DLL which forwards Solar2D's public APIs to this EXE's statically linked Solar2D APIs.
	// This ensures that plugins link with the simulator's library and not the non-simulator version of the library.
	CString coronaLibraryPath = GetApplicationDir() + _T("\\Resources\\CoronaLabs.Corona.Native.dll");
	if (!Rtt_VERIFY(::LoadLibrary(coronaLibraryPath)))
	{
		CString message =
			_T("Failed to load the Solar2D Simulator's library.\r\n")
			_T("This might mean that your Solar2D installation is corrupted.\r\n")
			_T("You may be able to fix this by re-installing the Solar2D.");
		AfxMessageBox(message, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	CWinApp::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	{
		WinString stringTranscoder(Interop::Storage::RegistryStoredPreferences::kAnscaCoronaKeyName);
		SetRegistryKey(stringTranscoder.GetTCHAR());

		WinString profileName(Interop::Storage::RegistryStoredPreferences::kCoronaSimulatorKeyName);
		m_pszProfileName = _tcsdup(profileName.GetTCHAR());

		// Hacks to make life easier
		CString ret = GetProfileString(L"Preferences", L"debugBuildProcess", L"");
		if (ret.GetLength() && _wgetenv(L"DEBUG_BUILD_PROCESS") == NULL) {
			_wputenv_s(L"DEBUG_BUILD_PROCESS", ret);
		}
		if (_wgetenv(L"CORONA_PATH") == NULL) {
			TCHAR coronaDir[MAX_PATH];
			GetModuleFileName(NULL, coronaDir, MAX_PATH);
			TCHAR* end = StrRChr(coronaDir, NULL, '\\');
			if (end) 
			{
				end[1] = 0;
				_wputenv_s(L"CORONA_PATH", coronaDir);
			}

		}
	}
	// Initialize WinGlobalProperties object which mirrors theApp properties
	// Make sure this is done before accessing any Solar2D functions
	WinString strRegistryKey, strRegistryProfile, strResourcesDir;
	strRegistryKey.SetTCHAR(m_pszRegistryKey);
	strRegistryProfile.SetTCHAR(m_pszProfileName);
	strResourcesDir.SetTCHAR(GetResourceDir());
	GetWinProperties()->SetRegistryKey(strRegistryKey.GetUTF8());
	GetWinProperties()->SetRegistryProfile(strRegistryProfile.GetUTF8());
	GetWinProperties()->SetResourcesDir(strResourcesDir.GetUTF8());

	// See if we ran successfully without a crash last time (mostly
	// used to detect crappy video drivers that crash us)
	int lastRunSucceeded = GetProfileInt(REGISTRY_SECTION, REGISTRY_LAST_RUN_SUCCEEDED, 1);

	if (!lastRunSucceeded)
	{
		CString message =
			_T("Solar2D Simulator crashed last time it was run\n\n")
			_T("This can happen because Windows or the video driver need to be updated.  ")
			_T("If it crashes again, make sure the software for your video card is up to date (you ")
			_T("may need to visit the manufacturer's web site to check this) and ensure that all ")
			_T("available Windows updates have been installed.\n\n")
			_T("If the problem persists, contact support@solar2d.com including as much detail as possible.");

		SHMessageBoxCheck(NULL,
			message,
			TEXT("Solar2D Simulator"),
			MB_OK | MB_ICONEXCLAMATION,
			IDOK,
			L"CoronaShowCrashWarning");
	}
	else
	{
#ifndef Rtt_DEBUG
		// Set the telltale to 0 so we'll know if we don't reset it in ExitInstance()
		// (but don't do it for debug builds which are probably running in the debugger)
		WriteProfileInt(REGISTRY_SECTION, REGISTRY_LAST_RUN_SUCCEEDED, 0);
#endif
	}

	// Parse for command line arguments.
	// Supports arguments starting with '-' and '/'. Arguments are not case sensitive.
	CString commandLine = m_lpCmdLine;
	commandLine.MakeLower();
	commandLine.Replace(TCHAR('/'), TCHAR('-'));

	// Don't buffer stdout and stderr as this makes debugging easier
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	// Have we been asked to run just a single instance at a time?
	if (commandLine.Find(_T("-singleton")) >= 0)
	{
		// Check whether we have already an instance running
		// (even if we get ERROR_ALREADY_EXISTS we increment the reference count so the semaphore
		// will exist so long as there is a running Simulator)
		HANDLE hSingleInstanceSemaphore = ::CreateSemaphore(NULL, 0, 1, SINGLE_INSTANCE_OBJECT_NAME);
		DWORD err = ::GetLastError();

		if (err == ERROR_ALREADY_EXISTS)
		{
			// The semaphore exists already, must have been created by a previous instance, tell it to quit 
			HWND windowHandle = (HWND)SimFirstInstanceHwnd;
			ASSERT(windowHandle != 0);
			if (::IsWindow(windowHandle))
			{
				// Fetch the other single instance app's process handle.
				HANDLE processHandle = nullptr;
				DWORD processId = 0;
				::GetWindowThreadProcessId(windowHandle, &processId);
				if (processId)
				{
					processHandle = ::OpenProcess(SYNCHRONIZE, FALSE, processId);
				}

				// Tell the other single instance app to quit, making this app instance the single instance.
				if (::IsWindowEnabled(windowHandle))
				{
					// The app isn't displaying a modal dialog.
					// This means we can close it gracefully with a close message.
					::PostMessage(windowHandle, WM_CLOSE, 0, 0);
				}
				else
				{
					// *** The app is currently displaying a modal dialog. ***

					// Stop the app's currently running Solar2D project. Avoids file locking issues, like with fonts.
					::PostMessage(windowHandle, WM_COMMAND, ID_FILE_CLOSE, 0);

					// Send a quit message to exit the app. (Not a clean way to exit an app.)
					// Note: Child dialogs and Doc/View will not receive close messages because of this.
					::PostMessage(windowHandle, WM_QUIT, 0, 0);
				}

				// Wait for the other app to exit before continuing.
				// This way it's last saved registry settings will be available to this app.
				if (processHandle)
				{
					::WaitForSingleObject(processHandle, 5000);
					::CloseHandle(processHandle);
				}
			}
		}
	}

    // Initialize GDIplus early to avoid crashes when double-clicking on a .lua file
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	// Handle the rest of the command line arguments.
	if (commandLine.Find(_T("-debug")) >= 0)
	{
		m_isDebugModeEnabled = true;
	}
	if (commandLine.Find(_T("-allowluaexit")) >= 0)
	{
		m_isLuaExitAllowed = true;
	}
	if (commandLine.Find(_T("-no-console")) >= 0)
	{
		// No console is required, caller will grab our output from stdout and stderr
		m_isConsoleEnabled = false;
	}

	// Display a logging window, if enabled.
	if (m_isConsoleEnabled)
	{
		// Use the following Solar2D application as our logging window.
		WinString outputViewerFilePath(GetApplicationDir());
		WinString outputViewerArgs;

		outputViewerArgs.Format("/parentProcess:%ld", ::GetCurrentProcessId());
		outputViewerArgs.Append(L" /disableClose /windowName:\"Corona Simulator Console\"");

		outputViewerFilePath.Append(L"\\Corona.Console.exe");
		Interop::Ipc::Process::LaunchSettings launchSettings{};
		launchSettings.FileNamePath = outputViewerFilePath.GetUTF16();
		launchSettings.CommandLineArguments = outputViewerArgs.GetTCHAR();
		launchSettings.IsStdInRedirectionEnabled = true;
		auto launchResult = Interop::Ipc::Process::LaunchUsing(launchSettings);
		m_outputViewerProcessPointer = launchResult.GetValue();
		if (m_outputViewerProcessPointer)
		{
			auto stdInHandle = m_outputViewerProcessPointer->GetStdInHandle();
			int stdInFD = _open_osfhandle((intptr_t)stdInHandle, _O_TEXT);
			if (!GetConsoleWindow()) {
				AllocConsole();
				ShowWindow(GetConsoleWindow(), SW_HIDE);
			}
			::SetStdHandle(STD_OUTPUT_HANDLE, stdInHandle);
			::SetStdHandle(STD_ERROR_HANDLE, stdInHandle);
			FILE* notused;
			freopen_s(&notused, "CONOUT$", "w", stdout);
			freopen_s(&notused, "CONOUT$", "w", stderr);
			int res = _dup2(stdInFD, _fileno(stdout));
			res = _dup2(stdInFD, _fileno(stderr));
			std::ios::sync_with_stdio();
		}
	}

	// Stop MFC from flashing the simulator window on startup.
	m_nCmdShow = SW_HIDE;

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	AfxInitRichEdit();
	AfxEnableControlContainer();

	int maxRecentFileCount = 10;
	LoadStdProfileSettings(maxRecentFileCount);  // Load standard INI file options (including MRU)

    // Delete the m_pRecentFileList created in the LoadStdProfileSettings.
	// We want to show directory name, not filename (which is always main.lua)
    delete m_pRecentFileList;

    // The nSize argument of the constructor is set to four because the  
    // LoadStdProfileSettings takes a default of four.  If you specify a  
    // different value for the nMaxMRU argument you need to change the
    // nSize argument for the constructor call.
    m_pRecentFileList = new CRecentDirList(0, _afxFileSection, _afxFileEntry, maxRecentFileCount);
    m_pRecentFileList->ReadList();

    // Override default CDocManager class to manage initial directory for open file dialog
    // Initial directory set below
    delete m_pDocManager;
    m_pDocManager = new CSimDocManager();

	// Register the simulator's document template.
	// This is used to manage an open Solar2D project with an MFC SDI document/view interface.
	// Note: This custom doc template allows the simulator to open Solar2D projects by directory or "main.lua".
	auto pDocTemplate = new CSimulatorDocTemplate();
	if (!pDocTemplate)
	{
		return FALSE;
	}
	AddDocTemplate(pDocTemplate);

    // Do this before any of the ways app can exit (including not authorized)
	printf("\nSolar2D Simulator %d.%d (%s %s)\n\n", Rtt_BUILD_YEAR, Rtt_BUILD_REVISION, __DATE__, __TIME__);

	// Load user preferences from registry
    // Initialize member variables used to write out preferences
    SetWorkingDir( GetProfileString( REGISTRY_SECTION, REGISTRY_WORKINGDIR, GetSampleDir() ));
    m_sDisplayName = GetProfileString( REGISTRY_SECTION, REGISTRY_DEVICE, _T("") );
    // need a cast because zoom is signed
    m_zoom = (int) GetProfileInt( REGISTRY_SECTION, REGISTRY_ZOOM, REGISTRY_ZOOM_DEFAULT );
    m_rotation = GetProfileInt( REGISTRY_SECTION, REGISTRY_ROTATION, REGISTRY_ROTATION_DEFAULT );

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Fake out command line if we want to automatically open the last open project.
	// This is necessary because ProcessShellCommand() initiates a "New File"
	// operation if there is no filename on the command line and this is very
	// hard to unravel and inject the remembered filename into.
	if (!m_isDebugModeEnabled && (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew && ShouldAutoOpenLastProject()))
	{
		CRecentFileList *recentFileListPointer = GetRecentFileList();
		if (recentFileListPointer && (recentFileListPointer->GetSize() > 0))
		{
			auto lastRanFilePath = (*recentFileListPointer)[0];
			if (!lastRanFilePath.IsEmpty() && ::PathFileExists(lastRanFilePath))
			{
				// We have a remembered last project so pretend we were asked to open it on the command line.
				cmdInfo.m_nShellCommand = CCommandLineInfo::FileOpen;
				cmdInfo.m_strFileName = lastRanFilePath;
			}
		}
	}

	// If a Solar2D project directory was provided at the command line, then append a "main.lua" file to the path.
	if (!cmdInfo.m_strFileName.IsEmpty() && ::PathIsDirectory(cmdInfo.m_strFileName))
	{
		TCHAR mainLuaFilePath[2048];
		mainLuaFilePath[0] = _T('\0');
		::PathCombine(mainLuaFilePath, cmdInfo.m_strFileName, _T("main.lua"));
		if (::PathFileExists(mainLuaFilePath))
		{
			cmdInfo.m_strFileName = mainLuaFilePath;
		}
	}

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	// This will cause a new "document" to be opened and most initialization to take place
	if (!ProcessShellCommand(cmdInfo))
	{
		return FALSE;
	}

    // If Resources dir doesn't exist, exit.
	CString sDir = GetResourceDir();
    if (!CheckDirExists(sDir))
	{
        CString msg;
        msg.Format( IDS_DIR_s_NOTFOUND_INSTALL, sDir );
		::AfxMessageBox( msg );
        return FALSE;
	}

    // Create modeless "Connecting to Server" window for later use
	m_pProgressWnd = new CProgressWnd();
    m_pProgressWnd->SetText( IDS_CONNECTINGTOSERVER );
    m_pProgressWnd->Create(CProgressWnd::IDD);

	// Get the current window size (it's been calculated from the current app)
	CRect cwr;
	m_pMainWnd->GetWindowRect(&cwr);
	m_WP.rcNormalPosition.left = GetProfileInt(REGISTRY_SECTION, REGISTRY_XPOS, REGISTRY_XPOS_DEFAULT);
	m_WP.rcNormalPosition.top = GetProfileInt(REGISTRY_SECTION, REGISTRY_YPOS, REGISTRY_YPOS_DEFAULT);
	// Don't use any remembered size as it might not be for the same app
	m_WP.rcNormalPosition.right = m_WP.rcNormalPosition.left + (cwr.right - cwr.left); 
	m_WP.rcNormalPosition.bottom = m_WP.rcNormalPosition.top + (cwr.bottom - cwr.top);
	m_WP.length = sizeof(WINDOWPLACEMENT);
    const WINDOWPLACEMENT * wp = &m_WP;
	// Places the window on the screen even if the information in WINDOWPLACEMENT puts the window off screen
	SetWindowPlacement(*m_pMainWnd, wp);

	// Remember the main window's HWND in the shared memory area so other instances
	// can find it and tell us to do things
	SimFirstInstanceHwnd = (LONG) (m_pMainWnd->GetSafeHwnd());

	// Set main window size based on device and zoom
    CMainFrame *pMainFrm = (CMainFrame *)m_pMainWnd;
    CSimulatorView *pView = (CSimulatorView *)pMainFrm->GetActiveView();

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Check if the current user is authorized to run this application.
	// If not, then this function will display a login window and connect to Corona Labs' server.
	BOOL isAuthorized = AuthorizeInstance();
	if (!isAuthorized)
	{
		return FALSE;
	}

#if 0
	// For now, at least, record all user's analytics (it's a free product, after all)
	// TODO: at some point remove the inactive checkbox from the preferences dialog

	// Start recording usage feedback to be posted to Corona Labs' server later.
	Rtt::SimulatorAnalytics *pAnalytics = GetWinProperties()->GetAnalytics();
	if (pAnalytics)
	{
		const Rtt::AuthorizationTicket *ticket = GetWinProperties()->GetTicket();
		if ( ticket && ticket->IsSubscriptionCurrent() )
		{
			// Enable/disable analytics based on the "User Feedback" setting set in the preferences window.
			bool isEnabled = AfxGetApp()->GetProfileInt(REGISTRY_SECTION, REGISTRY_ANALYTICS, REGISTRY_ANALYTICS_DEFAULT) ? true : false;
			pAnalytics->SetParticipating(isEnabled);
		}
	}
#endif

	// This application has been initialized and is authorized to continue.
	// Returning TRUE allows this application to continue running.
	return TRUE;
}

// CheckJarPath - see if the given path exists and show user a message if it doesn't
bool
CSimulatorApp::CheckJarPath( CString path )
{
	if (!CheckPathExists(path))
	{
		CString msg;
		msg.Format( IDS_FILE_s_NOTFOUND_INSTALL, path );
		::AfxMessageBox( msg );
        return false;
	}
	return true;
}

// InitJavaPaths - check if jarsigner.exe can be found, and warn the user if not.
// If JAVA_HOME is set to the JDK and %JAVA_HOME%\bin is in the path, load the
// required jars (ListKeyStore and ant).
bool
CSimulatorApp::InitJavaPaths()
{
    static bool bInitialized = false;

    // Only do this successfully once (set to true when jarsigner.exe found)
    if (bInitialized)
	{
        return true;
	}

	// Fetch the path to the JDK.
    auto utf8JdkPath = Rtt::JavaHost::GetJdkPath();
	if (nullptr == utf8JdkPath)
	{
        return false;
	}

    // Look for jarsigner.exe in PATH
	WinString jarSignerFilePath;
	jarSignerFilePath.SetUTF8(utf8JdkPath);
	if ((jarSignerFilePath.GetLength() > 0) && !jarSignerFilePath.EndsWith("\\"))
	{
		jarSignerFilePath.Append("\\");
	}
	jarSignerFilePath.Append("bin\\jarsigner.exe");
	int result = SearchPath( NULL, jarSignerFilePath.GetTCHAR(), NULL, 0, NULL, NULL );
	if (!result)
	{
		if (::AfxMessageBox(IDS_JDKNOTFOUND, MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
		{
			try
			{
				::ShellExecute(
						NULL, _T("open"),
						_T("http://www.oracle.com/technetwork/java/javase/downloads/index.html"),
						NULL, NULL, SW_SHOWNORMAL);
			}
			catch (...) { }
		}
	}
	else
	{
		bInitialized = true;
	}

	// Set JAVA_PATH and PATH environment variables for this process to reference JDK and JRE directories.
	// These are needed by ANT to build apps for Android and to run JAR files used by this app.
    if (bInitialized)
	{
		// Set the JAVA_HOME environment variable.
		WinString stringConverter;
		stringConverter.SetUTF8(utf8JdkPath);
		::SetEnvironmentVariableW(L"JAVA_HOME", stringConverter.GetUTF16());

		// Add to the PATH environment variable.
		stringConverter.SetUTF8(utf8JdkPath);
		if (stringConverter.GetLength() > 0)
		{
			std::wstring newUtf16PathString(L"");

			// Add the JDK's bin directory to the PATH.
			// This ensures that the expected JDK tools are used by ANT.
			if (!stringConverter.EndsWith("\\"))
			{
				stringConverter.Append("\\");
			}
			stringConverter.Append("bin;");
			newUtf16PathString += stringConverter.GetUTF16();

			// Add the JRE's bin directory to the PATH.
			// This is needed by the "jvm.dll" in case its dependencies could not be found
			// in the Windows system directory, which sometimes happens after a Java update.
			auto utf8JrePath = Rtt::JavaHost::GetJrePath();
			if (utf8JrePath)
			{
				stringConverter.SetUTF8(utf8JrePath);
				if (!stringConverter.EndsWith("\\"))
				{
					stringConverter.Append("\\");
				}
				stringConverter.Append("bin;");
				newUtf16PathString += stringConverter.GetUTF16();
			}

			// Fetch the system's current PATH string and append it to the end of the new PATH string.
			// Note: It's very important the above paths are at the beginning of the PATH in case the current system
			//       PATH contains references to incompatible JDK/JRE directories, such as 64-bit versions.
			auto pathCharacterCount = ::GetEnvironmentVariableW(L"PATH", nullptr, 0);
			if (pathCharacterCount > 0)
			{
				pathCharacterCount++;
				auto utf16Buffer = new wchar_t[pathCharacterCount];
				utf16Buffer[0] = L'\0';
				::GetEnvironmentVariableW(L"PATH", utf16Buffer, pathCharacterCount);
				newUtf16PathString += utf16Buffer;
				delete[] utf16Buffer;
			}

			// Apply the new PATH settings to this application process.
			::SetEnvironmentVariableW(L"PATH", newUtf16PathString.c_str());
		}
	}

#if USE_JNI
	// Load JAR files needed to run ANT and read key stores.
	if (bInitialized)
	{
		CString resourceDir = ((CSimulatorApp *)AfxGetApp())->GetResourceDir();
		CString jarPath;
		WinString strJarPath;

		jarPath = resourceDir + _T("\\ListKeyStore.jar");
		if ( !CheckJarPath( jarPath ) )
			return false;
		strJarPath.SetTCHAR( jarPath );
		Rtt::JavaHost::AddJar( strJarPath.GetUTF8() );

		jarPath = resourceDir + _T("\\ant.jar");
		if ( !CheckJarPath( jarPath ) )
			return false;
		strJarPath.SetTCHAR( jarPath );
		Rtt::JavaHost::AddJar( strJarPath.GetUTF8() );

		jarPath = resourceDir + _T("\\ant-launcher.jar");
		if ( !CheckJarPath( jarPath ) )
			return false;
		strJarPath.SetTCHAR( jarPath );
		Rtt::JavaHost::AddJar( strJarPath.GetUTF8() );

		jarPath = resourceDir + _T("\\AntInvoke.jar");
		if ( !CheckJarPath( jarPath ) )
			return false;
		strJarPath.SetTCHAR( jarPath );
		Rtt::JavaHost::AddJar( strJarPath.GetUTF8() );

		jarPath = resourceDir + _T("\\AntLiveManifest.jar");
		if (!CheckJarPath(jarPath))
			return false;
		strJarPath.SetTCHAR(jarPath);
		Rtt::JavaHost::AddJar(strJarPath.GetUTF8());
	}
#endif // USE_JNI

	return bInitialized;
}

// AuthorizeInstance - Call into shared code to check if the current instance is
// authorized, and show License & Login dialog if not.
BOOL CSimulatorApp::AuthorizeInstance()
{
    // Check for saved ticket and log in if necessary
	return TRUE;
}

// ShowProgressWnd - show or hide the modeless progress window.
// Center it over parent window if showing.
void CSimulatorApp::ShowProgressWnd( bool bShow, CWnd *pParent /* = NULL */ )
{
    // Place message relative to parent dialog
    if (nullptr == pParent)
	{
		pParent = CWnd::GetActiveWindow();
		if (nullptr == pParent)
		{
			pParent = AfxGetMainWnd()->GetLastActivePopup();
			if (nullptr == pParent)
			{
				pParent = AfxGetMainWnd();
			}
		}
	}

    if (pParent && bShow)
	{
        CRect rectProgress, rectParent;
        m_pProgressWnd->GetWindowRect( rectProgress );
        pParent->GetWindowRect( rectParent );
        // center progress msg over parent window
        rectProgress.left = rectParent.left + rectParent.Width() / 2 - rectProgress.Width() / 2;
        rectProgress.top = rectParent.top + rectParent.Height() / 2 - rectProgress.Height() / 2;

        m_pProgressWnd->SetWindowPos( NULL, rectProgress.left, rectProgress.top, 0, 0,  SWP_NOOWNERZORDER | SWP_NOSIZE );

		// TODO: set progress bar to 50%
	}

	m_pProgressWnd->ShowWindow( bShow ? SW_SHOW : SW_HIDE );
    m_pProgressWnd->UpdateWindow();
	if (bShow)
	{
		m_pProgressWnd->SetFocus();
		m_pProgressWnd->SetActiveWindow();
	}
}

/// Gets this application's absolute path without the file name.
CString CSimulatorApp::GetApplicationDir()
{
	// Fetch the app's absolute path without the file name, if not done so already.
	if (m_sApplicationDir.IsEmpty())
	{
		const int MAX_PATH_LENGTH = 1024;
		CString applicationPath;
		int result;
		int index;
		result = ::GetModuleFileName(nullptr, applicationPath.GetBuffer(MAX_PATH_LENGTH), MAX_PATH_LENGTH);
		applicationPath.ReleaseBuffer((int)result);
		if (result > 0)
		{
			// Remove the file name from the path string.
			index = applicationPath.ReverseFind(_T('\\'));
			if (index > 0)
			{
				m_sApplicationDir = applicationPath.Left(index);
			}
		}
	}

	// Return this application's absolute path.
	return m_sApplicationDir;
}

// GetResourceDir - Return directory for skin image files, and other needed files.
// Path is AppPath\Resources
CString CSimulatorApp::GetResourceDir()
{
    if (m_sResourceDir.IsEmpty())
	{
		m_sResourceDir = GetApplicationDir();
		m_sResourceDir += _T("\\Resources");
	}
    return m_sResourceDir;
}

// GetSampleDir - Path is AppPath\Sample Code\ 
CString CSimulatorApp::GetSampleDir()
{
	if (m_sSampleDir.IsEmpty())
	{
		m_sSampleDir = GetApplicationDir();
		m_sSampleDir += _T("\\Sample Code");
		if (CheckDirExists(m_sSampleDir) == false)
		{
			// Directory not found.
			// If this is a developer version, then check for samples under the "docs" directory.
			CString developerSampleDirectory = GetApplicationDir();
			developerSampleDirectory += _T("\\..\\..\\..\\..\\..\\docs\\SampleCode");
			if (CheckDirExists(developerSampleDirectory))
			{
				m_sSampleDir = developerSampleDirectory;
			}
		}
	}
	return m_sSampleDir;
}

/// Gets the absolute path and file name of the Home Screen's "main.lua" file.
/// @return Returns an absolute path and file name to the Home Screen's "main.lua" file.
CString CSimulatorApp::GetHomeScreenFilePath()
{
	if (m_sHomeScreenFilePath.GetLength() <= 0)
	{
		m_sHomeScreenFilePath = GetResourceDir();
		m_sHomeScreenFilePath += _T("\\homescreen\\main.lua");
	}
	return m_sHomeScreenFilePath;
}

/// Determines if the welcome/home screen is enabled and should be shown to the user.
/// @return Returns true if the enabled and the screen should be shown to the user.
///         Returns false if disabled and it should not be shown.
bool CSimulatorApp::IsHomeScreenEnabled()
{
	bool isDisabled = GetProfileInt(REGISTRY_SECTION, REGISTRY_NOWELCOME, REGISTRY_NOWELCOME_DEFAULT) ? true : false;
	return !isDisabled;
}

/// Updates the application settings to enable/disable the welcome/home screen.
/// @param enabled Set true to show the welcome screen. Set false to keep it hidden.
void CSimulatorApp::EnableHomeScreen(bool enabled)
{
	WriteProfileInt(REGISTRY_SECTION, REGISTRY_NOWELCOME, enabled ? 0 : 1);
}
void CSimulatorApp::ShowRuntimeErrors(bool showErrors)
{
	WriteProfileInt(REGISTRY_SECTION, REGISTRY_SHOWERRORS, showErrors ? 1 : 0);
}
bool CSimulatorApp::IsShowingRuntimeErrors()
{
	bool isEnabled = GetProfileInt(REGISTRY_SECTION, REGISTRY_SHOWERRORS, REGISTRY_SHOWERRORS_DEFAULT) ? true : false;
	return isEnabled;
}

void CSimulatorApp::AutoOpenLastProject(bool autoOpen)
{
	WriteProfileInt(REGISTRY_SECTION, REGISTRY_AUTOOPEN, autoOpen ? 1 : 0);
}
bool CSimulatorApp::ShouldAutoOpenLastProject()
{
	bool autoOpen = GetProfileInt(REGISTRY_SECTION, REGISTRY_AUTOOPEN, REGISTRY_AUTOOPEN_DEFAULT) ? true : false;
	return autoOpen;
}
void CSimulatorApp::SetRelaunchSimStyle(int relaunchSimStyle)
{
	WriteProfileInt(REGISTRY_SECTION, REGISTRY_RELAUNCHSIM, relaunchSimStyle);
}
int CSimulatorApp::GetRelaunchSimStyle()
{
	int style = GetProfileInt(REGISTRY_SECTION, REGISTRY_RELAUNCHSIM, REGISTRY_RELAUNCHSIM_DEFAULT);
	return style;
}
bool CSimulatorApp::ShouldShowWebBuildDlg()
{
	return true;
}
bool CSimulatorApp::ShouldShowLinuxBuildDlg()
{
	bool show = GetProfileInt(REGISTRY_SECTION, REGISTRY_SHOWLINUXBUILD, REGISTRY_SHOWLINUXBUILD_DEFAULT) ? true : false;
	return show;
}
bool CSimulatorApp::ShouldShowNXBuildDlg()
{
	bool show = getenv("NINTENDO_SDK_ROOT") != NULL;
	return show;
}

// CheckPathExists - return true if the file/directory exists.
// Make sure paths don't have trailing backslashes
bool CSimulatorApp::CheckPathExists(LPCTSTR path)
{ 
	WIN32_FIND_DATA  data; 
	HANDLE handle = FindFirstFile(path,&data); 
	if (handle != INVALID_HANDLE_VALUE)
	{
		FindClose(handle);
		return true;
	}
	return false;
}

// CheckDirExists - return true if dirName is directory and exists
// Make sure paths don't have trailing backslashes
bool CSimulatorApp::CheckDirExists(LPCTSTR dirName)
{ 
	WIN32_FIND_DATA  data; 
	HANDLE handle = FindFirstFile(dirName,&data); 
	if (handle != INVALID_HANDLE_VALUE)
	{
		FindClose(handle);
		return (0 != (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
	}
	return false;
}

// GetWorkingDir - return the directory to use for File Open dialogs
// Used to save value to registry on exit
CString CSimulatorApp::GetWorkingDir() 
{ 
	return ((CSimDocManager *)m_pDocManager)->GetInitialDir();
}

// SetWorkingDir - set the directory to use for File Open dialogs
// Remembered in registry on app exit/init, and set when project is opened
void CSimulatorApp::SetWorkingDir( CString sDir ) 
{ 
	((CSimDocManager *)m_pDocManager)->SetInitialDir( sDir );
}

// ExitInstance - save position, etc. to registry
int CSimulatorApp::ExitInstance()
{
	// Check if we're exiting before we initialized.
	if (m_pDocManager == nullptr)
	{
		return CWinApp::ExitInstance();
	}

    // Save user preferences to registry
    if (!GetDisplayName().IsEmpty())
	{
		WriteProfileString( REGISTRY_SECTION, REGISTRY_DEVICE, GetDisplayName());
	}
    if (!GetWorkingDir().IsEmpty())
	{
		WriteProfileString( REGISTRY_SECTION, REGISTRY_WORKINGDIR, GetWorkingDir() );
	}

    // Write window position (saved in CMainFrame::OnClose)
	WriteProfileInt(REGISTRY_SECTION, REGISTRY_ROTATION, GetRotation());
	WriteProfileInt(REGISTRY_SECTION, REGISTRY_XPOS, m_WP.rcNormalPosition.left);
    WriteProfileInt( REGISTRY_SECTION, REGISTRY_YPOS, m_WP.rcNormalPosition.top);

	// End the analytics session, if enabled.
	auto analyticsPointer = GetWinProperties()->GetAnalytics();
	if (analyticsPointer)
	{
		analyticsPointer->EndSession();
	}

	// Close the logging window if currently running.
	if (m_outputViewerProcessPointer)
	{
		// Re-enable the logging window's close [x] button.
		// Note: This window will ignore WM_CLOSE messages while the close button is disabled.
		auto outputViewerWindowHandle = m_outputViewerProcessPointer->FetchMainWindowHandle();
		if (outputViewerWindowHandle)
		{
			auto menuHandle = ::GetSystemMenu(outputViewerWindowHandle, FALSE);
			if (menuHandle)
			{
				::EnableMenuItem(menuHandle, SC_CLOSE, MF_ENABLED);
			}
		}

		// Close the logging window gracefully via a WM_CLOSE message.
		m_outputViewerProcessPointer->RequestCloseMainWindow();
		m_outputViewerProcessPointer = nullptr;

		if (!GetConsoleWindow()) {
			AllocConsole();
			ShowWindow(GetConsoleWindow(), SW_HIDE);
		}
		FILE* notused;
		freopen_s(&notused, "CONOUT$", "w", stdout);
		freopen_s(&notused, "CONOUT$", "w", stderr);
	}

	// Destroy the progress dialog, if allocated.
	if (m_pProgressWnd)
	{
		m_pProgressWnd->DestroyWindow();
		delete m_pProgressWnd;
	}

    // Uninitialize GDIplus
	Gdiplus::GdiplusShutdown(m_gdiplusToken);

	// If we got this far, we ran successfully without a crash (mostly
	// used to detect crappy video drivers that crash us)
	WriteProfileInt(REGISTRY_SECTION, REGISTRY_LAST_RUN_SUCCEEDED, 1);

	// Exit this application.
	return CWinApp::ExitInstance();
}

// PutWP - save window placement so it can be written to registry
void CSimulatorApp::PutWP(const WINDOWPLACEMENT& newval)
{	
	m_WP = newval;
	m_WP.length = sizeof(m_WP);
}

void CSimulatorApp::SaveZoomToRegistry(int zoom)
{
	CMainFrame *pMainFrm = (CMainFrame *)m_pMainWnd;
	CSimulatorView *pView = (CSimulatorView *)pMainFrm->GetActiveView();
	if (pView&& !pView->IsShowingInternalScreen())
	{
		WriteProfileInt( REGISTRY_SECTION, REGISTRY_ZOOM, zoom );
	}
}

void CSimulatorApp::LoadZoomFromRegistry()
{
	m_zoom = (int) GetProfileInt( REGISTRY_SECTION, REGISTRY_ZOOM, REGISTRY_ZOOM_DEFAULT );
	CMainFrame *pMainFrm = (CMainFrame *)m_pMainWnd;
	if (pMainFrm)
	{
		pMainFrm->SetZoom(m_zoom);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// CRecentDirList
/////////////////////////////////////////////////////////////////////////////////////////
/* This class is a subclass of CRecentFileList which overrides the
* GetDisplayName() member function to display the directory name instead of 
* the filename, which is always main.lua.
*/
BOOL CRecentDirList::GetDisplayName( CString &strName, int nIndex, LPCTSTR lpszCurDir, int nCurDir, BOOL bAtLeastName = 1) const
{
    // Change entry at nIndex to remove "main.lua"
    CString sOrigPath = m_arrNames[nIndex];
	CString sDirPath = CCoronaProject::RemoveMainLua( sOrigPath );
    m_arrNames[nIndex] = sDirPath;

    BOOL bRetval = CRecentFileList::GetDisplayName( strName, nIndex, lpszCurDir, nCurDir, bAtLeastName );

    // Restore entry
    m_arrNames[nIndex] = sOrigPath;

	return bRetval;
}



////////////////////////////////////////////////////////////////////////////////////////////
// CSimDocManager - our own version of CDocManager
////////////////////////////////////////////////////////////////////////////////////////////
// Overloaded to save working directory for File Open dialog
CSimDocManager::CSimDocManager()
{
}

CSimDocManager::CSimDocManager( CSimDocManager &mgr ) 
{
	m_sInitialDir = mgr.m_sInitialDir;
}

// defined below
void AFXAPI _AfxAppendFilterSuffix(
		CString& filter, OPENFILENAME& ofn, CDocTemplate* pTemplate, CString* pstrDefaultExt);

// Override undocumented class CDocManager in order to set initial directory for Open dialog
// This function copied from MFC source docmgr.cpp
BOOL CSimDocManager::DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate)
{
    // Call derived class which only selects main.lua
	CLuaFileDialog dlgFile(bOpenFileDialog, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL, 0);

    // Here's the big change
    // Set dialog's initial directory
    dlgFile.m_ofn.lpstrInitialDir = GetInitialDir();

	// Load the dialog title string.
	CString title;
	title.LoadString(nIDSTitle);

	// Set up the file filter string.
	CString strFilter;
	CString stringBuffer;
	stringBuffer.LoadString(IDS_SIMULATOR_FILES);
	strFilter += stringBuffer;
	strFilter += TCHAR('\0');
	strFilter += _T("main.lua");
	strFilter += TCHAR('\0');
	stringBuffer.LoadString(AFX_IDS_ALLFILTER);
	strFilter += stringBuffer;
	strFilter += (TCHAR)'\0';
	strFilter += _T("*.*");
	strFilter += (TCHAR)'\0';
	strFilter += (TCHAR)'\0';

	// Set up the dialog.
	dlgFile.m_ofn.Flags |= lFlags;
	dlgFile.m_ofn.nMaxCustFilter++;
	dlgFile.m_ofn.lpstrFilter = strFilter;
	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

	// Show the "Open File" dialog.
	INT_PTR nResult = dlgFile.DoModal();
	fileName.ReleaseBuffer();
	return (nResult == IDOK);
}

// Copied from docmgr.cpp because DoPromptFilename needs it.
/*
AFX_STATIC void AFXAPI _AfxAppendFilterSuffix(
	CString& filter, OPENFILENAME& ofn, CDocTemplate* pTemplate, CString* pstrDefaultExt)
{
	ENSURE_VALID(pTemplate);
	ASSERT_KINDOF(CDocTemplate, pTemplate);

	CString strFilterExt, strFilterName;
	if (pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt) &&
		!strFilterExt.IsEmpty() &&
		pTemplate->GetDocString(strFilterName, CDocTemplate::filterName) &&
		!strFilterName.IsEmpty())
	{
		if (pstrDefaultExt != NULL)
			pstrDefaultExt->Empty();

		// add to filter
		filter += strFilterName;
		ASSERT(!filter.IsEmpty());  // must have a file type name
		filter += (TCHAR)'\0';  // next string please

		int iStart = 0;
		do
		{
			CString strExtension = strFilterExt.Tokenize( _T( ";" ), iStart );

			if (iStart != -1)
			{
				// a file based document template - add to filter list
				int index = strExtension.Find(TCHAR('.'));
				if (index >= 0)
				{
					if ((pstrDefaultExt != NULL) && pstrDefaultExt->IsEmpty())
					{
						// set the default extension
						*pstrDefaultExt = strExtension.Mid( index + 1 );  // skip the '.'
						ofn.lpstrDefExt = const_cast< LPTSTR >((LPCTSTR)(*pstrDefaultExt));
						ofn.nFilterIndex = ofn.nMaxCustFilter + 1;  // 1 based number
					}
					if (0 == index)
					{
						filter += (TCHAR)'*';
					}
					filter += strExtension;
					filter += (TCHAR)';';  // Always append a ';'.  The last ';' will get replaced with a '\0' later.
				}
			}
		} while (iStart != -1);

		filter.SetAt( filter.GetLength()-1, '\0' );;  // Replace the last ';' with a '\0'
		ofn.nMaxCustFilter++;
	}
}
*/
/////////////////////////////////////////////////////////////////////////////////////
// CLuaFileDialog dialog - only allow selection of main.lua
/////////////////////////////////////////////////////////////////////////////////////
CString CLuaFileDialog::szCustomDefFilter(_T("All Files (*.*)|*.*|Lua Files (*.lua)|*.lua||"));
CString CLuaFileDialog::szCustomDefExt(_T("lua"));
CString CLuaFileDialog::szCustomDefFileName(_T("main"));

CLuaFileDialog::CLuaFileDialog(
	BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
	LPCTSTR lpszDefExt,
	LPCTSTR lpszFileName,
	DWORD dwFlags,
	LPCTSTR lpszFilter,
	CWnd* pParentWnd, 
	DWORD dwSize,
	BOOL bVistaStyle)
:	CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, dwSize, bVistaStyle)
{
}

// OnFileNameOK - override member of CFileDialog
// Returns 0 if filename allowed, 1 otherwise
// We only allow main.lua
BOOL CLuaFileDialog::OnFileNameOK()
{
	CString sPath = GetPathName();
    CString sFilename = _T("\\main.lua");
	if (sPath.Right(sFilename.GetLength()) == sFilename)
	{
		return 0;
	}

	::AfxMessageBox( IDS_ONLYMAINLUA );
	return 1;
}
