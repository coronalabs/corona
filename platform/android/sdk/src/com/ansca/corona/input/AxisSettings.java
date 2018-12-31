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
 * Stores the configuration settings of one axis input belonging to an input device.
 * <p>
 * An instance of this class is typically used to create an immutable AxisInfo object
 * via its static from() method.
 */
public class AxisSettings implements Cloneable {
	/** The type of axis input this settings object represents. */
	private AxisType fType;

	/** The minimum value that can be reported by the axis input. */
	private float fMinValue;

	/** The maximum value that can be reported by the axis input. */
	private float fMaxValue;

	/** The accuracy of the axis input device. */
	private float fAccuracy;

	/** Set true if axis provides absolute values. Set false if it provides relative values. */
	private boolean fIsProvidingAbsoluteValues;


	/** Creates a new settings object for one axis input. */
	public AxisSettings() {
		fType = AxisType.UNKNOWN;
		fMinValue = -1.0f;
		fMaxValue = 1.0f;
		fAccuracy = 0.0f;
		fIsProvidingAbsoluteValues = true;
	}

	/**
	 * Creates a new copy of this object.
	 * @return Returns a copy of this object.
	 */
	@Override
	public AxisSettings clone() {
		AxisSettings clone = null;
		try {
			clone = (AxisSettings)super.clone();
		}
		catch (Exception ex) { }
		return clone;
	}

	/**
	 * Gets the type of axis such as "X", "Y", "LEFT_TRIGGER", "RIGHT_TRIGGER", etc.
	 * @return Returns the type of axis this settings object represents.
	 */
	public AxisType getType() {
		return fType;
	}

	/**
	 * Sets the axis type such as "AxisType.X", "AxisType.Y", etc.
	 * @param type The type of axis such as "AxisType.X", "AxisType.Y", etc.
	 *             <p>
	 *             Setting this to null will assign this object the "AxisType.UNKNOWN" type.
	 */
	public void setType(AxisType type) {
		if (type == null) {
			type = AxisType.UNKNOWN;
		}
		fType = type;
	}

	/**
	 * Gets the minimum value that the axis input can provide.
	 * @return Returns the minimum value that the axis input can provide.
	 */
	public float getMinValue() {
		return fMinValue;
	}

	/**
	 * Sets the minimum value that the axis input can provide.
	 * @param value The minimum value that the axis input can provide.
	 *              This is typically set to -1 or 0.
	 */
	public void setMinValue(float value) {
		fMinValue = value;
	}

	/**
	 * Gets the maximum value that the axis input can provide.
	 * @return Returns the maximum value that the axis input can provide.
	 */
	public float getMaxValue() {
		return fMaxValue;
	}

	/**
	 * Sets the maximum value that the axis input can provide.
	 * @param value The maximum value that the axis input can provide.
	 *              This is typically set to 1 or 0.
	 */
	public void setMaxValue(float value) {
		fMaxValue = value;
	}

	/**
	 * Gets the accuracy of the axis input's data.
	 * @return Returns the axis input's accuracy.
	 */
	public float getAccuracy() {
		return fAccuracy;
	}

	/**
	 * Sets the accuracy of the axis input's data.
	 * <p>
	 * This value is typically received from the Android's "InputDevice.MotionRange.getFuzz()" method.
	 * @param value The accuracy of the axis input.
	 *              <p>
	 *              For example, setting this value to 0.1 indicates that values reported by the axis input
	 *              device are accuracte within +/- 0.1 of the reported value.
	 */
	public void setAccuracy(float value) {
		fAccuracy = value;
	}

	/**
	 * Determines if this axis provides absolute values or relative values.
	 * @return Returns true if this axis provides absolute values, which is positional data typically
	 *         recevied from joysticks and analog triggers.
	 *         <p>
	 *         Retruns false if this axis provides relative values, such as distance traveled since
	 *         the last axis measurement value was taken. This is common used by mice and touchpads.
	 */
	public boolean isProvidingAbsoluteValues() {
		return fIsProvidingAbsoluteValues;
	}

	/**
	 * Sets whether or not the axis input provides absolute values or relative values.
	 * @param value Set to true if the axis provides absolute values, which is positional data
	 *              typically returned by a joystick or analog trigger.
	 *              <p>
	 *              Set to false if the axis provides relative values, which is typically the distance
	 *              travelled since the last axis input measurement. This is commonly used by mice and touchpads.
	 */
	public void setIsProvidingAbsoluteValues(boolean value) {
		fIsProvidingAbsoluteValues = value;
	}

	/**
	 * Determines if the given axis settings exactly matches this object's axis settings
	 * @param settings Reference to the axis settings object to be compared with this object.
	 * @return Returns true if the given axis settings exactly matches this object's axis settings.
	 *         <p>
	 *         Returns false if given a null argument or if the given settings do not match
	 *         this object's axis settings.
	 */
	public boolean equals(AxisSettings settings) {
		// Not equal if given a null reference.
		if (settings == null) {
			return false;
		}

		// Not equal if the member variables differs.
		if ((settings.fType.equals(fType) == false) ||
		    (Math.abs(settings.fMinValue - fMinValue) > 0.001f) ||
		    (Math.abs(settings.fMaxValue - fMaxValue) > 0.001f) ||
		    (Math.abs(settings.fAccuracy - fAccuracy) > 0.001f) ||
		    (settings.fIsProvidingAbsoluteValues != fIsProvidingAbsoluteValues))
		{
			return false;
		}

		// The given settings equals this object's settings.
		return true;
	}

	/**
	 * Determines if the given object matches this object.
	 * @param value Reference to the object to be compared against this object.
	 * @return Returns true if the given object is of type AxisSettings and its settings exactly
	 *         matches this object's axis settings.
	 *         <p>
	 *         Returns false if given a null reference, an object that is not of type AxisSettings,
	 *         or if the given axis settings do not match this object's axis settings.
	 */
	@Override
	public boolean equals(Object value) {
		if ((value instanceof AxisSettings) == false) {
			return false;
		}
		return equals((AxisSettings)value);
	}
}
