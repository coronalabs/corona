//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_InputDeviceConnectionState_H__
#define _Rtt_InputDeviceConnectionState_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Types.h"


namespace Rtt
{

/// Indicates an input device's connection state such as connecting, connected, disconnected, etc.
/// Provides a unique IDs that are used by Corona events.
class InputDeviceConnectionState
{
	private:
		InputDeviceConnectionState(S32 integerId, const char *stringId);

	public:
		InputDeviceConnectionState();
		virtual ~InputDeviceConnectionState();

		static const InputDeviceConnectionState kDisconnected;
		static const InputDeviceConnectionState kConnected;
		static const InputDeviceConnectionState kConnecting;
		static const InputDeviceConnectionState kDisconnecting;

		S32 GetIntegerId() const;
		const char* GetStringId() const;
		bool IsConnected() const;
		bool Equals(const InputDeviceConnectionState &value) const;
		bool NotEquals(const InputDeviceConnectionState &value) const;
		bool operator==(const InputDeviceConnectionState &value) const;
		bool operator!=(const InputDeviceConnectionState &value) const;

		static InputDeviceConnectionState FromIntegerId(S32 id);
		static InputDeviceConnectionState FromStringId(const char *stringId);

	private:
		/// Stores the connection state's unique integer ID.
		S32 fIntegerId;

		/// The unique string ID assigned to the state such as "connected", "disconnected", etc.
		/// <br>
		/// The string ID is typically used by a Lua event table.
		const char *fStringId;
};

} // namespace Rtt

#endif // _Rtt_InputDeviceConnectionState_H__
