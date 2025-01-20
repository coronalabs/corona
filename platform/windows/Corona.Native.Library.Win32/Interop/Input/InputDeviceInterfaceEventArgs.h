//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
