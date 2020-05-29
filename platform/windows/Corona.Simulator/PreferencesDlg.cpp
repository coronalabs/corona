//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PreferencesDlg.h"
#include "CoronaInterface.h"
#include "Simulator.h"
#include "WinGlobalProperties.h"
#include "WinString.h"
#include "Core/Rtt_Build.h"
#include "Rtt_WinPlatformServices.h"

IMPLEMENT_DYNAMIC(CPreferencesDlg, CDialog)


#pragma region Constructor/Destructor
CPreferencesDlg::CPreferencesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPreferencesDlg::IDD, pParent)
{
}
CPreferencesDlg::~CPreferencesDlg()
{
}
#pragma endregion


#pragma region Message Mappings
/// Associates member variables with window's controls.
void CPreferencesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}
BEGIN_MESSAGE_MAP(CPreferencesDlg, CDialog)
	ON_WM_HELPINFO()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()

#pragma endregion


#pragma region Window Event Handlers
BOOL CPreferencesDlg::OnInitDialog()
{
	// Let the base class initialize the dialog first.
	CDialog::OnInitDialog();
	
	// Fetch application object.
    CSimulatorApp *pApp = (CSimulatorApp *)AfxGetApp();
		
	CButton *pShowRuntimeErrors = (CButton *)GetDlgItem( IDC_SHOW_RUNTIME_ERRORS );
	bool isShowingRuntimeErrors = true;
	if (isShowingRuntimeErrors)
	{
		isShowingRuntimeErrors = pApp->IsShowingRuntimeErrors();
	}
	pShowRuntimeErrors->SetCheck(isShowingRuntimeErrors ? BST_CHECKED : BST_UNCHECKED);

	CButton *pAutoOpenLastProject = (CButton *)GetDlgItem( IDC_AUTO_OPEN_LAST_PROJECT );
	pAutoOpenLastProject->SetCheck(pApp->ShouldAutoOpenLastProject() ? BST_CHECKED : BST_UNCHECKED);

	CButton *pRelaunchSimStyleAlways = (CButton *)GetDlgItem( IDC_SIM_RELAUNCH_ALWAYS );
	CButton *pRelaunchSimStyleNever = (CButton *)GetDlgItem( IDC_SIM_RELAUNCH_NEVER );
	CButton *pRelaunchSimStyleAsk = (CButton *)GetDlgItem( IDC_SIM_RELAUNCH_ASK );
	int relaunchStyle = pApp->GetRelaunchSimStyle();
	pRelaunchSimStyleAlways->SetCheck((relaunchStyle == RELAUNCH_SIM_ALWAYS) ? BST_CHECKED : BST_UNCHECKED);
	pRelaunchSimStyleNever->SetCheck((relaunchStyle == RELAUNCH_SIM_NEVER) ? BST_CHECKED : BST_UNCHECKED);
	pRelaunchSimStyleAsk->SetCheck((relaunchStyle == RELAUNCH_SIM_ASK) ? BST_CHECKED : BST_UNCHECKED);

	// Set up the welcome screen checkbox.
    CButton *pNoWelcome = (CButton *)GetDlgItem( IDC_NOWELCOME );
	bool bIsEnabled = pApp->IsHomeScreenEnabled();
	pNoWelcome->SetCheck( bIsEnabled ? BST_UNCHECKED : BST_CHECKED );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPreferencesDlg::OnHelpInfo(HELPINFO* helpInfoPointer)
{
	// Display help information related to this dialog.
	// Note: If we don't handle this here, then SimulatorView::OnHelp() will get called which displays
	//       the main Corona documentation page on Corona Labs' website.
	OnSysCommand(SC_CONTEXTHELP, 0);
	return TRUE;
}

void CPreferencesDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// Handle the received dialog system command.
	if (SC_CONTEXTHELP == nID)
	{
		// The help [?] button or F1 key was pressed.
		// Display documentation about simulator preferences on Corona Labs' website.
		try
		{
			const wchar_t kUrl[] =
					L"https://docs.coronalabs.com/daily/guide/start/installWin/index.html#corona-simulator-preferences";
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

void CPreferencesDlg::OnOK()
{
    CSimulatorApp *pApp = (CSimulatorApp *)AfxGetApp();

#if 0
    CButton *pAnalytics = (CButton *)GetDlgItem( IDC_ANALYTICS );
    bool bAnalytics = (pAnalytics->GetCheck() == BST_CHECKED);
    pApp->WriteProfileInt( REGISTRY_SECTION, REGISTRY_ANALYTICS, (int)bAnalytics );
#endif

    CButton *pNoWelcome = (CButton *)GetDlgItem( IDC_NOWELCOME );
    bool bNoWelcome = (pNoWelcome->GetCheck() == BST_CHECKED);
	pApp->EnableHomeScreen(!bNoWelcome);

	CButton *pShowRunTimeErrors = (CButton *)GetDlgItem( IDC_SHOW_RUNTIME_ERRORS );
    bool bShowErrors = (pShowRunTimeErrors->GetCheck() == BST_CHECKED);
	pApp->ShowRuntimeErrors(bShowErrors);
	
	CButton *pAutoOpenLastProject = (CButton *)GetDlgItem( IDC_AUTO_OPEN_LAST_PROJECT );
    bool bAutoOpenLastProject = (pAutoOpenLastProject->GetCheck() == BST_CHECKED);
	pApp->AutoOpenLastProject(bAutoOpenLastProject);

	CButton *pRelaunchSimStyleAlways = (CButton *)GetDlgItem( IDC_SIM_RELAUNCH_ALWAYS );
	CButton *pRelaunchSimStyleNever = (CButton *)GetDlgItem( IDC_SIM_RELAUNCH_NEVER );
	CButton *pRelaunchSimStyleAsk = (CButton *)GetDlgItem( IDC_SIM_RELAUNCH_ASK );
	if (pRelaunchSimStyleAlways->GetCheck() == BST_CHECKED)
		pApp->SetRelaunchSimStyle(RELAUNCH_SIM_ALWAYS);
	else if (pRelaunchSimStyleNever->GetCheck() == BST_CHECKED)
		pApp->SetRelaunchSimStyle(RELAUNCH_SIM_NEVER);
	else
		pApp->SetRelaunchSimStyle(RELAUNCH_SIM_ASK);

	CDialog::OnOK();
}

#pragma endregion
