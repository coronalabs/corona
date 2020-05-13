//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "CoronaInterface.h"
#include "WinString.h"
#include "ProgressWnd.h"
#include "LoginDlg.h"


// CLoginDlg dialog

IMPLEMENT_DYNAMIC(CLoginDlg, CDialog)

CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDlg::IDD, pParent),
      m_sUser( "" ),
      m_sPassword( "" )
{
}

CLoginDlg::~CLoginDlg()
{
    // not allocating AuthDelegate, Auth, so don't delete them
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CLoginDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set this dialog's icon to use the Corona Simulator app's icon.
	SetIcon(AfxGetApp()->LoadIconW(IDR_MAINFRAME), TRUE);

	// Login/OK btn starts disabled
    GetDlgItem( IDOK )->EnableWindow( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CLoginDlg, CDialog)
	ON_EN_CHANGE(IDC_LOGIN_EMAIL, &CLoginDlg::OnChangeInfo)
	ON_EN_CHANGE(IDC_LOGIN_PASSWORD, &CLoginDlg::OnChangeInfo)
	ON_BN_CLICKED(IDC_LOGIN_REGISTER, &CLoginDlg::OnBnClickedLoginRegister)
	ON_WM_HELPINFO()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()

// CLoginDlg message handlers
void CLoginDlg::OnChangeInfo()
{
    CString sUser, sPassword;
	GetDlgItemText( IDC_LOGIN_EMAIL, sUser );
    GetDlgItemText( IDC_LOGIN_PASSWORD, sPassword );

    GetDlgItem( IDOK )->EnableWindow( ! sUser.IsEmpty() && !sPassword.IsEmpty() );    
}

BOOL CLoginDlg::OnHelpInfo(HELPINFO* helpInfoPointer)
{
	// Display help information related to this dialog.
	// Note: If we don't handle this here, then SimulatorView::OnHelp() will get called which displays
	//       the main Corona documentation page on Corona Labs' website.
	OnSysCommand(SC_CONTEXTHELP, 0);
	return TRUE;
}

void CLoginDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// Handle the received dialog system command.
	if (SC_CONTEXTHELP == nID)
	{
		// The help [?] button or F1 key was pressed.
		// Display the Windows installation/activation documentation on Corona Labs' website.
		try
		{
			const wchar_t kUrl[] =
					L"https://docs.coronalabs.com/daily/guide/start/installWin/index.html#activating-the-sdk";
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

// OnOK - can only exit dialog via OK if login info is valid (can cancel instead)
void CLoginDlg::OnOK()  // OnLogin()
{
    // Retrieve values
	GetDlgItemText( IDC_LOGIN_EMAIL, m_sUser );
    GetDlgItemText( IDC_LOGIN_PASSWORD, m_sPassword );

	CDialog::OnOK();
}

void CLoginDlg::OnBnClickedLoginRegister()
{
	const TCHAR kUrlRegister[] = _T("https://coronalabs.com/links/simulator/register");

	::ShellExecute( NULL, _T("open"), kUrlRegister, NULL, NULL, SW_SHOWNORMAL );
}
