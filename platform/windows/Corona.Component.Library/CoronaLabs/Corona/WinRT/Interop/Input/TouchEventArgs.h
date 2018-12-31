// ----------------------------------------------------------------------------
// 
// TouchEventArgs.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif

#include "TouchPoint.h"
#include "TouchPhase.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Input {

/// <summary>Provides a touch event's coordinates, phase, and timestamp.</summary>
public ref class TouchEventArgs sealed
{
	public:
		/// <summary>Creates a new object for storing a touch event's data.</summary>
		/// <param name="pointerId">Unique integer ID assigned to the pointer/finger that generated this event.</param>
		/// <param name="phase">
		///  <para>Indicates if the touch has Began, Moved, Ended, or Canceled.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		/// <param name="point">Indicates where onscreen the touch occurred on.</param>
		TouchEventArgs(int pointerId, TouchPhase^ phase, TouchPoint point);

		/// <summary>Gets the unique integer ID assigned to the pointer/finger that generated this event.</summary>
		/// <value>
		///  The pointer/finger's unique ID. You use this ID to differentiate between the touch events generated
		///  by other pointers/fingers that are on the screen at the same time.
		/// </value>
		property int PointerId { int get(); }

		/// <summary>Indicates if touch input began, moved, ended, or was canceled.</summary>
		/// <value>The touch event's phase.</value>
		property TouchPhase^ Phase { TouchPhase^ get(); }

		/// <summary>Gets the coordinate on screen of the touch as well as its timestamp.</summary>
		/// <value>The x and y coordinate of where the touch occurred on.</value>
		property TouchPoint Point { TouchPoint get(); }

	private:
		int fPointerId;
		TouchPhase^ fPhase;
		TouchPoint fPoint;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Input
