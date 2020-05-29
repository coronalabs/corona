//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TapTracker.h"
#include <Windows.h>


namespace Interop { namespace Input {

#pragma region Constructors/Destructors
TapTracker::TapTracker()
:	fInputDeviceType(InputDeviceType::kMouse)
{
	// Reset member variables.
	Reset();

	// Fetch the system's default mouse double click time threshold.
	// Used to increment our tap count when the user rapidly taps/clicks the same point on the screen.
	UINT milliseconds = ::GetDoubleClickTime();
	if (milliseconds < 100)
	{
		milliseconds = 100;
	}
	else if (milliseconds > 5000)
	{
		milliseconds = 5000;
	}
	fMultitapThresholdTimeInMilliseconds = (int)milliseconds;
}

TapTracker::~TapTracker()
{
}

#pragma endregion


#pragma region Public Methods
bool TapTracker::HasTapOccurred() const
{
	return fTapPoint.IsNotNull();
}

POINT TapTracker::GetTapPoint() const
{
	if (fTapPoint.IsNull())
	{
		return POINT{};
	}
	return fTapPoint;
}

int TapTracker::GetTapCount() const
{
	if (fTapPoint.IsNull())
	{
		return 0;
	}
	return (fTapCount >= 0) ? fTapCount : 0;
}

void TapTracker::SetInputDeviceType(const TapTracker::InputDeviceType& type)
{
	fInputDeviceType = type;
}

TapTracker::InputDeviceType TapTracker::GetInputDeviceType() const
{
	return fInputDeviceType;
}

void TapTracker::Reset()
{
	fFirstTouchBeganPoint = nullptr;
	fLastTouchBeganPoint = nullptr;
	fLastTapTicks = nullptr;
	fTapPoint = nullptr;
	fTapCount = 0;
}

void TapTracker::UpdateWith(const POINT& point, const Rtt::TouchEvent::Phase& phase)
{
	// Fetch the current time in ticks.
	auto currentTimeInTicks = Ticks::FromCurrentTime();

	// Clear the last detected tap point.
	fTapPoint = nullptr;

	// Handle the give touch point.
	if (phase == Rtt::TouchEvent::kBegan)
	{
		// Store the "down" position of this touch point.
		// Will be used to check if the pointer was released near the same position later.
		if (fFirstTouchBeganPoint.IsNull())
		{
			fFirstTouchBeganPoint = point;
		}
		fLastTouchBeganPoint = point;

		// Reset the tap count if this touch point is nowhere near the last touch point.
		if (ArePointsWithinTapBounds(fLastTouchBeganPoint, fFirstTouchBeganPoint) == false)
		{
			fTapCount = 0;
			fLastTapTicks = nullptr;
			fFirstTouchBeganPoint = fLastTouchBeganPoint;
		}
	}
	else if ((phase == Rtt::TouchEvent::kEnded) && fLastTouchBeganPoint.IsNotNull())
	{
		// Register a "tap" event if pointer has been released at the same location it began.
		if (ArePointsWithinTapBounds(point, fLastTouchBeganPoint))
		{
			// Set the tap count. Increment it if this tap was received within a certain time since the last tap.
			if (fLastTapTicks.IsNull() ||
			    (Ticks(fLastTapTicks).AddMilliseconds(fMultitapThresholdTimeInMilliseconds) < currentTimeInTicks))
			{
				fTapCount = 1;
			}
			else if (fTapCount < INT_MAX)
			{
				fTapCount++;
			}

			// Store the tap point.
			fLastTapTicks = currentTimeInTicks;
			fTapPoint = point;

			// Clear the last touch event.
			fLastTouchBeganPoint = nullptr;
		}
	}
	else if (phase == Rtt::TouchEvent::kMoved)
	{
		// Reset tap tracking if the touch point was dragged too far from the start point.
		if (fLastTouchBeganPoint.IsNotNull() && (ArePointsWithinTapBounds(point, fLastTouchBeganPoint) == false))
		{
			Reset();
		}
	}
	else if (phase == Rtt::TouchEvent::kCancelled)
	{
		// Reset tap tracking if the last tracked touch input has been canceled.
		Reset();
	}
}

#pragma endregion


#pragma region Private Methods
bool TapTracker::ArePointsWithinTapBounds(const POINT& point1, const POINT& point2) const
{
	LONG tapTolerance = 1;
	if (InputDeviceType::kTouchscreen == fInputDeviceType)
	{
//TODO: Don't hard-code the tolerance/threshold for a touchscreen.
//      Should be set by the TapTracker's owner and be based on screen DPI.
		tapTolerance = 40;
	}
	auto deltaX = std::abs(point1.x - point2.x);
	auto deltaY = std::abs(point1.y - point2.y);
	return ((deltaX <= tapTolerance) && (deltaY <= tapTolerance));
}

#pragma endregion

} }	// namespace Interop::Input
