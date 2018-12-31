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

// Enables the Crypto++ library's MD4 and MD5 support without compiler warnings.
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include "Rtt_WinDevice.h"
#include "Core\Rtt_Build.h"
#include "CryptoPP\md5.h"
#include "Interop\MDeviceSimulatorServices.h"
#include "Interop\RuntimeEnvironment.h"
#include "Interop\ScopedComInitializer.h"
#include "Rtt_Event.h"
#include "Rtt_Runtime.h"
#include "WinString.h"
#include <iphlpapi.h>
#include <Sddl.h>
#include <sstream>
#include <stdio.h>
#include <string>
#include <WbemIdl.h>
#include <windows.h>


namespace Rtt
{

#pragma region Constructors/Destructors
WinDevice::WinDevice(Interop::RuntimeEnvironment& environment)
:	fEnvironment(environment),
	fDeviceIdentifier(nullptr),
	fComputerName(nullptr),
	fInputDeviceManager(environment)
{
}

WinDevice::~WinDevice()
{
	if (fDeviceIdentifier)
	{
		delete[] fDeviceIdentifier;
		fDeviceIdentifier = nullptr;
	}
	if (fComputerName)
	{
		delete fComputerName;
		fComputerName = nullptr;
	}
}

#pragma endregion


#pragma region Public Methods

const char* WinDevice::GetName() const
{
	if (nullptr == fComputerName)
	{
		const size_t kMaxCharacters = MAX_COMPUTERNAME_LENGTH + 1;
		wchar_t buffer[kMaxCharacters];
		buffer[0] = L'\0';
		DWORD bufferSize = (DWORD)kMaxCharacters;
		BOOL result = GetComputerNameW(buffer, &bufferSize);
		if (result && buffer[0])
		{
			WinString stringConverter;
			stringConverter.SetUTF16(buffer);
			int length = strlen(stringConverter.GetUTF8()) + 1;
			fComputerName = new char[length];
			strcpy_s(fComputerName, length, stringConverter.GetUTF8());
		}
	}
	return fComputerName ? fComputerName : "";
}

const char* WinDevice::GetManufacturer() const
{
	// Fetch the device/machine's manufacturer name, if not done already.
	if (fManufacturerName.empty())
	{
		auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
		if (deviceSimulatorServicesPointer)
		{
			// Fetch the simulated device's manufacturer name.
			fModelName = deviceSimulatorServicesPointer->GetManufacturerName();
		}
		else
		{
			// *** Fetch the PC's manufacturer name, if available. ***

			// Default to an unknown name if we can't fetch it below.
			WinString manufacturerName("Unknown");

			// Fetch the PC's manufacturer name via WMI and COM.
			auto comApartmentType = Interop::ScopedComInitializer::ApartmentType::kSingleThreaded;
			Interop::ScopedComInitializer scopedComInitializer(comApartmentType);
			IWbemLocator* locatorPointer = nullptr;
			IWbemServices* servicesPointer = nullptr;
			IEnumWbemClassObject* enumeratorPointer = nullptr;
			IWbemClassObject* objectPointer = nullptr;
			auto result = ::CoCreateInstance(
					__uuidof(WbemLocator), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&locatorPointer));
			if (SUCCEEDED(result) && locatorPointer)
			{
				result = locatorPointer->ConnectServer(
						L"root\\CIMV2", nullptr, nullptr, nullptr, WBEM_FLAG_CONNECT_USE_MAX_WAIT,
						nullptr, nullptr, &servicesPointer);
				if (SUCCEEDED(result) && servicesPointer)
				{
					result = ::CoSetProxyBlanket(
							servicesPointer, RPC_C_AUTHN_DEFAULT, RPC_C_AUTHZ_DEFAULT, COLE_DEFAULT_PRINCIPAL,
							RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, 0, EOAC_NONE);
					result = servicesPointer->ExecQuery(
							L"WQL", L"SELECT Manufacturer FROM Win32_ComputerSystem",
							WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &enumeratorPointer);
					if (SUCCEEDED(result) && enumeratorPointer)
					{
						ULONG count = 0;
						result = enumeratorPointer->Next(WBEM_INFINITE, 1, &objectPointer, &count);
						if ((WBEM_S_NO_ERROR == result) && objectPointer)
						{
							VARIANT variantValue;
							::VariantInit(&variantValue);
							result = objectPointer->Get(L"Manufacturer", 0, &variantValue, nullptr, nullptr);
							if (SUCCEEDED(result))
							{
								if ((VT_BSTR == variantValue.vt) && (variantValue.bstrVal[0] != L'\0'))
								{
									manufacturerName.SetUTF16(variantValue.bstrVal);
								}
								::VariantClear(&variantValue);
							}
						}
					}
				}
			}

			// Release the COM object references acquired above.
			if (objectPointer)
			{
				objectPointer->Release();
			}
			if (enumeratorPointer)
			{
				enumeratorPointer->Release();
			}
			if (servicesPointer)
			{
				servicesPointer->Release();
			}
			if (locatorPointer)
			{
				locatorPointer->Release();
			}

			// Store the manufacturer name in UTF-8 form.
			fManufacturerName = manufacturerName.GetUTF8();
		}
	}

