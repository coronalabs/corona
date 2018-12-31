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


#include "Rtt_AndroidInputDevice.h"
#include "Core/Rtt_Build.h"
#include "Input/Rtt_InputDeviceDescriptor.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "NativeToJavaBridge.h"


namespace Rtt
{	

// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new interface to an Android input device.
/// @param descriptor Unique descriptor used to identify this input device.
AndroidInputDevice::AndroidInputDevice(const InputDeviceDescriptor &descriptor, NativeToJavaBridge *ntjb)
:	Super(descriptor),
	fCoronaDeviceId(-1),
	fAndroidDeviceId(-1),
	fProductName(descriptor.GetAllocator()),
	fDisplayName(descriptor.GetAllocator()),
	fPermanentStringId(descriptor.GetAllocator()),
	fCanVibrate(false),
	fPlayerNumber(0),
	fConnectionState(InputDeviceConnectionState::kDisconnected),
	fNativeToJavaBridge(ntjb)
{
}

/// Destroys this input device and its resources.
AndroidInputDevice::~AndroidInputDevice()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Gets the unique integer ID assigned to this input device by the Java side of Corona.
/// This ID is never exposed to Lua and is only used internally by Corona.
/// @return Returns Corona's unique integer ID for this device.
int AndroidInputDevice::GetCoronaDeviceId()
{
	return fCoronaDeviceId;
}

/// Sets the unique integer ID assigned to this input device by the Java side of Corona.
/// This ID is never exposed to Lua and is only used internally by Corona.
/// @param id The unique integer ID to be assigned to this device.
void AndroidInputDevice::SetCoronaDeviceId(int id)
{
	fCoronaDeviceId = id;
}

/// Gets the unique integer ID assigned to this input device by Android.
/// This ID is returned by Android's InputDevice.getId() method in Java.
/// @return Returns the unique integer ID assigned to this input device by Android.
///         <br>
///         Returns -1 if this is not an Android managed input device. This indicates that
///         this is a non-HID device added to Corona via the "InputDeviceManager" in Java.
int AndroidInputDevice::GetAndroidDeviceId()
{
	return fAndroidDeviceId;
}

/// Sets the unique integer ID assigned to this input device by Android.
/// This ID is returned by Android's InputDevice.getId() method in Java.
/// @param id The unique integer ID that Android has assigned this device.
///           <br>
///           Set to -1 to indicate that this is not an Android managed device.
void AndroidInputDevice::SetAndroidDeviceId(int id)
{
	fAndroidDeviceId = id;
}

/// Gets the product name assigned to this device as set by the manufacturer.
/// @return Returns this device's product name.
///         <br>
///         Returns NULL or empty string if a product name could not be obtained.
const char* AndroidInputDevice::GetProductName()
{
	return fProductName.GetString();
}

/// Sets the product name assigned to this device as set by the manufacturer.
/// @param name The device's product name.
///             <br>
///             Set to NULL or empty string to indicate that this device does not have
///             a product name.
void AndroidInputDevice::SetProductName(const char *name)
{
	fProductName.Set(name);
}

/// Gets the human readable display name assigned to this device.
/// <br>
/// This name is typically the product name unless the end-user changes it via the bluetooth
/// settings screen on Android.
/// <br>
/// This name is typically returned by Android's InputDevice.getName() method in Java unless
/// it is a custom non-HID input device added to Corona via the InputDeviceServices class.
/// @return Returns the device's display name.
///         <br>
///         Returns NULL or empty string if this device does not have an display name.
const char* AndroidInputDevice::GetDisplayName()
{
	return fDisplayName.GetString();
}

/// Set the human readable display name assigned to this device.
/// <br>
/// This name is typically the product name unless the end-user changes it via the bluetooth
/// settings screen on Android.
/// <br>
/// This name is typically returned by Android's InputDevice.getName() method in Java unless
/// it is a custom non-HID input device added to Corona via the InputDeviceServices class.
/// @param name The device's display name.
///             <br>
///             Set to NULL or empty string if the device does not have an display name.
void AndroidInputDevice::SetDisplayName(const char *name)
{
	fDisplayName.Set(name);
}

/// Gets a unique string ID assigned to the device that can be saved to file.
/// <br>
/// Unlike the device's descriptor name or Android device ID, this string ID will always
/// identify this specific device. Even after restarting the app or rebooting the device.
/// <br>
/// This string ID is returned by Android's InputDevice.getDescriptor() method unless it
/// comes from a non-HID device that was added to Corona via the InputDeviceServices class.
/// @return Returns the device's unique string ID.
///         <br>
///         Returns NULL if the device does not have a permanent string ID assigned to it.
const char* AndroidInputDevice::GetPermanentStringId()
{
	return fPermanentStringId.GetString();
}

/// Sets a unique string ID that is assigned to the device that can be saved to file.
/// <br>
/// Unlike the device's descriptor name or Android device ID, this string ID will always
/// identify this specific device. Even after restarting the app or rebooting the device.
/// <br>
/// This string ID is returned by Android's InputDevice.getDescriptor() method unless it
/// comes from a non-HID device that was added to Corona via the InputDeviceServices class.
/// @param stringId The device's permanent string ID.
///                 <br>
///                 Set to NULL or empty string if the device does not have a permanent string ID.
void AndroidInputDevice::SetPermanentStringId(const char *stringId)
{
	fPermanentStringId.Set(stringId);
}

/// Gets a one based player number assigned to the device by the system.
/// @return Returns a 1 based number assigned to the device.
///         <br>
///         Returns a value less than 1 if a number was not assigned to the device.
int AndroidInputDevice::GetPlayerNumber()
{
	return fPlayerNumber;
}

/// Sets the player number assigned to the device by the system, if supported.
/// @param value A one based player number assigned to the game controller by the system.
///              <br>
///              Set to zero to indicate that a player number is not assigned to the device.
void AndroidInputDevice::SetPlayerNumber(int value)
{
	fPlayerNumber = value;
}

/// Gets the input device's current connection state such as kConnected, kDisconnected, etc.
/// @return Returns the input device's current connection state.
InputDeviceConnectionState AndroidInputDevice::GetConnectionState()
{
	return fConnectionState;
}

/// Sets the connection state of the input device.
/// @param value The current connection state such as kConnected, kDisconnected, etc.
void AndroidInputDevice::SetConnectionState(InputDeviceConnectionState state)
{
	fConnectionState = state;
}

/// Determines if the device supports vibration/rumble feedback.
/// @return Returns true if the device can be vibrated via the Vibrate() function.
///         <br>
///         Returns false if vibration/rumble feedback is not supported by the device.
bool AndroidInputDevice::CanVibrate()
{
	return fCanVibrate;
}

/// Sets whether or not the device supports vibration/rumble feedback.
/// <br>
/// Vibration support can be checked by calling the Android InputDevice.getVibrator().hasVibrator() method.
/// @param value Set true to indicate the device supports vibration via the Vibrate() function.
///              <br>
///              Set false if vibration/rumble feedback is not supported by the device.
void AndroidInputDevice::SetCanVibrate(bool value)
{
	fCanVibrate = value;
}

/// Makes the device vibrate/rumble, if supported.
/// <br>
/// You can determine if vibration/rumble feedback is supported by calling the CanVibrate() function.
void AndroidInputDevice::Vibrate()
{
	fNativeToJavaBridge->VibrateInputDevice(fCoronaDeviceId);
}


// ----------------------------------------------------------------------------
// Protected Member Functions
// ----------------------------------------------------------------------------

/// Called when Lua is attempting to access the input device object's property or function.
/// @param L The Lua state that is performing the access operation.
/// @param fieldName The name of the field being accessed such as "type", "productName", etc.
/// @return Returns the number of values being returned to Lua.
int AndroidInputDevice::OnAccessingField(lua_State *L, const char fieldName[])
{
	int result = 0;

	// Handle the accessed field.
	if (strcmp(fieldName, "androidDeviceId") == 0)
	{
		if (fAndroidDeviceId >= 0)
		{
			lua_pushinteger(L, fAndroidDeviceId);
		}
		else
		{
			lua_pushnil(L);
		}
		result = 1;
	}
	else
	{
		result = Super::OnAccessingField(L, fieldName);
	}

	// Return the number of Lua values returned by this field.
	return result;
}

} // namespace Rtt
