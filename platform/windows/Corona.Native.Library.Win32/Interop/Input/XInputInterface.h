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

#pragma once

#include "Core\Rtt_Macros.h"
#include <Xinput.h>

// Include Corona's "Key.h" header file to define XInput key code constants that are not available
// in older versions of the "XInput.h" header file.
#include "Key.h"

// Define additional XInput constants that are not available in older versions of the "XInput.h" header file.
#ifndef XUSER_MAX_COUNT
#	define XUSER_MAX_COUNT 4
#endif
#ifndef XINPUT_DEVSUBTYPE_UNKNOWN
#	define XINPUT_DEVSUBTYPE_UNKNOWN 0x00
#endif
#ifndef XINPUT_DEVSUBTYPE_WHEEL
#	define XINPUT_DEVSUBTYPE_WHEEL 0x02
#endif
#ifndef XINPUT_DEVSUBTYPE_ARCADE_STICK
#	define XINPUT_DEVSUBTYPE_ARCADE_STICK 0x03
#endif
#ifndef XINPUT_DEVSUBTYPE_FLIGHT_STICK
#	define XINPUT_DEVSUBTYPE_FLIGHT_STICK 0x04
#endif
#ifndef XINPUT_DEVSUBTYPE_DANCE_PAD
#	define XINPUT_DEVSUBTYPE_DANCE_PAD 0x05
#endif
#ifndef XINPUT_DEVSUBTYPE_GUITAR
#	define XINPUT_DEVSUBTYPE_GUITAR 0x06
#endif
#ifndef XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE
#	define XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE 0x07
#endif
#ifndef XINPUT_DEVSUBTYPE_DRUM_KIT
#	define XINPUT_DEVSUBTYPE_DRUM_KIT 0x08
#endif
#ifndef XINPUT_DEVSUBTYPE_GUITAR_BASS
#	define XINPUT_DEVSUBTYPE_GUITAR_BASS 0x0B
#endif
#ifndef XINPUT_DEVSUBTYPE_ARCADE_PAD
#	define XINPUT_DEVSUBTYPE_ARCADE_PAD 0x13
#endif
#ifndef XINPUT_CAPS_FFB_SUPPORTED
#	define XINPUT_CAPS_FFB_SUPPORTED 0x01
#endif


namespace Interop { namespace Input {

/// <summary>
///  <para>Singleton class providing an interface to the Microsoft XInput library's available functions.</para>
///  <para>This class ensures that the newest version of the XInput DLL available on the system gets loaded.</para>
///  <para>
///   You cannot create instances of this class.
///   Instead, you must access this class' singleton via the static GetInstance() function.
///  </para>
/// </summary>
class XInputInterface final
{
	Rtt_CLASS_NO_COPIES(XInputInterface)

	private:
		/// <summary>Creates a new interface to the XInput library.</summary>
		/// <remarks>Constructor made private to force developers to use static GetInstance() function.</remarks>
		XInputInterface();

		/// <summary>Destroys this object.</summary>
		virtual ~XInputInterface();

	public:
		/// <summary>
		///  <para>Determines if the Enable() method is supported by the currently loaded XInput library.</para>
		///  <para>The Enable() method is only supported if XInput DLL version 1.3 or newer was loaded.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns true if the Enable() method is supported and can be called.</para>
		///  <para>
		///   Returns false if the currently loaded XInput library does not support the Enable() method.
		///   This means that the Enable() method will no-op if called.
		///  </para>
		/// </returns>
		bool CanEnable() const;

		/// <summary>Enables/disables XInput device data reporting.</summary>
		/// <param name="value">
		///  <para>Set TRUE (the default) to allow the GetState() method to fetch XInput device data.</para>
		///  <para>Set FALSE to disable data collection. Causes the GetState() method to return neutral/zero data.</para>
		/// </param>
		void Enable(BOOL value) const;

		/// <summary>Fetches information about the input device for the given user index.</summary>
		/// <param name="userIndex">
		///  Zero based index of the user's controller. Must range between zero and XUSER_MAX_COUNT.
		/// </param>
		/// <param name="flags">
		///  <para>Flags that identify the controller type to query.</para>
		///  <para>Set to zero to fetch info for any controller type.</para>
		///  <para>Set to XINPUT_FLAG_GAMEPAD to only fetch info from an Xbox 360 gamepad.</para>
		/// </param>
		/// <param name="capabilitiesPointer">
		///  Pointer to a structure that will receive information about the input device for the given user index.
		/// </param>
		/// <returns>
		///  <para>Returns ERROR_SUCCESS if device information was successfully received.</para>
		///  <para>Returns ERROR_DEVICE_NOT_CONNECTED if a device is not available/connected for the given user index.</para>
		///  <para>Returns an error code defined in "WinError.h" for all other errors.</para>
		/// </returns>
		DWORD GetCapabilities(DWORD userIndex, DWORD flags, XINPUT_CAPABILITIES* capabilitiesPointer) const;

		/// <summary>Fetches the current data/state of the indexed input device.</summary>
		/// <param name="userIndex">
		///  Zero based index of the user's controller. Must range between zero and XUSER_MAX_COUNT.
		/// </param>
		/// <param name="statePointer">
		///  Pointer to a structure that will receive the input device's current state information.
		/// </param>
		/// <returns>
		///  <para>Returns ERROR_SUCCESS if the device's current state information was successfully received.</para>
		///  <para>Returns ERROR_DEVICE_NOT_CONNECTED if a device is not available/connected for the given user index.</para>
		///  <para>Returns an error code defined in "WinError.h" for all other errors.</para>
		/// </returns>
		DWORD GetState(DWORD userIndex, XINPUT_STATE* statePointer) const;

		/// <summary>Sets the indexed device's vibration motor speed.</summary>
		/// <param name="userIndex">
		///  Zero based index of the user's controller. Must range between zero and XUSER_MAX_COUNT.
		/// </param>
		/// <param name="vibrationPointer">Pointer to the vibration information to be used.</param>
		/// <returns>
		///  <para>Returns ERROR_SUCCESS if the vibration request was successfully transmitted.</para>
		///  <para>Returns ERROR_DEVICE_NOT_CONNECTED if a device is not available/connected for the given user index.</para>
		///  <para>Returns an error code defined in "WinError.h" for all other errors.</para>
		/// </returns>
		DWORD SetState(DWORD userIndex, XINPUT_VIBRATION* vibrationPointer) const;

		/// <summary>Fetches an interface to the newest version of the Microsoft XInput library.</summary>
		/// <returns>
		///  <para>Returns an interface pointer used to access the XInput library's functions.</para>
		///  <para>Returns null if the XInput library is not available on the system.</para>
		/// </returns>
		static XInputInterface* GetInstance();
};

} }	// namespace Interop::Input