	// Return the device/machine's manufacturer name.
	return fManufacturerName.empty() ? "" : fManufacturerName.c_str();
}

const char* WinDevice::GetModel() const
{
	// Fetch the device/machine's model name, if not done already.
	if (fModelName.empty())
	{
		auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
		if (deviceSimulatorServicesPointer)
		{
			// Fetch the simulated device's model name.
			fModelName = deviceSimulatorServicesPointer->GetModelName();
		}
		else
		{
			// *** Fetch the PC's model name, if available. ***

			// Default to an unknown name if we can't fetch it below.
			WinString modelName("Unknown");

			// Fetch the PC's model name via WMI and COM.
			auto comApartmentType = Interop::ScopedComInitializer::ApartmentType::kSingleThreaded;
			Interop::ScopedComInitializer scopedComInitializer(comApartmentType);
			IWbemLocator* locatorPointer = nullptr;
			IWbemServices* servicesPointer = nullptr;
			IEnumWbemClassObject* enumeratorPointer = nullptr;
			IWbemClassObject* objectPointer = nullptr;
			auto result = ::CoCreateInstance(
					__uuidof(WbemLocator), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&locatorPointer));
			if (SUCCEEDED(result) && locatorPointer)
			{
				result = locatorPointer->ConnectServer(
						L"root\\CIMV2", nullptr, nullptr, nullptr, WBEM_FLAG_CONNECT_USE_MAX_WAIT,
						nullptr, nullptr, &servicesPointer);
				if (SUCCEEDED(result) && servicesPointer)
				{
					result = ::CoSetProxyBlanket(
							servicesPointer, RPC_C_AUTHN_DEFAULT, RPC_C_AUTHZ_DEFAULT, COLE_DEFAULT_PRINCIPAL,
							RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, 0, EOAC_NONE);
					result = servicesPointer->ExecQuery(
							L"WQL", L"SELECT Model FROM Win32_ComputerSystem",
							WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &enumeratorPointer);
					if (SUCCEEDED(result) && enumeratorPointer)
					{
						ULONG count = 0;
						result = enumeratorPointer->Next(WBEM_INFINITE, 1, &objectPointer, &count);
						if ((WBEM_S_NO_ERROR == result) && objectPointer)
						{
							VARIANT variantValue;
							::VariantInit(&variantValue);
							result = objectPointer->Get(L"Model", 0, &variantValue, nullptr, nullptr);
							if (SUCCEEDED(result))
							{
								if ((VT_BSTR == variantValue.vt) && (variantValue.bstrVal[0] != L'\0'))
								{
									modelName.SetUTF16(variantValue.bstrVal);
								}
								::VariantClear(&variantValue);
							}
						}
					}
				}
			}

			// Release the COM object references acquired above.
			if (objectPointer)
			{
				objectPointer->Release();
			}
			if (enumeratorPointer)
			{
				enumeratorPointer->Release();
			}
			if (servicesPointer)
			{
				servicesPointer->Release();
			}
			if (locatorPointer)
			{
				locatorPointer->Release();
			}

			// Store the model name in UTF-8 form.
			fModelName = modelName.GetUTF8();
		}
	}

	// Return the device/machine's model name.
	return fModelName.empty() ? "" : fModelName.c_str();
}

