//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
