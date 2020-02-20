//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
