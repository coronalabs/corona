//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_HTTPClient.h"

#include <Windows.h>
#include <wininet.h>
#include <stdio.h>
#include <fstream>
#include <cstring>
#include <shlwapi.h>
#include "..\Corona.Native.Library.Win32\WinString.h"

namespace Rtt {

	bool HTTPClient::HttpDownload(const char* url, Rtt::String& resultData, String& errorMesg, const std::map<std::string, std::string>& headers)
	{
		resultData.Set("");
		errorMesg.Set("");
		WinString canonUrl;
		{
			WinString wsUrl(url);
			if (wsUrl.IsEmpty()) {
				errorMesg.Set("Empty URL was passed");
				return false;
			}
			
			DWORD canonUrlLen = 1024*10-1;
			canonUrl.Expand(canonUrlLen + 1);
			if (!InternetCanonicalizeUrl(wsUrl.GetTCHAR(), canonUrl.GetBuffer(), &canonUrlLen, 0)) {
				errorMesg.Set("Unable to canonicalize URL.");
				return false;
			}
		}


		HINTERNET hInternet = InternetOpen( L"Mozilla/5.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (!hInternet) {
			errorMesg.Set("Unable to start Internet session!");
			return false;
		}
		std::unique_ptr< HINTERNET, void (*)(HINTERNET*) > internetCloser(&hInternet, [](auto* h) { Rtt_VERIFY(InternetCloseHandle(*h)); });

		WinString headerStr;
		for ( auto const & kv : headers)
		{
			if (!headerStr.IsEmpty()) headerStr.Append(L"\n");
			headerStr.Append(kv.first.c_str());
			headerStr.Append(L": ");
			headerStr.Append(kv.second.c_str());
		}
		
		HINTERNET hHttpFile = InternetOpenUrl(hInternet, canonUrl.GetTCHAR(), headerStr.GetTCHAR(), headerStr.GetLength(), 0, 0 );

		if (!hHttpFile) {
			DWORD bufSize = 3000;
			WCHAR effBuff[3001] = {};
			DWORD errorMessageID = ::GetLastError();
			DWORD err = 0;
			WinString errStr(L"Unable to open URL: ");

			if (InternetGetLastResponseInfo(&err, effBuff, &bufSize) && bufSize > 0) {
				errStr.Append(effBuff);
			}
			else {
				LPWSTR messageBuffer = nullptr;
				size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
					GetModuleHandle(TEXT("wininet.dll")), errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);
				if (messageBuffer) {
					errStr.Append(messageBuffer);
					LocalFree(messageBuffer);
				}
				else {
					errStr.Append(std::to_wstring(errorMessageID).c_str());
				}
			}
			errorMesg.Set(errStr.GetUTF8());
			errorMesg.Trim("\n\r");
			return false;
		}
		std::unique_ptr< HINTERNET, void (*)(HINTERNET*) > urlFileCloser(&hHttpFile, [](auto* h) { Rtt_VERIFY(InternetCloseHandle(*h)); });

		std::vector<char> result;
		char buf[1024];
		DWORD read = 0;
		while (true) {
			if (InternetReadFile(hHttpFile, buf, sizeof(buf), &read)) {
				if (read > 0) {
					result.insert(result.end(), buf, buf + read);
				}
				else {
					break;
				}
			}
			else {
				errorMesg.Set("Error while reading server response!");
				return false;
			}
		}
		result.push_back(0);
		resultData.Set(result.data());

		return true;
	}

	bool HTTPClient::HttpDownloadFile(const char* url, const char* filename, String& errorMesg, const std::map<std::string, std::string>& headers)
	{
		errorMesg.Set("");
		WinString canonUrl;
		{
			WinString wsUrl(url);
			if (wsUrl.IsEmpty()) {
				errorMesg.Set("Empty URL was passed");
				return false;
			}

			DWORD canonUrlLen = 1024 * 10 - 1;
			canonUrl.Expand(canonUrlLen + 1);
			if (!InternetCanonicalizeUrl(wsUrl.GetTCHAR(), canonUrl.GetBuffer(), &canonUrlLen, 0)) {
				errorMesg.Set("Unable to canonicalize URL.");
				return false;
			}
		}


		HINTERNET hInternet = InternetOpen(L"Mozilla/5.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (!hInternet) {
			errorMesg.Set("Unable to start Internet session!");
			return false;
		}
		std::unique_ptr< HINTERNET, void (*)(HINTERNET*) > internetCloser(&hInternet, [](auto* h) { Rtt_VERIFY(InternetCloseHandle(*h)); });

		WinString headerStr;
		for (auto const& kv : headers)
		{
			if (!headerStr.IsEmpty()) headerStr.Append(L"\n");
			headerStr.Append(kv.first.c_str());
			headerStr.Append(L": ");
			headerStr.Append(kv.second.c_str());
		}

		HINTERNET hHttpFile = InternetOpenUrl(hInternet, canonUrl.GetTCHAR(), headerStr.GetTCHAR(), headerStr.GetLength(), 0, 0);

		if (!hHttpFile) {
			DWORD bufSize = 3000;
			WCHAR effBuff[3001] = {};
			DWORD errorMessageID = ::GetLastError();
			DWORD err = 0;
			WinString errStr(L"Unable to open URL: ");

			if (InternetGetLastResponseInfo(&err, effBuff, &bufSize) && bufSize > 0) {
				errStr.Append(effBuff);
			}
			else {
				LPWSTR messageBuffer = nullptr;
				size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
					GetModuleHandle(TEXT("wininet.dll")), errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);
				if (messageBuffer) {
					errStr.Append(messageBuffer);
					LocalFree(messageBuffer);
				}
				else {
					errStr.Append(std::to_wstring(errorMessageID).c_str());
				}
			}
			errorMesg.Set(errStr.GetUTF8());
			errorMesg.Trim("\n\r");
			return false;
		}
		std::unique_ptr< HINTERNET, void (*)(HINTERNET*) > urlFileCloser(&hHttpFile, [](auto* h) { Rtt_VERIFY(InternetCloseHandle(*h)); });


		FILE* dst = Rtt_FileOpen(filename, "wb");
		if (!dst) {
			errorMesg.Set("Unable to open destination file!");
			return false;
		}
		std::unique_ptr< FILE, void (*)(FILE*) > dstCloser(dst, [](FILE* h) { 
			Rtt_VERIFY(Rtt_FileClose(h)==0); 
			});


		char buf[1024];
		DWORD read = 0;
		while (true) {
			if (InternetReadFile(hHttpFile, buf, sizeof(buf), &read)) {
				if (read > 0) {
					fwrite(buf, sizeof(char), read, dst);
				}
				else {
					break;
				}
			}
			else {
				errorMesg.Set("Error while reading server response!");
				return false;
			}
		}

		return true;
	}

}


