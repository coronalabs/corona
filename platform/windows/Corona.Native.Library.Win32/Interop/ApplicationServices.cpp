//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ApplicationServices.h"
#include <Shlwapi.h>


namespace Interop {

#pragma region Static Member Variables
std::wstring ApplicationServices::sProductName(L"");

std::wstring ApplicationServices::sCompanyName(L"");

std::wstring ApplicationServices::sFileVersionString(L"");

std::wstring ApplicationServices::sCopyrightString(L"");

#pragma endregion


#pragma region Constructors/Destructors
ApplicationServices::ApplicationServices()
{
}

#pragma endregion


#pragma region Public Static Functions
bool ApplicationServices::IsCoronaSdkApp()
{
#ifdef Rtt_AUTHORING_SIMULATOR
	// This is the Corona Simulator. Always return true.
	return true;
#else
	// Check if this Win32 app template is the Corona Shell.
	auto companyName = GetCompanyName();
	if (wcsstr(companyName, L"Corona Labs") >= 0)
	{
		auto exeFileName = GetExeFileNameWithoutExtension();
		if (!_wcsicmp(exeFileName, L"corona.shell") || !_wcsicmp(exeFileName, L"corona simulator"))
		{
			return true;
		}
	}
	return false;
#endif
}

const wchar_t* ApplicationServices::GetExeFileName()
{
	static std::wstring sExeFileName;

	// Fetch the executable file name, if not done already.
	if (sExeFileName.empty())
	{
		const size_t kMaxCharacters = 1024;
		wchar_t utf16Path[kMaxCharacters];
		DWORD utf16PathLength = ::GetModuleFileNameW(nullptr, utf16Path, kMaxCharacters);
		if (utf16PathLength > 0)
		{
			::PathStripPathW(utf16Path);
			sExeFileName = utf16Path;
		}
	}

	// Return the file name.
	return sExeFileName.c_str();
}

const wchar_t* ApplicationServices::GetExeFileNameWithoutExtension()
{
	static std::wstring sExeFileNameWithoutExtension;

	// Fetch the executable file name, if not done already.
	if (sExeFileNameWithoutExtension.empty())
	{
		const size_t kMaxCharacters = 1024;
		wchar_t utf16Path[kMaxCharacters];
		DWORD utf16PathLength = ::GetModuleFileNameW(nullptr, utf16Path, kMaxCharacters);
		if (utf16PathLength > 0)
		{
			::PathStripPathW(utf16Path);
			::PathRemoveExtensionW(utf16Path);
			sExeFileNameWithoutExtension = utf16Path;
		}
	}

	// Return the file name.
	return sExeFileNameWithoutExtension.c_str();
}

const wchar_t* ApplicationServices::GetDirectoryPath()
{
	static std::wstring sAppDirectoryPath;

	// Fetch the executable's directory path, if not done already.
	// Note: This path excludes the exe file name and trailing slash.
	if (sAppDirectoryPath.empty())
	{
		const size_t kMaxCharacters = 1024;
		wchar_t utf16Path[kMaxCharacters];
		DWORD utf16PathLength = ::GetModuleFileNameW(nullptr, utf16Path, kMaxCharacters);
		if (utf16PathLength > 0)
		{
			::PathRemoveFileSpecW(utf16Path);
			sAppDirectoryPath = utf16Path;
		}
	}

	// Return an absolute path the application's directory in UTF-16 form.
	return sAppDirectoryPath.c_str();
}

const wchar_t* ApplicationServices::GetProductName()
{
	ApplicationServices::FetchVersionInfo();
	return sProductName.c_str();
}

const wchar_t* ApplicationServices::GetCompanyName()
{
	ApplicationServices::FetchVersionInfo();
	return sCompanyName.c_str();
}

const wchar_t* ApplicationServices::GetFileVersionString()
{
	ApplicationServices::FetchVersionInfo();
	return sFileVersionString.c_str();
}

const wchar_t* ApplicationServices::GetCopyrightString()
{
	ApplicationServices::FetchVersionInfo();
	return sCopyrightString.c_str();
}

#pragma endregion


#pragma region Private Static Functions
void ApplicationServices::FetchVersionInfo()
{
	// Do not continue if the executable file's version info was already fetched.
	static bool sWasFetched = false;
	if (sWasFetched)
	{
		return;
	}
	sWasFetched = true;

	// Fetch the version info resource from the executable file.
	LPVOID versionInfoResourcePointer = nullptr;
	{
		auto resourceHandle = ::FindResourceW(nullptr, MAKEINTRESOURCEW(VS_VERSION_INFO), RT_VERSION);
		if (resourceHandle)
		{
			auto resourceDataHandle = ::LoadResource(nullptr, resourceHandle);
			if (resourceDataHandle)
			{
				versionInfoResourcePointer = ::LockResource(resourceDataHandle);
			}
		}
	}
	if (!versionInfoResourcePointer)
	{
		return;
	}

	// Fetch an array of [LanguageId,CodePageId] objects.
	struct TranslationInfo
	{
		WORD wLanguageId;
		WORD wCodePageId;
	} *translationInfoArray = nullptr;
	UINT translationInfoArrayByteLength = 0;
	::VerQueryValueW(
			versionInfoResourcePointer, L"\\VarFileInfo\\Translation",
			(LPVOID*)&translationInfoArray, &translationInfoArrayByteLength);

	// Traverse each localized version info structure.
	int translationInfoCount = translationInfoArrayByteLength / sizeof(TranslationInfo);
	for (int index = 0; index < translationInfoCount; index++)
	{
		// Generate the root sub-block entry name for this localized entry.
		const size_t kMaxSubBlockRootNameLength = 32;
		wchar_t subBlockRootName[kMaxSubBlockRootNameLength];
		subBlockRootName[0] = L'\0';
		_snwprintf_s(
				subBlockRootName, kMaxSubBlockRootNameLength, L"\\StringFileInfo\\%04x%04x\\",
				translationInfoArray[index].wLanguageId, translationInfoArray[index].wCodePageId);

		// Fetch product name.
		{
			BYTE* bufferPointer = nullptr;
			UINT bufferByteLength = 0;
			std::wstring subBlockName(subBlockRootName);
			subBlockName.append(L"ProductName");
			::VerQueryValueW(versionInfoResourcePointer, subBlockName.c_str(), (LPVOID*)&bufferPointer, &bufferByteLength);
			if (bufferPointer && (bufferByteLength > 0))
			{
				sProductName = (wchar_t*)bufferPointer;
			}
		}

		// Fetch company name.
		{
			BYTE* bufferPointer = nullptr;
			UINT bufferByteLength = 0;
			std::wstring subBlockName(subBlockRootName);
			subBlockName.append(L"CompanyName");
			::VerQueryValueW(versionInfoResourcePointer, subBlockName.c_str(), (LPVOID*)&bufferPointer, &bufferByteLength);
			if (bufferPointer && (bufferByteLength > 0))
			{
				sCompanyName = (wchar_t*)bufferPointer;
			}
		}

		// Fetch file version string.
		{
			BYTE* bufferPointer = nullptr;
			UINT bufferByteLength = 0;
			std::wstring subBlockName(subBlockRootName);
			subBlockName.append(L"FileVersion");
			::VerQueryValueW(versionInfoResourcePointer, subBlockName.c_str(), (LPVOID*)&bufferPointer, &bufferByteLength);
			if (bufferPointer && (bufferByteLength > 0))
			{
				sFileVersionString = (wchar_t*)bufferPointer;
			}
		}

		// Fetch copyright string.
		{
			BYTE* bufferPointer = nullptr;
			UINT bufferByteLength = 0;
			std::wstring subBlockName(subBlockRootName);
			subBlockName.append(L"LegalCopyright");
			::VerQueryValueW(versionInfoResourcePointer, subBlockName.c_str(), (LPVOID*)&bufferPointer, &bufferByteLength);
			if (bufferPointer && (bufferByteLength > 0))
			{
				sCopyrightString = (wchar_t*)bufferPointer;
			}
		}
	}
}

#pragma endregion

}	// namespace Interop
