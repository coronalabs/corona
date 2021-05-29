//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Core\Rtt_Build.h"
#include "Rtt_WinPlatform.h"
#include "Rtt_WinPlatformServices.h"
#include "WinGlobalProperties.h"
#include "WinString.h"

#include <wincrypt.h>

namespace Rtt
{

#define Rtt_REGISTRY_SECTION _T("Preferences")

// These functions are used for WinPlatformServices::GetPreference() and SetPreference() 
// Registry code copied from MFC Source appui3.cpp (CWinApp)

// returns key for HKEY_CURRENT_USER\"Software"\RegistryKey\ProfileName
//  (or other top level key) creating it if it doesn't exist
// responsibility of the caller to call RegCloseKey() on the returned HKEY
// Modified to get registry key and profile name from WinGlobalProperties,
// initialized in CSimulatorApp::InitInstance() 
static HKEY GetAppRegistryKey( HKEY hkeyTopLevel ) 
{
    WinString strRegKey, strRegProfile;
#ifdef Rtt_NO_GUI
	strRegKey.SetUTF8("Ansca Corona");
	strRegProfile.SetUTF8("Corona Simulator");
#else
	Rtt_ASSERT( GetWinProperties()->GetRegistryKey() != NULL);
	Rtt_ASSERT( GetWinProperties()->GetRegistryProfile() != NULL);

    strRegKey.SetUTF8( GetWinProperties()->GetRegistryKey() );
    strRegProfile.SetUTF8( GetWinProperties()->GetRegistryProfile() );
#endif // Rtt_NO_GUI


	HKEY hAppKey = NULL;
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("software"), 0, KEY_WRITE|KEY_READ,
		&hSoftKey) == ERROR_SUCCESS)
	{
		DWORD dw;

		if (RegCreateKeyEx(hSoftKey, strRegKey.GetTCHAR(), 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
			&hCompanyKey, &dw) == ERROR_SUCCESS)
		{
			RegCreateKeyEx(hCompanyKey, strRegProfile.GetTCHAR(), 0, REG_NONE,
				REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
				&hAppKey, &dw);
		}
	}
	if (hSoftKey != NULL)
		RegCloseKey(hSoftKey);
	if (hCompanyKey != NULL)
		RegCloseKey(hCompanyKey);

	return hAppKey;
}

// returns key for:
//      HKEY_CURRENT_USER\"Software"\RegistryKey\AppName\lpszSection
// creating it if it doesn't exist.
// responsibility of the caller to call RegCloseKey() on the returned HKEY
// Returns void * to avoid including windows.h before header file
static HKEY GetSectionKey(LPCTSTR lpszSection, HKEY hkeyTopLevel )
{
	Rtt_ASSERT(lpszSection != NULL);

	HKEY hSectionKey = NULL;
	HKEY hAppKey = GetAppRegistryKey( hkeyTopLevel );
	if (hAppKey == NULL)
		return NULL;

	DWORD dw;
	RegCreateKeyEx(hAppKey, lpszSection, 0, REG_NONE,
		REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
		&hSectionKey, &dw);
	RegCloseKey(hAppKey);
	return hSectionKey;
}

static bool GetProfileString(
	WinString *pstrValue, LPCTSTR lpszSection, LPCTSTR lpszEntry, HKEY hkeyTopLevel = HKEY_CURRENT_USER)
{
	Rtt_ASSERT(lpszSection != NULL);
	Rtt_ASSERT(lpszEntry != NULL);

	HKEY hSecKey = GetSectionKey(lpszSection, hkeyTopLevel);
	if (hSecKey == NULL)
	{
        return false;
	}

	DWORD dwType=REG_NONE;
	DWORD dwCount=0;
	LONG lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
		NULL, &dwCount);
	if (lResult == ERROR_SUCCESS)
	{
		if( dwType == REG_SZ)  // if type is string
		{
            // dwCount is in bytes, Expand takes # of characters and adds 1
			(*pstrValue).Expand( dwCount/sizeof(TCHAR) );
			lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
				(LPBYTE)(*pstrValue).GetBuffer(), &dwCount);
		}
	}
	RegCloseKey(hSecKey);
	if (lResult == ERROR_SUCCESS)
	{
        // If we found the key, but type was not string (REG_SZ), return false
		return (dwType == REG_SZ);
	}

    return false;
}

