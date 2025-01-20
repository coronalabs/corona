//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RegistryStoredPreferences.h"
#include "Core\Rtt_Assert.h"
#include "Rtt_Preference.h"
#include "Rtt_PreferenceCollection.h"
#include "StoredPreferencesProxy.h"
#include <memory>
#include <sstream>
#include <WinReg.h>


namespace Interop { namespace Storage {

#pragma region Public Constants
const wchar_t* RegistryStoredPreferences::kAnscaCoronaKeyName = L"Ansca Corona";
const wchar_t* RegistryStoredPreferences::kCoronaSimulatorKeyName = L"Corona Simulator";
const wchar_t* RegistryStoredPreferences::kCoronaLabsKeyName = L"Corona Labs";
const wchar_t* RegistryStoredPreferences::kCoronaSdkKeyName = L"Corona SDK";
const wchar_t* RegistryStoredPreferences::kSimulatorProfileName = RegistryStoredPreferences::kAnscaCoronaKeyName;
const wchar_t* RegistryStoredPreferences::kSimulatorPreferencesKeyName = L"Preferences";

#pragma endregion


#pragma region Constructors/Destructors
RegistryStoredPreferences::RegistryStoredPreferences(const RegistryStoredPreferences::CreationSettings& settings)
:	fBaseRegistryPath(settings.BaseRegistryPath),
	fWow64ViewType(settings.Wow64ViewType),
	fIsUsingForwardSlashAsPathSeparator(settings.IsUsingForwardSlashAsPathSeparator)
{
	// If the given base path has a trailing slash, then remove it.
	fBaseRegistryPath.TrimEnd(L"\\");
	if (fIsUsingForwardSlashAsPathSeparator)
	{
		fBaseRegistryPath.TrimEnd(L"/");
	}
}

RegistryStoredPreferences::~RegistryStoredPreferences()
{
}

#pragma endregion


#pragma region Public Methods
const wchar_t* RegistryStoredPreferences::GetBaseRegistryPath() const
{
	auto stringPointer = fBaseRegistryPath.GetUTF16();
	return stringPointer ? stringPointer : L"";
}

RegistryStoredPreferences::Wow64ViewType RegistryStoredPreferences::GetWow64ViewType() const
{
	return fWow64ViewType;
}

bool RegistryStoredPreferences::IsUsingForwardSlashAsPathSeparator() const
{
	return fIsUsingForwardSlashAsPathSeparator;
}

Rtt::Preference::ReadValueResult RegistryStoredPreferences::Fetch(const char* keyName)
{
	// Validate.
	if (!keyName || ('\0' == keyName[0]))
	{
		return Rtt::Preference::ReadValueResult::FailedWith("Preference key cannot be null.");
	}

	// Build an absolute registry path.
	WinString registryPath(fBaseRegistryPath);
	if (registryPath.IsEmpty() == false)
	{
		if ((keyName[0] != '\\') && (!fIsUsingForwardSlashAsPathSeparator || (keyName[0] != '/')))
		{
			registryPath.Append('\\');
		}
	}
	registryPath.Append(keyName);
	if (fIsUsingForwardSlashAsPathSeparator)
	{
		registryPath.Replace("/", "\\");
	}

	// Extract the registry components (ie: hive key, key path, value name) from the above registry path.
	auto extractionResult = ExtractComponentsFromRegistryPath(registryPath.GetUTF16());
	if (extractionResult.HasFailed())
	{
		return Rtt::Preference::ReadValueResult::FailedWith(extractionResult.GetUtf8MessageAsSharedPointer());
	}
	auto registryPathComponents = extractionResult.GetValue();

	// Set up the registry reading flags.
	REGSAM flags = KEY_READ;
	if (RegistryStoredPreferences::Wow64ViewType::k32Bit == fWow64ViewType)
	{
		flags |= KEY_WOW64_32KEY;
	}
	else if (RegistryStoredPreferences::Wow64ViewType::k64Bit == fWow64ViewType)
	{
		flags |= KEY_WOW64_64KEY;
	}

	// Open the registry key, if it exists.
	HKEY keyHandle = 0;
	const auto openResult = ::RegOpenKeyExW(
			registryPathComponents.HiveKeyHandle, registryPathComponents.KeyPath->c_str(), 0, flags, &keyHandle);
	if (ERROR_FILE_NOT_FOUND == openResult)
	{
		return Rtt::Preference::ReadValueResult::kPreferenceNotFound;
	}
	if ((openResult != ERROR_SUCCESS) || (0 == keyHandle))
	{
		WinString errorMessage;
		if (openResult != ERROR_SUCCESS)
		{
			LPWSTR utf16Buffer;
			::FormatMessageW(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					nullptr, openResult,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPWSTR)&utf16Buffer, 0, nullptr);
			if (utf16Buffer)
			{
				errorMessage.SetUTF16(utf16Buffer);
				::LocalFree(utf16Buffer);
			}
		}
		if (errorMessage.IsEmpty())
		{
			errorMessage.SetUTF16(L"Unknown error occurred while opening registry key.");
		}
		return Rtt::Preference::ReadValueResult::FailedWith(errorMessage.GetUTF8());
	}

