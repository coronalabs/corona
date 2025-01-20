//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InputDeviceInterface.h"
#include "InputDeviceContext.h"


namespace Interop { namespace Input {

#pragma region Constructors/Destructors
InputDeviceInterface::InputDeviceInterface(InputDeviceContext& context)
:	fDeviceContext(context)
{
}

InputDeviceInterface::~InputDeviceInterface()
{
}

#pragma endregion


#pragma region Public Methods
InputDeviceInterface::StatusChangedEvent::HandlerManager& InputDeviceInterface::GetStatusChangedEventHandlers()
{
	return fDeviceContext.fStatusChangedEvent.GetHandlerManager();
}

InputDeviceInterface::ReceivedAxisInputEvent::HandlerManager& InputDeviceInterface::GetReceivedAxisInputEventHandlers()
{
	return fDeviceContext.fReceivedAxisInputEvent.GetHandlerManager();
}

InputDeviceInterface::ReceivedKeyInputEvent::HandlerManager& InputDeviceInterface::GetReceivedKeyInputEventHandlers()
{
	return fDeviceContext.fReceivedKeyInputEvent.GetHandlerManager();
}

InputDeviceInterface::ReceivedVibrationRequestEvent::HandlerManager& InputDeviceInterface::GetReceivedVibrationRequestEventHandlers()
{
	return fDeviceContext.fReceivedVibrationRequestEvent.GetHandlerManager();
}

InputDeviceDriverType InputDeviceInterface::GetDriverType() const
{
	return fDeviceContext.fDriverType;
}

std::shared_ptr<InputDeviceInfo> InputDeviceInterface::GetDeviceInfo() const
{
	return fDeviceContext.fDeviceInfoPointer;
}

Rtt::InputDeviceConnectionState InputDeviceInterface::GetConnectionState() const
{
	return fDeviceContext.fConnectionState;
}

bool InputDeviceInterface::IsConnected() const
{
	return fDeviceContext.fConnectionState.IsConnected();
}

void InputDeviceInterface::RequestVibrate()
{
	if (fDeviceContext.fDeviceInfoPointer->CanVibrate())
	{
		fDeviceContext.fReceivedVibrationRequestEvent.Raise(*this, EventArgs::kEmpty);
	}
}

#pragma endregion

} }	// namespace Interop::Input
