//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

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
