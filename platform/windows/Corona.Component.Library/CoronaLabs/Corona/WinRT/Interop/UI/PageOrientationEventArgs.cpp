//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "PageOrientationEventArgs.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

#pragma region Consructors/Destructors
PageOrientationEventArgs::PageOrientationEventArgs(PageOrientation^ orientation)
:	fOrientation(orientation)
{
	if (orientation == nullptr)
	{
		throw ref new Platform::NullReferenceException();
	}
}

#pragma endregion


#pragma region Public Functions/Properties
PageOrientation^ PageOrientationEventArgs::Orientation::get()
{
	return fOrientation;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI
