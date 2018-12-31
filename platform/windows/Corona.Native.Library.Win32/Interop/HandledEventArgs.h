//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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