	// Attempt to fetch information about the registry value, if it exists.
	DWORD valueType = REG_NONE;
	DWORD valueByteCount = 0;
	auto queryResult = ::RegQueryValueExW(
			keyHandle, registryPathComponents.ValueName->c_str(), nullptr, &valueType, nullptr, &valueByteCount);
	if (ERROR_FILE_NOT_FOUND == queryResult)
	{
		return Rtt::Preference::ReadValueResult::kPreferenceNotFound;
	}
	if ((queryResult != ERROR_SUCCESS) || (REG_NONE == valueType) || (valueByteCount <= 0))
	{
		WinString errorMessage;
		if (queryResult != ERROR_SUCCESS)
		{
			LPWSTR utf16Buffer;
			::FormatMessageW(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					nullptr, queryResult,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPWSTR)&utf16Buffer, 0, nullptr);
			if (utf16Buffer)
			{
				errorMessage.SetUTF16(utf16Buffer);
				::LocalFree(utf16Buffer);
			}
		}
		if (errorMessage.IsEmpty())
		{
			errorMessage.SetUTF16(L"Unknown error occurred while querying registry value.");
		}
		::RegCloseKey(keyHandle);
		return Rtt::Preference::ReadValueResult::FailedWith(errorMessage.GetUTF8());
	}

