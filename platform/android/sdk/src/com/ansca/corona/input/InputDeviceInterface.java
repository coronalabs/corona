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

package com.ansca.corona.input;


/** Provides read-only access to the configuration and current status of one input device. */
public class InputDeviceInterface {
	/** Provides access to one input device's information and features. */
	private InputDeviceContext fDeviceContext;


	/**
	 * Creates a new input device interface.
	 * <p>
	 * This is an internal constructor that can only be called by Corona.
	 * Instances of this class are expected to be created by Corona's InputDeviceServices class.
	 * @param deviceContext Reference to the device context to be wrapped. Cannot be null.
	 */
	InputDeviceInterface(InputDeviceContext deviceContext) {
		// Validate.
		if (deviceContext == null) {
			throw new NullPointerException();
		}

		// Store the given information.
		fDeviceContext = deviceContext;
	}

	/**
	 * Gets the unique integer ID assigned to this device by Corona.
	 * @return Returns this device's unique integer ID assigned by Corona.
	 */
	public int getCoronaDeviceId() {
		return fDeviceContext.getCoronaDeviceId();
	}

	/**
	 * Gets this device's congifuration information.
	 * @return Returns this device's information.
	 */
	public InputDeviceInfo getDeviceInfo() {
		return fDeviceContext.getDeviceInfo();
	}

	/**
	 * Determines if the input device is currently connected to the system and can provide input.
	 * @return Returns true if the input device is currently connected. Returns false if not.
	 */
	public boolean isConnected() {
		return fDeviceContext.isConnected();
	}

	/**
	 * Gets the input device's current connection state such as CONNECTED, DISCONNECTED, etc.
	 * @return Return the input device's current connection state with the Android system.
	 */
	public ConnectionState getConnectionState() {
		return fDeviceContext.getConnectionState();
	}

	/** Requests the input device to vibrate/rumble. */
	public void vibrate() {
		fDeviceContext.vibrate();
	}

	/**
	 * Gets a reference to the device context that this interface wraps.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 * The device context is not something you would want to expose to 3rd party developers
	 * via Corona Enterprise because it would given them write access to device information
	 * that they should not be allowed to manipulate.
	 * @return Returns a reference to the device context.
	 */
	InputDeviceContext getContext() {
		return fDeviceContext;
	}
}
