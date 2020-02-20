//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "TouchTracker.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Input {

#pragma region Consructors/Destructors
TouchTracker::TouchTracker(int pointerId)
{
	fPointerId = pointerId;
	Reset();
}

#pragma endregion


#pragma region Public Properties
bool TouchTracker::IsTracking::get()
{
	return (fPhase != nullptr);
}

bool TouchTracker::IsNotTracking::get()
{
	return (fPhase == nullptr);
}

int TouchTracker::PointerId::get()
{
	return fPointerId;
}

TouchPoint TouchTracker::StartPoint::get()
{
	return fStartPoint;
}

TouchPoint TouchTracker::LastPoint::get()
{
	return fLastPoint;
}

TouchPhase^ TouchTracker::Phase::get()
{
	return fPhase;
}

#pragma endregion


#pragma region Public Methods
void TouchTracker::Reset()
{
	memset(&fStartPoint, 0, sizeof(TouchPoint));
	memset(&fLastPoint, 0, sizeof(TouchPoint));
	fPhase = nullptr;
}

void TouchTracker::UpdateWith(TouchPhase^ phase, TouchPoint point)
{
	// Validate.
	if (nullptr == phase)
	{
		throw ref new Platform::NullReferenceException("phase");
	}

	// Set the start point, if applicable.
	if ((TouchPhase::Began == phase) || (0 == fStartPoint.Timestamp.UniversalTime))
	{
		fStartPoint = point;
	}

	// Store the current touch point and phase.
	fLastPoint = point;
	fPhase = phase;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Input
