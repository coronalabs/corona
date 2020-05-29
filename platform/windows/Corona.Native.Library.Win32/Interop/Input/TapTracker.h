//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Build.h"
#include "Interop\Nullable.h"
#include "Interop\Ticks.h"
#include "Rtt_Event.h"
#include <Windows.h>


namespace Interop { namespace Input {

/// <summary>
///  <para>Tracker used to determine if a tap event has occurred based on given touch events.</para>
///  <para>The owner of this object is expected to pass touch event to the UpdateWith() method.</para>
///  <para>
///   After calling UpdateWith(), the owner is expected to immediately call HasTapOccurred() to determine
///   if a tap was detected.
///  </para>
/// </summary>
/// <remarks>
///  This class is used by the WinInputDeviceManager class to determine if a Corona "tap" event
///  should be dispatched to Lua.
/// </remarks>
class TapTracker final
{
	public:
		#pragma region Public InputDeviceType Enum
		enum class InputDeviceType
		{
			kMouse,
			kTouchscreen
		};

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new tap tracker.</summary>
		TapTracker();

		/// <summary>Destroys this object.</summary>
		virtual ~TapTracker();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Determines if the last call to the UpdateWith() method has detected a tap event.</summary>
		/// <returns>
		///  <para>
		///   Returns true if a tap event was detected. In which case, methods GetTapPoint() and GetTapCount()
		///   will provide information about the tap event.
		///  </para>
		///  <para>Returns false if a tap event has not been detected.</para>
		/// </returns>
		bool HasTapOccurred() const;

		/// <summary>
		///  <para>Gets the coordinates of the last detected tap event.</para>
		///  <para>Should not be called until the HasTapOccurred() method returns true.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns the coordinates of the last detected tap event.</para>
		///  <para>Returns (0,0) if the HasTapOccurred() method returns false (no tap detected).</para>
		/// </returns>
		POINT GetTapPoint() const;

		/// <summary>
		///  <para>Gets the number of consecutive taps detected at the same point on the screen.</para>
		///  <para>Should not be called until the HasTapOccurred() method returns true.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns the number of taps detected at the same coordinate on the screen.</para>
		///  <para>Returns zero if the HasTapOccurred() method returns false (no tap detected).</para>
		/// </returns>
		int GetTapCount() const;

		/// <summary>
		///  Sets which input device (mouse or touchscreen) the next call to UpdateWith() is getting touch data from.
		/// </summary>
		/// <param name="type">Set to kMouse or kTouchscreen.</param>
		void SetInputDeviceType(const TapTracker::InputDeviceType& type);

		/// <summary>
		///  Gets the input device (mouse or touchscreen) used by the UpdateWith() method when selecting
		///  a touch distance tolerance for determine if a tap event occurred.
		/// </summary>
		/// <returns>Returns kMouse or kTouchscreen.</returns>
		InputDeviceType GetInputDeviceType() const;

		/// <summary>Resets touch/tap tracking data passed to the UpdateWith() method.</summary>
		void Reset();

		/// <sumamry>
		///  <para>Updates this tracker with the given touch data and determines if a tap event has occurred.</para>
		///  <para>You are expected to call the HasTapOccurred() method after calling this method.</para>
		///  <para>
		///   If HasTapOccurred() returns true, then methods GetTapPoint() and GetTapCount() will provide information
		///   about the tap event that has just occurred.
		///  </para>
		/// </summary>
		/// <param name="point">
		///  <para>The touch point that was just received.</para>
		///  <para>This tracker does not care about the coordinate's units.</para>
		/// </param>
		/// <param name="phase">The touch phase such as kBegan, kMoved, kEnded, etc.</param>
		void UpdateWith(const POINT& point, const Rtt::TouchEvent::Phase& phase);

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>Determines if the given touch coordinates are close enough to be considered a tap event.</summary>
		/// <param name="point1">The coordinates of the first touch point.</param>
		/// <param name="point2">The coordinates of the second touch point.</param>
		/// <returns>
		///  <para>Returns true if the coordinates are close enough to be considered a tap event.</para>
		///  <para>Returns false if the two coordinates are too far apart.</para>
		/// </returns>
		bool ArePointsWithinTapBounds(const POINT& point1, const POINT& point2) const;

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>
		///  <para>Indicates if the touch data received by UpdateWith() is from a mouse or touchscreen.</para>
		///  <para>
		///   Used to select the max distance between a "began" and "ended" touch event to determine if it is a tap.
		///  </para>
		/// </summary>
		InputDeviceType fInputDeviceType;

		/// <summary>The coordinates where the first "down" touch event occurred. Will be null if not set.</summary>
		Nullable<POINT> fFirstTouchBeganPoint;

		/// <summary>The coordinates where the last "down" touch event occurred. Will be null if not set.</summary>
		Nullable<POINT> fLastTouchBeganPoint;

		/// <summary>
		///  <para>The last tap point's timestamp in system ticks.</para>
		///  <para>Used to determine if the next tap should increment the tap count if received in time.</para>
		///  <para>Will be null if a touch hasn't been detected yet.</para>
		/// </summary>
		Nullable<Ticks> fLastTapTicks;

		/// <summary>
		///  <para>The coordinates of the last detected tap event.</para>
		///  <para>Will be null if a tap has not been detected.</para>
		/// </summary>
		Nullable<POINT> fTapPoint;

		/// <summary>Stores the number of consecutive taps that have occurred at the same coordinates.</summary>
		int fTapCount;

		/// <summary>
		///  Set to the maximum number of milliseconds between taps on the same coordinate in order
		///  for the tap count to be incremented.
		/// </summary>
		int fMultitapThresholdTimeInMilliseconds;

		#pragma endregion
};

} }	// namespace Interop::Input
