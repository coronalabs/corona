// ----------------------------------------------------------------------------
// 
// PageOrientationEventArgs.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

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
