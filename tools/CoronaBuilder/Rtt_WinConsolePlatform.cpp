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

// ------------------------------------------------
// This is a stub to enable building CoronaBuilder
// without importing most of the Simulator code
// ------------------------------------------------

#include "stdafx.h"
#include "Rtt_WinConsolePlatform.h"
#include "Rtt_WinConsoleDevice.h"
#include "Core\Rtt_Build.h"
#include "CoronaLua.h"
#include "Rtt_Allocator.h"
#include "Rtt_FileSystem.h"
#include "Rtt_LuaContext.h"
#include "Rtt_PlatformInAppStore.h"
#include "Rtt_PreferenceCollection.h"
#include "Rtt_Runtime.h"
#include "Rtt_String.h"
#include "WinString.h"
#include <algorithm>
#include <io.h>
#include <shellapi.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <WinHttp.h>

#if !defined( Rtt_CUSTOM_CODE )
Rtt_EXPORT const luaL_Reg* Rtt_GetCustomModulesList()
{
	return nullptr;
}
#endif


namespace Rtt
{

	bool getLastErrorAsString(String& message)
	{
		//Get the error message, if any.
		DWORD errorMessageID = ::GetLastError();
		if (errorMessageID == 0)
		{
			return false; //No error message has been recorded
		}

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		std::string str(messageBuffer, size);
		message.Set(str.c_str());

		//Free the buffer.
		LocalFree(messageBuffer);
		return true;
	}

	const char *
	WinConsolePlatform::GetUtf8PathFor(MPlatform::Directory baseDir) const
	{
		// Validate argument.
		if ((baseDir < 0) || (baseDir >= Rtt::MPlatform::kNumDirs))
		{
			return nullptr;
		}

		// Fetch the given directory's path.
		return fDirectoryPaths[baseDir].GetUTF8();
	}

	bool CopySimulatorRootAppDataDirectoryPathTo(std::wstring& path)
	{
		// Fetch a path to the user's roaming app data directory.
		wchar_t appDataDirectoryPath[1024];
		appDataDirectoryPath[0] = L'\0';
		HRESULT result = ::SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appDataDirectoryPath);
		if (FAILED(result) || (L'\0' == appDataDirectoryPath[0]))
		{
			return false;
		}

		// Append the Corona Simulator's subdirectory path and copy it to the given argument.
		path = appDataDirectoryPath;
		path += L"\\Corona Labs\\Corona Simulator";
		return true;
	}

	bool CopySimulatorPluginDirectoryPathTo(std::wstring& path)
	{
		// Fetch the Corona Simulator root path under the user's AppData directory.
		std::wstring pluginPath;
		auto result = CopySimulatorRootAppDataDirectoryPathTo(pluginPath);
		if (!result || pluginPath.empty())
		{
			return false;
		}

		// Append the simulator's plugin subdirectory to the path and copy it to the given argument.
		path = pluginPath;
		path += L"\\Plugins";
		return true;
	}


