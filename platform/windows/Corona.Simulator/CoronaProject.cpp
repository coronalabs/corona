//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Simulator.h"
#include "SimulatorView.h"
#include "ProgressWnd.h"

#include <ctype.h>  // _istalpha()
#include <wincrypt.h>  // CryptProtectData(), CryptUnprotectData()

#include "CoronaInterface.h"    // appAndroidBuild()
#include "WinString.h"

#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include "Rtt_JavaHost.h"   // GetJavaHome()
#include "Rtt_PlatformAppPackager.h"
#include "CoronaProject.h"
#include "ListKeyStore.h"

// Enables the Crypto++ library's MD5 support without compiler warnings.
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "CryptoPP\md5.h"


///////////////////////////////////////////////////////////////////////////////
// CCoronaProject
///////////////////////////////////////////////////////////////////////////////

// Constructor without arguments
CCoronaProject::CCoronaProject()
:	m_iAndroidVersionCode(_ttoi(REGISTRY_ANDROID_VERSION_CODE_DEFAULT)),
	m_sAndroidVersionName(REGISTRY_VERSION_STRING_DEFAULT),
	m_sWin32VersionString(REGISTRY_VERSION_STRING_DEFAULT),
	m_TargetPlatform(Rtt::TargetDevice::kAndroidPlatform),
	m_sUseStandartResources(true),
	m_sCreateFBInstantArchive(false)
{
}

// Constructor with a project path, call Init to read registry
CCoronaProject::CCoronaProject( CString sPath )
:	CCoronaProject()
{
	m_sPath = sPath;
    Init( sPath );
}

// Destructor - no cleanup needed
CCoronaProject::~CCoronaProject(void)
{
}

