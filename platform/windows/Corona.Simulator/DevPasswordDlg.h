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
