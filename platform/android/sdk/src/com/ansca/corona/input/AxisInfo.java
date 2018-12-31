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
 * Stores the read-only configuration settings of one axis input belonging to an input device.
 * <p>
 * Instances of this class are immutable.
 * <p>
 * Instances of this class can only be created via the static from() method.
 */
public class AxisInfo {
	/** A clone of the axis settings this obejct represents. */
	private AxisSettings fSettings;


	/**
	 * Creates a new axis information object for one axis input.
	 * @param settings Settings for one axis input that the new axis info object will copy from.
	 */
	private AxisInfo(AxisSettings settings) {
		if (settings == null) {
			throw new NullPointerException();
		}
		fSettings = settings.clone();
	}

	/**
	 * Gets the type of axis such as "X", "Y", "LEFT_TRIGGER", "RIGHT_TRIGGER", etc.
	 * @return Returns the type of axis this object represents.
	 */
	public AxisType getType() {
		return fSettings.getType();
	}

	/**
	 * Gets the minimum value that the axis input can provide.
	 * @return Returns the minimum value that the axis input can provide.
	 */
	public float getMinValue() {
		return fSettings.getMinValue();
	}

	/**
	 * Gets the maximum value that the axis input can provide.
	 * @return Returns the maximum value that the axis input can provide.
	 */
	public float getMaxValue() {
		return fSettings.getMaxValue();
	}

	/**
	 * Gets the accuracy of the axis input's data.
	 * @return Returns the axis input's accuracy.
	 */
	public float getAccuracy() {
		return fSettings.getAccuracy();
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
		return fSettings.isProvidingAbsoluteValues();
	}

	/**
	 * Determines if the given axis information exactly matches this axis' information.
	 * @param axisInfo Reference to the axis info to be compared with this object.
	 * @return Returns true if the given axis information exactly matches this axis' information.
	 *         <p>
	 *         Returns false if given a null argument or if the given information does not match
	 *         this axis' information.
	 */
	public boolean equals(AxisInfo axisInfo) {
		if (axisInfo == null) {
			return false;
		}
		return fSettings.equals(axisInfo.fSettings);
	}

	/**
	 * Determines if the given object matches this object.
	 * @param value Reference to the object to be compared against this object.
	 * @return Returns true if the given object is of type AxisInfo and its information exactly
	 *         matches this axis' information.
	 *         <p>
	 *         Returns false if given a null reference, an object that is not of type AxisInfo,
	 *         or if the given axis information does not match this object's axis information.
	 */
	@Override
	public boolean equals(Object value) {
		if ((value instanceof AxisInfo) == false) {
			return false;
		}
		return equals((AxisInfo)value);
	}

	/**
	 * Creates a new axis information object using the information copied from the given settings.
	 * @param settings The axis settings to be copied by the new axis information object.
	 * @return Returns a new axis information object with the given settings.
	 *         <p>
	 *         Returns null if given a null argument.
	 */
	public static AxisInfo from(AxisSettings settings) {
		if (settings == null) {
			return null;
		}
		return new AxisInfo(settings);
	}
}
