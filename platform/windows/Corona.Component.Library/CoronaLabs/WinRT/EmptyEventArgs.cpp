//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

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
