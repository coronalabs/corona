//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "TouchEventArgs.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Input {

#pragma region Consructors/Destructors
TouchEventArgs::TouchEventArgs(int pointerId, TouchPhase^ phase, TouchPoint point)
:	fPointerId(pointerId),
	fPhase(phase),
	fPoint(point)
{
	if (!fPhase)
	{
		throw ref new Platform::NullReferenceException("phase");
	}
}

#pragma endregion


#pragma region Public Functions/Properties
int TouchEventArgs::PointerId::get()
{
	return fPointerId;
}

TouchPhase^ TouchEventArgs::Phase::get()
{
	return fPhase;
}

TouchPoint TouchEventArgs::Point::get()
{
	return fPoint;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Input