	// Copy the registry value to a PreferenceValue object.
	bool wasSuccessful = false;
	WinString errorMessage;
	Rtt::PreferenceValue preferenceValue;
	switch (valueType)
	{
		case REG_DWORD:
		case REG_DWORD_BIG_ENDIAN:
		{
			U32 value = 0;
			queryResult = ::RegQueryValueExW(
					keyHandle, registryPathComponents.ValueName->c_str(), nullptr,
					&valueType, (LPBYTE)&value, &valueByteCount);
			if (ERROR_SUCCESS == queryResult)
			{
				if (REG_DWORD_BIG_ENDIAN == valueType)
				{
					value = _byteswap_ulong(value);
				}
				preferenceValue = Rtt::PreferenceValue(value);
				wasSuccessful = true;
			}
			else
			{
				errorMessage.SetUTF16(L"Failed to fetch registry value type 'REG_DWORD'.");
			}
			break;
		}
		case REG_QWORD:
		{
			U64 value = 0;
			queryResult = ::RegQueryValueExW(
					keyHandle, registryPathComponents.ValueName->c_str(), nullptr,
					&valueType, (LPBYTE)&value, &valueByteCount);
			if (ERROR_SUCCESS == queryResult)
			{
				preferenceValue = Rtt::PreferenceValue(value);
				wasSuccessful = true;
			}
			else
			{
				errorMessage.SetUTF16(L"Failed to fetch registry value type 'REG_QWORD'.");
			}
			break;
		}
		case REG_SZ:
		case REG_MULTI_SZ:
		case REG_EXPAND_SZ:
		{
			int characterCount = valueByteCount / sizeof(wchar_t);
			WinString stringBuffer;
			stringBuffer.Expand(characterCount);
			queryResult = ::RegQueryValueExW(
					keyHandle, registryPathComponents.ValueName->c_str(), nullptr,
					&valueType, (LPBYTE)stringBuffer.GetBuffer(), &valueByteCount);
			if (ERROR_SUCCESS == queryResult)
			{
				if (L'\0' == stringBuffer.GetBuffer()[characterCount - 1])
				{
					characterCount--;
				}
				auto sharedStringPointer = Rtt_MakeSharedConstStdStringPtr(stringBuffer.GetUTF8(), characterCount);
				preferenceValue = Rtt::PreferenceValue(sharedStringPointer);
				wasSuccessful = true;
			}
			else
			{
				errorMessage.SetUTF16(L"Failed to fetch string registry value type.");
			}
			break;
		}
		case REG_BINARY:
		{
			auto stringBuffer = new char[valueByteCount];
			if (stringBuffer)
			{
				queryResult = ::RegQueryValueExW(
						keyHandle, registryPathComponents.ValueName->c_str(), nullptr,
						&valueType, (LPBYTE)stringBuffer, &valueByteCount);
				if (ERROR_SUCCESS == queryResult)
				{
					auto sharedStringPointer = Rtt_MakeSharedConstStdStringPtr(stringBuffer, valueByteCount);
					preferenceValue = Rtt::PreferenceValue(sharedStringPointer);
					wasSuccessful = true;
				}
				else
				{
					errorMessage.SetUTF16(L"Failed to fetch registry value type 'REG_BINARY'.");
				}
				delete[] stringBuffer;
			}
			else
			{
				errorMessage.SetUTF16(L"Failed to allocate byte buffer for registry value type 'REG_BINARY'.");
			}
			break;
		}
		default:
		{
			std::wstringstream stringStream;
			stringStream.imbue(std::locale::classic());
			stringStream << L"Failed to read unsupported registry value type ID: ";
			stringStream << valueType;
			errorMessage.SetUTF16(stringStream.str().c_str());
			break;
		}
	}

	// Close the registry key.
	::RegCloseKey(keyHandle);

	// Do not continue if there was an error reading the registry value.
	if (!wasSuccessful)
	{
		if (errorMessage.IsEmpty())
		{
			errorMessage = L"Unknown error occurred fetching registry value.";
		}
		return Rtt::Preference::ReadValueResult::FailedWith(errorMessage.GetUTF8());
	}

	// Return the read registry value.
	return Rtt::Preference::ReadValueResult::SucceededWith(preferenceValue);
}

Rtt::OperationResult RegistryStoredPreferences::UpdateWith(const Rtt::Preference& preference)
{
	Rtt::PreferenceCollection collection;
	collection.Add(preference);
	return UpdateWith(collection);
}

