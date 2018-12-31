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

#include "CoronaProject.h"
#include <memory>
extern "C"
{
	struct lua_State;
}

// registry section for whole dialog
#define REGISTRY_BUILD_WEB				_T("BuildWeb")

// registry keys for whole dialog
#define REGISTRY_TARGETOS				_T("TargetOS")
#define REGISTRY_LASTKEYSTOREPWD		_T("Marker")
#define REGISTRY_LASTBUILDDIR			_T("LastBuildDir")

// bitmasks to track whether Build (IDOK) should be enabled or not
#define VALID_MASK      0x3F
#define MASK_APPNAME    0x01
#define MASK_VERSION    0x02
#define MASK_PACKAGE    0x04
#define MASK_SAVETO     0x20

// BuildWebDlg dialog

class CBuildWebDlg : public CDialog
{
	DECLARE_DYNAMIC(CBuildWebDlg)

public:
	CBuildWebDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBuildWebDlg();

// Dialog Data
	enum { IDD = IDD_BUILD_WEB };

public:
	void SetProject( const std::shared_ptr<CCoronaProject>& pProject );

protected:
    std::shared_ptr<CCoronaProject> m_pProject;  // stored info about project being built
    int m_nValidFields;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
protected:
	virtual void OnOK();
	virtual void OnCancel();
	void OnClickedCreateFBInstantArchive();
	void OnClickedIncludeStandartResources();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseSaveto();

protected:
	void DisplayWarningMessage(UINT nMessageID);
	void LogAnalytics(const char *eventName, const char *key = NULL, const char *value = NULL);
};
