//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BuildWin32AppDlg.h"
#include "Core\Rtt_Build.h"
#include "Core\Rtt_String.h"
#include "Interop\Ipc\Process.h"
#include "Interop\UI\TaskDialog.h"
#include "Interop\SimulatorRuntimeEnvironment.h"
#include "CoronaInterface.h"
#include "CoronaProject.h"
#include "BrowseDirDialog.h"
#include "MessageDlg.h"
#include "Rtt_SimulatorAnalytics.h"
#include "Rtt_Win32AppPackager.h"
#include "Rtt_Win32AppPackagerParams.h"
#include "Rtt_WinPlatform.h"
#include "Rtt_WinPlatformServices.h"
#include "Simulator.h"
#include "SimulatorDoc.h"
#include "SimulatorView.h"
#include "WinGlobalProperties.h"
#include "WinString.h"


IMPLEMENT_DYNAMIC(CBuildWin32AppDlg, CDialog)


#pragma region Constructor/Destructor
CBuildWin32AppDlg::CBuildWin32AppDlg(CWnd* pParent /*=NULL*/)
:	CDialog(CBuildWin32AppDlg::IDD, pParent),
	fProjectPointer(nullptr),
	fHasRanBuiltApp(false)
{
}

CBuildWin32AppDlg::~CBuildWin32AppDlg()
{
}

#pragma endregion


#pragma region Message Mappings
void CBuildWin32AppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBuildWin32AppDlg, CDialog)
	ON_BN_CLICKED(IDC_BUILD_BROWSE_SAVETO, &CBuildWin32AppDlg::OnBrowseSaveto)
	ON_WM_HELPINFO()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()

#pragma endregion


#pragma region Public Methods
BOOL CBuildWin32AppDlg::OnInitDialog()
{
	// Initialize base class first.
	CDialog::OnInitDialog();

	// Do not display this window if not given a project settings object.
	if (!fProjectPointer)
	{
		CDialog::OnCancel();
		return TRUE;
	}

	// Set up the app name field.
	((CEdit*)GetDlgItem(IDC_BUILD_APPNAME))->SetLimitText(128);
	SetDlgItemText(IDC_BUILD_APPNAME, fProjectPointer->GetName());

	// Set up the EXE file name field.
	((CEdit*)GetDlgItem(IDC_BUILD_EXE_FILE_NAME))->SetLimitText(32);
	SetDlgItemText(IDC_BUILD_EXE_FILE_NAME, fProjectPointer->GetExeFileName());

	// Set up the version string field.
	((CEdit*)GetDlgItem(IDC_BUILD_VERSION_NAME))->SetLimitText(16);
	SetDlgItemText(IDC_BUILD_VERSION_NAME, fProjectPointer->GetWin32VersionString());

	// Set up the copyright string field.
	((CEdit*)GetDlgItem(IDC_BUILD_COPYRIGHT))->SetLimitText(128);
	CString copyrightString = fProjectPointer->GetCopyright();
	copyrightString.Trim();
	if (copyrightString.IsEmpty())
	{
		copyrightString.Format(_T("Copyright %d"), CTime::GetCurrentTime().GetYear());
	}
	SetDlgItemText(IDC_BUILD_COPYRIGHT, copyrightString);

	// Set up the company name field.
	((CEdit*)GetDlgItem(IDC_BUILD_COMPANY_NAME))->SetLimitText(128);
	CString companyName = fProjectPointer->GetCompanyName();
	companyName.Trim();
	SetDlgItemText(IDC_BUILD_COMPANY_NAME, companyName);

	// Set up the app description field.
	((CEdit*)GetDlgItem(IDC_BUILD_APP_DESCRIPTION))->SetLimitText(128);
	SetDlgItemText(IDC_BUILD_APP_DESCRIPTION, fProjectPointer->GetAppDescription());

	// Set up the project path field.
	SetDlgItemText(IDC_BUILD_PROJECTPATH, fProjectPointer->GetDir());

	// Set up the destination path field.
	SetDlgItemText(IDC_BUILD_SAVETO, fProjectPointer->GetSaveDir());
	((CEdit*)GetDlgItem(IDC_BUILD_SAVETO))->SetLimitText(1024);

	// Return TRUE unless you set the focus to a control.
	return TRUE;
}

