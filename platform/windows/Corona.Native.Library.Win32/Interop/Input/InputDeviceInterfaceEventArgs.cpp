//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InputDeviceInterfaceEventArgs.h"
#include "InputDeviceInterface.h"


namespace Interop { namespace Input {

#pragma region Constructors/Destructors
InputDeviceInterfaceEventArgs::InputDeviceInterfaceEventArgs(InputDeviceInterface& deviceInterface)
:	EventArgs(),
	fDeviceInterface(deviceInterface)
{
}

InputDeviceInterfaceEventArgs::~InputDeviceInterfaceEventArgs()
{
}

#pragma endregion


#pragma region Public Methods
InputDeviceInterface& InputDeviceInterfaceEventArgs::GetDeviceInterface() const
{
	return fDeviceInterface;
}

#pragma endregion

} }	// namespace Interop::Input
