//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "CoronaRuntimeEventArgs.h"
#include "CoronaRuntimeEnvironment.h"


namespace CoronaLabs { namespace Corona { namespace WinRT {

#pragma region Consructors/Destructors
CoronaRuntimeEventArgs::CoronaRuntimeEventArgs(CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ environment)
:	fEnvironment(environment)
{
	if (environment == nullptr)
	{
		throw ref new Platform::NullReferenceException();
	}
}

#pragma endregion


#pragma region Public Functions/Properties
CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ CoronaRuntimeEventArgs::CoronaRuntimeEnvironment::get()
{
	return fEnvironment;
}

#pragma endregion

} } }	// namespace CoronaLabs::Corona::WinRT