void CBuildWin32AppDlg::OnBrowseSaveto()
{
	// Fetch the "Save to Folder" field's path.
	CString directoryPath;
	GetDlgItemText(IDC_BUILD_SAVETO, directoryPath);
	directoryPath.Trim();

	// Default to the system's Documents if the above field is empty.
	if (directoryPath.IsEmpty())
	{
		::SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, 0, directoryPath.GetBuffer(MAX_PATH));
		directoryPath.ReleaseBuffer();
	}

	// Update the "Save to Folder" field if a selection was made.
	if (CBrowseDirDialog::Browse(directoryPath, IDS_SELECT_BUILD_OUTPUT_FOLDER_DESCRIPTION))
	{
		SetDlgItemText(IDC_BUILD_SAVETO, directoryPath);
	}
}

BOOL CBuildWin32AppDlg::OnHelpInfo(HELPINFO* helpInfoPointer)
{
	// Display help information related to this dialog.
	// Note: If we don't handle this here, then SimulatorView::OnHelp() will get called which displays
	//       the main Corona documentation page on Corona Labs' website.
	OnSysCommand(SC_CONTEXTHELP, 0);
	return TRUE;
}

void CBuildWin32AppDlg::SetProject(const std::shared_ptr<CCoronaProject>& projectPointer)
{
	// Not allowed to change the project while this dialog is currently displaying another project's settings.
	if (GetSafeHwnd() && IsWindowVisible())
	{
		return;
	}

	// Store the given project.
	fProjectPointer = projectPointer;
}

std::shared_ptr<CCoronaProject> CBuildWin32AppDlg::GetProject() const
{
	return fProjectPointer;
}

bool CBuildWin32AppDlg::HasRanBuiltApp() const
{
	return fHasRanBuiltApp;
}

#pragma endregion


