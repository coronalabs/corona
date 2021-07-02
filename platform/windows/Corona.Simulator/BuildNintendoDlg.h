//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
#define REGISTRY_BUILD_NINTENDO				_T("BuildNintendo")

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

// BuildNintendoDlg dialog

class CBuildNintendoDlg : public CDialog
{
	DECLARE_DYNAMIC(CBuildNintendoDlg)

public:

	// Dialog Data
	enum { IDD = IDD_BUILD_NINTENDO };

	CBuildNintendoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBuildNintendoDlg();
	void SetProject(const std::shared_ptr<CCoronaProject>& pProject);
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseSaveto();

protected:
	std::shared_ptr<CCoronaProject> m_pProject;  // stored info about project being built
	int m_nValidFields;
	CString fNmetaPath;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	virtual void OnCancel();
	void OnClickedIncludeStandartResources();
	void DisplayWarningMessage(UINT nMessageID);
	void LogAnalytics(const char* eventName, const char* key = NULL, const char* value = NULL);
};
