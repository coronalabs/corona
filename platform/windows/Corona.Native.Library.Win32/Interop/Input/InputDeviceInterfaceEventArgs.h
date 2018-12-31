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

#include "Interop\EventArgs.h"


namespace Interop { namespace Input {

class InputDeviceInterface;

/// <summary>
///  <para>Provides information about an input device associated with an event.</para>
///  <para>Instances of this class are immutable.</para>
/// </summary>
class InputDeviceInterfaceEventArgs : public EventArgs
{
	public:
		/// <summary>Creates a new event arguments object with the given information.</summary>
		/// <param name="deviceInterface">The input device associated with the event.</param>
		InputDeviceInterfaceEventArgs(InputDeviceInterface& deviceInterface);

		/// <summary>Destroys this object.</summary>
		virtual ~InputDeviceInterfaceEventArgs();

		/// <summary>Gets a reference to the input device associated with the event.</summary>
		/// <returns>Returns a reference to the input device associated with the event.</returns>
		InputDeviceInterface& GetDeviceInterface() const;

	private:
		/// <summary>Assignment operator made private to make this object immutable.</summary>
		void operator=(const InputDeviceInterfaceEventArgs& value) {}

		/// <summary>Reference to an input device associated with the event.</summary>
		InputDeviceInterface& fDeviceInterface;
};

} }	// namespace Interop::Input
