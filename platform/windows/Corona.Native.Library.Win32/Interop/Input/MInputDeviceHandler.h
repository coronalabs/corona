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


namespace Interop { namespace Input {

class InputDeviceContext;

/// <summary>
///  <para>Handler used to access, control, and read data from one input device. </para>
///  <para>
///   This is an abstract class whose derived classes will handle a device by its driver type such
///   as XInput, DirectInput, or RawInput.
///  </para>
/// </summary>
class MInputDeviceHandler
{
	public:
		/// <summary>Destroys this object.</summary>
		virtual ~MInputDeviceHandler() {}

		/// <summary>
		///  <para>Determines if this handler is currently attached/bound to a connected input device.</para>
		///  <para>This object's Poll() method will only collect input data if attached to a device.</para>
		/// </summary>
		/// <returns>Returns true if currently connected to a device. Returns false if not.</returns>
		virtual bool IsAttached() const = 0;

		/// <summary>
		///  <para>Detaches this handler from its input device.</para>
		///  <para>This prevents the Poll() method from working since it is no longer attached.</para>
		/// </summary>
		virtual void Detach() = 0;

		/// <summary>
		///  <para>Gets a device context that this handler uses store the device config and its connection status.</para>
		///  <para>
		///   This context will raise events when the device status has changed or when new input data has been received.
		///  </para>
		/// </summary>
		/// <returns>
		///  Returns a reference to this handler's device context which stores the device config, connection status,
		///  and provides device related events.
		/// </returns>
		virtual InputDeviceContext& GetContext() = 0;

		/// <summary>
		///  <para>
		///   Checks the device's connection status, fetches new input data (if available), and controls the device's
		///   vibration/rumble motors.
		///  </para>
		///  <para>This method is expected to be called at regular intervals.</para>
		/// </summary>
		virtual void Poll() = 0;
};

} }	// namespace Interop::Input