WinConsolePlatform::WinConsolePlatform()
{
	WinString coronaSDKPath = GetDirectoryPath().c_str();
	WinString resourceDirectoryPath;

	resourceDirectoryPath.SetUTF8(coronaSDKPath.GetUTF8());
	resourceDirectoryPath.Append("Resources");

	TCHAR tempPath[MAX_PATH + 1];
	GetTempPath(MAX_PATH, tempPath);
	WinString temporaryDirectoryPath;
	temporaryDirectoryPath.Format("%SCoronaBuilder_%ld", tempPath, GetCurrentProcessId());

	std::wstring pluginsDirectoryPath;
	CopySimulatorPluginDirectoryPathTo(pluginsDirectoryPath);

	fDirectoryPaths[Rtt::MPlatform::kResourceDir].SetUTF16(resourceDirectoryPath.GetTCHAR());
	fDirectoryPaths[Rtt::MPlatform::kProjectResourceDir].SetUTF16(resourceDirectoryPath.GetTCHAR());
#ifdef Rtt_AUTHORING_SIMULATOR
	fDirectoryPaths[Rtt::MPlatform::kSystemResourceDir].SetUTF16(settings.SystemResourceDirectoryPath);
#else
	fDirectoryPaths[Rtt::MPlatform::kSystemResourceDir].SetUTF16(resourceDirectoryPath.GetTCHAR());
#endif
	//fDirectoryPaths[Rtt::MPlatform::kDocumentsDir].SetUTF16(settings.DocumentsDirectoryPath);
	fDirectoryPaths[Rtt::MPlatform::kTmpDir].SetUTF16(temporaryDirectoryPath.GetTCHAR());
	fDirectoryPaths[Rtt::MPlatform::kCachesDir].SetUTF16(temporaryDirectoryPath.GetTCHAR());
	fDirectoryPaths[Rtt::MPlatform::kSystemCachesDir].SetUTF16(temporaryDirectoryPath.GetTCHAR());
	fDirectoryPaths[Rtt::MPlatform::kPluginsDir].SetUTF16(pluginsDirectoryPath.c_str());
	//fDirectoryPaths[Rtt::MPlatform::kSkinResourceDir].SetUTF16(settings.SkinResourceDirectoryPath);
	//fDirectoryPaths[Rtt::MPlatform::kUserSkinsDir].SetUTF16(settings.UserSkinsDirectoryPath);

	fDevice = new WinConsoleDevice(*Rtt_AllocatorCreate());
}

WinConsolePlatform::~WinConsolePlatform()
{
	delete fDevice;
}

FontMetricsMap WinConsolePlatform::GetFontMetrics(const PlatformFont& font) const 
{
	Rtt_ASSERT_MSG( 0, "Code should NOT be reached" );
	return FontMetricsMap();
}

void WinConsolePlatform::GetSafeAreaInsetsPixels(Rtt_Real &top, Rtt_Real &left, Rtt_Real &bottom, Rtt_Real &right) const
{
}

Rtt_Allocator& WinConsolePlatform::GetAllocator() const
{
	return *Rtt_AllocatorCreate();
}

MPlatformDevice& WinConsolePlatform::GetDevice() const
{
	return *(fDevice);
}

const MCrypto& WinConsolePlatform::GetCrypto() const
{
	return fCrypto;
}

MPlatform::StatusBarMode WinConsolePlatform::GetStatusBarMode() const
{
	return kDefaultStatusBar;
}

bool WinConsolePlatform::FileExists(const char *filename) const
{
	return (Rtt_FileExists(filename) != 0);
}

void WinConsolePlatform::PathForFile(const char *filename, MPlatform::Directory baseDir, U32 flags, String &result) const
{
//#if 0
	// Default to the "Documents" directory if given an invalid base directory type.
	if ((baseDir < 0) || (baseDir >= MPlatform::kNumDirs))
	{
		baseDir = MPlatform::kDocumentsDir;
	}

	// Initialize result to an empty string in case the file was not found.
	result.Set(nullptr);

	// Fetch the absolute path for the given base directory type.
	const char *directoryPath = GetUtf8PathFor(baseDir);

	// Always check for file existence if referencing a resource file.
	if ((MPlatform::kResourceDir == baseDir) || (MPlatform::kSystemResourceDir == baseDir))
	{
		// Only check for existance if given a file name.
		// Note: If a file name was not provided, then the caller is fetching the path for the given base directory.
		if (Rtt_StringIsEmpty(filename) == false)
		{
			flags |= MPlatform::kTestFileExists;
		}
	}

	// Set the "result" to an absolute path for the given file.
	if (directoryPath)
	{
		result.Set(directoryPath);
		if (filename && (strlen(filename) > 0))
		{
			result.Append("\\");
			result.Append(filename);
		}
	}

	// Check if the file exists, if enabled.
	// Result will be set to an empty string if the file could not be found.
	WinString coronaResourceDirectoryPath;
	if (flags & MPlatform::kTestFileExists)
	{
		// Check if the given file name exists.
		bool doesFileExist = FileExists(result.GetString());
//TODO: Need a solution for widget resources. It might be better to embed these files within the Corona library.
#if defined(Rtt_AUTHORING_SIMULATOR)
		if ((false == doesFileExist) && ((MPlatform::kResourceDir == baseDir) || (MPlatform::kSystemResourceDir)))
		{
			// File not found. Since it is a resource file, check if it is installed under the Corona Simulator directory.
			coronaResourceDirectoryPath.SetUTF16(GetDirectoryPath());
			coronaResourceDirectoryPath.Append(L"\\Resources\\Corona");
			WinString coronaResourceFilePath(coronaResourceDirectoryPath.GetUTF16());
			coronaResourceFilePath.Append(L'\\');
			coronaResourceFilePath.Append(filename);
			doesFileExist = FileExists(coronaResourceFilePath.GetUTF8());
			if (doesFileExist)
			{
				result.Set(coronaResourceFilePath.GetUTF8());
				directoryPath = coronaResourceDirectoryPath.GetUTF8();
			}
		}
#endif

		if (false == doesFileExist)
		{
			// File not found. Return a null path.
			result.Set(nullptr);
		}
	}
//#endif
}

