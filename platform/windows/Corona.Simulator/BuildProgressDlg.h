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

#include "Resource.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "Rtt_TargetAndroidAppStore.h"
#include "BuildResult.h"
#include "CoronaProject.h"
#include <memory>


class CBuildProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CBuildProgressDlg)

public:
	CBuildProgressDlg(CWnd* pParent = NULL);
	virtual ~CBuildProgressDlg();

	void SetProjectSettings(const std::shared_ptr<CCoronaProject>& projectSettingsPointer);
	std::shared_ptr<CCoronaProject> GetProjectSettings();
	void SetTargetedAppStore(Rtt::TargetAndroidAppStore *targetedAppStorePointer);
	void SetNoServer() { fNoServerMsd = true; }
	Rtt::TargetAndroidAppStore* GetTargetedAppStore();
	CBuildResult GetBuildResult();
	void BuildForPlatform();

	// Dialog Data
	enum { IDD = IDD_BUILD_PROGRESS };

private:
	void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	void OnCancel();
	void BuildForAndroid();
	void BuildForWeb();
	void BuildForLinux();
	LRESULT OnBuildComplete(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDownloadProgressUpdate(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	CProgressCtrl fProgressBar;
	CStatic fStatusMessage;
	std::shared_ptr<CCoronaProject> fProjectSettingsPointer;
	Rtt::TargetAndroidAppStore *fTargetedAppStorePointer;
	CBuildResult fBuildResult;
	Interop::SimulatorRuntimeEnvironment* fRuntimeEnvironment;
	HANDLE  fBuilderThread;
	bool fNoServerMsd;
public:
	afx_msg void OnBnClickedStopBuild();
};
