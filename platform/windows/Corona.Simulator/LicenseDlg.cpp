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
#include "Simulator.h"
#include "LicenseDlg.h"


// CLicenseDlg dialog

IMPLEMENT_DYNAMIC(CLicenseDlg, CDialog)

CLicenseDlg::CLicenseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLicenseDlg::IDD, pParent)
{

}

CLicenseDlg::~CLicenseDlg()
{
}

void CLicenseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLicenseDlg, CDialog)
END_MESSAGE_MAP()


/// Initializes this dialog's controls.
BOOL CLicenseDlg::OnInitDialog()
{
	// Base class initialization must come first.
	CDialog::OnInitDialog();

	// Display the contents of the license agreement file on the dialog.
    CRichEditCtrl *pLicense = (CRichEditCtrl*)GetDlgItem(IDC_LICENSE);
	try
	{
		CString fileName;
		fileName = ((CSimulatorApp*)::AfxGetApp())->GetApplicationDir();
		fileName += _T("\\Corona_License.rtf");
		CFile file(fileName, CFile::modeRead);
		EDITSTREAM editStreamInfo;
		editStreamInfo.dwCookie = (DWORD_PTR)&file;
		editStreamInfo.pfnCallback = EditStreamCallback;
		pLicense->LimitText(LONG_MAX);
		pLicense->StreamIn(SF_RTF, editStreamInfo);
	}
	catch (...)
	{
		pLicense->SetWindowText(_T("License file not found!"));
		GetDlgItem(IDOK)->EnableWindow(FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/// Called by CRichEditCtrl::StreamIn() function to stream in the contents of a file to the control.
/// @param dwCookie Pointer to an open CFile object to stream in contents of license agreement file.
/// @param pbBuff Buffer to copy file's bytes to. These are displayed in the rich edit control.
/// @param cb The number of bytes allocated for argument "pbBuff".
/// @param pcb Set to the number of bytes written to "pbBuff" by this function.
///            This function will not be called again if "pcb" is less than "cb".
/// @return Returns zero on a successful stream in. Returns non-zero upon error.
DWORD CLicenseDlg::EditStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	// Validate.
	if (NULL == dwCookie)
	{
		return 1;
	}

	// Copy contents of file to the rich text control.
	CFile *pFile = (CFile*)dwCookie;
	*pcb = pFile->Read(pbBuff, cb);
	return 0;
}