static BOOL WriteProfileString(
	LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue, HKEY hkeyTopLevel = HKEY_CURRENT_USER)
{
	Rtt_ASSERT(lpszSection != NULL);
    Rtt_ASSERT(lpszEntry != NULL);

	LONG lResult;
#if 0 // I don't think we want the whole section deleted by accident...
	if (lpszEntry == NULL) //delete whole section
	{
		HKEY hAppKey = GetAppRegistryKey();
		if (hAppKey == NULL)
			return FALSE;
		lResult = ::RegDeleteKey(hAppKey, lpszSection);
		RegCloseKey(hAppKey);
	}
	else 
#endif
    if (lpszValue == NULL)
	{
		HKEY hSecKey = GetSectionKey(lpszSection, hkeyTopLevel);
		if (hSecKey == NULL)
			return FALSE;
		// necessary to cast away const below
		lResult = ::RegDeleteValue(hSecKey, (LPTSTR)lpszEntry);
		RegCloseKey(hSecKey);
	}
	else
	{
		HKEY hSecKey = GetSectionKey(lpszSection, hkeyTopLevel);
		if (hSecKey == NULL)
			return FALSE;
		lResult = RegSetValueEx(hSecKey, lpszEntry, NULL, REG_SZ,
			(LPBYTE)lpszValue, (lstrlen(lpszValue)+1)*sizeof(TCHAR));
		RegCloseKey(hSecKey);
	}
	return lResult == ERROR_SUCCESS;
}

static bool GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, BYTE** ppData, UINT* pBytes)
{
	Rtt_ASSERT(lpszSection != NULL);
	Rtt_ASSERT(lpszEntry != NULL);
	Rtt_ASSERT(ppData != NULL);
	Rtt_ASSERT(pBytes != NULL);
	*ppData = NULL;
	*pBytes = 0;

	HKEY hSecKey = GetSectionKey(lpszSection, HKEY_CURRENT_USER );
	if (hSecKey == NULL)
	{
		return false;
	}

	DWORD dwType=0;
	DWORD dwCount=0; 
	LONG lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType, NULL, &dwCount);
	*pBytes = dwCount;
	if (lResult == ERROR_SUCCESS)
	{
		if( dwType == REG_BINARY )  // if type is binary
		{
			*ppData = new BYTE[*pBytes];
			lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
				*ppData, &dwCount);
		}
	}
	RegCloseKey(hSecKey);
	// If we found the key, but type was not binary, delete data and return false
	if (lResult == ERROR_SUCCESS && (dwType == REG_BINARY))
	{
		return true;
	}
	else
	{
		delete [] *ppData;
		*ppData = NULL;
	}
	return false;
}

static bool WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes)
{
	Rtt_ASSERT(lpszSection != NULL);

	LONG lResult;
	HKEY hSecKey = GetSectionKey(lpszSection, HKEY_CURRENT_USER );
	if (hSecKey == NULL)
		return false;
    if( NULL == pData ) // delete key if pData is null
		lResult = ::RegDeleteValue(hSecKey, (LPTSTR)lpszEntry);
	else lResult = RegSetValueEx(hSecKey, lpszEntry, NULL, REG_BINARY,
                                 pData, nBytes);
	RegCloseKey(hSecKey);
	return lResult == ERROR_SUCCESS;
}

static void EncryptString( const char *sSecret, BYTE **paBytes, UINT *pnBytes )
{
    *paBytes = NULL;
    *pnBytes = 0;

    if( NULL == sSecret )
        return;

    DATA_BLOB unencryptedData, encryptedData;
    unencryptedData.pbData = (BYTE *)sSecret;

    // Save the NULL character in the data
    // We need to multiply the length of the string by the
    // size of the data contained therein to support multi-
    // byte character sets.
    unencryptedData.cbData = (strlen( sSecret ) + 1)  * sizeof( sSecret[0] );
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
    LocalFree( encryptedData.pbData );
}

static bool DecryptString( WinString *pStr, BYTE *aBytes, UINT nBytes )
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
        return false;
    }

    // And the data description string as well.
    LocalFree(dataDescription);

    // NOTE: Contains NULL terminator
    pStr->SetUTF8( (char *)unencryptedData.pbData );
    
    // Cleanup
    LocalFree(unencryptedData.pbData);

    return true;
}

WinPlatformServices::WinPlatformServices( const MPlatform& platform )
:	fPlatform( platform )
{
}

const MPlatform&
WinPlatformServices::Platform() const
{
	return fPlatform;
}

void
WinPlatformServices::GetPreference( const char *key, String * value ) const
{
	WinString strResult, strKey;
    strKey.SetUTF8( key );
    bool bFound = GetProfileString( &strResult, Rtt_REGISTRY_SECTION, strKey.GetTCHAR());

    if( bFound )
	{
		value->Set( strResult.GetUTF8() );
	}
}

