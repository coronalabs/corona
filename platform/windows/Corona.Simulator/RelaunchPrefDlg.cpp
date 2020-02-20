//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RelaunchPrefDlg.h"
#include "Simulator.h"


// CRelaunchPrefDlg dialog

IMPLEMENT_DYNAMIC(CRelaunchPrefDlg, CDialog)

CRelaunchPrefDlg::CRelaunchPrefDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRelaunchPrefDlg::IDD, pParent)
{

}

CRelaunchPrefDlg::~CRelaunchPrefDlg()
{
}

void CRelaunchPrefDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRelaunchPrefDlg, CDialog)
	ON_BN_CLICKED(IDC_REMEMBER_PREF, &CRelaunchPrefDlg::OnBnClickedRememberPref)
	ON_BN_CLICKED(IDOK, &CRelaunchPrefDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CRelaunchPrefDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CRelaunchPrefDlg message handlers

void CRelaunchPrefDlg::OnBnClickedRememberPref()
{
	// TODO: Add your control notification handler code here
}

void CRelaunchPrefDlg::OnBnClickedOk()
{
	CSimulatorApp *pApp = (CSimulatorApp*)AfxGetApp();
	CButton *pRememberPref = (CButton *)GetDlgItem( IDC_REMEMBER_PREF );

	if (pRememberPref->GetCheck() == BST_CHECKED)
	{
		pApp->SetRelaunchSimStyle(RELAUNCH_SIM_ALWAYS);
	}

	OnOK();
}

void CRelaunchPrefDlg::OnBnClickedCancel()
{
	CSimulatorApp *pApp = (CSimulatorApp*)AfxGetApp();
	CButton *pRememberPref = (CButton *)GetDlgItem( IDC_REMEMBER_PREF );

	if (pRememberPref->GetCheck() == BST_CHECKED)
	{
		pApp->SetRelaunchSimStyle(RELAUNCH_SIM_NEVER);
	}

	OnCancel();
}
