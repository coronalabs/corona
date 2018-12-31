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
 * Stores the coordinate of a single touch event and the time it occurred on.
 * <p>
 * Instances of this class are immutable.
 */
public class TouchPoint {
	/** The X coordinate that the touch point occurred at. */
	private float fPointX;

	/** The Y coordinate that the touch point occurred at. */
	private float fPointY;

	/** The time the touch event occurred in milliseconds since bootup. */
	private long fTimestamp;


	/**
	 * Creates a new touch data point set to current time.
	 * @param x The X coordinate of the touch point.
	 * @param y The Y coordinate of the touch point.
	 */
	public TouchPoint(float x, float y) {
		this(x, y, android.os.SystemClock.uptimeMillis());
	}

	/**
	 * Creates a new touch data point.
	 * @param x The X coordinate of the touch point.
	 * @param y The Y coordinate of the touch point.
	 * @param timestamp The time the touch event occurred in milliseconds since bootup.
	 *                  <p>
	 *                  This value should be taken from MotionEvent.getEventTime() or SystemClock.uptimeMillis().
	 */
	public TouchPoint(float x, float y, long timestamp) {
		fPointX = x;
		fPointY = y;
		fTimestamp = timestamp;
	}

	/**
	 * Gets the touch point's x coordinate.
	 * @return Returns the x coordinate.
	 */
	public float getX() {
		return fPointX;
	}

	/**
	 * Gets the touch point's y coordinate.
	 * @return Returns the y coordinate.
	 */
	public float getY() {
		return fPointY;
	}

	/**
	 * Gets the time when the touch event occurred on.
	 * @return Returns a timestamp in milliseconds since bootup.
	 *         <p>
	 *         Can be compared against the value returned by MotionEvent.getEventTime()
	 *         or SystemClock.uptimeMillis().
	 */
	public long getTimestamp() {
		return fTimestamp;
	}

	/**
	 * Gets this touch point's data in human readable string form.
	 * @return Returns a non-localized string of this touch point's data.
	 */
	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();
		builder.append("(");
		builder.append(Float.toString(fPointX));
		builder.append(", ");
		builder.append(Float.toString(fPointY));
		builder.append(", ");
		builder.append(Long.toString(fTimestamp));
		builder.append(")");
		return builder.toString();
	}
}
