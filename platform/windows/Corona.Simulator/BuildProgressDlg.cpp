//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <gdiplus.h>
#include "Interop\SimulatorRuntimeEnvironment.h"
#include "BuildProgressDlg.h"
#include "Core\Rtt_Build.h"
#include "CoronaInterface.h"
#include "CoronaProject.h"
#include "SimulatorView.h"
#include "WinGlobalProperties.h"
#include "WinString.h"
#include "Simulator.h"


IMPLEMENT_DYNAMIC(CBuildProgressDlg, CDialog)


#pragma region Constructor/Destructor
/// Creates a new build progress window.
CBuildProgressDlg::CBuildProgressDlg(CWnd* pParent /*=NULL*/)
 :	CDialog(CBuildProgressDlg::IDD, pParent),
	fBuildResult(5, CString((LPCSTR)IDS_BUILD_FAILED))
{
	fTargetedAppStorePointer = (Rtt::TargetAndroidAppStore*)&Rtt::TargetAndroidAppStore::kNone;
}

/// Destructor.
CBuildProgressDlg::~CBuildProgressDlg()
{
}

#pragma endregion


#pragma region Message Mappings
/// Associates member variables with window's controls.
void CBuildProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, fProgressBar);
	DDX_Control(pDX, IDC_STATUS_MESSAGE, fStatusMessage);
}

BEGIN_MESSAGE_MAP(CBuildProgressDlg, CDialog)
	ON_WM_TIMER()
	ON_MESSAGE(WMU_BUILD_COMPLETE, &CBuildProgressDlg::OnBuildComplete)
	ON_BN_CLICKED(IDC_STOP_BUILD, &CBuildProgressDlg::OnBnClickedStopBuild)
	ON_MESSAGE(WMU_DOWNLOAD_PROGRESS_UPDATE, &CBuildProgressDlg::OnDownloadProgressUpdate)
END_MESSAGE_MAP()

#pragma endregion

// OnBuildComplete - 
// wParam = unused
// lParam = unused
LRESULT CBuildProgressDlg::OnBuildComplete(WPARAM wParam, LPARAM lParam)
{
	CDialog::OnOK();

	return 0;
}


static DWORD WINAPI BuildThreadFunction(LPVOID contextPtr)
{
	CBuildProgressDlg *thisPtr = (CBuildProgressDlg *)contextPtr;

	thisPtr->BuildForPlatform();

	return 0;
}

#pragma region Window Event Handlers
/// Initializes this dialog's controls.
BOOL CBuildProgressDlg::OnInitDialog()
{
	// Initialize base class first.
	CDialog::OnInitDialog();
	
	// Turn off fancy progress bar because the cylon effect is horrible
	SetWindowTheme(fProgressBar.GetSafeHwnd(), L" ", L" ");

	// Do not continue if not given any project settings.
	if (!fProjectSettingsPointer)
	{
		CDialog::OnCancel();
		return TRUE;
	}

	// Set up progress control to show a marquee at first.
	// Will be changed to a progress bar later when downloading the build
	fProgressBar.ModifyStyle(0, PBS_MARQUEE);
	fProgressBar.SetMarquee(TRUE, 0);
	
	CFrameWnd *pFrameWnd = (CFrameWnd *)AfxGetMainWnd();
	CSimulatorView *pView = (CSimulatorView *)pFrameWnd->GetActiveView();
	fRuntimeEnvironment = pView->GetRuntimeEnvironment();
	CSimulatorApp *pApp = (CSimulatorApp*)AfxGetApp();

	pApp->SetStopBuildRequested(false);

	if (fNoServerMsd)
	{
		fStatusMessage.SetWindowText(L"Packaging app...");
	}

	DWORD   dwThreadId;

	fBuilderThread = CreateThread(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		BuildThreadFunction,       // thread function name
		this,          // argument to thread function 
		0,                      // use default creation flags 
		&dwThreadId);   // returns the thread identifier 


	if (fBuilderThread == NULL)
	{
		Rtt_LogException("Failed to create builder thread");

		return TRUE;
	}

	// Return TRUE unless you set the focus to a control.
	return TRUE;
}

/// Overriden to prevent the user from canceling or escaping out of this dialog.
/// The build operation will automatically close this window instead.
void CBuildProgressDlg::OnCancel()
{
}

