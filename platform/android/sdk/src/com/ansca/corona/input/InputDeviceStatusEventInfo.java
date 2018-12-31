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


/**
 * Provides information about what aspects of a device's current status has changed,
 * such as from connected to disconnected or if it has been reconfigured.
 * <p>
 * Instances of this class are immutable.
 */
public class InputDeviceStatusEventInfo {
	/** Stores the information provided by this event. */
	private InputDeviceStatusEventInfo.Settings fSettings;


	/**
	 * Create a new information object about an input device status change event.
	 * @param settings Information about the event. Cannot be null.
	 */
	public InputDeviceStatusEventInfo(InputDeviceStatusEventInfo.Settings settings) {
		if (settings == null) {
			throw new NullPointerException();
		}
		fSettings = settings.clone();
	}

	/**
	 * Determines if the input device's connection with the system has changed, such as
	 * from connected to disconnected.
	 * @return Returns true if the connection state has changed. Returns false if not.
	 */
	public boolean hasConnectionStateChanged() {
		return fSettings.hasConnectionStateChanged();
	}

	/**
	 * Determines if the input device's configuration has changed, such as when axis inputs
	 * or vibration support has been added or removed.
	 * @return Returns true if the device has been reconfigured.
	 *         <p>
	 *         Return false if the device's configuration has not changed.
	 */
	public boolean wasReconfigured() {
		return fSettings.wasReconfigured();
	}


	/**
	 * Provides information about what aspects of a device's current status has changed,
	 * such as from connected to disconnected or if it has been reconfigured.
	 * <p>
	 * Used to create an immutable InputDeviceStatusEventInfo object, which is needed
	 * to raise an "inputDeviceStatus" event in Lua.
	 */
	public static class Settings implements Cloneable {
		/** Set to true if the input device's connection state has changed. */
		private boolean fHasConnectionStateChanged;

		/** Set to true if the input device's configuration has been changed. */
		private boolean fWasReconfigured;


		/** Creates new settings used to configure and create an InputDeviceStatusEventInfo object. */
		public Settings() {
			fHasConnectionStateChanged = false;
			fWasReconfigured = false;
		}

		/**
		 * Creates a new copy of this object.
		 * @return Returns a copy of this object.
		 */
		@Override
		public InputDeviceStatusEventInfo.Settings clone() {
			InputDeviceStatusEventInfo.Settings clone = null;
			try {
				clone = (InputDeviceStatusEventInfo.Settings)super.clone();
			}
			catch (Exception ex) { }
			return clone;
		}

		/**
		 * Determines if the input device's connection with the system has changed, such as
		 * from connected to disconnected.
		 * @return Returns true if the connection state has changed. Returns false if not.
		 */
		public boolean hasConnectionStateChanged() {
			return fHasConnectionStateChanged;
		}

		/**
		 * Sets whether or not the connection state of the device has changed.
		 * @param value Set to true to indicate the connection state of the device has changed,
		 *              such as connected to disconnected.
		 *              <p>
		 *              Set to false if the connection state has not changed.
		 */
		public void setHasConnectionStateChanged(boolean value) {
			fHasConnectionStateChanged = value;
		}

		/**
		 * Determines if the input device's configuration has changed, such as when axis inputs
		 * or vibration support has been added or removed.
		 * @return Returns true if the device has been reconfigured.
		 *         <p>
		 *         Return false if the device's configuration has not changed.
		 */
		public boolean wasReconfigured() {
			return fWasReconfigured;
		}

		/**
		 * Sets whether or not the input device has been reconfigured, such as axis inputs have
		 * been added or removed from the device.
		 * @param value Set to true if the device has been reconfigured.
		 *              <p>
		 *              Set to false if the device's configuration has been unchanged.
		 */
		public void setWasReconfigured(boolean value) {
			fWasReconfigured = value;
		}
	}
}
