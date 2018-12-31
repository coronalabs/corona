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