const char* WinDevice::GetUniqueIdentifier(IdentifierType idType) const
{
	const char *result = nullptr;

	switch (idType)
	{
		case MPlatformDevice::kDeviceIdentifier:
			// Create the unique string ID if not done already.
			if (nullptr == fDeviceIdentifier)
			{
				char stringBuffer[256];

				// *** Fetch the local machine's SID. This is a unqiue ID assigned by Windows. ***
				// -------------------------------------------------------------------------------
				// The SID is assigned by Windows upon installation. Microsoft uses it to identify
				// machines and users to manage their permissions. Especially on a domain.
				// The SID can change, but only for the following reasons:
				// 1) Re-installation of Windows.
				// 2) Microsoft's sysprep tool is used.
				// 3) Microsoft's NewSID tool is used. (No longer supported by Microsoft.)
				// -------------------------------------------------------------------------------
				if (CopyMachineSidStringTo(stringBuffer, sizeof(stringBuffer)) == false)
				{
					// Unable to retrieve the SID. Attempt to use the first Ethernet device's MAC address as the unique ID.
#ifdef Rtt_DEBUG
					_snprintf_s(stringBuffer, sizeof(stringBuffer),
							"Warning: Unable to fetch primary machine ID. (Error Code: %d)\r\n", ::GetLastError());
					Rtt_LogException(stringBuffer);
#endif // Rtt_DEBUG
					if (CopyFirstEthernetMacAddressStringTo(stringBuffer, sizeof(stringBuffer)) == false)
					{
						// Unable to find a MAC address. Fallback to a hardcoded "0" for the string ID.
						Rtt_LogException("Warning: Unable to fetch secondary machine ID ('deviceID' not set)\r\n");
						stringBuffer[0] = '0';
						stringBuffer[1] = 0;
					}
				}

				// MD5 hash the unique string ID retrieved up above.
				U8 hash[CryptoPP::Weak1::MD5::DIGESTSIZE];
				CryptoPP::Weak1::MD5 md5;
				md5.CalculateDigest(hash, (const byte*)stringBuffer, strlen(stringBuffer));
				fDeviceIdentifier = new char[(CryptoPP::Weak1::MD5::DIGESTSIZE * 2) + 1];
				char *p = fDeviceIdentifier;
				for (int index = 0; index < CryptoPP::Weak1::MD5::DIGESTSIZE; index++)
				{
					p += sprintf_s(p, 3, "%02x", hash[index]);
				}
			}
			result = fDeviceIdentifier;
			break;
		case MPlatformDevice::kMacIdentifier:
			// TODO
			break;
		case MPlatformDevice::kUdidIdentifier:
			// TODO
			break;
		default:
			break;
	}

	return result;
}

/// Sets the "primary" ID for this machine that will be returned by the GetUniqueIdentifier() function.
///
/// This is needed because Windows does not have a single unique ID that represents the machine.
/// A PC has multiple hardware IDs that can be used and the hardware can be swapped out over time.
/// This also means that the primary ID fetched by GetUniqueIdentifier() might change over time.
/// To compensate for this, you must check if the authorization token's device ID still exists within
/// the machine by calling ContainsUniqueIdentifier() and if it does then call this function to
/// set the primary ID.
///
/// @param idString The string to be used as the unique ID for this machine. Cannot be NULL or empty.
///                 This string is copied by this object as is (will not be hashed) and it will be
///                 returned by the GetUniqueIdentifier() function.
void WinDevice::SetUniqueIdentifier(const char *idString)
{
	size_t length;

	// Validate argument.
	if (nullptr == idString)
	{
		return;
	}

	// Fetch the given string's length.
	length = strlen(idString);
	if (length <= 0)
	{
		return;
	}

	// Replace the last unique ID string with the given one.
	if (fDeviceIdentifier)
	{
		delete fDeviceIdentifier;
	}
	fDeviceIdentifier = new char[length + 1];
	strncpy_s(fDeviceIdentifier, length + 1, idString, length);
}