void WinConsolePlatform::GetPreference(Category category, Rtt::String * value) const
{
	// Validate.
	if (nullptr == value)
	{
		return;
	}

	// Fetch the requested preference value.
	char stringBuffer[MAX_PATH];
	const char *resultPointer = stringBuffer;
	DWORD dwValue = 0;
	stringBuffer[0] = '\0';
	switch (category)
	{
		case kLocaleLanguage:
			::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, stringBuffer, MAX_PATH);
			break;
		case kLocaleCountry:
			::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, stringBuffer, MAX_PATH);
			break;
		case kLocaleIdentifier:
		case kUILanguage:
		{
			// Fetch the ISO 639 language code with an ISO 15924 script code appended to it if available.
			// Note: This will return a 3 letter ISO 639-2 code if current language is not in the 2 letter ISO 639-1 standard.
			//       For example, this can happen with the Hawaiian language, which will return "haw".
			::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SPARENT, stringBuffer, MAX_PATH);
			if (stringBuffer[0] != '\0')
			{
				// Special handling for older OS versions.
				// Replace non-standard Chinese "CHS" and "CHT" script names with respective ISO 15924 codes.
				if (_stricmp(stringBuffer, "zh-chs") == 0)
				{
					strncpy_s(stringBuffer, MAX_PATH, "zh-Hans", MAX_PATH);
				}
				else if (_stricmp(stringBuffer, "zh-cht") == 0)
				{
					strncpy_s(stringBuffer, MAX_PATH, "zh-Hant", MAX_PATH);
				}
			}
			else
			{
				// Use an older API that only fetches the ISO 639 language code if the above API call fails.
				::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, stringBuffer, MAX_PATH);
			}

			// Fetch and append the ISO 3166 country code string to the language code string.
			// This is appended with an underscore '_' to be consistent with Apple and Android platforms.
			char countryCode[16];
			countryCode[0] = '\0';
			::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, countryCode, MAX_PATH);
			if (countryCode[0] != '\0')
			{
				auto stringBufferLength = strlen(stringBuffer);
				stringBuffer[stringBufferLength] = '_';
				stringBufferLength++;
				stringBuffer[stringBufferLength] = '\0';
				strcat_s(stringBuffer + stringBufferLength, MAX_PATH - stringBufferLength, countryCode);
			}
			break;
		}
#if 0
		case kSubscription:
			Interop::MDeviceSimulatorServices *deviceSimulatorServicesPointer;
			deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
			if (deviceSimulatorServicesPointer)
			{
				resultPointer = deviceSimulatorServicesPointer->GetAuthorizationTicketString();
			}
			break;
#endif
		default:
			resultPointer = nullptr;
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	// Copy the requested preference string to the given value.
	// Set's value to NULL (an empty string) if the given preference is not supported.
	value->Set(resultPointer);
}

Preference::ReadValueResult WinConsolePlatform::GetPreference(const char* categoryName, const char* keyName) const
{
	std::string message;
	message = "Category name '";
	message += categoryName ? categoryName : "";
	message += "' is not supported on this platform.";
	return Preference::ReadValueResult::FailedWith(message.c_str());
}

