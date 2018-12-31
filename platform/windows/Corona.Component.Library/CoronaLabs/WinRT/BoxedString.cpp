// ----------------------------------------------------------------------------
// 
// BoxedString.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "BoxedString.h"


namespace CoronaLabs { namespace WinRT {

#pragma region Consructors/Destructors
BoxedString::BoxedString(Platform::String^ value)
:	fValue(value)
{
}

#pragma endregion


#pragma region Public Methods/Properties
Platform::String^ BoxedString::Value::get()
{
	return fValue;
}

Platform::String^ BoxedString::ToString()
{
	return fValue;
}

#pragma endregion

} }	// namespace CoronaLabs::WinRT