/// Checks if this machine contains the given unique device ID that was once returned by
/// the GetUniqueIdentifier() function. Remember that a PC contains multiple device IDs.
/// @param idString The unique device ID in string form to check for.
/// @return Returns TRUE if this machine contains a device having the given unique string ID.
///         Returns FALSE if a match was not found or if given a NULL or empty string.
bool WinDevice::ContainsUniqueIdentifier(const char *idString)
{
	std::list<std::string> stringCollection;
	std::list<std::string>::iterator iter;
	U8 hash[CryptoPP::Weak1::MD5::DIGESTSIZE];
	size_t idStringLength;
	char stringBuffer[256];
	bool wasSidReceived = false;

	// Do not continue if given an empty string.
	if (nullptr == idString)
	{
		return false;
	}
	idStringLength = strlen(idString);
	if (idStringLength <= 0)
	{
		return false;
	}

	// First check if the given string matches what is returned by GetUniqueIdentifier() function.
	// The returned unique ID is already hashed and odds are this will match the given string.
	if (strncmp(idString, GetUniqueIdentifier(MPlatformDevice::kDeviceIdentifier), idStringLength) == 0)
	{
		return true;
	}

	// Add the machine's SID string to the collection.
	wasSidReceived = CopyMachineSidStringTo(stringBuffer, sizeof(stringBuffer));
	if (wasSidReceived)
	{
		stringCollection.push_back(std::string(stringBuffer));
	}

	// Add the machine's MAC address strings to the collection.
	AddEthernetMacAddressStringsTo(stringCollection);

	// Traverse all device IDs for a match.
	for (iter = stringCollection.begin(); iter != stringCollection.end(); iter++)
	{
		// MD5 hash the device ID to a hexadecimal string.
		CryptoPP::Weak1::MD5 md5;
		md5.CalculateDigest(hash, (const byte*)(*iter).c_str(), (*iter).length());
		char *p = stringBuffer;
		for (int index = 0; index < CryptoPP::Weak1::MD5::DIGESTSIZE; index++)
		{
			p += sprintf_s(p, 3, "%02x", hash[index]);
		}

		// Check if the given string ID matches this device's hashed string ID.
		if (strncmp(idString, stringBuffer, idStringLength) == 0)
		{
			return true;
		}
	}

	// The given ID was not found.
	return false;
}

void WinDevice::Vibrate() const
{
}

bool WinDevice::HasEventSource(EventType type) const
{
	auto deviceSimulatorPointer = fEnvironment.GetDeviceSimulatorServices();
	bool hasEventSource = false;
	switch (type)
	{
		case MPlatformDevice::kGyroscopeEvent:
		case MPlatformDevice::kHeadingEvent:
			hasEventSource = false;
			break;
		case MPlatformDevice::kMultitouchEvent:
			hasEventSource = fInputDeviceManager.IsMultitouchSupported();
			break;
		case MPlatformDevice::kAccelerometerEvent:
		case MPlatformDevice::kOrientationEvent:
		case MPlatformDevice::kLocationEvent:
			// Only supported if running under the device simulator.
			// This is because the Corona Simulator is able to simulate/mock these events.
			hasEventSource = (deviceSimulatorPointer != nullptr);
			break;
		case MPlatformDevice::kInputDeviceStatusEvent:
			if (deviceSimulatorPointer)
			{
				hasEventSource = deviceSimulatorPointer->AreInputDevicesSupported();
			}
			else
			{
				hasEventSource = true;
			}
			break;
		case MPlatformDevice::kKeyEvent:
			if (deviceSimulatorPointer)
			{
				hasEventSource = deviceSimulatorPointer->AreKeyEventsSupported();
			}
			else
			{
				hasEventSource = true;
			}
			break;
		case MPlatformDevice::kMouseEvent:
			if (deviceSimulatorPointer)
			{
				hasEventSource = deviceSimulatorPointer->IsMouseSupported();
			}
			else
			{
				hasEventSource = true;
			}
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}
	return hasEventSource;
}

void WinDevice::BeginNotifications(EventType type) const
{
	fTracker.BeginNotifications(type);

	if (fEnvironment.GetDeviceSimulatorServices())
	{
		auto runtimePointer = fEnvironment.GetRuntime();
		if (runtimePointer)
		{
			if (kLocationEvent == type)
			{
				LocationEvent event(37.448485, -122.158911, 0.0, 50.0, -1.0, -1.0, (double)time(nullptr));
				runtimePointer->DispatchEvent(event);
			}
		}
	}
}

void WinDevice::EndNotifications(EventType type) const
{
	fTracker.EndNotifications(type);
}

bool WinDevice::DoesNotify(EventType type) const
{
	return fTracker.DoesNotify(type);
}

