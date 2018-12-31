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

// CMessageDlg dialog

class CMessageDlg : public CDialog
{
	DECLARE_DYNAMIC(CMessageDlg)

public:
	CMessageDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMessageDlg();

    void SetTitle( int nID );
	void SetTitle( CString sTitle ) { m_sTitle = sTitle; }
    void SetText( int nID );
	void SetText( CString sText ) { m_sText = sText; }
    void SetDefaultText( int nID );
	void SetDefaultText( CString sDefText ) { m_sButtonDefault = sDefText; }
    void SetAltText( int nID );
	void SetAltText( CString sAltText ) { m_sButtonAlt = sAltText; }
    void SetButton3Text( int nID );
	void SetButton3Text( CString sButton3 ) { m_sButton3 = sButton3; }
	bool SetUrl( const char *sUrl );
	bool SetUrl( const TCHAR *sUrl );
	bool SetFolder( CString sFolder );
	void SetIconStyle( int style ) { m_nIconStyle = style; }

	void SetNativeAlertInfo( void *pLuaResource );
	bool IsNativeAlert();
	void EndNativeAlert( int idButton );


// Dialog Data
	enum { IDD = IDD_MESSAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    CString m_sTitle;
    CString m_sText;
    CString m_sButtonDefault;
    CString m_sButtonAlt;
    CString m_sButton3;
    CString m_sUrl;
    CString m_sFolder;
    int m_nIconStyle;
    void *m_pLuaResource;

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
	virtual void OnCancel();
public:
	afx_msg void OnButton2();
	afx_msg void OnButton3();
};
