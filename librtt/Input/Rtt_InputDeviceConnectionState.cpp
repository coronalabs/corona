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

#include "Core/Rtt_Build.h"
#include "Rtt_InputDeviceConnectionState.h"
#include <string.h>


namespace Rtt
{

// ----------------------------------------------------------------------------
// Predefined Input Device Connection States
// ----------------------------------------------------------------------------

/// Indicates that the device is disconnected from the system.
const InputDeviceConnectionState InputDeviceConnectionState::kDisconnected(0, "disconnected");

/// Indicates that the device is connected and available to the system.
const InputDeviceConnectionState InputDeviceConnectionState::kConnected(1, "connected");

/// Indicates that the device is connecting to the system.
const InputDeviceConnectionState InputDeviceConnectionState::kConnecting(2, "connecting");

/// Indicates that the device is disconnecting from the system.
const InputDeviceConnectionState InputDeviceConnectionState::kDisconnecting(3, "disconnecting");


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new input device type defaulted to kDisconnected.
InputDeviceConnectionState::InputDeviceConnectionState()
:	fIntegerId(kDisconnected.fIntegerId),
	fStringId(kDisconnected.fStringId)
{
}

/// Creates a new connection state object.
/// <br>
/// This is a private constructor that is used to create this class' preallocated connection state objects.
/// @param integerId The unique integer ID assigned to this connection state.
/// @param stringId The unique string ID assigned to this connection state
InputDeviceConnectionState::InputDeviceConnectionState(S32 integerId, const char *stringId)
:	fIntegerId(integerId),
	fStringId(stringId)
{
}

/// Destroys this device type.
InputDeviceConnectionState::~InputDeviceConnectionState()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Gets the unique integer ID assigned to this connection state.
/// @return Returns a unique integer ID.
S32 InputDeviceConnectionState::GetIntegerId() const
{
	return fIntegerId;
}

/// Gets the unique string ID assigned to this connection state such as "connected", "disconnected", etc.
/// This string ID is typically used in Lua.
/// @return Returns the connection state's unique string ID.
const char* InputDeviceConnectionState::GetStringId() const
{
	return fStringId;
}

/// Determines if this object is in the "connected" state.
/// @return Returns true if this object is in the connected state.
///         <br>
///         Returns false if this object is in the connecting, disconnecting, and disconnected states.
bool InputDeviceConnectionState::IsConnected() const
{
	return (kConnected.GetIntegerId() == fIntegerId);
}

/// Determines if this connection state matches the given connection state.
/// @param value Reference to the connection state to compare against.
/// @return Returns true if the two states match. Returns false if not.
bool InputDeviceConnectionState::Equals(const InputDeviceConnectionState &value) const
{
	return (fStringId == value.fStringId);
}

/// Determines if this connection state matches the given connection state.
/// @param value Reference to the connection state to compare against.
/// @return Returns true if the two states match. Returns false if not.
bool InputDeviceConnectionState::operator==(const InputDeviceConnectionState &value) const
{
	return Equals(value);
}

/// Determines if this connection state does not match the given connection state.
/// @param value Reference to the connection state to compare against.
/// @return Returns true if the connection states do not match. Returns false if they do.
bool InputDeviceConnectionState::NotEquals(const InputDeviceConnectionState &value) const
{
	return !Equals(value);
}

/// Determines if this connection state does not match the given connection state.
/// @param value Reference to the connection state to compare against.
/// @return Returns true if the connection states do not match. Returns false if they do.
bool InputDeviceConnectionState::operator!=(const InputDeviceConnectionState &value) const
{
	return !Equals(value);
}


// ----------------------------------------------------------------------------
// Public Static Functions
// ----------------------------------------------------------------------------

/// Fetches a connection state matching the given integer ID.
/// @param id The unique integer ID of the connection state
///           Matches the value returned by the InputDeviceConnectionState.GetIntegerId() function.
/// @return Returns a connection state object matching the given integer ID.
///         <br>
///         Will return kDisconnected if given an unknown integer ID.
InputDeviceConnectionState InputDeviceConnectionState::FromIntegerId(S32 id)
{
	if (kConnected.GetIntegerId() == id)
	{
		return kConnected;
	}
	else if (kConnecting.GetIntegerId() == id)
	{
		return kConnecting;
	}
	else if (kDisconnecting.GetIntegerId() == id)
	{
		return kDisconnecting;
	}
	return kDisconnected;
}

/// Fetches a connection state matching the given string ID.
/// @param stringId The unique string ID of the connection state.
///                 Matches the string returned by the InputDeviceConnectionState.GetStringId() function.
/// @return Returns a connection state object matching the given string ID.
///         <br>
///         Will return kDisconnected if given an unknown string or NULL.
InputDeviceConnectionState InputDeviceConnectionState::FromStringId(const char *stringId)
{
	if (stringId != NULL)
	{
		if (strcmp(kConnected.GetStringId(), stringId) == 0)
		{
			return kConnected;
		}
		else if (strcmp(kConnecting.GetStringId(), stringId) == 0)
		{
			return kConnecting;
		}
		else if (strcmp(kDisconnecting.GetStringId(), stringId) == 0)
		{
			return kDisconnecting;
		}
	}
	return kDisconnected;
}

// ----------------------------------------------------------------------------

} // namespace Rtt