Rtt::OperationResult RegistryStoredPreferences::UpdateWith(const Rtt::PreferenceCollection& preferences)
{
	// Do not continue if the collection is empty.
	if (preferences.GetCount() <= 0)
	{
		return Rtt::OperationResult::kSucceeded;
	}

	// Attempt to insert/update all of the given preferences to the registry.
	WinString errorMessage;
	int preferencesWritten = 0;
	for (int index = 0; index < preferences.GetCount(); index++)
	{
		// Fetch the next preference to write to the registry.
		auto preferencePointer = preferences.GetByIndex(index);
		if (!preferencePointer)
		{
			continue;
		}

		// Convert the preference value to a type supported by the registry.
		Rtt::PreferenceValue::TypeSet supportedTypes;
		supportedTypes.Add(Rtt::PreferenceValue::kTypeUnsignedInt32);
		supportedTypes.Add(Rtt::PreferenceValue::kTypeUnsignedInt64);
		supportedTypes.Add(Rtt::PreferenceValue::kTypeString);
		auto valueConversionResult = preferencePointer->GetValue().ToClosestValueTypeIn(supportedTypes);
		if (valueConversionResult.HasFailed())
		{
			if (errorMessage.IsEmpty() == false)
			{
				errorMessage.Append(L'\n');
			}
			errorMessage.Append(L"Failed to convert preference '");
			errorMessage.Append(preferencePointer->GetKeyName());
			errorMessage.Append(L"' value to a type that the registry supports.\n- Reason: ");
			if (Rtt_StringIsEmpty(valueConversionResult.GetUtf8Message()))
			{
				errorMessage.Append(L"Unknown");
			}
			else
			{
				errorMessage.Append(valueConversionResult.GetUtf8Message());
			}
			continue;
		}

		// Build an absolute registry path.
		WinString registryPath(fBaseRegistryPath);
		if (registryPath.IsEmpty() == false)
		{
			auto keyName = preferencePointer->GetKeyName();
			if ((keyName[0] != L'\\') && (!fIsUsingForwardSlashAsPathSeparator || (keyName[0] != L'/')))
			{
				registryPath.Append(L'\\');
			}
		}
		registryPath.Append(preferencePointer->GetKeyName());
		if (fIsUsingForwardSlashAsPathSeparator)
		{
			registryPath.Replace("/", "\\");
		}

		// Extract the registry components (ie: hive key, key path, value name) from the above registry path.
		auto extractionResult = ExtractComponentsFromRegistryPath(registryPath.GetUTF16());
		if (extractionResult.HasFailed())
		{
			if (errorMessage.IsEmpty() == false)
			{
				errorMessage.Append(L'\n');
			}
			errorMessage.Append(L"Failed to write preference '");
			errorMessage.Append(preferencePointer->GetKeyName());
			errorMessage.Append(L"' to the registry.\n- Registry Path: ");
			errorMessage.Append(registryPath.GetUTF16());
			errorMessage.Append(L"\n- Reason: ");
			errorMessage.Append(extractionResult.GetUtf8Message());
			continue;
		}
		auto registryPathComponents = extractionResult.GetValue();

		// Set up the registry reading flags.
		REGSAM flags = KEY_READ | KEY_WRITE;
		if (RegistryStoredPreferences::Wow64ViewType::k32Bit == fWow64ViewType)
		{
			flags |= KEY_WOW64_32KEY;
		}
		else if (RegistryStoredPreferences::Wow64ViewType::k64Bit == fWow64ViewType)
		{
			flags |= KEY_WOW64_64KEY;
		}

		// Attempt to create/open the registry key.
		HKEY keyHandle = 0;
		const auto openResult = ::RegCreateKeyExW(
				registryPathComponents.HiveKeyHandle, registryPathComponents.KeyPath->c_str(), 0, nullptr,
				REG_OPTION_NON_VOLATILE, flags, nullptr, &keyHandle, nullptr);
		if ((openResult != ERROR_SUCCESS) || (0 == keyHandle))
		{
			if (errorMessage.IsEmpty() == false)
			{
				errorMessage.Append(L'\n');
			}
			errorMessage.Append(L"Failed to create/open registry key for preference '");
			errorMessage.Append(preferencePointer->GetKeyName());
			errorMessage.Append(L"'.\n- Reason: ");
			if (openResult != ERROR_SUCCESS)
			{
				LPWSTR utf16Buffer;
				::FormatMessageW(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						nullptr, openResult,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPWSTR)&utf16Buffer, 0, nullptr);
				if (utf16Buffer)
				{
					errorMessage.Append(utf16Buffer);
					::LocalFree(utf16Buffer);
				}
				else
				{
					errorMessage.Append(L"Unknown");
				}
			}
			else
			{
				errorMessage.Append(L"Failed to acquire key handle.");
			}
			continue;
		}

		// Write the preference value to the registry.
		LSTATUS writeResult = ERROR_INVALID_PARAMETER;
		auto preferenceValue = valueConversionResult.GetValue();
		switch (preferenceValue.GetType())
		{
			case Rtt::PreferenceValue::kTypeUnsignedInt32:
			{
				auto value = preferenceValue.ToUnsignedInt32().GetValue();
				writeResult = ::RegSetValueExW(
						keyHandle, registryPathComponents.ValueName->c_str(), 0,
						REG_DWORD, (LPBYTE)&value, sizeof(value));
				break;
			}
			case Rtt::PreferenceValue::kTypeUnsignedInt64:
			{
				auto value = preferenceValue.ToUnsignedInt64().GetValue();
				writeResult = ::RegSetValueExW(
						keyHandle, registryPathComponents.ValueName->c_str(), 0,
						REG_QWORD, (LPBYTE)&value, sizeof(value));
				break;
			}
			default:
			{
				auto stringPointer = preferenceValue.ToString().GetValue();
				if (stringPointer.NotNull())
				{
					auto nullCharacterIndex = stringPointer->find('\0');
					if ((nullCharacterIndex != std::string::npos) &&
					    ((nullCharacterIndex + 1) < stringPointer->length()))
					{
						// The string contains at least 1 embedded null character.
						// Write it as a byte array so that the string won't get truncated.
						writeResult = ::RegSetValueExW(
								keyHandle, registryPathComponents.ValueName->c_str(), 0,
								REG_BINARY, (LPBYTE)stringPointer->c_str(), stringPointer->length());
					}
					else
					{
						// Write a null terminated string to the registry.
						WinString stringTranscoder(stringPointer->c_str());
						writeResult = ::RegSetValueExW(
								keyHandle, registryPathComponents.ValueName->c_str(), 0,
								REG_SZ, (LPBYTE)stringTranscoder.GetUTF16(),
								stringTranscoder.GetBufferNumChars() * sizeof(wchar_t));
					}
				}
				else
				{
					// Write an empty string to the registry.
					const wchar_t kEmptyString[] = L"";
					writeResult = ::RegSetValueExW(
							keyHandle, registryPathComponents.ValueName->c_str(), 0,
							REG_SZ, (LPBYTE)&kEmptyString, sizeof(kEmptyString));
				}
				break;
			}
		}
		if (ERROR_SUCCESS == writeResult)
		{
			preferencesWritten++;
		}
		else
		{
			if (errorMessage.IsEmpty() == false)
			{
				errorMessage.Append(L'\n');
			}
			errorMessage.Append(L"Failed to write preference '");
			errorMessage.Append(preferencePointer->GetKeyName());
			errorMessage.Append(L"' to the registry.\n- Reason: ");
			{
				LPWSTR utf16Buffer;
				::FormatMessageW(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						nullptr, writeResult,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPWSTR)&utf16Buffer, 0, nullptr);
				if (utf16Buffer)
				{
					errorMessage.Append(utf16Buffer);
					::LocalFree(utf16Buffer);
				}
				else
				{
					errorMessage.Append(L"Unknown");
				}
			}
		}

		// Close the registry key.
		::RegCloseKey(keyHandle);
	}

	// Return the final result of the above write(s) to the registry.
	// Note: We return an error if we've failed to write at least 1 preference.
	if (preferencesWritten < preferences.GetCount())
	{
		return Rtt::OperationResult::FailedWith(errorMessage.GetUTF8());
	}
	else if (errorMessage.IsEmpty() == false)
	{
		return Rtt::OperationResult::SucceededWith(errorMessage.GetUTF8());
	}
	return Rtt::OperationResult::kSucceeded;
}

