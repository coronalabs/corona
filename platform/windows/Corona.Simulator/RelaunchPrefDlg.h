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

// CRelaunchPrefDlg dialog

class CRelaunchPrefDlg : public CDialog
{
	DECLARE_DYNAMIC(CRelaunchPrefDlg)

public:
	CRelaunchPrefDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRelaunchPrefDlg();

// Dialog Data
	enum { IDD = IDD_RELAUNCH_PREF };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRememberPref();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
