//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

// CLoginDlg dialog

class CLoginDlg : public CDialog
{
	DECLARE_DYNAMIC(CLoginDlg)

public:
	CLoginDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLoginDlg();

// Dialog Data
	enum { IDD = IDD_LOGIN };

	CString GetUser() { return m_sUser; }
	CString GetPassword() { return m_sPassword; }

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeInfo();
	afx_msg BOOL OnHelpInfo(HELPINFO* helpInfoPointer);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    CString m_sUser;
	CString m_sPassword;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	virtual void OnOK();
public:
	afx_msg void OnBnClickedLoginRegister();
};