Rtt::OperationResult RegistryStoredPreferences::Delete(const char* keyName)
{
	// Validate.
	if (!keyName)
	{
		return Rtt::OperationResult::FailedWith("Preference key cannot be null.");
	}

	// Delete the preference.
	const char* keyNameArray[] = { keyName };
	return Delete(keyNameArray, 1);
}

Rtt::OperationResult RegistryStoredPreferences::Delete(const char* keyNameArray[], int count)
{
	// Validate.
	if (!keyNameArray)
	{
		return Rtt::OperationResult::FailedWith("Key array cannot be null.");
	}
	if (count <= 0)
	{
		return Rtt::OperationResult::kSucceeded;
	}

	// Delete all of the given preferences from the registry.
	WinString errorMessage;
	int preferencesDeleted = 0;
	for (int keyIndex = 0; keyIndex < count; keyIndex++)
	{
		// Fetch the next preference key name.
		auto keyName = keyNameArray[keyIndex];
		if (!keyName || (L'\0' == keyName[0]))
		{
			if (errorMessage.IsEmpty() == false)
			{
				errorMessage.Append(L'\n');
			}
			errorMessage.Append(L"Preference key cannot be null or empty string.");
			continue;
		}

		// Build an absolute registry path.
		WinString registryPath(fBaseRegistryPath);
		if (registryPath.IsEmpty() == false)
		{
			if ((keyName[0] != L'\\') && (!fIsUsingForwardSlashAsPathSeparator || (keyName[0] != L'/')))
			{
				registryPath.Append(L'\\');
			}
		}
		registryPath.Append(keyName);
		if (fIsUsingForwardSlashAsPathSeparator)
		{
			registryPath.Replace("/", "\\");
		}

		// Extract the registry components (ie: hive key, key path, value name) from the above registry path.
		auto extractionResult = ExtractComponentsFromRegistryPath(registryPath.GetUTF16());
		if (extractionResult.HasFailed())
		{
			if (errorMessage.IsEmpty() == false)
			{
				errorMessage.Append(L'\n');
			}
			errorMessage.Append(L"Failed to delete preference '");
			errorMessage.Append(keyName);
			errorMessage.Append(L"' from the registry.\n- Registry Path: ");
			errorMessage.Append(registryPath.GetUTF16());
			errorMessage.Append(L"\n- Reason: ");
			errorMessage.Append(extractionResult.GetUtf8Message());
			continue;
		}
		auto registryPathComponents = extractionResult.GetValue();

		// Set up the registry read/write flags.
		REGSAM flags = KEY_READ | KEY_WRITE;
		if (RegistryStoredPreferences::Wow64ViewType::k32Bit == fWow64ViewType)
		{
			flags |= KEY_WOW64_32KEY;
		}
		else if (RegistryStoredPreferences::Wow64ViewType::k64Bit == fWow64ViewType)
		{
			flags |= KEY_WOW64_64KEY;
		}

		// Delete the registry key's value, if it exists.
		// Flag this operation as successful if deleted or if the entry was not found.
		// Note: Don't log an error if the entry was not found. What matters most is that it doesn't exist in storage.
		HKEY keyHandle = 0;
		auto resultCode = ::RegOpenKeyExW(
				registryPathComponents.HiveKeyHandle, registryPathComponents.KeyPath->c_str(), 0, flags, &keyHandle);
		if (ERROR_SUCCESS == resultCode)
		{
			resultCode = ::RegDeleteValueW(keyHandle, registryPathComponents.ValueName->c_str());
			::RegCloseKey(keyHandle);
		}
		if ((ERROR_SUCCESS == resultCode) || (ERROR_FILE_NOT_FOUND == resultCode))
		{
			preferencesDeleted++;
		}
		else
		{
			if (errorMessage.IsEmpty() == false)
			{
				errorMessage.Append(L'\n');
			}
			errorMessage.Append(L"Failed to delete preference '");
			errorMessage.Append(keyName);
			errorMessage.Append(L"' from the registry.\n- Reason: ");
			LPWSTR utf16Buffer;
			::FormatMessageW(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					nullptr, resultCode,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPWSTR)&utf16Buffer, 0, nullptr);
			if (utf16Buffer)
			{
				errorMessage.Append(utf16Buffer);
				::LocalFree(utf16Buffer);
			}
			else
			{
				errorMessage.Append(L"Unkonwn registry error.");
			}
		}
	}

	// Return an error if we've failed to delete at least 1 preference in the collection.
	if (preferencesDeleted < count)
	{
		if (errorMessage.IsEmpty())
		{
			errorMessage.SetUTF16(L"Failed to delete preference(s).");
		}
		return Rtt::OperationResult::FailedWith(errorMessage.GetUTF8());
	}

	// Return a success result.
	if (errorMessage.IsEmpty() == false)
	{
		return Rtt::OperationResult::SucceededWith(errorMessage.GetUTF8());
	}
	return Rtt::OperationResult::kSucceeded;
}

