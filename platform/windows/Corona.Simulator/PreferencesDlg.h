//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "resource.h"


class CPreferencesDlg : public CDialog
{
	DECLARE_DYNAMIC(CPreferencesDlg)

public:
	CPreferencesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPreferencesDlg();

// Dialog Data
	enum { IDD = IDD_PREFERENCES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	virtual void OnOK();

public:
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelpInfo(HELPINFO* helpInfoPointer);
};