// Init - use path as registry section, read in stored values if any
void
CCoronaProject::Init( CString sPath )
{
	// Fetch a pointer to the Corona Simulator's main application instance.
	auto appPointer = AfxGetApp();

	// Store the path of the main.lua file which "should" include the file name.
    m_sPath = sPath;

	// Extract the project's path without the file name.
	CString sDirectory = RemoveMainLua( m_sPath );

	// Build a session ID string for analytics. This is an MD5 hash of the project path without the file name.
	U8 hash[CryptoPP::Weak1::MD5::DIGESTSIZE];
	WinString path;
	path.SetTCHAR(sDirectory);
	CryptoPP::Weak1::MD5 md5;
	md5.CalculateDigest(hash, (const byte*)path.GetUTF8(), strlen(path.GetUTF8()));
	char *p = m_sAnalyticsSessionId.GetBuffer((CryptoPP::Weak1::MD5::DIGESTSIZE * 2) + 1);
	for (int index = 0; index < CryptoPP::Weak1::MD5::DIGESTSIZE; index++)
	{
		p += sprintf_s(p, 3, "%02x", hash[index]);
	}
	m_sAnalyticsSessionId.ReleaseBuffer();

    // Use lower-case version of path as section
    CString sSection = GetDir();
    sSection.MakeLower();
	sSection.Replace(_T("\\\\"), _T(""));  // Fixes issue with network shares
    RegistryGet( sSection );

    // If no stored name, get project name from last directory name in path
    if( m_sName == REGISTRY_NAME_DEFAULT )
	{
		 int i = sDirectory.ReverseFind( _T('\\') );
		 if( i == -1 )
			 m_sName = sDirectory;
		 else
			 m_sName = sDirectory.Right( sDirectory.GetLength() - i - 1);
	}

	// If an EXE file name is not assigned, then generate one via the application name.
	if (m_sExeFileName.IsEmpty())
	{
		WinString appName;
		appName.SetTCHAR(m_sName);
		if (Rtt_StringEndsWithNoCase(appName.GetUTF8(), ".exe") == false)
		{
			appName.Append(".exe");
		}
		Rtt::String utf8EscapedExeFileName;
		Rtt::PlatformAppPackager::EscapeFileName(appName.GetUTF8(), utf8EscapedExeFileName, false);
		appName.SetUTF8(utf8EscapedExeFileName.GetString());
		m_sExeFileName = appName.GetTCHAR();
	}

	// If the destinatin path is empty, then choose a default path.
	if (m_sSaveDir.IsEmpty())
	{
		// First, attempt to fetch the last build path the used by another project.
		m_sSaveDir = appPointer->GetProfileString(REGISTRY_SECTION, REGISTRY_BUILD_DIR);
		if (m_sSaveDir.IsEmpty())
		{
			// Older versions of the Corona Simulator stored last path under the "BuildAndroid" registry key.
			m_sSaveDir = appPointer->GetProfileString(_T("BuildAndroid"), REGISTRY_BUILD_DIR);
		}

		// If we still don't have a path, then this is the first time the user is doing a build.
		// Choose a nice directory under the system's Documents directory by default.
		if (m_sSaveDir.IsEmpty())
		{
			::SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, 0, m_sSaveDir.GetBuffer(MAX_PATH));
			m_sSaveDir.ReleaseBuffer();
			if (m_sSaveDir.IsEmpty() == false)
			{
				TCHAR lastCharacter = m_sSaveDir.GetAt(m_sSaveDir.GetLength() - 1);
				if ((lastCharacter != _T('\\')) && (lastCharacter != _T('/')))
				{
					m_sSaveDir.Append(_T("\\"));
				}
				m_sSaveDir.Append(_T("Corona Built Apps"));
			}
		}
	}

	// If this is a new project, then attempt to choose a default company name.
	// Note: If the project's existing company name registry entry is blank, then respect that setting.
	if (m_sCompanyName.IsEmpty() && !RegistryEntryExists(sSection, REGISTRY_COMPANY_NAME))
	{
		if (RegistryEntryExists(REGISTRY_SECTION, REGISTRY_COMPANY_NAME))
		{
			// Use the last built app's company name.
			m_sCompanyName = appPointer->GetProfileString(REGISTRY_SECTION, REGISTRY_COMPANY_NAME);
		}
		else
		{
			// Attempt to fetch the company name registered to the Windows system.
			// Note: This will only happen if an app was never built before or the simulator's preferences were deleted.
			const TCHAR kRegistryKeyName[] = _T("Software\\Microsoft\\Windows NT\\CurrentVersion");
			const TCHAR kRegistryEntryName[] = _T("RegisteredOrganization");
			HKEY registryKeyHandle = nullptr;
			::RegOpenKeyEx(HKEY_LOCAL_MACHINE, kRegistryKeyName, 0, KEY_READ | KEY_WOW64_64KEY, &registryKeyHandle);
			if (registryKeyHandle)
			{
				DWORD valueType = REG_NONE;
				DWORD valueByteLength = 0;
				::RegQueryValueEx(registryKeyHandle, kRegistryEntryName, nullptr, &valueType, nullptr, &valueByteLength);
				if ((REG_SZ == valueType) && (valueByteLength > 0))
				{
					::RegQueryValueEx(
							registryKeyHandle, kRegistryEntryName, nullptr, &valueType,
							(LPBYTE)m_sCompanyName.GetBuffer(128), &valueByteLength);
					m_sCompanyName.ReleaseBuffer();
					m_sCompanyName.Trim();
				}
				::RegCloseKey(registryKeyHandle);
			}
		}
	}
}

// Save - use path as registry section, save values
void
CCoronaProject::Save()
{
    // Use all lowercase version of path as registry section
    CString sPath = GetDir();
    sPath.MakeLower();
	sPath.Replace(_T("\\\\"), _T(""));  // Fixes issue with network shares

	// Save settings to the project's registry key.
    RegistryPut( sPath );

	// Save last used build path and company name to the application's global "Preferences" registry key.
	// To be used as defaults for new projects in the future.
	AfxGetApp()->WriteProfileString(REGISTRY_SECTION, REGISTRY_BUILD_DIR, m_sSaveDir);
	AfxGetApp()->WriteProfileString(REGISTRY_SECTION, REGISTRY_COMPANY_NAME, m_sCompanyName);
}

