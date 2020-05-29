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

// CDevPasswordDlg dialog

class CDevPasswordDlg : public CDialog
{
	DECLARE_DYNAMIC(CDevPasswordDlg)

public:
	CDevPasswordDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDevPasswordDlg();

// Dialog Data
	enum { IDD = IDD_DEVPASSWORD };

	CString GetPassword() { return m_sPassword; }

	void SetUser( CString sUser ) { m_sUser = sUser; }

public:
	afx_msg void OnChangeInfo();
	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    CString m_sUser;
	CString m_sPassword;

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
};
