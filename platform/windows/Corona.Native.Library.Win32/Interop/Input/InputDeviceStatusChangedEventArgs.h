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
