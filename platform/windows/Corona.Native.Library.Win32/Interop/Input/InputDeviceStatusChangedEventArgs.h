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

/// <summary>
///  Provides information about what aspects of an input device's current status has changed,
///  such as from connected to disconnected or if it has been reconfigured.
/// </summary>
class InputDeviceStatusChangedEventArgs : public EventArgs
{
	public:
		/// <summary>Settings used to create an InputDeviceStatusChangedEventArgs instance.</summary>
		struct Settings
		{
			/// <summary>
			///  <para>Set true to indicate the connection to the input device has changed.</para>
			///  <para>Such as from connected to disconnected or vice-versa.</para>
			/// </summary>
			bool HasConnectionStateChanged;

			/// <summary>
			///  <para>Set true to indicate that the input device's configuration has changed.
			///  <para>Such as when axis inputs or vibration support has been added or removed.</para>
			/// </summary>
			bool WasReconfigured;
		};


		/// <summary>
		///  Creates a new event arguments object providing information about the status change of one input device.
		/// </summary>
		/// <param name="settings">Information about the device status change event.</param>
		InputDeviceStatusChangedEventArgs(const InputDeviceStatusChangedEventArgs::Settings& settings);

		/// <summary>Destroys this object.</summary>
		virtual ~InputDeviceStatusChangedEventArgs();


		/// <summary>
		///  <para>Determines if the connection to the input device has changed.</para>
		///  <para>Such as from connected to disconnected or vice-versa.</para>
		/// </summary>
		/// <returns>Returns true if the connection state has changed. Returns false if unchanged.</returns>
		bool HasConnectionStateChanged() const;

		/// <summary>
		///  <para>Determines if the input device's configuration has changed.</para>
		///  <para>Such as when axis inputs or vibration support has been added or removed.</para>
		/// </summary>
		bool WasReconfigured() const;

		/// <summary>Gets a copy of the settings used to create this event arguments object.</summary>
		/// <returns>Returns a copy of the settings used to create this event arguments object.</returns>
		InputDeviceStatusChangedEventArgs::Settings ToSettings() const;

	private:
		/// <summary>Assignment operator made private to make this object immutable.</summary>
		void operator=(const InputDeviceStatusChangedEventArgs& value) {}


		/// <summary>Stores information about the device status change event.</summary>
		InputDeviceStatusChangedEventArgs::Settings fSettings;
};

} }	// namespace Interop::Input
