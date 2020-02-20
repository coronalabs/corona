//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