OperationResult WinConsolePlatform::SetPreferences(
	const char* categoryName, const PreferenceCollection& collection) const
{
	return OperationResult::FailedWith("This API is not supported on this platform.");
}

OperationResult WinConsolePlatform::DeletePreferences(
	const char* categoryName, const char** keyNameArray, U32 keyNameCount) const
{
	return OperationResult::FailedWith("This API is not supported on this platform.");
}

#if 0
int WinConsolePlatform::PushSystemInfo(lua_State *L, const char *key) const
{
	// Validate.
	if (!L)
	{
		return 0;
	}

	// Push the requested system information to Lua.
	int pushedValues = 0;
	if (Rtt_StringCompare(key, "appName") == 0)
	{
		// Fetch the application's name.
		WinString appName;
		CopyAppNameTo(appName);
		lua_pushstring(L, appName.GetUTF8());
		pushedValues = 1;
	}
	else if (Rtt_StringCompare(key, "appVersionString") == 0)
	{
		// Return an empty version string since it is unknown by the simulator.
		if (Interop::ApplicationServices::IsCoronaSdkApp())
		{
			lua_pushstring(L, "");
		}
		else
		{
			WinString stringConverter(Interop::ApplicationServices::GetFileVersionString());
			lua_pushstring(L, stringConverter.GetUTF8());
		}
		pushedValues = 1;
	}
	else if (Rtt_StringCompare(key, "isoCountryCode") == 0)
	{
		// Fetch the ISO 3166-1 country code.
		char stringBuffer[16];
		stringBuffer[0] = '\0';
		::GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, stringBuffer, sizeof(stringBuffer));
		lua_pushstring(L, stringBuffer);
		pushedValues = 1;
	}
	else if (Rtt_StringCompare(key, "isoLanguageCode") == 0)
	{
		// Fetch the ISO 639 language code with an ISO 15924 script code appended to it if available.
		// Note: This will return a 3 letter ISO 639-2 code if current language is not in the 2 letter ISO 639-1 standard.
		//       For example, this can happen with the Hawaiian language, which will return "haw".
		WinString languageCode;
		const size_t kUtf16StringBufferMaxLength = 128;
		wchar_t utf16StringBuffer[kUtf16StringBufferMaxLength];
		utf16StringBuffer[0] = L'\0';
		::GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SPARENT, utf16StringBuffer, kUtf16StringBufferMaxLength);
		if (utf16StringBuffer[0] != L'\0')
		{
			// Special handling for older OS versions.
			// Replace non-standard Chinese "chs" and "cht" script names with respective ISO 15924 codes.
			languageCode.SetUTF16(utf16StringBuffer);
			languageCode.MakeLowerCase();
			if (languageCode.Equals("zh-chs"))
			{
				languageCode.SetUTF16(L"zh-hans");
			}
			else if (languageCode.Equals("zh-cht"))
			{
				languageCode.SetUTF16(L"zh-hant");
			}
		}
		else
		{
			// Use an older API that only fetches the ISO 639 language code if the above API call fails.
			::GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, utf16StringBuffer, kUtf16StringBufferMaxLength);
			languageCode.SetUTF16(utf16StringBuffer);
		}
		languageCode.MakeLowerCase();
		lua_pushstring(L, languageCode.GetUTF8());
		pushedValues = 1;
	}
	else
	{
		// Push nil if given a key that is unknown on this platform.
		lua_pushnil(L);
		pushedValues = 1;
	}

	// Return the number of values pushed into Lua.
	return pushedValues;
}
#endif

void WinConsolePlatform::RuntimeErrorNotification(const char *errorType, const char *message, const char *stacktrace) const
{
	fprintf(stderr, "ERROR: %s - %s\n%s\n", errorType, message, stacktrace);
}

int WinConsolePlatform::RunSystemCommand(std::wstring command)
{
//	Rtt_Log("Running: %S\n", command.c_str());

	// system(3) on Win32 wants the whole command in double quotes
	std::wstring quotedCommand(L"\"");
	quotedCommand.append(command);
	quotedCommand.append(L"\"");

	return _wsystem(quotedCommand.c_str());
}

