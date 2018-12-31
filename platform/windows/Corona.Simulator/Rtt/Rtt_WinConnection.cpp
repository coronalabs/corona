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

#include "stdafx.h"

#include "Core/Rtt_Build.h"

#include <WinInet.h>

#include "WinString.h"

#include "TimXmlRpc.h"  // downloaded from http://sourceforge.net/projects/xmlrpcc4win/
#include "Rtt_WinConnection.h"

#include "Rtt_MPlatformServices.h"
#include "Rtt_WinDictionaryWrapper.h"

#include <string.h>
#include <time.h>

#include "SimulatorView.h"
#include "WinGlobalProperties.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

static String &
DictionaryToString( const KeyValuePair *pairs, int numPairs, int indent = 0 )
{
	static String output;
	char buf[1024];

	output.Set(" {\n");

 	for ( int i = 0; i < numPairs; i++ )
	{
		const KeyValuePair& item = pairs[i];
		String str;

		if ( item.type <= 0 )
		{
			switch( item.type )
			{
				case kStringValueType:
					if (item.value != NULL && strlen((const char *)item.value) > 1024)
					{
						snprintf(buf, 1024, "\"%s\" = <%d bytes of data elided>", item.key, strlen((const char *)item.value));
					}
					else
					{
						snprintf(buf, 1024, "\"%s\" = \"%s\"", item.key, (item.value != NULL ? (const char *)item.value : "(null)"));
					}
					break;
				case kIntegerValueType:
					snprintf(buf, 1024, "\"%s\" = %d", item.key, (S32)item.value);
					break;
				default:
					Rtt_ASSERT_NOT_REACHED();
					break;
			}

			if (indent > 0)
			{
				for (int in = 0; in < (indent * 4); in++)
				{
					output.Append(" ");
				}
			}

			output.Append(buf);
			output.Append("\n");
		}
		else  // value is an array/struct of values
		{
			output.Append(DictionaryToString( (const KeyValuePair*)item.value, item.type, ++indent ).GetString());
		}
	}

	output.Append("}\n");

	return output;
}

// ----------------------------------------------------------------------------

WinConnection::WinConnection( const MPlatformServices& platform, const char* url )
:	Super( platform ),
	m_sUrl( NULL ),
	m_sError( NULL ),
	m_connection( NULL ),
	m_hInternetFile( NULL )
{
	int length = strlen(url) + 1;
	m_sUrl = new char [ length ];
	strcpy_s( m_sUrl, length, url );

    String prefVal;
    fPlatform.GetPreference("debugWebServices", &prefVal);
    if (! prefVal.IsEmpty())
    {
        fDebugWebServices = (int) strtol(prefVal.GetString(), (char **)NULL, 10);
    }
    else
    {
        fDebugWebServices = 0;
    }
}

WinConnection::~WinConnection()
{
    if( m_sUrl )
        delete m_sUrl;
    if( m_sError )
        delete m_sError;
}

// Caller responsible for deleting returned value
PlatformDictionaryWrapper*
WinConnection::Call( const char* method, const KeyValuePair* pairs, int numPairs )
{
	time_t startTime = 0;

	m_connection = new XmlRpcClient(m_sUrl);
//	m_connection.setIgnoreCertificateAuthority();  // TODO: need this?
	m_connection->setIgnoreCookies(true);  // Cookies cause login problems.

    XmlRpcValue *pResult = new XmlRpcValue;  // deleted by caller
    XmlRpcValue *pParams = CreateDictionary( pairs, numPairs );  // deleted below

    PlatformDictionaryWrapper *pWrapper = NULL;

	if (fDebugWebServices > 0)
	{
        Rtt_LogException("WebServices: Call: %s %s", method, ((numPairs == 0 && fDebugWebServices > 1) ? "(no params)" : ""));

		if (numPairs > 0 && fDebugWebServices > 1)
		{
			Rtt_LogException("           params: %s", DictionaryToString(pairs, numPairs, 1).GetString());
		}

		startTime = time(NULL);
	}

    if ( ! m_connection->execute( method, *pParams, *pResult ) )
	{
		std::string sErr = m_connection->getError();
		m_sError = new char [sErr.size()+1];
		strcpy_s(m_sError, sErr.size()+1, sErr.c_str());

		Rtt_LogException("WebServices: Call %s failed in %lld seconds: %s", method, (time(NULL) - startTime), m_sError);
		Rtt_LogException("     params: %s", DictionaryToString(pairs, numPairs, 1).GetString());

        delete pResult;  // will return NULL to signal error
	}
	else
	{
		if (pResult != NULL)
		{
			if (fDebugWebServices > 2)
			{
				std::ostringstream ostr;

				pResult->buildCall(method, ostr);

				if (ostr.str().size() > 2048)
				{
					Rtt_LogException("WebServices: Response received in %lld seconds: <%d bytes of data elided>",
						(time(NULL) - startTime), ostr.str().size());
				}
				else
				{
					Rtt_LogException("WebServices: Response received in %lld seconds: %s", (time(NULL) - startTime), ostr.str().c_str());
				}
			}
			else if (fDebugWebServices > 0)
			{
				Rtt_LogException("WebServices: Response received in %lld seconds", (time(NULL) - startTime));
			}

			pWrapper = new WinDictionaryWrapper( pResult );
		}
		else
		{
			Rtt_LogException("WebServices: Unexpected NULL response from XMLRPC");
		}
	}

	delete pParams;

	delete m_connection;
	m_connection = NULL;

	return pWrapper;
}

