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
#include "afxwin.h"
#include "SystemTray.h"
#define WM_ICON_NOTIFY (WM_APP+32)

// CCoronaLiveServerDlg dialog
class CCoronaLiveServerDlg : public CDialog
{
// Construction
public:
	CCoronaLiveServerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CORONALIVESERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIconBig, m_hIconSmall;
	RECT m_rcDlg, m_rcProj, m_rcAdd, m_rcRemove;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	CListBox m_ProjectList;
	CButton m_AddButton;
	bool m_openURLButtonAdded;
public:
	CButton m_RemoveButton;
	void ReEnableControls();
	afx_msg void OnBnClickedRemove();
	afx_msg void OnBnClickedAdd();
	bool AddProject(CString str);
	void SetHTML5Directory(CString str);
	void BrowseForHTML5Dir();
	void OpenHTML5URL();
	afx_msg void OnLbnSelchangeProjectList();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);
	CSystemTray m_trayIcon;
	afx_msg void OnPopupExit();
	afx_msg void OnPopupShow();
	virtual void OnCancel();
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	void ParseCommandLine(CString str);
private:
	BOOL ShowHelp();
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnLbnDblclkProjectList();
};
