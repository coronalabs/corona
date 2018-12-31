// ----------------------------------------------------------------------------
// 
// TouchTracker.h
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

#include "TouchPhase.h"
#include "TouchPoint.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Input {

/// <summary>Tracks the touch events from one pointer/finger.</summary>
public ref class TouchTracker sealed
{
	public:
		/// <summary>Creates a new tracker for recording touch events from a single pointer/finger.</summary>
		/// <param name="pointerId">Unique integer ID assigned to the pointer/finger that is generating the touch events.</param>
		TouchTracker(int pointerId);

		/// <summary>Determines if touch tracking has started.</summary>
		/// <value>
		///  <para>
		///   Set to true if this tracker has received at least one touch point via its UpdateWith() method.
		///   This means that the StartPoint, LastPoint, and Phase properties will provide valid values.
		///  </para>
		///  <para>Set to false if this tracker has not received any data yet.</para>
		/// </value>
		property bool IsTracking { bool get(); }
		
		/// <summary>Determines if touch tracking has not started yet.</summary>
		/// <value>
		///  <para>
		///   Set to true if this tracker has not received any data yet.
		/// </para>
		///  <para>
		///   Set to false if this tracker has received at least one touch point via its UpdateWith() method.
		///   This means that the StartPoint, LastPoint, and Phase properties will provide valid values.
		///  </para>
		/// </value>
		property bool IsNotTracking { bool get(); }
		
		/// <summary>Gets the unique integer ID assigned to the pointer/finger.</summary>
		/// <value>
		///  Unique integer ID assigned to the pointer/finger. This ID is used to differentiate between multiple
		///  pointers/fingers that are on the screen at the same time (ie: multitouch).
		/// </value>
		property int PointerId { int get(); }
		
		/// <summary>Gets the point where touch tracking began.</summary>
		/// <remarks>You should not access this property until the IsTracking property returns true.</remarks>
		/// <value>
		///  <para>The point where touch events started.</para>
		///  <para>The point's coordinates and timestamp will be zero if tracking has not started yet.</para>
		/// </value>
		property TouchPoint StartPoint { TouchPoint get(); }
		
		/// <summary>Gets the current/last touch point received.</summary>
		/// <remarks>You should not access this property until the IsTracking property returns true.</remarks>
		/// <value>
		///  <para>The current/last touch point received.</para>
		///  <para>The point's coordinates and timestamp will be zero if tracking has not started yet.</para>
		/// </value>
		property TouchPoint LastPoint { TouchPoint get(); }
		
		/// <summary>Gets the current touch phase such as Began, Moved, Ended, or Canceled.</summary>
		/// <remarks>You should not access this property until the IsTracking property returns true.</remarks>
		/// <value>
		///  <para>The current touch phase such as Began, Moved, Ended, or Canceled.</para>
		///  <para>Set to null if tracking has not started yet.</para>
		/// </value>
		property TouchPhase^ Phase { TouchPhase^ get(); }
		
		/// <summary>Resets touch tracking by clearing the start and last points and nulling out the phase.</summary>
		/// <remarks>
		///  After resetting this tracker, the IsTracking property will return false. You should not access this
		///  tracker's StartPoint, LastPoint, and Phase properties until you've given this tracking its first
		///  touch data via the UpdateWith() method.
		/// </remarks>
		void Reset();
		
		/// <summary>Updates this tracker with the given point.</summary>
		/// <remarks>
		///  <para>
		///   The StartPoint property will return the given touch point if this is the first time this method has
		///   been called or if you set the phase to Began.
		///  </para>
		///  <para>The IsTracking property will return true after calling this method.</para>
		/// </remarks>
		/// <param name="phase">
		///  <para>The touch phase such as Began, Moved, Ended, or Canceled.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		/// <param name="point">The touch point that was received.</param>
		void UpdateWith(TouchPhase^ phase, TouchPoint point);

	private:
		int fPointerId;
		TouchPoint fStartPoint;
		TouchPoint fLastPoint;
		TouchPhase^ fPhase;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Input