void WinDevice::SetAccelerometerInterval(U32 frequency) const
{
	Rtt_WARN_SIM(frequency >= 10 && frequency <= 100, ("WARNING: Accelerometer frequency must be in the range [10,100] Hz"));
}

void WinDevice::SetGyroscopeInterval(U32 frequency) const
{
	Rtt_WARN_SIM(frequency >= 10 && frequency <= 100, ("WARNING: Gyroscope frequency must be in the range [10,100] Hz"));
}

void WinDevice::SetLocationAccuracy(Real meters) const
{
}

void WinDevice::SetLocationThreshold(Real meters) const
{
}

DeviceOrientation::Type WinDevice::GetOrientation() const
{
	// If we're simulating a device, then use the current simulated orientation.
	auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
	if (deviceSimulatorServicesPointer)
	{
		return deviceSimulatorServicesPointer->GetOrientation();
	}

	// Use the Corona project's default orientation, if provided.
	auto orientation = fEnvironment.GetProjectSettings().GetDefaultOrientation();
	if (Rtt::DeviceOrientation::IsInterfaceOrientation(orientation))
	{
		return orientation;
	}

	// If all else fails, default to an upright orientation.
	return Rtt::DeviceOrientation::kUpright;
}

Rtt::MPlatformDevice::EnvironmentType WinDevice::GetEnvironment() const
{
#ifdef Rtt_AUTHORING_SIMULATOR
	return kSimulatorEnvironment;
#else
	if (fEnvironment.GetDeviceSimulatorServices())
	{
		return kSimulatorEnvironment;
	}
	return kDeviceEnvironment;
#endif
}

const char* WinDevice::GetPlatformName() const
{
	return "Win";
}

const char* WinDevice::GetPlatform() const
{
#ifdef Rtt_AUTHORING_SIMULATOR
	// If we're simulating a device, then use the current simulated orientation.
	auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
	if (deviceSimulatorServicesPointer)
	{
		return deviceSimulatorServicesPointer->GetOSName();
	}
#else
	return "win32";
#endif
}

const char* WinDevice::GetPlatformVersion() const
{
	static std::string sWindowsVersionString;

	// Fetch the operating system's version string, if not done already.
	if (sWindowsVersionString.empty())
	{
		int majorVersion = -1;
		int minorVersion = -1;

		// First, attempt to do so via WMI and COM.
		// Note: This is preferred over using GetVersionEx() since it'll return the wrong version numbers on Windows 8.1
		//       and newer operating systems unless the EXE's manifest settings target those newer OS versions.
		Interop::ScopedComInitializer scopedComInitializer;
		IWbemLocator* locatorPointer = nullptr;
		IWbemServices* servicesPointer = nullptr;
		IEnumWbemClassObject* enumeratorPointer = nullptr;
		IWbemClassObject* objectPointer = nullptr;
		try
		{
			auto result = ::CoCreateInstance(
					__uuidof(WbemLocator), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&locatorPointer));
			if (SUCCEEDED(result) && locatorPointer)
			{
				result = locatorPointer->ConnectServer(
						L"root\\CIMV2", nullptr, nullptr, nullptr, WBEM_FLAG_CONNECT_USE_MAX_WAIT,
						nullptr, nullptr, &servicesPointer);
				if (SUCCEEDED(result) && servicesPointer)
				{
					result = ::CoSetProxyBlanket(
							servicesPointer, RPC_C_AUTHN_DEFAULT, RPC_C_AUTHZ_DEFAULT, COLE_DEFAULT_PRINCIPAL,
							RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, 0, EOAC_NONE);
					result = servicesPointer->ExecQuery(
							L"WQL", L"SELECT Version FROM Win32_OperatingSystem",
							WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &enumeratorPointer);
					if (SUCCEEDED(result) && enumeratorPointer)
					{
						ULONG count = 0;
						result = enumeratorPointer->Next(WBEM_INFINITE, 1, &objectPointer, &count);
						if ((WBEM_S_NO_ERROR == result) && objectPointer)
						{
							VARIANT variantValue;
							::VariantInit(&variantValue);
							result = objectPointer->Get(L"Version", 0, &variantValue, nullptr, nullptr);
							if (SUCCEEDED(result))
							{
								WinString fullVersionString;
								if ((VT_BSTR == variantValue.vt) && (variantValue.bstrVal[0] != L'\0'))
								{
									fullVersionString.SetUTF16(variantValue.bstrVal);
								}
								::VariantClear(&variantValue);
								if (!fullVersionString.IsEmpty())
								{
									// Extract the major and minor version numbers from the string.
									auto invariantLocalePointer = _wcreate_locale(LC_ALL, L"C");
									_swscanf_s_l(
											fullVersionString.GetUTF16(), L"%d.%d", invariantLocalePointer,
											&majorVersion, &minorVersion);
									_free_locale(invariantLocalePointer);
								}
							}
						}
					}
				}
			}
		}
		catch (...) {}

		// Release the COM object references acquired above.
		if (objectPointer)
		{
			try { objectPointer->Release(); }
			catch (...) {}
		}
		if (enumeratorPointer)
		{
			try { enumeratorPointer->Release(); }
			catch (...) {}
		}
		if (servicesPointer)
		{
			try { servicesPointer->Release(); }
			catch (...) {}
		}
		if (locatorPointer)
		{
			try { locatorPointer->Release(); }
			catch (...) {}
		}

		// If we've failed to fetch the version above, then fallback to using Microsoft's deprecated Win32 API.
		if ((majorVersion < 0) || (minorVersion < 0))
		{
			OSVERSIONINFOW osVersionInfo{};
			osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
			auto result = ::GetVersionExW(&osVersionInfo);
			if (result)
			{
				majorVersion = osVersionInfo.dwMajorVersion;
				minorVersion = osVersionInfo.dwMinorVersion;
			}
		}

		// Generate a "<Major>.<Minor>" version string.
		if ((majorVersion >= 0) && (minorVersion >= 0))
		{
			std::stringstream stringStream;
			stringStream.imbue(std::locale::classic());
			stringStream << majorVersion << '.' << minorVersion;
			sWindowsVersionString = stringStream.str();
		}
		else
		{
			sWindowsVersionString = "Unknown";
		}
	}

	// Return the requested OS version string.
	return sWindowsVersionString.c_str();
}

