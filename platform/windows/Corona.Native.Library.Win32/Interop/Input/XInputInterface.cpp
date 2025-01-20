//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XInputInterface.h"
#include <Windows.h>


namespace Interop { namespace Input {

#pragma region Static Member Variables
/// <summary>
///  <para>Stores the handle the XInput library loaded via the Win32 LoadLibrary() function.</para>
///  <para>Set to null if XInput library has not been loaded yet or if failed to load the library.</para>
/// </summary>
static HMODULE sModuleHandle;

typedef void(WINAPI *XInputEnableCallback)(BOOL);
static XInputEnableCallback sEnableCallback;

typedef DWORD(WINAPI *XInputGetCapabilitiesCallback)(DWORD, DWORD, XINPUT_CAPABILITIES*);
static XInputGetCapabilitiesCallback sGetCapabilitiesCallback;

typedef DWORD(WINAPI *XInputGetStateCallback)(DWORD, XINPUT_STATE*);
static XInputGetStateCallback sGetStateCallback;

typedef DWORD(WINAPI *XInputSetStateCallback)(DWORD, XINPUT_VIBRATION*);
static XInputSetStateCallback sSetStateCallback;

#pragma endregion


#pragma region Constructors/Destructors
XInputInterface::XInputInterface()
{
	// Attempt to load the newest XInput library available on the system.
	if (!sModuleHandle)
	{
		sModuleHandle = ::LoadLibraryW(L"xinput1_4");
		if (!sModuleHandle)
		{
			sModuleHandle = ::LoadLibraryW(L"xinput1_3");
			if (!sModuleHandle)
			{
				sModuleHandle = ::LoadLibraryW(L"xinput9_1_0");
			}
		}
	}
	if (!sModuleHandle)
	{
		return;
	}

	// Fetch callbacks to the XInput library's functions.
	sEnableCallback = (XInputEnableCallback)::GetProcAddress(sModuleHandle, "XInputEnable");
	sGetCapabilitiesCallback = (XInputGetCapabilitiesCallback)::GetProcAddress(sModuleHandle, "XInputGetCapabilities");
	sGetStateCallback = (XInputGetStateCallback)::GetProcAddress(sModuleHandle, "XInputGetState");
	sSetStateCallback = (XInputSetStateCallback)::GetProcAddress(sModuleHandle, "XInputSetState");
}

XInputInterface::~XInputInterface()
{
}

#pragma endregion


#pragma region Public Methods
bool XInputInterface::CanEnable() const
{
	return (sEnableCallback != nullptr);
}

void XInputInterface::Enable(BOOL value) const
{
	if (sEnableCallback)
	{
		sEnableCallback(value);
	}
}

DWORD XInputInterface::GetCapabilities(DWORD userIndex, DWORD flags, XINPUT_CAPABILITIES* capabilitiesPointer) const
{
	if (!sGetCapabilitiesCallback)
	{
		return ERROR_NOT_SUPPORTED;
	}
	return sGetCapabilitiesCallback(userIndex, flags, capabilitiesPointer);
}

DWORD XInputInterface::GetState(DWORD userIndex, XINPUT_STATE* statePointer) const
{
	if (!sGetStateCallback)
	{
		return ERROR_NOT_SUPPORTED;
	}
	return sGetStateCallback(userIndex, statePointer);
}

DWORD XInputInterface::SetState(DWORD userIndex, XINPUT_VIBRATION* vibrationPointer) const
{
	if (!sSetStateCallback)
	{
		return ERROR_NOT_SUPPORTED;
	}
	return sSetStateCallback(userIndex, vibrationPointer);
}

#pragma endregion


#pragma region Public Static Functions
XInputInterface* XInputInterface::GetInstance()
{
	// Create the singleton. The constructor will load the XInput library.
	// Note: C++11 guarantees that static initialization is thread safe.
	static XInputInterface sXInputInterface;

	// Return null if we've failed to load the XInput library.
	if (!sModuleHandle)
	{
		return nullptr;
	}

	// The XInput library is available. Returns a pointer to this class' singleton.
	return &sXInputInterface;
}

#pragma endregion

} }	// namespace Interop::Input