// RegistryGet - read each saved value from the given section
// Passwords are managed separately because they're encrypted
void
CCoronaProject::RegistryGet( CString sSection )
{
	CString stringBuffer;
    CWinApp *pApp = AfxGetApp();

    m_sName = pApp->GetProfileString( sSection, REGISTRY_NAME, REGISTRY_NAME_DEFAULT );
    stringBuffer = pApp->GetProfileString( sSection, REGISTRY_ANDROID_VERSION_CODE, REGISTRY_ANDROID_VERSION_CODE_DEFAULT );
	m_iAndroidVersionCode = _ttoi(stringBuffer);
    m_sAndroidVersionName = pApp->GetProfileString( sSection, REGISTRY_ANDROID_VERSION_NAME, REGISTRY_VERSION_STRING_DEFAULT );
    m_sWin32VersionString = pApp->GetProfileString( sSection, REGISTRY_WIN32_VERSION_STRING, REGISTRY_VERSION_STRING_DEFAULT );
    m_sPackage = pApp->GetProfileString( sSection, REGISTRY_PACKAGE, REGISTRY_PACKAGE_DEFAULT );
    m_sKeystorePath = pApp->GetProfileString( sSection, REGISTRY_KEYSTOREPATH, REGISTRY_KEYSTOREPATH_DEFAULT );
    m_sAlias = pApp->GetProfileString( sSection, REGISTRY_ALIAS, REGISTRY_ALIAS_DEFAULT );
    m_sSaveDir = pApp->GetProfileString( sSection, REGISTRY_SAVEDIR, REGISTRY_SAVEDIR_DEFAULT );
	m_sCopyright = pApp->GetProfileString( sSection, REGISTRY_COPYRIGHT );
	m_sCompanyName = pApp->GetProfileString( sSection, REGISTRY_COMPANY_NAME );
	m_sAppDescription = pApp->GetProfileString( sSection, REGISTRY_APP_DESCRIPTION );
	m_sExeFileName = pApp->GetProfileString( sSection, REGISTRY_EXE_FILE_NAME );
	m_sUseStandartResources = pApp->GetProfileIntW( sSection, REGISTRY_WEB_USER_STANDARD_RESOURCES, 1 );
	m_sCreateFBInstantArchive = pApp->GetProfileIntW( sSection, REGISTRY_WEB_USER_CREATE_FB_INSTANT_ARCHIVE, 0 );

	// Third argument is what this is a password for, not value of password
    m_KeystorePassword.RegistryGet( sSection, REGISTRY_KEYSTOREPWD, GetKeystorePath() );
    m_AliasPassword.RegistryGet( sSection, REGISTRY_ALIASPWD, GetAlias() );
	stringBuffer = pApp->GetProfileString(sSection, REGISTRY_CREATE_LIVE_BUILD, REGISTRY_CREATE_LIVE_BUILD_DEFAULT);
	m_CreateLiveBuild = _ttoi(stringBuffer) ? true : false;
	m_sAfterBuild = pApp->GetProfileString( sSection, REGISTRY_AFTER_BUILD );
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
	stringBuffer = pApp->GetProfileString(sSection, REGISTRY_ENABLE_MONETIZATION, REGISTRY_ENABLE_MONETIZATION_DEFAULT);
	m_EnableMonetization = _ttoi(stringBuffer) ? true : false;
#endif
}

