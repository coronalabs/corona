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

//////////////////////////////////////////////////////////////////////////////////////////////////
// CProgressWnd modeless dialog
//////////////////////////////////////////////////////////////////////////////////////////////////

class CProgressWnd : public CDialog
{
	DECLARE_DYNAMIC(CProgressWnd)

public:
	CProgressWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProgressWnd();

// Dialog Data
	enum { IDD = IDD_PROGRESSMSG };

public:
	virtual BOOL OnInitDialog();

	void SetText( CString sText ) { m_sText = sText; }
	void SetText( int nID );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();

protected:
    CString m_sText;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
// CAutoProgressWnd
//////////////////////////////////////////////////////////////////////////////////////////////////

class CAutoProgressWnd
{
public:
	CAutoProgressWnd();
	CAutoProgressWnd( CWnd *pParent );
	~CAutoProgressWnd();
	void Close();
};