#pragma endregion


#pragma region Public Static Functions
std::shared_ptr<RegistryStoredPreferences> RegistryStoredPreferences::ForSimulatorPreferencesUserKey()
{
	static std::shared_ptr<RegistryStoredPreferences> sRegistryStoredPreferencesPointer;

	// Create the requested registry accessing object, if not done already.
	if (!sRegistryStoredPreferencesPointer)
	{
		// Set up the base registry path.
		// References the Corona Simulator's "Preferences" key under the current user's section of the registry.
		std::wstringstream stringStream;
		stringStream << L"HKCU\\Software\\";
		stringStream << RegistryStoredPreferences::kAnscaCoronaKeyName;
		stringStream << L'\\';
		stringStream << RegistryStoredPreferences::kCoronaSimulatorKeyName;
		stringStream << L'\\';
		stringStream << RegistryStoredPreferences::kSimulatorPreferencesKeyName;
		std::wstring baseRegistryPath(stringStream.str());

		// Create the registry accessing object.
		RegistryStoredPreferences::CreationSettings settings{};
		settings.BaseRegistryPath = baseRegistryPath.c_str();
		settings.Wow64ViewType = RegistryStoredPreferences::Wow64ViewType::kDefault;
		settings.IsUsingForwardSlashAsPathSeparator = true;
		sRegistryStoredPreferencesPointer = std::make_shared<RegistryStoredPreferences>(settings);
	}

	// Return the reqeusted registry accessing object.
	return sRegistryStoredPreferencesPointer;
}