#pragma endregion


#pragma region Public Functions
/// Sets the project settings this window should build an Android app with.
/// Must be called before displaying this window.
/// @param projectSettingsPointer Project settings to build an Android app with.
void CBuildProgressDlg::SetProjectSettings(const std::shared_ptr<CCoronaProject>& projectSettingsPointer)
{
	fProjectSettingsPointer = projectSettingsPointer;
}

/// Gets the project settings currently being used by this window.
/// @return Returns a pointer to the project settings object being used.
///         Returns NULL if project settings have not been assigned.
std::shared_ptr<CCoronaProject> CBuildProgressDlg::GetProjectSettings()
{
	return fProjectSettingsPointer;
}

/// Sets the app store and platform this window will build for.
/// Must be called before displaying this window.
/// @param targetedAppStorePointer Pointer to the app store settings to build for. Cannot be NULL.
void CBuildProgressDlg::SetTargetedAppStore(Rtt::TargetAndroidAppStore *targetedAppStorePointer)
{
	fTargetedAppStorePointer = targetedAppStorePointer;
}

/// Gets the app store and platform that this build window is currently targeting.
/// @return Returns a pointer to the app store settings.
Rtt::TargetAndroidAppStore* CBuildProgressDlg::GetTargetedAppStore()
{
	return fTargetedAppStorePointer;
}

/// Gets the build result. Should be called after this window is closed.
/// @return Returns the result for the last build done by this window.
CBuildResult CBuildProgressDlg::GetBuildResult()
{
	return fBuildResult;
}

#pragma endregion


#pragma region Private Functions

void CBuildProgressDlg::BuildForPlatform()
{
	if (fProjectSettingsPointer)
	{
		if (fProjectSettingsPointer->GetTargetPlatform() == Rtt::TargetDevice::kWebPlatform)
		{
			BuildForWeb();
		}
		else if (fProjectSettingsPointer->GetTargetPlatform() == Rtt::TargetDevice::kLinuxPlatform)
		{
			BuildForLinux();
		}
		else if (fProjectSettingsPointer->GetTargetPlatform() == Rtt::TargetDevice::kNxSPlatform)
		{
			BuildForNxS();
		}
		else
		{
			BuildForAndroid();
		}
	}
}

/// Build an Android app using the window's given project settings.
/// Warning: This is a blocking call.
void CBuildProgressDlg::BuildForAndroid()
{
	// Do not continue if project settings were not provided.
	if (!fProjectSettingsPointer || !fTargetedAppStorePointer)
	{
		fBuildResult = CBuildResult(5, CString((LPCSTR)IDS_BUILD_FAILED));
		CDialog::OnOK();
		return;
	}

	// Display a "wait" mouse cursor.
	CWaitCursor waitCursor;

	// Get copy of string settings.
	WinString strSrcDir, strName, strVersion, strPackage;
    WinString strKeystore, strKeystorePwd, strAlias, strAliasPwd, strSaveDir, strTargetOS;

    strSrcDir.SetTCHAR( fProjectSettingsPointer->GetDir() );
    strName.SetTCHAR( fProjectSettingsPointer->GetName() );
    strVersion.SetTCHAR( fProjectSettingsPointer->GetAndroidVersionName() );
    strPackage.SetTCHAR( fProjectSettingsPointer->GetPackage() );
    strKeystore.SetTCHAR( fProjectSettingsPointer->GetKeystorePath() );
    strKeystorePwd.SetTCHAR( fProjectSettingsPointer->GetKeystorePassword() );
    strAlias.SetTCHAR( fProjectSettingsPointer->GetAlias() );
    strAliasPwd.SetTCHAR( fProjectSettingsPointer->GetAliasPassword() );
    strSaveDir.SetTCHAR( fProjectSettingsPointer->GetSaveDir() );
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
	bool includeFusePlugins = AfxGetApp()->GetProfileInt(REGISTRY_SECTION, REGISTRY_DEBUG_MONETIZATION_PLUGIN,
		REGISTRY_DEBUG_MONETIZATION_PLUGIN_DEFAULT) ? true : false;
#endif


	// Build the project. (This is a blocking call.)
	fBuildResult = appAndroidBuild(
							fRuntimeEnvironment, strSrcDir.GetUTF8(),
							strName.GetUTF8(), strVersion.GetUTF8(), strPackage.GetUTF8(),
							strKeystore.GetUTF8(), strKeystorePwd.GetUTF8(),
							strAlias.GetUTF8(), strAliasPwd.GetUTF8(), strSaveDir.GetUTF8(),
							fTargetedAppStorePointer->GetPlatform(),
							fTargetedAppStorePointer->GetStringId(),
							fProjectSettingsPointer->IsDistribution(),
							fProjectSettingsPointer->GetAndroidVersionCode(),
							fProjectSettingsPointer->GetCreateLiveBuild()
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
							, includeFusePlugins,
							fProjectSettingsPointer->GetEnableMonetization()
#endif
							);

	PostMessage(WMU_BUILD_COMPLETE, 0, 0);
}