// RegistryPut - save each value to the given section
// Passwords are managed separately because they're encrypted
void
CCoronaProject::RegistryPut( CString sSection )
{
	CString stringBuffer;
    CWinApp *pApp = AfxGetApp();

    pApp->WriteProfileString( sSection, REGISTRY_NAME, m_sName );
	stringBuffer.Format(_T("%d"), m_iAndroidVersionCode);
	pApp->WriteProfileString( sSection, REGISTRY_ANDROID_VERSION_CODE, stringBuffer );
    pApp->WriteProfileString( sSection, REGISTRY_ANDROID_VERSION_NAME, m_sAndroidVersionName );
    pApp->WriteProfileString( sSection, REGISTRY_WIN32_VERSION_STRING, m_sWin32VersionString );
    pApp->WriteProfileString( sSection, REGISTRY_PACKAGE, m_sPackage );
    pApp->WriteProfileString( sSection, REGISTRY_KEYSTOREPATH, m_sKeystorePath );
    pApp->WriteProfileString( sSection, REGISTRY_ALIAS, m_sAlias );
    pApp->WriteProfileString( sSection, REGISTRY_SAVEDIR, m_sSaveDir );
    pApp->WriteProfileString( sSection, REGISTRY_COPYRIGHT, m_sCopyright );
    pApp->WriteProfileString( sSection, REGISTRY_COMPANY_NAME, m_sCompanyName );
    pApp->WriteProfileString( sSection, REGISTRY_APP_DESCRIPTION, m_sAppDescription );
    pApp->WriteProfileString( sSection, REGISTRY_EXE_FILE_NAME, m_sExeFileName );
		pApp->WriteProfileInt( sSection, REGISTRY_WEB_USER_STANDARD_RESOURCES, (int)GetUseStandartResources() );
		pApp->WriteProfileInt( sSection, REGISTRY_WEB_USER_CREATE_FB_INSTANT_ARCHIVE, (int)GetCreateFBInstantArchive() );

	// Third argument is what this is a password for, not value of password
    m_KeystorePassword.RegistryPut( sSection, REGISTRY_KEYSTOREPWD, GetKeystorePath() );
	m_AliasPassword.RegistryPut(sSection, REGISTRY_ALIASPWD, GetAlias());
	stringBuffer.Format(_T("%d"), m_CreateLiveBuild);
	pApp->WriteProfileString(sSection, REGISTRY_CREATE_LIVE_BUILD, stringBuffer);

	pApp->WriteProfileString( sSection, REGISTRY_AFTER_BUILD, m_sAfterBuild );
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
	stringBuffer.Format(_T("%d"), m_EnableMonetization);
	pApp->WriteProfileString(sSection, REGISTRY_ENABLE_MONETIZATION, stringBuffer);
#endif
}

bool
CCoronaProject::RegistryEntryExists( CString sSection, CString sEntryName )
{
	bool wasEntryFound = false;
	HKEY registryKeyHandle = nullptr;
	::RegOpenKeyEx(AfxGetApp()->GetAppRegistryKey(), sSection, 0, KEY_READ, &registryKeyHandle);
	if (registryKeyHandle)
	{
		auto result = ::RegQueryValueEx(registryKeyHandle, sEntryName, nullptr, nullptr, nullptr, nullptr);
		if (ERROR_SUCCESS == result)
		{
			wasEntryFound = true;
		}
		::RegCloseKey(registryKeyHandle);
	}
	return wasEntryFound;
}

// RegistryPutKeystorePassword - save keystore password as requested
// Third argument is what this is a password for, not value of password
// This function used to save last used keystore password
void
CCoronaProject::RegistryPutKeystorePassword( CString sSection, CString sKey, CString sKeystorePath )
{
	m_KeystorePassword.RegistryPut( sSection, sKey, sKeystorePath );
}

// RegistryGetKeystorePassword - read in keystore password as requested
// Third argument is what this is a password for, not value of password
// This function used to read last used keystore password
void
CCoronaProject::RegistryGetKeystorePassword( CString sSection, CString sKey, CString sKeystorePath )
{
	m_KeystorePassword.RegistryGet( sSection, sKey, sKeystorePath );
}

// RemoveMainLua - Returns string with \\main.lua removed, if present.
// Does not modify incoming string.
CString
CCoronaProject::RemoveMainLua( CString sPath )
{
    CString sFilename = _T("\\main.lua");

   // remove main.lua from path
    if( sPath.Right( sFilename.GetLength() ) == sFilename )
        sPath = sPath.Left( sPath.GetLength() - sFilename.GetLength() );

    return sPath;
}

// GetDir - return directory of project minus main.lua
CString
CCoronaProject::GetDir()
{
    return RemoveMainLua( m_sPath );
}