const std::wstring WinConsolePlatform::GetDirectoryPath()
{
	static std::wstring coronaSDKPath = _wgetenv(_T("CORONA_PATH"));

	return coronaSDKPath;
}

bool WinConsolePlatform::HttpDownloadBuffer(const char* url, std::vector<char>& result, String& errorMesg, const std::map<std::string, std::string>& headers) const
{
	std::wstring wurl(url, url + strlen(url));		// string ==> wstring
	result.clear();
	errorMesg.Set("");
	bool rc = false;

	// sync access
	HINTERNET hRequest = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET httpSession = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (httpSession)
	{
		URL_COMPONENTS urlInfo;
		ZeroMemory(&urlInfo, sizeof(urlInfo));
		urlInfo.dwStructSize = sizeof(urlInfo);

		// Set required component lengths to non-zero so that they are cracked.
		urlInfo.dwSchemeLength = (DWORD)-1;
		urlInfo.dwHostNameLength = (DWORD)-1;
		urlInfo.dwUrlPathLength = (DWORD)-1;
		urlInfo.dwExtraInfoLength = (DWORD)-1;

		// parse URL
		if (WinHttpCrackUrl(wurl.c_str(), wurl.size(), 0, &urlInfo))
		{
			// connect
			std::wstring hostName(urlInfo.lpszHostName, urlInfo.dwHostNameLength);
			hConnect = WinHttpConnect(httpSession, hostName.c_str(), urlInfo.nPort, 0);
			if (hConnect)
			{
				DWORD requestFlags = urlInfo.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0;
				// GET
				std::wstring resource(urlInfo.lpszUrlPath, urlInfo.dwUrlPathLength);
				std::wstring extraInfo(urlInfo.lpszExtraInfo, urlInfo.dwExtraInfoLength);
				hRequest = WinHttpOpenRequest(hConnect, L"GET", (resource+extraInfo).c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, requestFlags);
				if (hRequest)
				{
					// read result
					std::string headersAsString;
					for (const auto &it : headers)
					{
						headersAsString += it.first + ": " + it.second + "\r\n";
					}
					std::wstring wheadersAsString(headersAsString.c_str(), headersAsString.c_str() + headersAsString.size());

					if (WinHttpSendRequest(hRequest, wheadersAsString.c_str(), -1, WINHTTP_NO_REQUEST_DATA, 0, 0, NULL) == true)
					{
						// read response
						if (WinHttpReceiveResponse(hRequest, NULL) == true)
						{
							// Allocate space for the buffer.
							int bufsize = 4096;
							char* buf = (char*)malloc(bufsize + 1);		// ZERO ended

							DWORD readBytes = 0;
							while (WinHttpReadData(hRequest, buf, bufsize, &readBytes) == true && readBytes > 0)
							{
								result.insert(result.end(), buf, buf + readBytes);
							}

							free(buf);
							rc = true;
						}
					}
				}
			}
		}
	}

	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (httpSession) WinHttpCloseHandle(httpSession);

	return rc;
}

bool WinConsolePlatform::HttpDownload(const char* url, String& result, String& errorMesg, const std::map<std::string, std::string>& headers) const
{
	std::vector<char> data;
	bool rc = HttpDownloadBuffer(url, data, errorMesg, headers);
	if (rc)
	{
		data.push_back(0);
		result.Set(data.data());
	}
	else
	{
		result.Set("");
	}
	return rc; 
}

bool WinConsolePlatform::HttpDownloadFile(const char* url, const char* filename, String& errorMesg, const std::map<std::string, std::string>& headers) const
{
	std::vector<char> data;
	bool rc = HttpDownloadBuffer(url, data, errorMesg, headers);
	if (rc)
	{
		FILE *fp = Rtt_FileOpen(filename, "wb");
		if (fp != NULL)
		{
			size_t charsWritten = fwrite(data.data(), 1, data.size(), fp);
			Rtt_ASSERT(charsWritten == data.size());
		}
		Rtt_FileClose(fp);
	}
	return rc;
}


}	// namespace Rtt

