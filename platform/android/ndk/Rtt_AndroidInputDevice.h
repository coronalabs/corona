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


#ifndef _Rtt_AndroidInputDevice_H__
#define _Rtt_AndroidInputDevice_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include "Input/Rtt_PlatformInputDevice.h"
#include "Input/Rtt_InputDeviceConnectionState.h"


// Forward declarations.
class NativeToJavaBridge;
struct lua_State;
namespace Rtt
{
	class InputDeviceDescriptor;
}


namespace Rtt
{

/// Provides access to one Android input device's information and functionality.
/// Also provides a Lua interface that can be pushed into the Lua state.
class AndroidInputDevice : public PlatformInputDevice
{
	public:
		AndroidInputDevice(const InputDeviceDescriptor &descriptor, NativeToJavaBridge *ntjb);
		virtual ~AndroidInputDevice();

		typedef PlatformInputDevice Super;

		int GetCoronaDeviceId();
		void SetCoronaDeviceId(int id);
		int GetAndroidDeviceId();
		void SetAndroidDeviceId(int id);
		const char* GetProductName();
		void SetProductName(const char *name);
		const char* GetDisplayName();
		void SetDisplayName(const char *name);
		const char* GetPermanentStringId();
		void SetPermanentStringId(const char *stringId);
		int GetPlayerNumber();
		void SetPlayerNumber(int value);
		InputDeviceConnectionState GetConnectionState();
		void SetConnectionState(InputDeviceConnectionState state);
		bool CanVibrate();
		void SetCanVibrate(bool value);
		void Vibrate();

	protected:
		int OnAccessingField(lua_State *L, const char fieldName[]);

	private:
		/// Assignment operator made private to prevent instances from being overwritten.
		void operator=(const AndroidInputDevice &device) { };


		// Unique integer ID assigned to the device by Corona's InputDeviceServices Java class.
		int fCoronaDeviceId;

		/// Unique integer ID assigned to the device by Android.
		/// This value is returned by Android's InputDevice.getId() method in Java.
		int fAndroidDeviceId;

		/// The name of the devices as assigned by the manufacturer.
		String fProductName;

		/// The display name assigned to the input device by the end-user.
		/// This string is returned by Android's InputDevice.getName() method in Java.
		String fDisplayName;

		/// Unique string ID which can uniquely identify the device, even after a reboot.
		/// This string is returned by Android's InputDevice.getDescriptor() method in Java.
		String fPermanentStringId;

		/// Set to true if the device can be vibrate/rumble.
		bool fCanVibrate;

		/// The player number assigned to a game controller by the system.
		/// Set to zero if not assigned a number or if running on an OS older than Android 5.0.
		int fPlayerNumber;

		/// The input device's current connection state such as kConnected, kDisconnected, etc.
		InputDeviceConnectionState fConnectionState;

		NativeToJavaBridge *fNativeToJavaBridge;
};

} // namespace Rtt

#endif // _Rtt_AndroidInputDevice_H__
