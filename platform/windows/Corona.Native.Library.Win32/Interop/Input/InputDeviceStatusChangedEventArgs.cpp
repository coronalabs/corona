//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InputDeviceStatusChangedEventArgs.h"


namespace Interop { namespace Input {

#pragma region Constructors/Destructors
InputDeviceStatusChangedEventArgs::InputDeviceStatusChangedEventArgs(
	const InputDeviceStatusChangedEventArgs::Settings& settings)
:	EventArgs(),
	fSettings(settings)
{
}

InputDeviceStatusChangedEventArgs::~InputDeviceStatusChangedEventArgs()
{
}

#pragma endregion


#pragma region Public Methods
bool InputDeviceStatusChangedEventArgs::HasConnectionStateChanged() const
{
	return fSettings.HasConnectionStateChanged;
}

bool InputDeviceStatusChangedEventArgs::WasReconfigured() const
{
	return fSettings.WasReconfigured;
}

InputDeviceStatusChangedEventArgs::Settings InputDeviceStatusChangedEventArgs::ToSettings() const
{
	return fSettings;
}

#pragma endregion

} }	// namespace Interop::Input
