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

// CLicenseDlg dialog

class CLicenseDlg : public CDialog
{
	DECLARE_DYNAMIC(CLicenseDlg)

public:
	CLicenseDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLicenseDlg();

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_LICENSE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

	static DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
};
