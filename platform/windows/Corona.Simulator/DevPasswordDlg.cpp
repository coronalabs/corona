//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DevPasswordDlg.h"

// CDevPasswordDlg dialog

IMPLEMENT_DYNAMIC(CDevPasswordDlg, CDialog)

CDevPasswordDlg::CDevPasswordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDevPasswordDlg::IDD, pParent),
      m_sUser( "" ),
      m_sPassword( "" )
{
}

CDevPasswordDlg::~CDevPasswordDlg()
{
}

void CDevPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CDevPasswordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Login/OK btn starts disabled
    GetDlgItem( IDOK )->EnableWindow( FALSE );    

    // Set (non-editable) username
	CStatic *pUser = (CStatic *)GetDlgItem( IDC_LOGIN_EMAIL );
	pUser->SetWindowTextW( m_sUser );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CDevPasswordDlg, CDialog)
	ON_EN_CHANGE(IDC_LOGIN_PASSWORD, &CDevPasswordDlg::OnChangeInfo)
END_MESSAGE_MAP()


// CDevPasswordDlg message handlers
void CDevPasswordDlg::OnChangeInfo()
{
    CString sPassword;
    GetDlgItemText( IDC_LOGIN_PASSWORD, sPassword );

    GetDlgItem( IDOK )->EnableWindow( !sPassword.IsEmpty() );    
}

// Just remember password, don't do authorization checks
void CDevPasswordDlg::OnOK()  // OnLogin()
{
    // Retrieve password value
    GetDlgItemText( IDC_LOGIN_PASSWORD, m_sPassword );

	CDialog::OnOK();
}
