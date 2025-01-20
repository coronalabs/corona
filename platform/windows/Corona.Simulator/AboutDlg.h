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


class CAboutDlg : public CDialog
{
//	DECLARE_DYNAMIC(CPreferencesDlg)

public:
	CAboutDlg(CWnd* pParent = NULL);
	~CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClickLinkWebsite(NMHDR *pNMHDR, LRESULT *pResult);
};