void CBuildProgressDlg::BuildForWeb()
{
	// Do not continue if project settings were not provided.
	if (!fProjectSettingsPointer)
	{
		fBuildResult = CBuildResult(5, CString((LPCSTR)IDS_BUILD_FAILED));
		CDialog::OnOK();
		return;
	}

	// Display a "wait" mouse cursor.
	CWaitCursor waitCursor;

	// Get copy of string settings.
	WinString strSrcDir, strName, strVersion, strPackage;
	WinString strKeystore, strKeystorePwd, strAlias, strAliasPwd, strSaveDir, strTargetOS;

	strSrcDir.SetTCHAR( fProjectSettingsPointer->GetDir() );
	strName.SetTCHAR( fProjectSettingsPointer->GetName() );
	strVersion.SetTCHAR( fProjectSettingsPointer->GetAndroidVersionName() );
	//strPackage.SetTCHAR( fProjectSettingsPointer->GetPackage() );
	//strKeystore.SetTCHAR( fProjectSettingsPointer->GetKeystorePath() );
	//strKeystorePwd.SetTCHAR( fProjectSettingsPointer->GetKeystorePassword() );
	//strAlias.SetTCHAR( fProjectSettingsPointer->GetAlias() );
	//strAliasPwd.SetTCHAR( fProjectSettingsPointer->GetAliasPassword() );
	strSaveDir.SetTCHAR( fProjectSettingsPointer->GetSaveDir() );
	//strTargetOS.SetTCHAR( fProjectSettingsPointer->GetTargetOS() );
	CFrameWnd *pFrameWnd = (CFrameWnd *) AfxGetApp()->GetMainWnd(); 
	CSimulatorView *pView = (CSimulatorView *)pFrameWnd->GetActiveView();

	// Build the project. (This is a blocking call.)
	fBuildResult = appWebBuild(
		pView->GetRuntimeEnvironment(), strSrcDir.GetUTF8(),
		strName.GetUTF8(), strVersion.GetUTF8(),
		strSaveDir.GetUTF8(),
		Rtt::TargetDevice::kWebPlatform,
		strTargetOS.GetUTF8(),
		fProjectSettingsPointer->IsDistribution(),
		fProjectSettingsPointer->GetAndroidVersionCode(),
		fProjectSettingsPointer->GetUseStandartResources(),
		fProjectSettingsPointer->GetCreateFBInstantArchive()
	 );

	PostMessage(WMU_BUILD_COMPLETE, 0, 0);
}

void CBuildProgressDlg::BuildForLinux()
{
	// Do not continue if project settings were not provided.
	if (!fProjectSettingsPointer)
	{
		fBuildResult = CBuildResult(5, CString((LPCSTR)IDS_BUILD_FAILED));
		CDialog::OnOK();
		return;
	}

	// Display a "wait" mouse cursor.
	CWaitCursor waitCursor;

	// Get copy of string settings.
	WinString strSrcDir, strName, strVersion, strPackage;
	WinString strKeystore, strKeystorePwd, strAlias, strAliasPwd, strSaveDir, strTargetOS;

	strSrcDir.SetTCHAR( fProjectSettingsPointer->GetDir() );
	strName.SetTCHAR( fProjectSettingsPointer->GetName() );
	strVersion.SetTCHAR( fProjectSettingsPointer->GetAndroidVersionName() );
	strSaveDir.SetTCHAR( fProjectSettingsPointer->GetSaveDir() );
	CFrameWnd *pFrameWnd = (CFrameWnd *) AfxGetApp()->GetMainWnd(); 
	CSimulatorView *pView = (CSimulatorView *)pFrameWnd->GetActiveView();

	// Build the project. (This is a blocking call.)
	fBuildResult = appLinuxBuild(
		pView->GetRuntimeEnvironment(), strSrcDir.GetUTF8(),
		strName.GetUTF8(), strVersion.GetUTF8(),
		strSaveDir.GetUTF8(),
		Rtt::TargetDevice::kWebPlatform,
		strTargetOS.GetUTF8(),
		fProjectSettingsPointer->IsDistribution(),
		fProjectSettingsPointer->GetAndroidVersionCode(),
		fProjectSettingsPointer->GetUseStandartResources()
	 );

	PostMessage(WMU_BUILD_COMPLETE, 0, 0);
}