// ValidatePackage - return true if sPackage is a valid Java Package name
// check for [a-zA-Z]+(.[a-zA-Z]+)+ e.g. com.acme.games
// Second and further characters after a period can be numbers
// _ (underscored) also allowed
// There must be at least one period surrounded by at least one character on each side
// No Java Reserved Words - interface, for, etc.
bool
CCoronaProject::ValidatePackage( CString sPackage )
{
    int length = sPackage.GetLength();
    int prevPeriod = -1;
    int nextPeriod = -1;

    bool bCorrect = true;

  	while( true )  // break when no more periods found, or non-valid string
	{
         nextPeriod = sPackage.Find( _T('.'), prevPeriod + 1);
		 if( nextPeriod == -1 )  // no more periods
		 {
             if( (prevPeriod == -1)  // no periods at all
                  || ! allalphanum(sPackage, prevPeriod + 1, sPackage.GetLength() - 1)
				  || isJavaReservedWord( sPackage, prevPeriod + 1, sPackage.GetLength() - 1) )
			 {
                  bCorrect = false;
			 }
             break;
		 }
		 if( (nextPeriod == 0)  // starting with period
             || (nextPeriod == length - 1) // ending with period
             || (nextPeriod == prevPeriod + 1 ) // two periods in a row
		     || ! allalphanum( sPackage, prevPeriod + 1, nextPeriod - 1)
             || isJavaReservedWord( sPackage, prevPeriod + 1, nextPeriod - 1) )
		 {
             bCorrect = false;
             break;
		 }

         prevPeriod = nextPeriod;
	}

   return bCorrect;
}

// GetTrialKeystorePath - we install AppPath\Resources\debug.keystore
CString
CCoronaProject::GetTrialKeystorePath()
{
	CSimulatorApp *pApp = (CSimulatorApp *)AfxGetApp();

	return pApp->GetResourceDir() + _T("\\debug.keystore");
}

// Check that all the characters between start and end, inclusive, are alphanum or '_',
// except the first one, which must be alpha or '_'.
bool
CCoronaProject::allalphanum( CString string, int start, int end )
{
   if( start < 0 || (end >= string.GetLength()) || (start > end) )
       return false;

   if( ! _istalpha(string[start]) && (string[start] != _T('_')) )
       return false;

    for( int i = start + 1; i <= end; i++ )
        if( ! _istalnum(string[i]) && (string[i] != _T('_')) )
            return false;

    return true;
}

// isJavaReservedWord - return true if all the characters between start and end, inclusive,
// form a Java reserved word.  http://www.javacamp.org/javaI/reservedWords.html
// Note that case matters: "interface" is reserved, but "Interface" isn't.
bool
CCoronaProject::isJavaReservedWord( CString string, int start, int end )
{
   // validate function input first
   if( start < 0 || (end >= string.GetLength()) || (start > end) )
       return false;

   CString sSubstring = string.Mid( start, end - start + 1 );

   if(    sSubstring == _T("abstract")
	   || sSubstring == _T("assert")
	   || sSubstring == _T("boolean")
	   || sSubstring == _T("break")
	   || sSubstring == _T("byte")
	   || sSubstring == _T("case")
	   || sSubstring == _T("catch")
	   || sSubstring == _T("char")
	   || sSubstring == _T("class")
	   || sSubstring == _T("const")
	   || sSubstring == _T("continue")
	   || sSubstring == _T("default")
	   || sSubstring == _T("do")
	   || sSubstring == _T("double")
	   || sSubstring == _T("else")
	   || sSubstring == _T("extends")
	   || sSubstring == _T("false")
	   || sSubstring == _T("final")
	   || sSubstring == _T("finally")
	   || sSubstring == _T("float")
	   || sSubstring == _T("for")
	   || sSubstring == _T("goto")
	   || sSubstring == _T("if")
	   || sSubstring == _T("implements")
	   || sSubstring == _T("import")
	   || sSubstring == _T("instanceof")
	   || sSubstring == _T("int")
	   || sSubstring == _T("interface")
	   || sSubstring == _T("long")
	   || sSubstring == _T("native")
	   || sSubstring == _T("new")
	   || sSubstring == _T("null")
	   || sSubstring == _T("package")
	   || sSubstring == _T("private")
	   || sSubstring == _T("protected")
	   || sSubstring == _T("public")
	   || sSubstring == _T("return")
	   || sSubstring == _T("short")
	   || sSubstring == _T("static")
	   || sSubstring == _T("strictfp")
	   || sSubstring == _T("super")
	   || sSubstring == _T("switch")
	   || sSubstring == _T("synchronized")
	   || sSubstring == _T("this")
	   || sSubstring == _T("throw")
	   || sSubstring == _T("throws")
	   || sSubstring == _T("transient")
	   || sSubstring == _T("true")
	   || sSubstring == _T("try")
	   || sSubstring == _T("void")
	   || sSubstring == _T("volatile")
	   || sSubstring == _T("while") )
   {
       return true;
   }

    return false;
}

