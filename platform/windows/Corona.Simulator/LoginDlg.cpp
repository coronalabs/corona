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
#include "resource.h"

#include "CoronaInterface.h"
#include "WinString.h"
#include "Rtt_WinAuthorizationDelegate.h"

#include "ProgressWnd.h"
#include "LoginDlg.h"


// CLoginDlg dialog

IMPLEMENT_DYNAMIC(CLoginDlg, CDialog)

CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDlg::IDD, pParent),
      m_sUser( "" ),
      m_sPassword( "" ),
      m_pAuthDelegate( NULL ),
      m_pAuth( NULL )
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

void CLoginDlg::SetAuthorizer( Rtt::WinAuthorizationDelegate *pDel, const Rtt::Authorization *pAuth )
{
    ASSERT( pDel );
    ASSERT( pAuth );

    m_pAuthDelegate = pDel;
    m_pAuth = pAuth;
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

	// Log in the user. This displays a progress window and a message box indicating if it succeeded or failed.
	if (m_pAuthDelegate->Login((Rtt::Authorization*)m_pAuth))
	{
		// Log in was successful. Close this dialog.
		CDialog::OnOK();
	}
}

void CLoginDlg::OnBnClickedLoginRegister()
{
	const TCHAR kUrlRegister[] = _T("https://coronalabs.com/links/simulator/register");

	::ShellExecute( NULL, _T("open"), kUrlRegister, NULL, NULL, SW_SHOWNORMAL );
}