void
WinPlatformServices::SetPreference( const char *key, const char *value ) const
{
	if ( Rtt_VERIFY( key ) )
	{
        WinString strKey, strValue;
        strKey.SetUTF8( key );
        strValue.SetUTF8( value );
        WriteProfileString( Rtt_REGISTRY_SECTION, strKey.GetTCHAR(), strValue.GetTCHAR() );
	}
}


void
WinPlatformServices::GetSecurePreference( const char *key, String * value ) const
{
    const char *sResult = NULL;

	BYTE *aBytes;
	UINT nBytes;
    WinString strKey;
    strKey.SetUTF8( key );

    bool bFound = GetProfileBinary(Rtt_REGISTRY_SECTION, strKey.GetTCHAR(), &aBytes, &nBytes);

    if( bFound )
	{
        WinString strResult;
		if( DecryptString( &strResult, aBytes, nBytes ) )
		{
			value->Set( strResult.GetUTF8() );
		}
	}
}

bool
WinPlatformServices::SetSecurePreference( const char *key, const char *value ) const
{
	bool result = false;

	if ( Rtt_VERIFY( key ) )
	{
		BYTE *aBytes;
		UINT nBytes;
		EncryptString( value, &aBytes, &nBytes );

		WinString strKey;
		strKey.SetUTF8( key );

		result = WriteProfileBinary( Rtt_REGISTRY_SECTION, strKey.GetTCHAR(), aBytes, nBytes );

         if( aBytes )
			 delete aBytes;
    }
	return result;
}

void 
WinPlatformServices::GetLibraryPreference( const char *key, String * value ) const
{
	// Get the value from HKLM
	WinString strResult, strKey;
    strKey.SetUTF8( key );
    bool bFound = GetProfileString( &strResult, Rtt_REGISTRY_SECTION, strKey.GetTCHAR(),
                                    HKEY_LOCAL_MACHINE );

    if( bFound )
	{
		value->Set( strResult.GetUTF8() );
	}
}

void 
WinPlatformServices::SetLibraryPreference( const char *key, const char *value ) const
{
	// Set the value in HKLM
	if ( Rtt_VERIFY( key ) )
	{
        WinString strKey, strValue;
        strKey.SetUTF8( key );
        strValue.SetUTF8( value );
        WriteProfileString( Rtt_REGISTRY_SECTION, strKey.GetTCHAR(), strValue.GetTCHAR(),
                            HKEY_LOCAL_MACHINE );
	}
}

// Checks for Internet connectivity.
// Returns true if the Internet is assumed available. Returns false if not.
bool
WinPlatformServices::IsInternetAvailable() const
{
	MIB_IPFORWARDTABLE *pRoutingTable;
	DWORD dwBufferSize = 0;
	DWORD dwRowCount;
	DWORD dwIndex;
	DWORD dwResult;
	bool bIsInternetAvailable = false;

	// Fetch routing table information.
	// We'll assume that the Internet is available if we can find a default route to a gateway.
	GetIpForwardTable(NULL, &dwBufferSize, FALSE);
	pRoutingTable = (MIB_IPFORWARDTABLE*)new BYTE[dwBufferSize];
	dwResult = GetIpForwardTable(pRoutingTable, &dwBufferSize, FALSE);
	if (NO_ERROR == dwResult)
	{
		dwRowCount = pRoutingTable->dwNumEntries;
		for (dwIndex = 0; dwIndex < dwRowCount; dwIndex++)
		{
			// Default route designated by 0.0.0.0 in table.
			if (0 == pRoutingTable->table[dwIndex].dwForwardDest)
			{
				bIsInternetAvailable = true;
				break;
			}
		} 
	}
	delete pRoutingTable;

	// Return the result.
	return bIsInternetAvailable;
}

bool 
WinPlatformServices::IsLocalWifiAvailable() const
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
	return false;
}

void 
WinPlatformServices::Terminate() const
{
	// Exit the application.
	CWnd* windowPointer = ::AfxGetMainWnd();
	if (windowPointer)
	{
		SendMessage(windowPointer->GetSafeHwnd(), WM_CLOSE, 0, 0);
	}
}

void 
WinPlatformServices::Sleep( int milliseconds ) const
{
	if (milliseconds >= 0)
	{
		::Sleep( (DWORD)milliseconds );
	}
}

} // namespace Rtt
