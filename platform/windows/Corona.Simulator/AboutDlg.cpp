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
#include "AboutDlg.h"
#include "WinString.h"
#include "Core/Rtt_Build.h"


//IMPLEMENT_DYNAMIC(CAboutDlg, CDialog)


#pragma region Constructor/Destructor
CAboutDlg::CAboutDlg(CWnd* pParent /*=NULL*/) : CDialog(CAboutDlg::IDD, pParent)
{
}

CAboutDlg::~CAboutDlg()
{
}

#pragma endregion


#pragma region Message Mappings
/// Associates member variables with window's controls.
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_NOTIFY(NM_CLICK, IDC_LINK_ANSCAMOBILE, &CAboutDlg::OnClickLinkWebsite)
	ON_NOTIFY(NM_RETURN, IDC_LINK_ANSCAMOBILE, &CAboutDlg::OnClickLinkWebsite)
END_MESSAGE_MAP()

#pragma endregion


#pragma region Window Event Handlers
BOOL CAboutDlg::OnInitDialog()
{
	// Base class initialization must come first.
	CDialog::OnInitDialog();

	// Set up SDK version label.
	WinString buildVersion;
	WinString buildDate;
	CString fullVersionString;
	buildVersion.SetUTF8( Rtt_STRING_BUILD );
	buildDate.SetUTF8( Rtt_STRING_BUILD_DATE );
	fullVersionString.Format( _T("%s (%s)"), buildVersion.GetTCHAR(), buildDate.GetTCHAR() );
	GetDlgItem(IDC_ABOUT_VERSION)->SetWindowText(fullVersionString);

	// Set up copyright label.
	TCHAR appFileName[512];
	DWORD length = ::GetModuleFileName(NULL, appFileName, sizeof(appFileName));
	if (length > 0)
	{
		DWORD handle;
		length = ::GetFileVersionInfoSize(appFileName, &handle);
		if (length > 0)
		{
			BYTE *versionDataPointer = new BYTE[length];
			if (::GetFileVersionInfo(appFileName, handle, length, (LPVOID)versionDataPointer))
			{
				WORD *languagePointer = NULL;
				LPVOID valuePointer = NULL;
				UINT valueLength;
				if (::VerQueryValue(versionDataPointer, _T("\\VarFileInfo\\Translation"), (LPVOID*)&languagePointer, &valueLength))
				{
					CString copyrightPath;
					copyrightPath.Format(
							_T("\\StringFileInfo\\%04x%04x\\LegalCopyright"),
							languagePointer[0], languagePointer[1]);
					if (::VerQueryValue(versionDataPointer, copyrightPath, &valuePointer, &valueLength))
					{
						GetDlgItem(IDC_COPYRIGHT)->SetWindowText((LPTSTR)valuePointer);
					}
				}
			}
			delete versionDataPointer;
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAboutDlg::OnClickLinkWebsite(NMHDR *pNMHDR, LRESULT *pResult)
{
	try
	{
		PNMLINK pNMLink = (PNMLINK)pNMHDR;
		::ShellExecute(NULL, _T("open"), pNMLink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
	}
	catch (...) { }
	*pResult = 0;
}

#pragma endregion
