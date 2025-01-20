//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "EventArgs.h"


namespace Interop {

/// <summary>Event arguments providing a flag indicating if a raised event was "handled" by its event handler.</summary>
class HandledEventArgs : public EventArgs
{
	public:
		/// <summary>Creates a new event arguments object that is flagged as unhandled by default.</summary>
		HandledEventArgs();

		/// <summary>Destroys this object.</summary>
		virtual ~HandledEventArgs();

		/// <summary>Determines if the event was handled or not.</summary>
		/// <returns>
		///  <para>Returns true if SetHandled() was called, indicating that the event was handled.</para>
		///  <para>Returns false if the event was not handled.</para>
		/// </returns>
		bool WasHandled() const;

		/// <summary>Flags the event as handled, which causes the WasHandled() method to return true.</summary>
		void SetHandled();

	private:
		/// <summary>Indicates if the event was handled.</summary>
		bool fWasHandled;
};

}	// namespace Interop
