//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "HandledEventArgs.h"


namespace CoronaLabs { namespace WinRT {

#pragma region Consructors/Destructors
HandledEventArgs::HandledEventArgs()
:	fWasHandled(false)
{
}

#pragma endregion


#pragma region Public Properties
bool HandledEventArgs::Handled::get()
{
	return fWasHandled;
}

void HandledEventArgs::Handled::set(bool value)
{
	fWasHandled = value;
}

#pragma endregion

} }	// namespace CoronaLabs::WinRT
