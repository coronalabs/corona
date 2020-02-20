//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PasswordDlg.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
// CPasswordDlg dialog
//////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CPasswordDlg, CDialog)

// Constructor - initialize all members
CPasswordDlg::CPasswordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPasswordDlg::IDD, pParent),
      m_sPassword( _T("") ),
      m_sKey( _T("") ),
      m_bSavePassword( false ),
      m_bShowSaveBox( true )
{

}

// Destructor - nothing to clean up
CPasswordDlg::~CPasswordDlg()
{
}

BEGIN_MESSAGE_MAP(CPasswordDlg, CDialog)
END_MESSAGE_MAP()


// CPasswordDlg message handlers

// OnInitDialog - set "key" text and save password checkbox (set before calling DoModal)
BOOL CPasswordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    SetDlgItemText( IDC_KEY, m_sKey );
    SetWindowText( m_sKey );
    
    // Set Save Password checkbox - defaults to unchecked
	CButton *pSavePass = (CButton *)GetDlgItem( IDC_SAVEPASSWORD );
	pSavePass->SetCheck( m_bSavePassword ? BST_CHECKED : BST_UNCHECKED );

    // Show SavePassword checkbox? - defaults to show
	pSavePass->ShowWindow( m_bShowSaveBox ? SW_SHOW : SW_HIDE );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// OnOK - save current values of password and save checkbox
void CPasswordDlg::OnOK()
{
	GetDlgItemText( IDC_PASSWORD, m_sPassword );

	CButton *pSavePass = (CButton *)GetDlgItem( IDC_SAVEPASSWORD );
	m_bSavePassword = (pSavePass->GetCheck() == BST_CHECKED);
 
	CDialog::OnOK();
}

