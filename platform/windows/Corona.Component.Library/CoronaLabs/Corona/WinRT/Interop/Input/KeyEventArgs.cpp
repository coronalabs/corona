// ----------------------------------------------------------------------------
// 
// KeyEventArgs.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "KeyEventArgs.h"
#include "Key.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Input {

#pragma region Consructors/Destructors
KeyEventArgs::KeyEventArgs(
	CoronaLabs::Corona::WinRT::Interop::Input::Key^ key,
	bool isAltDown, bool isControlDown, bool isShiftDown, bool isCommandDown)
:	fKey(key),
	fIsAltDown(isAltDown),
	fIsControlDown(isControlDown),
	fIsShiftDown(isShiftDown),
	fIsCommandDown(isCommandDown)
{
	if (!key)
	{
		throw ref new Platform::NullReferenceException("key");
	}
}

#pragma endregion


#pragma region Public Functions/Properties
CoronaLabs::Corona::WinRT::Interop::Input::Key^ KeyEventArgs::Key::get()
{
	return fKey;
}

bool KeyEventArgs::IsAltDown::get()
{
	return fIsAltDown;
}

bool KeyEventArgs::IsControlDown::get()
{
	return fIsControlDown;
}

bool KeyEventArgs::IsShiftDown::get()
{
	return fIsShiftDown;
}

bool KeyEventArgs::IsCommandDown::get()
{
	return fIsCommandDown;
}

bool KeyEventArgs::Handled::get()
{
	return fWasHandled;
}

void KeyEventArgs::Handled::set(bool value)
{
	fWasHandled = value;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Input