// ValidateKeystoreAliasPassword - return true if passwords valid
// Verify keystore & alias passwords by doing test compile & signing
// CSimulatorApp::InitJavaPaths() should be called before this function is called.
bool CCoronaProject::ValidateKeystoreAliasPassword()
{
	ListKeyStore listKeyStore;
	WinString keystorePathStr = GetKeystorePath(); //  .GetBuffer();
	WinString keystorePasswordStr = GetKeystorePassword(); //  .GetBuffer() );
	WinString aliasStr = GetAlias(); // .GetBuffer() );
	WinString aliasPasswordStr = GetAliasPassword(); // .GetBuffer() );
	WinString resourcesDirStr = ((CSimulatorApp *)AfxGetApp())->GetResourceDir();

	return listKeyStore.AreKeyStoreAndAliasPasswordsValid(keystorePathStr.GetUTF8(), keystorePasswordStr.GetUTF8(), aliasStr.GetUTF8(), aliasPasswordStr.GetUTF8(), resourcesDirStr);

#if 0
    // Get Windows temp directory
	TCHAR TempPath[MAX_PATH];
	GetTempPath(MAX_PATH, TempPath);  // ends in '\\'
	// TODO - should add "Corona Labs" to this temp path, at least

	AntHost	antTask;

	WinString resourceDir, kp, ks, ka, ap, tempDir;
	resourceDir.SetTCHAR( ((CSimulatorApp *)AfxGetApp())->GetResourceDir() );

	char buildFilePath[1024];

	snprintf( buildFilePath, 1024, "%s\\build.xml", resourceDir.GetUTF8() );

	tempDir.SetTCHAR( TempPath );
	ks.SetTCHAR( GetKeystorePath().GetBuffer() );
	kp.SetTCHAR( GetKeystorePassword().GetBuffer() );
	ka.SetTCHAR( GetAlias().GetBuffer() );
	ap.SetTCHAR( GetAliasPassword().GetBuffer() );

	antTask.SetProperty( "TEMP_DIR", tempDir.GetUTF8() );
	antTask.SetProperty( "BUNDLE_DIR", resourceDir.GetUTF8() );
	antTask.SetProperty( "KS", ks.GetUTF8() );
	antTask.SetProperty( "KP", kp.GetUTF8() );
	antTask.SetProperty( "KA", ka.GetUTF8() );
	antTask.SetProperty( "AP", ap.GetUTF8() );

	Rtt::String antResult;
	int antCode;

	// TODO - test-sign doesn't clean up the files it creates
	antCode = antTask.AntCall( buildFilePath, "test-sign", 0, &antResult );
	Rtt_ASSERT( antCode == 1 );
	if ( antCode == 0 )
	{
		WinString mesg;

		mesg.SetUTF8("ANT failed while checking keystore password:\n  ");
		mesg.Append(antResult.GetString());
		mesg.Append("\n  Keystore: ");
		mesg.Append(GetKeystorePath().GetString());
		mesg.Append("\n  Alias: ");
		mesg.Append(GetAlias().GetString());
		mesg.Append("\n  Build file: ");
		mesg.Append(buildFilePath);
		mesg.Append("\n  TEMP_DIR: ");
		mesg.Append(tempDir.GetUTF8());
		mesg.Append("\n  BUNDLE_DIR: ");
		mesg.Append(resourceDir.GetUTF8());
		mesg.Append("\n  JDK Path: ");
		mesg.Append(Rtt::JavaHost::GetJdkPath());
		mesg.Append("\n  JRE Path: ");
		mesg.Append(Rtt::JavaHost::GetJrePath());

		Rtt_LogException(mesg.GetUTF8());
	}

	return antCode == 1;
#endif
}