#pragma region Protected Methods
void CBuildWin32AppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// Handle the received dialog system command.
	if (SC_CONTEXTHELP == nID)
	{
		// The help [?] button or F1 key was pressed.
		// Display the Win32 app build documentation on Corona Labs' website.
		try
		{
			const wchar_t kUrl[] = L"https://docs.coronalabs.com/daily/guide/distribution/win32Build";
			::ShellExecuteW(nullptr, L"open", kUrl, nullptr, nullptr, SW_SHOWNORMAL);
		}
		catch (...) {}
	}
	else
	{
		// Let the dialog perform its default handling on the received system command.
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CBuildWin32AppDlg::OnOK()
{
	WinString stringTranscoder;
	CString appName;
	CString exeFileName;
	CString versionString;
	CString copyright;
	CString companyName;
	CString appDescription;
	CString rootDestinationDirectoryPath;
	time_t startTime = time(NULL);

	// Fetch and validate the app name field.
	GetDlgItemText(IDC_BUILD_APPNAME, appName);
	appName.Trim();
	if (appName.IsEmpty())
	{
		DisplayWarningMessage(IDS_BUILD_APP_NAME_NOT_PROVIDED);
		GetDlgItem(IDC_BUILD_APPNAME)->SetFocus();
		LogAnalytics("build-bungled", "reason", "BUILD_APP_NAME_NOT_PROVIDED");
		return;
	}

	// Fetch and validate the EXE file name field.
	GetDlgItemText(IDC_BUILD_EXE_FILE_NAME, exeFileName);
	exeFileName.Trim();
	if (exeFileName.IsEmpty())
	{
		DisplayWarningMessage(IDS_BUILD_EXE_FILE_NAME_NOT_PROVIDED);
		GetDlgItem(IDC_BUILD_EXE_FILE_NAME)->SetFocus();
		LogAnalytics("build-bungled", "reason", "BUILD_EXE_FILE_NAME_NOT_PROVIDED");
		return;
	}
	TCHAR invalidFileCharacters[] = L"\\/:*?\"<>|";
	if (exeFileName.FindOneOf(invalidFileCharacters) >= 0)
	{
		CString extraText = _T("    ");
		int characterCount = (int)_tcsclen(invalidFileCharacters);
		for (int index = 0; index < characterCount; index++)
		{
			extraText.AppendChar(_T(' '));
			extraText.AppendChar(invalidFileCharacters[index]);
		}
		DisplayWarningMessage(IDS_BUILD_EXE_FILE_NAME_CHARS_INVALID, extraText);
		GetDlgItem(IDC_BUILD_EXE_FILE_NAME)->SetFocus();
		LogAnalytics("build-bungled", "reason", "BUILD_EXE_FILE_NAME_INVALID");
		return;
	}

	// Fetch and validate the version string field.
	GetDlgItemText(IDC_BUILD_VERSION_NAME, versionString);
	versionString.Trim();
	if (versionString.IsEmpty())
	{
		DisplayWarningMessage(IDS_BUILD_VERSION_STRING_NOT_PROVIDED);
		GetDlgItem(IDC_BUILD_VERSION_NAME)->SetFocus();
		LogAnalytics("build-bungled", "reason", "BUILD_VERSION_NAME_NOT_PROVIDED");
		return;
	}
	{
		bool isValid = true;
		bool wasLastCharacterAPeriod = false;
		int periodCount = 0;
		CString validVersionCharacters(_T("0123456789."));
		for (int index = versionString.GetLength() - 1; index >= 0; index--)
		{
			TCHAR nextCharacter = versionString.GetAt(index);
			if (validVersionCharacters.Find(nextCharacter) < 0)
			{
				// Version string contains an invalid character.
				isValid = false;
				break;
			}
			if (_T('.') == nextCharacter)
			{
				if (wasLastCharacterAPeriod)
				{
					// Version string not allowed to have 2 periods next to each other.
					// Periods must have number inbetween them.
					isValid = false;
					break;
				}
				periodCount++;
				wasLastCharacterAPeriod = true;
				if (periodCount > 3)
				{
					// Windows version string are not allowed to have more than 3 periods in them.
					// Only supports up to 4 version number parts:  <Major>.<Minor>.<Build>.<Special>
					isValid = false;
					break;
				}
			}
			else
			{
				wasLastCharacterAPeriod = false;
			}
		}
		if ((_T('.') == versionString.GetAt(0)) || (_T('.') == versionString.GetAt(versionString.GetLength() - 1)))
		{
			// Version string cannot begin or end with a period.
			isValid = false;
		}
		if (!isValid)
		{
			DisplayWarningMessage(IDS_BUILD_WIN32_VERSION_STRING_INVALID);
			GetDlgItem(IDC_BUILD_VERSION_NAME)->SetFocus();
			LogAnalytics("build-bungled", "reason", "BUILD_VERSION_NAME_INVALID");
			return;
		}
	}

	// Fetch and validate the root build destination path.
	GetDlgItemText(IDC_BUILD_SAVETO, rootDestinationDirectoryPath);
	rootDestinationDirectoryPath.TrimLeft();
	rootDestinationDirectoryPath.TrimRight(_T(" \\/"));
	if (rootDestinationDirectoryPath.IsEmpty())
	{
		DisplayWarningMessage(IDS_BUILD_PATH_NOT_PROVIDED);
		GetDlgItem(IDC_BUILD_SAVETO)->SetFocus();
		LogAnalytics("build-bungled", "reason", "BUILD_PATH_NOT_PROVIDED");
		return;
	}
	if (rootDestinationDirectoryPath.Compare(fProjectPointer->GetDir()) == 0)
	{
		DisplayWarningMessage(IDS_BUILD_PATH_INVALID);
		GetDlgItem(IDC_BUILD_SAVETO)->SetFocus();
		LogAnalytics("build-bungled", "reason", "BUILD_PATH_INVALID");
		return;
	}
	if (::PathIsDirectory(rootDestinationDirectoryPath) == FALSE)
	{
		// The given path does not exist. Attempt to create it. If we are unable to create the directory,
		// then that may mean the path is invalid or we do not have permission to create it at that location.
		int result = ::SHCreateDirectoryEx(NULL, rootDestinationDirectoryPath, NULL);
		if (result != ERROR_SUCCESS)
		{
			DisplayWarningMessage(IDS_BUILD_PATH_CREATION_FAILED, rootDestinationDirectoryPath);
			GetDlgItem(IDC_BUILD_SAVETO)->SetFocus();
			LogAnalytics("build-bungled", "reason", "BUILD_PATH_CREATION_FAILED");
			return;
		}
	}

	// Set up a subdirectory path to copy the built Win32 app's files to.
	// This will be a subfolder under the root destination directory as follows.
	// Ex:    <DestinationDirectory>\<AppName>.Win32
	WinString win32DestinationDirectoryPath(rootDestinationDirectoryPath);
	win32DestinationDirectoryPath.Append(L"\\");
	{
		stringTranscoder.SetTCHAR(appName);
		Rtt::String escapedAppName;
		Rtt::PlatformAppPackager::EscapeFileName(stringTranscoder.GetUTF8(), escapedAppName, false);
		if (escapedAppName.IsEmpty() == false)
		{
			win32DestinationDirectoryPath.Append(escapedAppName.GetString());
			win32DestinationDirectoryPath.Append(".");
		}
	}
	win32DestinationDirectoryPath.Append("Win32");

	// Fetch the optional fields. It is okay for these to be empty.
	GetDlgItemText(IDC_BUILD_COPYRIGHT, copyright);
	copyright.Trim();
	GetDlgItemText(IDC_BUILD_COMPANY_NAME, companyName);
	companyName.Trim();
	GetDlgItemText(IDC_BUILD_APP_DESCRIPTION, appDescription);
	appDescription.Trim();

	// Fetch the Corona runtime that we're building for.
	auto simulatorViewPointer = (CSimulatorView*)((CFrameWnd*)::AfxGetMainWnd())->GetActiveView();
	auto runtimeEnvironmentPointer = simulatorViewPointer->GetRuntimeEnvironment();
	if (!runtimeEnvironmentPointer || !runtimeEnvironmentPointer->GetRuntime())
	{
		// A Corona project is not currently selected by the simulator.
		// This should never happen, but if it does, attempt to restart the simulator with this dialog's selected project.
		CString mainLuaFilePath = fProjectPointer->GetDir() + _T("\\main.lua");
		simulatorViewPointer->GetDocument()->GetDocTemplate()->OpenDocumentFile(mainLuaFilePath);
		return;
	}

	// Do not continue if not all plugins for this Corona project have been downloaded yet.
	// We do this because local Win32 app builds require the plugins zips to be downloaded first.
	if (simulatorViewPointer->VerifyAllPluginsAcquired() == false)
	{
		return;
	}

	// Create the packager used to build the app.
	auto platformServicesPointer = GetWinProperties()->GetServices();
	Rtt::Win32AppPackager packager(*platformServicesPointer);

	// Read and validate the project's "build.settings" file.
	WinString projectDirectoryPath;
	projectDirectoryPath.SetTCHAR(fProjectPointer->GetDir());
	bool wasBuildSettingsFileRead = packager.ReadBuildSettings(projectDirectoryPath.GetUTF8());
	if (!wasBuildSettingsFileRead)
	{
		LogAnalytics("build-bungled", "reason", "bad-build-settings");
		DisplayWarningMessage(IDS_BUILD_SETTINGS_FILE_ERROR);
		return;
	}

	// Save the project's build settings.
	fProjectPointer->SetName(appName);
	fProjectPointer->SetExeFileName(exeFileName);
	fProjectPointer->SetWin32VersionString(versionString);
	fProjectPointer->SetCopyright(copyright);
	fProjectPointer->SetCompanyName(companyName);
	fProjectPointer->SetAppDescription(appDescription);
	fProjectPointer->SetSaveDir(rootDestinationDirectoryPath);
	fProjectPointer->Save();

	// Log to the analytics system that the user is building for Win32.
	LogAnalytics("build");

	// Configure the Win32 app packager's parameters.
	Rtt::Win32AppPackagerParams::CoreSettings paramsSettings{};
	WinString appNameTranscoder(appName);
	paramsSettings.AppName = appNameTranscoder.GetUTF8();
	WinString versionStringTranscoder(versionString);
	paramsSettings.VersionString = versionStringTranscoder.GetUTF8();
	WinString destinationDirectoryPathTranscoder(win32DestinationDirectoryPath.GetUTF8());
	paramsSettings.DestinationDirectoryPath = destinationDirectoryPathTranscoder.GetUTF8();
	paramsSettings.SourceDirectoryPath = projectDirectoryPath.GetUTF8();
	Rtt::Win32AppPackagerParams params(paramsSettings);
	stringTranscoder.SetTCHAR(exeFileName);
	params.SetExeFileName(stringTranscoder.GetUTF8());
	stringTranscoder.SetTCHAR(companyName);
	params.SetCompanyName(stringTranscoder.GetUTF8());
	stringTranscoder.SetTCHAR(copyright);
	params.SetCopyrightString(stringTranscoder.GetUTF8());
	stringTranscoder.SetTCHAR(appDescription);
	params.SetAppDescription(stringTranscoder.GetUTF8());
	params.SetIncludeBuildSettings(true);
	params.SetStripDebug(true);
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
	params.SetIncludeFusePlugins(false);
	params.SetUsesMonetization(false);
#endif
	params.SetRuntime(runtimeEnvironmentPointer->GetRuntime());
	{
		std::string utf8BuildSettingsPath(projectDirectoryPath.GetUTF8());
		if (!projectDirectoryPath.EndsWith("\\") && !projectDirectoryPath.EndsWith("/"))
		{
			utf8BuildSettingsPath.append("\\");
		}
		utf8BuildSettingsPath.append("build.settings");
		params.SetBuildSettingsPath(utf8BuildSettingsPath.c_str());
	}

	// Choose a good hidden temp directory path to be used by the build system.
	// Will be used as an intermediate directory for compiled Lua files, app template extraction, and other purposes.
	// The build system will automatically delete this directory once the build ends.
	WinString tempDirectoryPath;
	{
		const size_t kBufferSize = 1024;
		wchar_t utf16TempDirectoryPath[kBufferSize];
		utf16TempDirectoryPath[0] = L'\0';
		::GetTempPathW(kBufferSize, utf16TempDirectoryPath);
		if (L'\0' == utf16TempDirectoryPath[0])
		{
			CStringW title;
			title.LoadStringW(IDS_WARNING);
			MessageBoxW(L"Failed to create temp directory for build.", title, MB_OK | MB_ICONWARNING);
			return;
		}
		wcscat_s(utf16TempDirectoryPath, kBufferSize, L"Corona Labs\\Win32 Build XXXXXX");
		_wmktemp_s(utf16TempDirectoryPath, wcslen(utf16TempDirectoryPath) + 1);
		tempDirectoryPath.SetUTF16(utf16TempDirectoryPath);
	}

	// Build the Win32 desktop application.
	BeginWaitCursor();
	int buildResultCode = packager.Build(&params, tempDirectoryPath.GetUTF8());
	EndWaitCursor();

	// Display an error message if the build failed.
	if (buildResultCode != 0)
	{
		// Display the error message reported by the build system.
		CStringW title;
		title.LoadStringW(IDS_WARNING);
		WinString errorMessage;
		errorMessage.SetUTF8(params.GetBuildMessage());
		MessageBoxW(errorMessage.GetUTF16(), title, MB_OK | MB_ICONEXCLAMATION);

		// Log the build failure.
		CStringA logMesg;
		logMesg.Format("[%ld] %s", buildResultCode, params.GetBuildMessage());
		LogAnalytics("build-failed", "reason", logMesg);
		Rtt_LogException(params.GetBuildMessage());
		return;
	}

	// Log that the build was successful.
	LogAnalytics("build-succeeded");
	time_t elapsedTime = (time(NULL) - startTime);
	Rtt_LogException("Win32 desktop app build succeeded in %lld second%s", elapsedTime, (elapsedTime == 1 ? "" : "s"));

	// Display a message box to the user indicating that the build has finished.
	// This also provides the following options:
	// - Open the build destination directory in Windows Explorer.
	// - Run the built application via the Corona Console.
	{
		CStringW viewInExplorerMessage;
		viewInExplorerMessage.LoadStringW(IDS_VIEW_EXPLORER);

		Interop::UI::TaskDialog messageDialog;
		messageDialog.GetSettings().SetParentWindowHandle(GetSafeHwnd());
		messageDialog.GetSettings().SetTitleText(L"Question");
		messageDialog.GetSettings().SetMessageText(
				L"Your application was built successfully.\r\nWhat would you like to do next?");
		messageDialog.GetSettings().GetButtonLabels().push_back(L"&Run the App");
		messageDialog.GetSettings().GetButtonLabels().push_back((LPCWSTR)viewInExplorerMessage);
		messageDialog.GetSettings().GetButtonLabels().push_back(L"&Close");
		messageDialog.Show();
		switch (messageDialog.GetLastPressedButtonIndex())
		{
			case 0:
			{
				// Run the built app via the Corona Console.
				WinString outputViewerFilePath(((CSimulatorApp*)::AfxGetApp())->GetApplicationDir());
				outputViewerFilePath.Append(L"\\Corona.Console.exe");
				WinString commandLineArguments;
				commandLineArguments.Append(L"/command:run \"");
				commandLineArguments.Append(win32DestinationDirectoryPath.GetUTF16());
				commandLineArguments.Append(L'\\');
				commandLineArguments.Append(exeFileName);
				commandLineArguments.Append(L'\"');
				Interop::Ipc::Process::LaunchSettings launchSettings{};
				launchSettings.FileNamePath = outputViewerFilePath.GetUTF16();
				launchSettings.CommandLineArguments = commandLineArguments.GetUTF16();
				auto launchResult = Interop::Ipc::Process::LaunchUsing(launchSettings);
				fHasRanBuiltApp = launchResult.HasSucceeded();
				break;
			}
			case 1:
			{
				// Display the build destination directory in Windows Explorer.
				try
				{
					::ShellExecuteW(
						nullptr, L"open", win32DestinationDirectoryPath.GetUTF16(),
						nullptr, nullptr, SW_SHOWNORMAL);
				}
				catch (...) {}
				break;
			}
		}
	}

	// Close this window.
	CDialog::OnOK();
}

void CBuildWin32AppDlg::OnCancel()
{
	// Close this window.
	CDialog::OnCancel();
}

void CBuildWin32AppDlg::DisplayWarningMessage(UINT messageID, const TCHAR* extraText /*= nullptr*/)
{
	CString title;
	CString message;

	title.LoadString(IDS_WARNING);
	message.LoadString(messageID);
	if (extraText)
	{
		message.Append(L"\r\n\r\n");
		message.Append(extraText);
	}
	MessageBox(message, title, MB_OK | MB_ICONWARNING);
}


void CBuildWin32AppDlg::LogAnalytics(const char *eventName, const char *key /* = NULL */, const char *value /* = NULL */)
{
	Rtt_ASSERT(GetWinProperties()->GetAnalytics() != NULL);
	Rtt_ASSERT(eventName != NULL && strlen(eventName) > 0);

	// NEEDSWORK: this is horrible
	size_t numItems = 2;
	char **dataKeys = (char **)calloc(sizeof(char *), numItems);
	char **dataValues = (char **)calloc(sizeof(char *), numItems);

	dataKeys[0] = _strdup("target");
	dataValues[0] = _strdup("win32");
	if (key != NULL && value != NULL)
	{
		dataKeys[1] = _strdup(key);
		dataValues[1] = _strdup(value);
	}
	else
	{
		numItems = 1;
	}

	if (GetWinProperties()->GetAnalytics() != NULL)
	{
		GetWinProperties()->GetAnalytics()->Log(eventName, numItems, dataKeys, dataValues);
	}

	free(dataKeys[0]);
	free(dataValues[0]);
	if (numItems > 1)
	{
		free(dataKeys[1]);
		free(dataValues[1]);
	}
	free(dataKeys);
	free(dataValues);
}

#pragma endregion
