// ----------------------------------------------------------------------------
// 
// HandledEventArgs.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

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