bool
CCoronaProject::IsDistribution()
{
	return 0 != CCoronaProject::GetTrialKeystorePath().CompareNoCase( GetKeystorePath() );
}
Rtt::TargetDevice::Platform CCoronaProject::GetTargetPlatform()
{
	return m_TargetPlatform;
}
void CCoronaProject::SetTargetPlatform(Rtt::TargetDevice::Platform targetDevice)
{
	m_TargetPlatform = targetDevice;
}

bool CCoronaProject::GetCreateLiveBuild()
{
	return m_CreateLiveBuild;
}
void CCoronaProject::SetCreateLiveBuild(bool createLiveBuild)
{
	m_CreateLiveBuild = createLiveBuild;
}

CString CCoronaProject::GetAfterBuild()
{
	return m_sAfterBuild;
}
void CCoronaProject::SetAfterBuild(CString afterBuild)
{
	m_sAfterBuild = afterBuild;
}

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
bool CCoronaProject::GetEnableMonetization()
{
	return m_EnableMonetization;
}
void CCoronaProject::SetEnableMonetization(bool enableMonetization)
{
	m_EnableMonetization = enableMonetization;
}
#endif // AUTO_INCLUDE_MONETIZATION_PLUGIN
///////////////////////////////////////////////////////////////////////////////
// CEncryptedKeeper
///////////////////////////////////////////////////////////////////////////////

// constructor, no args
CEncryptedKeeper::CEncryptedKeeper() :
    m_aData( NULL ),
    m_nData( 0 ),
    m_aSavedData( NULL ),
    m_nSavedData( 0 ),
    m_bSaveToRegistry( false ),
    m_sSavedMatch( _T("") )
{
}

// Copy constructor
CEncryptedKeeper::CEncryptedKeeper( const CEncryptedKeeper& keeperIn ) :
    m_aData( NULL ),
    m_nData( 0 ),
    m_aSavedData( NULL ),
    m_nSavedData( 0 ),
    m_bSaveToRegistry( false )
{
    if( keeperIn.m_aData )
	{
        m_aData = new BYTE [keeperIn.m_nData];
        memcpy( m_aData, keeperIn.m_aData, keeperIn.m_nData );
        m_nData = keeperIn.m_nData;
	}
    if( keeperIn.m_aSavedData )
	{
        m_aSavedData = new BYTE [keeperIn.m_nSavedData];
        memcpy( m_aSavedData, keeperIn.m_aSavedData, keeperIn.m_nSavedData );
        m_nSavedData = keeperIn.m_nSavedData;
	}
}

// destructor - delete allocated data
CEncryptedKeeper::~CEncryptedKeeper(void)
{
    ClearAll();
}

// Get - return value held by this Keeper
// sMatch is value which must match the one for saved data
CString
CEncryptedKeeper::Get( CString sMatch )
{
    CString sValue( _T("") );

	// Return temp data if available, else saved if available and still matches.
	if( m_aData != NULL && m_nData > 0 )
		sValue = DecryptString( m_aData, m_nData );
	else if( sMatch == m_sSavedMatch && m_aSavedData != NULL && m_nSavedData > 0 )
		sValue= DecryptString( m_aSavedData, m_nSavedData );

    return sValue;
}

// Set - save this value as current data
void
CEncryptedKeeper::Set( CString sValue )
{
    // EncryptString allocates a new buffer, so free the old one first.
    ClearCurrent();

    EncryptString( sValue, &m_aData, &m_nData );
}

