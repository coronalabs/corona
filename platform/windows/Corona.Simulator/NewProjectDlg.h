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

#pragma once

#include "Resource.h"
#include "afxcmn.h"
#include "afxwin.h"

/// Dialog used to create a new Corona project.
class CNewProjectDlg : public CDialog
{
	DECLARE_DYNAMIC(CNewProjectDlg)

public:
	CNewProjectDlg(CWnd* pParent = NULL);
	virtual ~CNewProjectDlg();
	CString GetNewProjectPath();

	// Dialog Data
	enum { IDD = IDD_NEW_PROJECT };

private:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	void OnOK();
	void UpdateProjectLocationField();
	void DisplayWarningMessage(UINT messageId);
	void DisplayWarningMessage(CString &message);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnChangeAppNameEditBox();
	afx_msg void OnScreenSizeComboBoxSelectionChanged();
	afx_msg void OnClickedBrowseButton();
	afx_msg void OnClickedTemplateEbook();

	CEdit fAppNameEditBox;
	CEdit fProjectLocationEditBox;
	CComboBox fScreenSizeComboBox;
	CEdit fScreenWidthEditBox;
	CEdit fScreenHeightEditBox;
	CSize fPhoneScreenSize;
	CSize fTabletScreenSize;
	CFont fBoldRadioButtonFont;
	CString fNewProjectPath;
};
