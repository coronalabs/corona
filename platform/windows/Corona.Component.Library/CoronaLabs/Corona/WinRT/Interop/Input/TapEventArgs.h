//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif

#include "TouchPoint.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Input {

/// <summary>Provides a tap event's screen coordinate, timestamp, and tap count.</summary>
public ref class TapEventArgs sealed
{
	public:
		/// <summary>Creates a new object for storing a tap event's data.</summary>
		/// <param name="tapPoint">Indicates where onscreen the tap occurred on.</param>
		/// <param name="tapCount">
		///  <para>The number of consecutive taps that have occurred at the given tap point.</para>
		///  <para>
		///   This value must be set to 1 or higher. Setting this to zero or a negative value will cause
		///   an exception to be thrown.
		///  </para>
		/// </param>
		TapEventArgs(TouchPoint tapPoint, int tapCount);

		/// <summary>Gets the screen coordinates where the tap occurred.</summary>
		/// <value>
		///  Provides the x and y coordinates of where the tap occurred onscreen, as well as the time it occurred on.
		/// </value>
		property TouchPoint TapPoint { TouchPoint get(); }

		/// <summary>Gets the number of consecutive taps that have occurred at the same coordinates.</summary>
		/// <value>
		///  <para>The number of taps that have occurred at the same spot on screen, such as 1 or 2.</para>
		///  <para>This value can never be less than 1.</para>
		/// </value>
		property int TapCount { int get(); }

		/// <summary>Gets or sets whether or not the tap event was consumed by the event handler.</summary>
		/// <value>
		///  <para>
		///   Set to false if this tap event was not handled. In which case, the control which received this event
		///   will perform its default action.
		///  </para>
		///  <para>
		///   Set to true if the event handler has handled this event. This overrides the default behavior of the
		///   control that received this event.
		///  </para>
		/// </value>
		property bool Handled { bool get(); void set(bool value); }

	private:
		TouchPoint fTapPoint;
		int fTapCount;
		bool fWasHandled;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Input
