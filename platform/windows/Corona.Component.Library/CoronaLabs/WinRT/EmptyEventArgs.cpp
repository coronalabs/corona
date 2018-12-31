// ----------------------------------------------------------------------------
// 
// EmptyEventArgs.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "EmptyEventArgs.h"


namespace CoronaLabs { namespace WinRT {

#pragma region Consructors/Destructors
EmptyEventArgs::EmptyEventArgs()
{
}

#pragma endregion


#pragma region Public Static Functions
EmptyEventArgs^ EmptyEventArgs::Instance::get()
{
	static EmptyEventArgs kInstance;
	return %kInstance;
}

#pragma endregion

} }	// namespace CoronaLabs::WinRT