// RegistryGet - read encrypted value from registry, corresponding to sMatch
void
CEncryptedKeeper::RegistryGet( CString sSection, CString sKey, CString sMatch )
{
    CWinApp *pApp = AfxGetApp();

    // GetProfileBinary allocates a new buffer, so free the old one first, and temp is also invalid
    ClearAll();

    m_sSavedMatch = sMatch;
	pApp->GetProfileBinary( sSection, sKey, &m_aSavedData, &m_nSavedData  );
}

// RegistryPut - save stored value to registry, encrypted
void
CEncryptedKeeper::RegistryPut( CString sSection, CString sKey, CString sMatch )
{
    if( ! m_bSaveToRegistry )
        return;

    CWinApp *pApp = AfxGetApp();

    if( m_aData != NULL && m_nData > 0 ) // Write current data to registry
	{
		pApp->WriteProfileBinary( sSection, sKey, m_aData, m_nData );

        // current data is now saved data
        ClearSaved();
        m_aSavedData = new BYTE [m_nData];
        memcpy( m_aSavedData, m_aData, m_nData );
        m_nSavedData = m_nData;
        m_sSavedMatch = sMatch;  // saved data matches this
        // temp data no longer needed
        ClearCurrent();
	}
	// Re-save of same data, could be to different registry location
	else if( m_aSavedData != NULL && m_nSavedData > 0 )
	{
		pApp->WriteProfileBinary( sSection, sKey, m_aSavedData, m_nSavedData );
	}

}

// ClearCurrent - clear current data (entered in dialog)
void
CEncryptedKeeper::ClearCurrent()
{
   if( m_aData )
	{
        delete m_aData;
        m_aData = NULL;
	}

    m_nData = 0;
}

// ClearSaved - clear saved data (read from registry)
void
CEncryptedKeeper::ClearSaved()
{
   if( m_aSavedData )
	{
        delete m_aSavedData;
        m_aSavedData = NULL;
	}

    m_nSavedData = 0;
    m_sSavedMatch = _T("");
}

// ClearAll - clear both current and saved data
void
CEncryptedKeeper::ClearAll()
{
    ClearCurrent();
    ClearSaved();
}

// EncryptString - encrypt a string (to save to registry)
void CEncryptedKeeper::EncryptString( CString sSecret, BYTE **paBytes, UINT *pnBytes )
{
    *paBytes = NULL;
    *pnBytes = 0;

    DATA_BLOB unencryptedData, encryptedData;
    unencryptedData.pbData = (BYTE *)sSecret.GetBuffer();

    // Save the NULL character in the data
    // We need to multiply the length of the string by the
    // size of the data contained therein to support multi-
    // byte character sets.
    unencryptedData.cbData = (sSecret.GetLength() + 1)  * sizeof( sSecret[0] );
    if (!CryptProtectData(
        &unencryptedData,
        _T("Marker"),
        NULL,
        NULL,
        NULL,
        0,
        &encryptedData))
    {
        return;
    }

    // Save the encrypted data buffer in heap-allocated memory
    *paBytes = new BYTE [encryptedData.cbData];
    memcpy( *paBytes, encryptedData.pbData, encryptedData.cbData );
    *pnBytes = encryptedData.cbData;

    // clean up
    sSecret.ReleaseBuffer();
    LocalFree( encryptedData.pbData );
}

// DecryptString - decrypt a string (read from registry)
CString CEncryptedKeeper::DecryptString( BYTE *aBytes, UINT nBytes )
{
    DATA_BLOB encryptedData, unencryptedData;

    encryptedData.pbData = aBytes;
    encryptedData.cbData = nBytes;
    LPWSTR dataDescription; // Receives the description saved with data
    if (!CryptUnprotectData(
        &encryptedData,
        &dataDescription,
        NULL,
        NULL,
        NULL,
        0,
        &unencryptedData))
    {
        return _T("");
    }

    // And the data description string as well.
    LocalFree(dataDescription);

    // NOTE: Contains NULL terminator
    CString sSecret( (LPCTSTR) unencryptedData.pbData, unencryptedData.cbData );

    // Cleanup
    LocalFree(unencryptedData.pbData);

    return sSecret;
}
