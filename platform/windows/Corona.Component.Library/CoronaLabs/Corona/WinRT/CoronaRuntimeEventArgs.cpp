// ----------------------------------------------------------------------------
// 
// CoronaRuntimeEventArgs.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

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