const char* WinDevice::GetArchitectureInfo() const
{
	SYSTEM_INFO sysInfo;
	GetNativeSystemInfo(&sysInfo);
	switch (sysInfo.wProcessorArchitecture)
	{
		case PROCESSOR_ARCHITECTURE_AMD64:
			return "x64"; //AMD or Intel
		case PROCESSOR_ARCHITECTURE_ARM:
			return "ARM";
		case PROCESSOR_ARCHITECTURE_IA64:
			return "IA64";
		case PROCESSOR_ARCHITECTURE_INTEL:
			return "x86";
	}
	return "unknown";
}

PlatformInputDeviceManager& WinDevice::GetInputDeviceManager()
{
	return fInputDeviceManager;
}

#pragma endregion


#pragma region Private Methods
// Fetches the local machine's SID (Security Identifier) and copies it to the given argument.
// Argument "sidString" is the character buffer to be copied to. Will receive a SID string if function returns true.
// Argument "sidStringSize" is the number of characters allocated in "sidString" by the caller.
// Returns true if this function has successfully retrieved the SID and copied it to argument "sidString".
bool WinDevice::CopyMachineSidStringTo(char *sidString, int sidStringSize) const
{
	PSID sidPointer = nullptr;
	SID_NAME_USE sidUseType;
	char *domainName = nullptr;
	char *sidBuffer = nullptr;
	DWORD sidSize = 0;
	DWORD domainNameLength = 0;
	BOOL hasSucceeded;
	bool wasCopied = false;

	// Validate arguments.
	if ((nullptr == sidString) || (sidStringSize <= 0))
	{
		return false;
	}

	// First obtain the size of the SID structure and domain name. This may vary between operating systems.
	hasSucceeded = ::LookupAccountNameA(
			nullptr, GetName(), nullptr, &sidSize, nullptr, &domainNameLength, &sidUseType);
	if ((sidSize > 0) && (domainNameLength > 0))
	{
		// Allocate the SID and domain name to the required sizes.
		sidPointer = (PSID)(new BYTE[sidSize]);
		memset(sidPointer, 0, sidSize);
		domainName = new char[domainNameLength];
		memset(domainName, 0, domainNameLength);

		// Attempt to fetch the local machine's SID.
		hasSucceeded = ::LookupAccountNameA(
				nullptr, GetName(), sidPointer, &sidSize, domainName, &domainNameLength, &sidUseType);
		if (hasSucceeded && ::IsValidSid(sidPointer))
		{
			// We have successfully obtained the SID. Now convert it to a string. We must do this because we
			// don't know how to access the struct's bytes since its size varies on different operating systems.
			if (::ConvertSidToStringSidA(sidPointer, &sidBuffer))
			{
				// Copy the SID string to the argument.
				if ((sidBuffer != nullptr) && (strlen(sidBuffer) < (size_t)sidStringSize))
				{
					strcpy_s(sidString, sidStringSize, sidBuffer);
					wasCopied = true;
				}

				// Delete the SID string buffer. (It was not allocated by us.)
				::LocalFree(sidBuffer);
			}
		}

		// Cleanup.
		delete sidPointer;
		delete domainName;
	}

	// Return true if we successfully copied the SID string to the given argument.
	return wasCopied;
}

