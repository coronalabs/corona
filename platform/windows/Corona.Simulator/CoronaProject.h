//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_TargetDevice.h"


// CCoronaProject - store info for CBuildAndroidDlg
// Also CEncryptedKeeper - manage encrypted registry entries

// Registry entry names.
#define REGISTRY_NAME _T("Name")
#define REGISTRY_ANDROID_VERSION_CODE _T("Version")
#define REGISTRY_ANDROID_VERSION_NAME _T("VersionName")
#define REGISTRY_WIN32_VERSION_STRING _T("Win32VersionString")
#define REGISTRY_PACKAGE _T("Package")
#define REGISTRY_KEYSTOREPATH _T("Path")
#define REGISTRY_ALIAS _T("Alias")
#define REGISTRY_SAVEDIR _T("SaveDir")
#define REGISTRY_BUILD_DIR _T("BuildDir")
#define REGISTRY_KEYSTOREPWD _T("Marker")
#define REGISTRY_ALIASPWD _T("Marker2")
#define REGISTRY_CREATE_LIVE_BUILD _T("CreateLiveBuild")
#define REGISTRY_ENABLE_MONETIZATION _T("EnableMonetization")
#define REGISTRY_COPYRIGHT _T("Copyright")
#define REGISTRY_COMPANY_NAME _T("CompanyName")
#define REGISTRY_APP_DESCRIPTION _T("AppDescription")
#define REGISTRY_EXE_FILE_NAME _T("ExeFileName")
#define REGISTRY_WEB_USER_STANDARD_RESOURCES _T("html5IncludeResources")
#define REGISTRY_WEB_USER_CREATE_FB_INSTANT_ARCHIVE _T("html5CreateFBInstantArchive")

// Registry defaults for each individual project.
#define REGISTRY_NAME_DEFAULT _T("")
#define REGISTRY_ANDROID_VERSION_CODE_DEFAULT _T("1")
#define REGISTRY_VERSION_STRING_DEFAULT _T("1.0.0")
#define REGISTRY_PACKAGE_DEFAULT _T("")
#define REGISTRY_KEYSTOREPATH_DEFAULT _T("")
#define REGISTRY_ALIAS_DEFAULT _T("")
#define REGISTRY_SAVEDIR_DEFAULT _T("")
#define REGISTRY_CREATE_LIVE_BUILD_DEFAULT _T("0")
#define REGISTRY_AFTER_BUILD _T("0")
#define REGISTRY_ENABLE_MONETIZATION_DEFAULT _T("1")

///////////////////////////////////////////////////////////////////////////////
// CEncryptedKeeper - manage encrypted registry entries
///////////////////////////////////////////////////////////////////////////////
class CEncryptedKeeper : public CObject
{
public:
    static void EncryptString( CString sSecret, BYTE **paBytes, UINT *pnBytes );
    static CString DecryptString( BYTE *aBytes, UINT nBytes );

    CEncryptedKeeper();
    CEncryptedKeeper( const CEncryptedKeeper& );
    ~CEncryptedKeeper();

    CString Get( CString sMatch );
    void Set( CString sValue );

	void SetSave( bool bSave ) { m_bSaveToRegistry = bSave; }

    void RegistryGet( CString sSection, CString sKey, CString sMatch );
    void RegistryPut( CString sSection, CString sKey, CString sMatch );

    void ClearCurrent();
    void ClearSaved();
    void ClearAll();
protected:
    BYTE *m_aData;  // current data active in dialog
    UINT m_nData;
    BYTE *m_aSavedData; // remember what was read from registry
    UINT m_nSavedData;

    CString m_sSavedMatch;

    bool m_bSaveToRegistry;
};

