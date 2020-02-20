//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------
// This is a stub to enable building CoronaBuilder
// without importing most of the Simulator code
// ------------------------------------------------

#include "stdafx.h"

#include "Core/Rtt_Build.h"

// Enables the Crypto++ library's MD4 and MD5 support without compiler warnings.
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include "CryptoPP\md5.h"

#include "WinString.h"

#include "Rtt_WinConsoleDevice.h"

#include <iphlpapi.h>
#include <Sddl.h>
#include <sstream>
#include <stdio.h>
#include <string>
#include <WbemIdl.h>
#include <windows.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static char kNull[] = "(null)";

WinConsoleDevice::WinConsoleDevice(Rtt_Allocator &allocator)
	: fInputDeviceManager(&allocator),
	fDeviceIdentifier(nullptr),
	fComputerName(nullptr)
{
}

WinConsoleDevice::~WinConsoleDevice()
{
	if (fDeviceIdentifier != NULL)
	{
		delete fDeviceIdentifier;
	}
	if (fComputerName != NULL)
	{
		delete fComputerName;
	}
	
}

const char*
WinConsoleDevice::GetModel() const
{
	return kNull;
}

const char*
WinConsoleDevice::GetName() const
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

const char* 
WinConsoleDevice::GetUniqueIdentifier(IdentifierType idType) const
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
				_snprintf_s(stringBuffer, sizeof(stringBuffer),
					"Warning: Unable to fetch primary machine ID. (Error Code: %d)\r\n", ::GetLastError());
				Rtt_LogException(stringBuffer);
				if (CopyFirstEthernetMacAddressStringTo(stringBuffer, sizeof(stringBuffer)) == false)
				{
					// Unable to find a MAC address. Fallback to a hardcoded "0" for the string ID.
					Rtt_LogException("Warning: Unable to fetch secondary machine ID.\r\n");
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

MPlatformDevice::EnvironmentType
WinConsoleDevice::GetEnvironment() const
{
	return kDeviceEnvironment;
}

const char*
WinConsoleDevice::GetPlatformName() const
{
	return "Win";
}
	
const char*
WinConsoleDevice::GetPlatformVersion() const
{
	return NULL;
}

const char*
WinConsoleDevice::GetArchitectureInfo() const
{
	return NULL;
}

PlatformInputDeviceManager&
WinConsoleDevice::GetInputDeviceManager()
{
	return fInputDeviceManager;
}

void
WinConsoleDevice::Vibrate() const
{
}

bool
WinConsoleDevice::HasEventSource( EventType type ) const
{
	bool hasEventSource = false;
	
	return hasEventSource;
}

void
WinConsoleDevice::BeginNotifications( EventType type ) const
{
}

void
WinConsoleDevice::EndNotifications( EventType type ) const
{
}

bool
WinConsoleDevice::DoesNotify( EventType type ) const
{
	return false;
}

void
WinConsoleDevice::SetAccelerometerInterval( U32 frequency ) const
{
}

void
WinConsoleDevice::SetGyroscopeInterval( U32 frequency ) const
{
}

void
WinConsoleDevice::SetLocationAccuracy( Real meters ) const
{
}

void
WinConsoleDevice::SetLocationThreshold( Real meters ) const
{
}

DeviceOrientation::Type
WinConsoleDevice::GetOrientation() const
{
	return DeviceOrientation::kUnknown;
}

#pragma region Private Methods
// Fetches the local machine's SID (Security Identifier) and copies it to the given argument.
// Argument "sidString" is the character buffer to be copied to. Will receive a SID string if function returns true.
// Argument "sidStringSize" is the number of characters allocated in "sidString" by the caller.
// Returns true if this function has successfully retrieved the SID and copied it to argument "sidString".
bool WinConsoleDevice::CopyMachineSidStringTo(char *sidString, int sidStringSize) const
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
bool WinConsoleDevice::CopyFirstEthernetMacAddressStringTo(char *addressString, int addressStringSize) const
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
int WinConsoleDevice::AddEthernetMacAddressStringsTo(std::list<std::string> &stringCollection) const
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

// ----------------------------------------------------------------------------