void CBuildProgressDlg::BuildForNxS()
{
	// Do not continue if project settings were not provided.
	if (!fProjectSettingsPointer)
	{
		fBuildResult = CBuildResult(5, CString((LPCSTR)IDS_BUILD_FAILED));
		CDialog::OnOK();
		return;
	}

	// Display a "wait" mouse cursor.
	CWaitCursor waitCursor;

	// Get copy of string settings.
	WinString strSrcDir, strName, strVersion, strPackage, strNmetaPath;
	WinString strKeystore, strKeystorePwd, strAlias, strAliasPwd, strSaveDir, strTargetOS;

	strSrcDir.SetTCHAR(fProjectSettingsPointer->GetDir());
	strName.SetTCHAR(fProjectSettingsPointer->GetName());
	strNmetaPath.SetTCHAR(fProjectSettingsPointer->GetDir() + L"\\build.nmeta");		// it's .nmeta path
	strSaveDir.SetTCHAR(fProjectSettingsPointer->GetSaveDir());
	CFrameWnd* pFrameWnd = (CFrameWnd*)AfxGetApp()->GetMainWnd();
	CSimulatorView* pView = (CSimulatorView*)pFrameWnd->GetActiveView();

	// Build the project. (This is a blocking call.)
	fBuildResult = appNxSBuild(
		pView->GetRuntimeEnvironment(), 
		strSrcDir.GetUTF8(),
		strNmetaPath.GetUTF8(),
		strName.GetUTF8(), 
		strVersion.GetUTF8(),
		strSaveDir.GetUTF8(),
		Rtt::TargetDevice::kNxSPlatform,
		strTargetOS.GetUTF8(),
		fProjectSettingsPointer->IsDistribution(),
		fProjectSettingsPointer->GetAndroidVersionCode(),
		fProjectSettingsPointer->GetUseStandartResources()
	);

	PostMessage(WMU_BUILD_COMPLETE, 0, 0);
}

#pragma endregion


void CBuildProgressDlg::OnBnClickedStopBuild()
{
	CSimulatorApp *pApp = (CSimulatorApp*)AfxGetApp();

	fStatusMessage.SetWindowText(L"Stopping build...");

	pApp->SetStopBuildRequested(true);
}

// OnDownloadProgressUpdate - 
// wParam = 0 for set, 1 for update
// lParam = total/current count
LRESULT CBuildProgressDlg::OnDownloadProgressUpdate(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0)
	{
		// Initial message with total expected download size
		fProgressBar.ModifyStyle(PBS_MARQUEE, 0);
		fProgressBar.SetMarquee(FALSE, 0);
		fProgressBar.SetRange32(0, lParam);
		fProgressBar.SetBkColor(RGB(192, 192, 192));

		fStatusMessage.SetWindowText(L"Downloading build from server...");

		//Rtt_LogException("OnDownloadProgressUpdate: content length %ld\n", lParam);
	}
	else
	{
		fProgressBar.SetPos(lParam);

		int lower = 0, upper = 0;
		fProgressBar.GetRange(lower, upper);
		if (lParam == upper)
		{
			// Download complete
			fProgressBar.ModifyStyle(0, PBS_MARQUEE);
			fProgressBar.SetMarquee(TRUE, 0);
			fProgressBar.SetBkColor(CLR_DEFAULT);

			fStatusMessage.SetWindowText(L"Finalizing build...");
		}

		//Rtt_LogException("OnDownloadProgressUpdate: received %ld\n", lParam);
	}

	return 0;
}

