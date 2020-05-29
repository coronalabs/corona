//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "TapEventArgs.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Input {

#pragma region Consructors/Destructors
TapEventArgs::TapEventArgs(TouchPoint tapPoint, int tapCount)
:	fTapPoint(tapPoint),
	fTapCount(tapCount),
	fWasHandled(false)
{
	if (tapCount < 1)
	{
		throw ref new Platform::InvalidArgumentException("Argument 'tapCount' cannot be set to less than 1.");
	}
}

#pragma endregion


#pragma region Public Functions/Properties
TouchPoint TapEventArgs::TapPoint::get()
{
	return fTapPoint;
}

int TapEventArgs::TapCount::get()
{
	return fTapCount;
}

bool TapEventArgs::Handled::get()
{
	return fWasHandled;
}

void TapEventArgs::Handled::set(bool value)
{
	fWasHandled = value;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Input
