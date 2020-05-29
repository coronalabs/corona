//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.input;


/**
 * Stores the settings for one input device.
 * <p>
 * An instance of this class is typically used to create an immutable InputDeviceInfo object
 * via its static from() method.
 */
public class InputDeviceSettings implements Cloneable {
	/** The type of input device this is such as KEYBOARD, MOUSE, JOYSTICK, etc. */
	private InputDeviceType fDeviceType;

	/** The device's unique string ID that persists after rebooting the Android device. */
	private String fPermanentStringId;

	/** The input device's product name as assigned by the manufacturer. */
	private String fProductName;

	/** The device's display name. Typically set via Android's "Bluetooth" settings activity. */
	private String fDisplayName;

	/** Stores a collection of axis settings. */
	private java.util.ArrayList<AxisSettings> fAxisCollection;


	/** Creates a new settings object used to create an immutable InputDeviceInfo object. */
	public InputDeviceSettings() {
		fDeviceType = InputDeviceType.UNKNOWN;
		fPermanentStringId = null;
		fProductName = "";
		fDisplayName = "";
		fAxisCollection = new java.util.ArrayList<AxisSettings>();
	}

	/**
	 * Creates a new copy of this object.
	 * @return Returns a copy of this object.
	 */
	@Override
	public InputDeviceSettings clone() {
		InputDeviceSettings clone = null;
		try {
			clone = (InputDeviceSettings)super.clone();
			clone.fAxisCollection.clear();
			for (AxisSettings axisSettings : fAxisCollection) {
				if (axisSettings != null) {
					clone.fAxisCollection.add(axisSettings.clone());
				}
				else {
					clone.fAxisCollection.add(null);
				}
			}
		}
		catch (Exception ex) { }
		return clone;
	}

	/**
	 * Gets the kind of input device this info object represents.
	 * @return Returns the type of input device such as "KEYBOARD", "MOUSE", "JOYSTICK", etc.
	 */
	public InputDeviceType getType() {
		return fDeviceType;
	}

	/**
	 * Sets the type of input device such as KEYBOARD, MOUSE, JOYSTICK, etc.
	 * @param type The type of input device this settings object represents.
	 *             Setting this to null will assign this object the UNKNOWN type.
	 */
	public void setType(InputDeviceType type) {
		if (type == null) {
			type = InputDeviceType.UNKNOWN;
		}
		fDeviceType = type;
	}

	/**
	 * Determines if this input device has a permanent string ID assigned to it.
	 * <p>
	 * This is an ID which remains the same after rebooting the Android device and typically
	 * comes from Android's InputDevice.getDescriptor() method. However, that method is only
	 * supported on Android 4.1 (API Level 16).
	 * @return Returns true if this device has a valid permanent ID assigned to it, meaning that
	 *         the getPermanentStringId() method will return a valid value.
	 *         <p>
	 *         Returns false if this device does not have a permanent ID.
	 */
	public boolean hasPermanentStringId() {
		return ((fPermanentStringId != null) && (fPermanentStringId.length() > 0));
	}

	/**
	 * Gets the device's unique string ID which persists after rebooting the Android device.
	 * This ID typically comes from Android's InputDevice.getDescriptor() method.
	 * @return Returns the device's unique string ID.
	 *         <p>
	 *         Returns null if this device does not have a permanent ID.
	 */
	public String getPermanentStringId() {
		return fPermanentStringId;
	}

	/**
	 * Sets the unique string ID that will remain the same after rebooting the Android device.
	 * This ID typically comes from Android's InputDevice.getDescriptor() method.
	 * @param value The device's permanent string ID.
	 *              <p>
	 *              Set to null or empty string to indicate the device does not have a permanent ID.
	 */
	public void setPermanentStringId(String value) {
		if ((value != null) && (value.length() <= 0)) {
			value = null;
		}
		fPermanentStringId = value;
	}

	/**
	 * Gets the device's product name as set by the manufacturer.
	 * @return Returns the device's product name.
	 *         <p>
	 *         Returns an empty string if the device does not have a product name.
	 */
	public String getProductName() {
		return fProductName;
	}

	/**
	 * Sets the device's product name as assigned by the manufacturer.
	 * @param name The device's product name.
	 *             <p>
	 *             Set to null or empty string if the device does not have a name.
	 */
	public void setProductName(String name) {
		if (name == null) {
			name = "";
		}
		fProductName = name;
	}

	/**
	 * Gets the device's display name, which typically comes from Android's InputDevice.getName() method.
	 * This name is intended to be displayed to the end-user.
	 * <p>
	 * This name is typically the product name unless the end-user changes it via the Bluetooth
	 * settings screen on the Android device.
	 * @return Returns the device's display name.
	 */
	public String getDisplayName() {
		return fDisplayName;
	}

	/**
	 * Sets the device's display name, which typically comes from Android's InputDevice.getName() method.
	 * This name is intended to be displayed to the end-user.
	 * <p>
	 * This name is typically the product name unless the end-user changes it via the Bluetooth
	 * settings screen on the Android device.
	 * @param name The input device's display name.
	 *             <p>
	 *             Set to null or empty string if the device does not have an display name. Although,
	 *             it is recommended that you at least assign it the device's product name instead.
	 */
	public void setDisplayName(String name) {
		if (name == null) {
			name = "";
		}
		fDisplayName = name;
	}

	/**
	 * Gets a collection of axis input configurations that this input device has.
	 * <p>
	 * You are allowed to add, update, and remove axis configurations in this collection.
	 * @return Returns a collection of axis configurations.
	 */
	public java.util.List<AxisSettings> getAxes() {
		return fAxisCollection;
	}
}
