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
#define REGISTRY_BUILD_ANDROID			_T("BuildAndroid")

// registry keys for whole dialog
#define REGISTRY_TARGET_APP_STORE		_T("TargetAppStore")
#define REGISTRY_TARGETOS				_T("TargetOS")
#define REGISTRY_KEYSTOREDIR			_T("KeystoreDir")
#define REGISTRY_LASTKEYSTORE			_T("LastKeystore")
#define REGISTRY_LASTKEYSTOREPWD		_T("Marker")
#define REGISTRY_LASTBUILDDIR			_T("LastBuildDir")

// bitmasks to track whether Build (IDOK) should be enabled or not
#define VALID_MASK      0x3F
#define MASK_APPNAME    0x01
#define MASK_VERSION    0x02
#define MASK_PACKAGE    0x04
#define MASK_KEYSTORE   0x08
#define MASK_ALIASLIST  0x10
#define MASK_SAVETO     0x20

// BuildAndroidDlg dialog

class CBuildAndroidDlg : public CDialog
{
	DECLARE_DYNAMIC(CBuildAndroidDlg)

public:
	CBuildAndroidDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBuildAndroidDlg();

// Dialog Data
	enum { IDD = IDD_BUILD_DROID };

public:
	void SetProject(const std::shared_ptr<CCoronaProject>& pProject);

protected:
    std::shared_ptr<CCoronaProject> m_pProject;  // stored info about project being built
    CString m_sPrevKeystorePath;
    CString m_sPrevKeystoreAlias;
    int m_nValidFields;
    CString m_sKeystoreDir;
	lua_State* m_androidValidatorLuaState;
	static CString m_sHelpURL;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
protected:
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	virtual void OnOK();
	virtual void OnCancel();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseKeystore();
	afx_msg void OnBrowseSaveto();
	afx_msg void OnKillFocusKeystorePath();
	afx_msg void OnChangeAliasList();
	afx_msg void OnSetFocusAliasList();
	afx_msg BOOL OnHelpInfo(HELPINFO* helpInfoPointer);

protected:
	bool ReadKeystore( CString sKeystorePath, CString sPassword, bool bShowErrors = true );
    void UpdateAliasList();
	CString GetTrialKeystorePassword();
	CString GetTrialKeyAliasPassword();
	void DisplayWarningMessage(UINT nMessageID);
	void DisplayWarningMessage(UINT nMessageID, CString filename); // override with additional info
	UINT DisplayWarningMessageWithHelp(UINT nTitleID, UINT nMessageID, CString helpURL, bool isYesNo = true);
	static void CALLBACK CBuildAndroidDlg::HelpCallback(LPHELPINFO lpHelpInfo);
	afx_msg void OnBnClickedCreateLiveBuild();
	void LogAnalytics(const char *eventName, const char *key = NULL, const char *value = NULL);

};
