//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "afxwin.h"
#include "resource.h"


//////////////////////////////////////////////////////////////////////////////////////////////////
// CPasswordDlg dialog
//////////////////////////////////////////////////////////////////////////////////////////////////

class CPasswordDlg : public CDialog
{
	DECLARE_DYNAMIC(CPasswordDlg)

public:
	CPasswordDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPasswordDlg();

	CString GetPassword() { return m_sPassword; }    
	CString GetKey() { return m_sKey; }
	void SetKey( CString sKey )    { m_sKey = sKey; }
	bool GetSavePassword() { return m_bSavePassword; }
	void SetSavePassword( bool bSave ) { m_bSavePassword = bSave; }

	void ShowSaveBox( bool bShow ) { m_bShowSaveBox = bShow; }

// Dialog Data
	enum { IDD = IDD_PASSWORD };

protected:
    CString m_sPassword;
    CString m_sKey;
    bool m_bSavePassword;
    bool m_bShowSaveBox;

	DECLARE_MESSAGE_MAP()

	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
};
