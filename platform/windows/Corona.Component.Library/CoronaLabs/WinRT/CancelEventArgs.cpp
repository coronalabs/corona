//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "CancelEventArgs.h"


namespace CoronaLabs { namespace WinRT {

#pragma region Consructors/Destructors
CancelEventArgs::CancelEventArgs()
:	fWasCanceled(false)
{
}

#pragma endregion


#pragma region Public Properties
bool CancelEventArgs::Cancel::get()
{
	return fWasCanceled;
}

void CancelEventArgs::Cancel::set(bool value)
{
	fWasCanceled = value;
}

#pragma endregion

} }	// namespace CoronaLabs::WinRT