void
WinConnection::CloseConnection()
{
	if (m_connection != NULL)
	{
		// Stop any active XML request
		m_connection->closeConnection();
	}

	if (m_hInternetFile != NULL)
	{
		// Stop any active download
		InternetCloseHandle(m_hInternetFile);
	}
}

// Function needs to return an allocated value because it can be called recursively
// for nested values.
// Caller is responsible for deleting returned object.
XmlRpcValue *
WinConnection::CreateDictionary( const KeyValuePair *pairs, int numPairs )
{
    XmlRpcValue *pParams = new XmlRpcValue;   // type will be array of (unnamed) values

    // XmlRpc code expects "Type invalid" (uninitialized) param structure if no params.
    if( numPairs == 0 )
	{
         return pParams;
	}

	Rtt_ASSERT( numPairs > 0 );

    pParams->setSize( numPairs );  // array of size numPairs

    // iterate through KeyValuePair array and translate to XmlRpcValue array
    // keys are ignored
	for ( int i = 0; i < numPairs; i++ )
	{
		const KeyValuePair& item = pairs[i];

		if ( item.type <= 0 )
		{
			switch( item.type )
			{
				case kStringValueType:
					(*pParams)[i] = (const char *)item.value;
					break;
				case kIntegerValueType:
					(*pParams)[i] = (S32)item.value;
					break;
				default:
					Rtt_ASSERT_NOT_REACHED();
					(*pParams)[i] = "";
					break;
			}
		}
		else  // value is an array/struct of values
		{
			(*pParams)[i] = * CreateDictionary( (const KeyValuePair*)item.value, item.type );
		}
	}

	return pParams;
}

const char*
WinConnection::Error() const
{
	return m_sError;
}

const char*
WinConnection::Url() const
{
	return m_sUrl;
}

void
WinConnection::ReportError(const char *mesg)
{
	auto errorCode = ::GetLastError();
	std::string message(mesg);

	if (errorCode != ERROR_SUCCESS)
	{
		LPWSTR utf16Buffer = nullptr;

		HMODULE hModule = NULL; // default to system source
		DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_FROM_SYSTEM;

		// If dwLastError is in the network range, grab the WinINET message source
		if (errorCode >= INTERNET_ERROR_BASE && errorCode <= INTERNET_ERROR_LAST)
		{
			DWORD flags =
				GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
				GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;

			::GetModuleHandleExW(flags, (LPCWSTR)InternetOpenUrl, &hModule);

			if (hModule != NULL)
			{
				dwFormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
			}
		}

		DWORD dwResult = FormatMessageW(
			dwFormatFlags,
			hModule,
			errorCode,
			0, // MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPWSTR) &utf16Buffer,
			0,
			nullptr);

		if (utf16Buffer != nullptr)
		{
			WinString stringConverter;
			stringConverter.SetUTF16(utf16Buffer);
			message.append(": ");
			message.append(stringConverter.GetUTF8());
			::LocalFree(utf16Buffer);
		}
	}

	m_sError = _strdup(message.c_str());
	Rtt_LogException("%s", message.c_str());
}