std::shared_ptr<RegistryStoredPreferences> RegistryStoredPreferences::ForSimulatorPreferencesMachineKey()
{
	static std::shared_ptr<RegistryStoredPreferences> sRegistryStoredPreferencesPointer;

	// Create the requested registry accessing object, if not done already.
	if (!sRegistryStoredPreferencesPointer)
	{
		// Set up the base registry path.
		// References the Corona Simulator's "Preferences" key under the machine section of the registry.
		std::wstringstream stringStream;
		stringStream << L"HKLM\\Software\\";
		stringStream << RegistryStoredPreferences::kAnscaCoronaKeyName;
		stringStream << L'\\';
		stringStream << RegistryStoredPreferences::kCoronaSimulatorKeyName;
		stringStream << L'\\';
		stringStream << RegistryStoredPreferences::kSimulatorPreferencesKeyName;
		std::wstring baseRegistryPath(stringStream.str());

		// Create the registry accessing object.
		RegistryStoredPreferences::CreationSettings settings{};
		settings.BaseRegistryPath = baseRegistryPath.c_str();
		settings.Wow64ViewType = RegistryStoredPreferences::Wow64ViewType::kDefault;
		settings.IsUsingForwardSlashAsPathSeparator = true;
		sRegistryStoredPreferencesPointer = std::make_shared<RegistryStoredPreferences>(settings);
	}

	// Return the reqeusted registry accessing object.
	return sRegistryStoredPreferencesPointer;
}

std::shared_ptr<StoredPreferencesChain> RegistryStoredPreferences::ForSimulatorPreferences()
{
	static std::shared_ptr<StoredPreferencesChain> sStoredPreferencesChainPointer;

	// Create the requested registry accessing object, if not done already.
	if (!sStoredPreferencesChainPointer)
	{
		// Set up a chain of registry accessing objects. (Mimics Apple's NSUserDefaults behavior.)
		// 1st object accesses the Corona Simulator's "Preferences" key under HKCU.
		// 2nd object has read-only access to the Corona Simulator's "Preferences" key under the HKLM.
		StoredPreferencesChain::ListType list;
		list.push_back(ForSimulatorPreferencesUserKey());
		list.push_back(StoredPreferencesProxy::CreateReadOnlyUsing(ForSimulatorPreferencesMachineKey()));
		sStoredPreferencesChainPointer = std::make_shared<StoredPreferencesChain>(list);
	}

	// Return the reqeusted registry accessing object.
	return sStoredPreferencesChainPointer;
}

