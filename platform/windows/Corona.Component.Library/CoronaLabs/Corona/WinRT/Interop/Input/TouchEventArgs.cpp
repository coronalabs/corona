// ----------------------------------------------------------------------------
// 
// TouchEventArgs.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

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