bool
WinConnection::Download(const char *urlStr, const char *filename)
{
	WinString urlWinString = urlStr;
	WinString filenameWinString = filename;
	DWORD numberOfBytesToRead = (BUFSIZ * 8);
	BYTE buf[BUFSIZ * 8];
	DWORD numberOfBytesRead = 0;
	DWORD numberOfBytesWritten = 0;
	DWORD totalBytesReceived = 0;
	time_t startTime = time(NULL);
	HWND progressHWnd = GetForegroundWindow();

	if (fDebugWebServices > 1)
	{
		Rtt_LogException("WebServices: download request: %s", urlStr);
		Rtt_LogException("WebServices:          to file: %s", filename);
	}
	else
	{
		Rtt_LogException("WebServices: downloading build");
	}

	HINTERNET hInternetRoot = InternetOpen(
		TEXT("UserAgent"),				// _In_ LPCTSTR lpszAgent,
		INTERNET_OPEN_TYPE_PRECONFIG,	// _In_ DWORD   dwAccessType,
		NULL,							// _In_ LPCTSTR lpszProxyName,
		NULL,							// _In_ LPCTSTR lpszProxyBypass,
		0								// _In_ DWORD   dwFlags
		);

	if (hInternetRoot == NULL)
	{
		ReportError("ERROR: download failed to initialize network");

		return false;
	}

	m_hInternetFile = InternetOpenUrl(
		hInternetRoot,					// _In_ HINTERNET hInternet,
		urlWinString.GetTCHAR(),		// _In_ LPCTSTR   lpszUrl,
		NULL,							// _In_ LPCTSTR   lpszHeaders,
		-1L,							// _In_ DWORD     dwHeadersLength,
		INTERNET_FLAG_HYPERLINK | INTERNET_FLAG_NO_CACHE_WRITE, // _In_ DWORD     dwFlags,
		NULL							// _In_ DWORD_PTR dwContext
		);

	if (m_hInternetFile == NULL)
	{
		ReportError("ERROR: download failed to open URL");

		return false;
	}

	HANDLE hOut = CreateFile(filenameWinString.GetTCHAR(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
	if (hOut == INVALID_HANDLE_VALUE)
	{
		InternetCloseHandle(m_hInternetFile);
		InternetCloseHandle(hInternetRoot);
		return false;
	}

	if (m_hInternetFile == NULL)
	{
		ReportError("ERROR: download failed to open output file");

		return false;
	}
	DWORD contentLength = 0;
	DWORD bufSize = sizeof(DWORD);
	if (HttpQueryInfo(m_hInternetFile, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &contentLength, &bufSize, 0))
	{
		if (fDebugWebServices > 1)
		{
			Rtt_Log("WebServices: downloading %d bytes", contentLength);
		}

		if (progressHWnd != NULL)
		{
			::SendMessage(progressHWnd, WMU_DOWNLOAD_PROGRESS_UPDATE, 0, contentLength);
		}
	}

	do
	{
		if ( !InternetReadFile(
			m_hInternetFile,					// _In_  HINTERNET hFile,
			buf,							// _Out_ LPVOID    lpBuffer,
			numberOfBytesToRead,			// _In_  DWORD     dwNumberOfBytesToRead,
			&numberOfBytesRead				// _Out_ LPDWORD   lpdwNumberOfBytesRead
			))
		{
			ReportError("ERROR: download failed to read data");

			CloseHandle(hOut);
			InternetCloseHandle(m_hInternetFile);
			InternetCloseHandle(hInternetRoot);
			return false;
		}

		WriteFile(hOut, &buf[0], numberOfBytesRead, &numberOfBytesWritten, NULL);

		if (numberOfBytesWritten != numberOfBytesRead)
		{
			ReportError("ERROR: download failed to write data");

			CloseHandle(hOut);
			InternetCloseHandle(m_hInternetFile);
			InternetCloseHandle(hInternetRoot);
			return false;
		}

		totalBytesReceived += numberOfBytesRead;

		if (progressHWnd != NULL)
		{
			::SendMessage(progressHWnd, WMU_DOWNLOAD_PROGRESS_UPDATE, 1, totalBytesReceived);
		}

	} while (numberOfBytesRead > 0);

	CloseHandle(hOut);

	InternetCloseHandle(m_hInternetFile);
	m_hInternetFile = NULL;

	InternetCloseHandle(hInternetRoot);

	if (fDebugWebServices > 0)
	{
		time_t totalTime = time(NULL) - startTime;
		double MBps = ((double)totalBytesReceived / totalTime) / (1024 * 1024);

		Rtt_LogException("WebServices: Download of %ld bytes complete in %lld second%s (%.2fMB/s)",
			totalBytesReceived, totalTime, (totalTime == 1 ? "" : "s"), MBps);

		totalBytesReceived = 0;
	}

	return true;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