#pragma endregion


#pragma region Private Static Functions
Rtt::ValueResult<RegistryStoredPreferences::ExtractComponentsResult>
RegistryStoredPreferences::ExtractComponentsFromRegistryPath(const wchar_t* path)
{
	// Validate.
	if (!path || (L'\0' == path[0]))
	{
		const char kMessage[] = "Registry path cannot be null or empty.";
		return Rtt::ValueResult<ExtractComponentsResult>::FailedWith(kMessage);
	}

	// Fetch a pointer to the registry key path which comes after the hive name.
	// Note: This pointer will still have the registry value name at the end. We'll extract it later.
	const wchar_t* keyPathPointer = wcschr(path, L'\\');
	if (!keyPathPointer || (keyPathPointer == path))
	{
		const char kMessage[] = "Registry hive name is missing at the front of the path.";
		return Rtt::ValueResult<ExtractComponentsResult>::FailedWith(kMessage);
	}
	keyPathPointer++;

	// Determine which registry hive the given path references.
	HKEY hiveKeyHandle = 0;
	const auto hiveNameLength = (size_t)((keyPathPointer - 1) - path);
	if (!_wcsnicmp(L"HKCU", path, hiveNameLength) || !_wcsnicmp(L"HKEY_CURRENT_USER", path, hiveNameLength))
	{
		hiveKeyHandle = HKEY_CURRENT_USER;
	}
	else if (!_wcsnicmp(L"HKLM", path, hiveNameLength) || !_wcsnicmp(L"HKEY_LOCAL_MACHINE", path, hiveNameLength))
	{
		hiveKeyHandle = HKEY_LOCAL_MACHINE;
	}
	else if (!_wcsnicmp(L"HKU", path, hiveNameLength) || !_wcsnicmp(L"HKEY_USERS", path, hiveNameLength))
	{
		hiveKeyHandle = HKEY_USERS;
	}
	else if (!_wcsnicmp(L"HKCR", path, hiveNameLength) || !_wcsnicmp(L"HKEY_CLASSES_ROOT", path, hiveNameLength))
	{
		hiveKeyHandle = HKEY_CLASSES_ROOT;
	}
	else if (!_wcsnicmp(L"HKCC", path, hiveNameLength) || !_wcsnicmp(L"HKEY_CURRENT_CONFIG", path, hiveNameLength))
	{
		hiveKeyHandle = HKEY_CURRENT_CONFIG;
	}
	if (0 == hiveKeyHandle)
	{
		std::wstring hiveName(path, hiveNameLength);
		WinString message;
		message.Format(L"Given unknown registry hive name: %s", hiveName.c_str());
		return Rtt::ValueResult<ExtractComponentsResult>::FailedWith(message.GetUTF8());
	}

	// Fetch a pointer to the registry value name at the end of the given path.
	const wchar_t* valueNamePointer = wcsrchr(path, L'\\');
	if (!valueNamePointer)
	{
		const char kMessage[] = "Failed to find value name in registry path.";
		return Rtt::ValueResult<ExtractComponentsResult>::FailedWith(kMessage);
	}
	valueNamePointer++;

	// Extract the registry key path and copy it to a shared string pointer.
	// This path excludes the registry hive name at the front and the registry value name at the end.
	std::shared_ptr<std::wstring> keyPathSharedPointer;
	if (keyPathPointer != valueNamePointer)
	{
		keyPathSharedPointer = std::make_shared<std::wstring>(
				keyPathPointer, (size_t)((valueNamePointer - 1) - keyPathPointer));
	}
	else
	{
		keyPathSharedPointer = std::make_shared<std::wstring>(L"");
	}

	// Return the registry components.
	ExtractComponentsResult result;
	result.HiveKeyHandle = hiveKeyHandle;
	result.KeyPath = keyPathSharedPointer;
	result.ValueName = std::make_shared<std::wstring>(valueNamePointer);
	return Rtt::ValueResult<ExtractComponentsResult>::SucceededWith(result);
}

#pragma endregion

} }	// namespace Interop::Storage