///////////////////////////////////////////////////////////////////////////////
// CCoronaProject - info about a project being built in CAndroidBuildDlg
///////////////////////////////////////////////////////////////////////////////
class CCoronaProject :
	public CObject
{
public:
	CCoronaProject();
    CCoronaProject( CString sPath );
  	virtual ~CCoronaProject();

	static CString RemoveMainLua( CString sPath );  // for RecentlyUsed menu items
	static bool ValidatePackage( CString sPackage );
	static CString GetTrialKeystorePath();

protected:
	static bool allalphanum( CString string, int start, int end );
	static bool isJavaReservedWord( CString string, int start, int end );

public:
    void Init( CString sPath );
    void Save();

	bool ValidateKeystoreAliasPassword();

	CString GetPath() { return m_sPath; }
	CString GetDir();  // computed from path, no "SetDir()"

	CString GetName()  { return m_sName; }
	void SetName( CString sName ) { m_sName = sName; }

	int GetAndroidVersionCode() { return m_iAndroidVersionCode; }
	void SetAndroidVersionCode(int version) { m_iAndroidVersionCode = version; }

	CString GetAndroidVersionName() { return m_sAndroidVersionName; }
	void SetAndroidVersionName( CString sVersionName ) { m_sAndroidVersionName = sVersionName; }

	CString GetWin32VersionString() { return m_sWin32VersionString; }
	void SetWin32VersionString(const CString& value) { m_sWin32VersionString = value; }

	CString GetPackage()  { return m_sPackage; }
	void SetPackage( CString sPackage ) { m_sPackage = sPackage; }

	CString GetKeystorePath() { return m_sKeystorePath; }
	void SetKeystorePath( CString sPath ) { m_sKeystorePath = sPath; }

	CString GetKeystorePassword() { return m_KeystorePassword.Get( GetKeystorePath() ); }
	void SetKeystorePassword( CString sPassword ) { m_KeystorePassword.Set( sPassword ); }
	void ClearKeystorePassword() { m_KeystorePassword.ClearCurrent(); }

	void SetSaveKeystorePassword( bool bSave ) { m_KeystorePassword.SetSave( bSave ); }

	// Used for last-used keystore password
	void RegistryGetKeystorePassword( CString sSection, CString sKey, CString sKeystorePath );
	void RegistryPutKeystorePassword( CString sSection, CString sKey, CString sKeystorePath );
	CString GetKeystorePassword( CString sPath ) { return m_KeystorePassword.Get( sPath ); }

	CString GetAlias() { return m_sAlias; }
	void SetAlias( CString sAlias ) { m_sAlias = sAlias; }

	CString GetAliasPassword() { return m_AliasPassword.Get( GetAlias() ); }
	void SetAliasPassword( CString sPassword )  { m_AliasPassword.Set( sPassword ); }
	void ClearAliasPassword() { m_AliasPassword.ClearCurrent(); }

	void SetSaveAliasPassword( bool bSave ) { m_AliasPassword.SetSave( bSave ); }

	CString GetSaveDir() { return m_sSaveDir; }
	void SetSaveDir( CString sPath ) { m_sSaveDir = sPath; }

	CString GetTargetOS() { return m_sTargetOS; }
	void SetTargetOS( CString sTargetOS ) { m_sTargetOS = sTargetOS; }

	CStringA GetAnalyticsSessionId() { return m_sAnalyticsSessionId; }

	bool IsDistribution();

	Rtt::TargetDevice::Platform GetTargetPlatform();
	void SetTargetPlatform(Rtt::TargetDevice::Platform targetDevice);

	bool GetCreateLiveBuild();
	void SetCreateLiveBuild(bool createLiveBuild);
  CString GetAfterBuild();
	void SetAfterBuild(CString afterBuild);

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
	bool GetEnableMonetization();
	void SetEnableMonetization(bool enableMonetization);
#endif

	CString GetCopyright() { return m_sCopyright; }
	void SetCopyright(const CString& value) { m_sCopyright = value; }

	CString GetCompanyName() { return m_sCompanyName; }
	void SetCompanyName(const CString& value) { m_sCompanyName = value; }

	CString GetAppDescription() { return m_sAppDescription; }
	void SetAppDescription(const CString& value) { m_sAppDescription = value; }

	CString GetExeFileName() { return m_sExeFileName; }
	void SetExeFileName(const CString& value) { m_sExeFileName = value; }

	bool GetUseStandartResources() { return m_sUseStandartResources; }
	void SetUseStandartResources(bool value) { m_sUseStandartResources = value; }

	bool GetCreateFBInstantArchive() { return m_sCreateFBInstantArchive; }
	void SetCreateFBInstantArchive(bool value) { m_sCreateFBInstantArchive = value; }

protected:
    void RegistryGet( CString sSection );
    void RegistryPut( CString sSection );
	bool RegistryEntryExists( CString sSection, CString sEntryName );

protected:
    CString m_sPath;  // full path, incl. main.lua
    CString m_sName;  // default dir name
	int m_iAndroidVersionCode;
    CString m_sAndroidVersionName;
	CString m_sWin32VersionString;
    CString m_sPackage;
    CString m_sKeystorePath;
    CString m_sAlias;
    CString m_sSaveDir;
    CString m_sTargetOS;
	CString m_sCopyright;
	CString m_sCompanyName;
	CString m_sAppDescription;
	CString m_sExeFileName;
	CStringA m_sAnalyticsSessionId;

	bool m_sUseStandartResources;

	// store encrypted passwords
	CEncryptedKeeper m_KeystorePassword;
	CEncryptedKeeper m_AliasPassword;

	Rtt::TargetDevice::Platform m_TargetPlatform;

	bool m_CreateLiveBuild;

  CString m_sAfterBuild;

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
	bool m_EnableMonetization;
#endif

	bool m_sCreateFBInstantArchive;
};