// Fetches the first Ethernet device's MAC address on the local machine and copies it to the given argument.
// Argument "addressString" is the character buffer to copy the MAC address string to.
// Argument "addressStringSize" is the number of characters allocated in "addressString" by the caller.
// Returns true if a MAC address was found and successfully copied to the given argument.
bool WinDevice::CopyFirstEthernetMacAddressStringTo(char *addressString, int addressStringSize) const
{
	std::list<std::string> stringCollection;
	std::list<std::string>::iterator iter;

	// Validate arguments.
	if ((nullptr == addressString) || (addressStringSize <= 0))
	{
		return false;
	}

	// Fetch the MAC address strings of all Ethernet devices from the local machine.
	AddEthernetMacAddressStringsTo(stringCollection);

	// Fetch the first MAC address string from the collection, if it has one.
	iter = stringCollection.begin();
	if (iter == stringCollection.end())
	{
		return false;
	}

	// Do not continue if the given string buffer isn't big enough to contain the MAC address.
	if (addressStringSize < (int)(*iter).size())
	{
		return false;
	}

	// Copy the MAC address string to the argument.
	strncpy_s(addressString, addressStringSize, (*iter).c_str(), (*iter).length());
	return true;
}

/// Fetches the MAC address strings (in hexadecimal form) for all Ethernet devices found on this machine.
/// @param stringCollection The collection object to add all MAC address strings to.
/// @return Returns the number of MAC address strings added to the given collection.
///         Returns zero if no Ethernet devices were found.
int WinDevice::AddEthernetMacAddressStringsTo(std::list<std::string> &stringCollection) const
{
	PIP_ADAPTER_INFO adapterCollectionPointer;
	PIP_ADAPTER_INFO adapterInfoPointer;
	ULONG bufferLength;
	int stringCount = 0;

	// First fetch the size of the buffer needed to hold all network adapter objects.
	bufferLength = 0;
	::GetAdaptersInfo(NULL, &bufferLength);
	if (bufferLength > 0)
	{
		// Fetch all network adapters.
		adapterCollectionPointer = (PIP_ADAPTER_INFO)(new BYTE[bufferLength]);
		if (::GetAdaptersInfo(adapterCollectionPointer, &bufferLength) == ERROR_SUCCESS)
		{
			for (adapterInfoPointer = adapterCollectionPointer;
			     adapterInfoPointer != nullptr;
			     adapterInfoPointer = adapterInfoPointer->Next)
			{
				if (MIB_IF_TYPE_ETHERNET == adapterInfoPointer->Type)
				{
					// Found an Ethernet device. Add its MAC address to the given collection.
					int maxBytes = adapterInfoPointer->AddressLength;
					char substring[3];
					std::string addressString;
					addressString.reserve((maxBytes * 2) + 1);
					for (int byteIndex = 0; byteIndex < maxBytes; byteIndex++)
					{
						BYTE value = adapterInfoPointer->Address[byteIndex];
						sprintf_s(substring, sizeof(substring), "%02x", value);
						addressString.append(substring);
					}
					stringCollection.push_back(addressString);
					stringCount++;
				}
			}
		}
		delete adapterCollectionPointer;
	}

	// Return the number of strings added to the given collection.
	return stringCount;
}

#pragma endregion

} // namespace Rtt
