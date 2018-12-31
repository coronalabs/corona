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
#include "VibrationRequestManager.h"


namespace Interop { namespace Input {

#pragma region Constructors/Destructors
VibrationRequestManager::VibrationRequestManager()
:	fReceivedVibrationRequestEventHandler(this, &VibrationRequestManager::OnReceivedVibrationRequest)
{
	Reset();
}

VibrationRequestManager::~VibrationRequestManager()
{
}

#pragma endregion


#pragma region Public Methods
const InputDeviceInterface::ReceivedVibrationRequestEvent::Handler* VibrationRequestManager::GetRequestHandler() const
{
	return &fReceivedVibrationRequestEventHandler;
}

VibrationRequestManager::RequestType VibrationRequestManager::ProcessRequests()
{
	// Determine if we need to update the device's vibration state.
	auto result = RequestType::kDoNothing;
	if (fHasReceivedVibrationRequest)
	{
		// A vibration request has been received since the last call to this ProcessRequests() method.
		// Reset the flag so that we can detect the next vibration request.
		fHasReceivedVibrationRequest = false;

		// Schedule the vibration to end 100 milliseconds from now.
		// If the device is already vibrating, then this extends the vibration time.
		fEndVibrationTimeInTicks = Ticks::FromCurrentTime().AddMilliseconds(100);

		// If the device is not currently vibrating, then request the caller to start vibration.
		if (!fIsDeviceVibrating)
		{
			fIsDeviceVibrating = true;
			result = RequestType::kStart;
		}
	}
	else
	{
		// A vibration request has not been received since the last call to this ProcessRequests() method.

		// If the device is currently vibrating, then check if its time to stop it.
		if (fIsDeviceVibrating && (Ticks::FromCurrentTime() >= fEndVibrationTimeInTicks))
		{
			// Request the caller to stop vibrating the device.
			fIsDeviceVibrating = false;
			result = RequestType::kStop;
		}
	}

	// The returned result requests the caller to either start, stop, or do nothing with the device's vibration state.
	return result;
}

void VibrationRequestManager::Reset()
{
	fIsDeviceVibrating = false;
	fHasReceivedVibrationRequest = false;
}

bool VibrationRequestManager::WasVibrationRequested() const
{
	return fIsDeviceVibrating;
}

#pragma endregion


#pragma region Private Methods
void VibrationRequestManager::OnReceivedVibrationRequest(InputDeviceInterface& sender, const EventArgs& arguments)
{
	// Flag that a vibration request has been received.
	// To be applied later when this object's ProcessRequests() method gets called.
	fHasReceivedVibrationRequest = true;
}

#pragma endregion

} }	// namespace Interop::Input
