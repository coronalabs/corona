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
 * Stores an axis input's value and timestamp.
 * <p>
 * Instances of this class are immutable.
 */
public class AxisDataPoint {
	/** The axis value that was recorded. */
	private float fValue;

	/** The time the axis data was recorded on in milliseconds since bootup. */
	private long fTimestamp;


	/**
	 * Creates a new axis data point set to current time.
	 * @param value The axis value that was recorded.
	 */
	public AxisDataPoint(float value) {
		this(value, android.os.SystemClock.uptimeMillis());
	}

	/**
	 * Creates a new axis data point.
	 * @param value The axis value that was recorded.
	 * @param timestamp The time the axis event occurred in milliseconds since bootup.
	 *                  <p>
	 *                  This value should be taken from MotionEvent.getEventTime() or SystemClock.uptimeMillis().
	 */
	public AxisDataPoint(float value, long timestamp) {
		fValue = value;
		fTimestamp = timestamp;
	}

	/**
	 * Gets the axis value that was recorded.
	 * @return Returns the axis value.
	 */
	public float getValue() {
		return fValue;
	}

	/**
	 * Gets the time when the axis event occurred.
	 * @return Returns a timestamp in milliseconds since bootup.
	 *         <p>
	 *         Can be compared against the value returned by MotionEvent.getEventTime()
	 *         or SystemClock.uptimeMillis().
	 */
	public long getTimestamp() {
		return fTimestamp;
	}

	/**
	 * Gets this axis data in human readable string form.
	 * @return Returns a non-localized string of this axis data point.
	 */
	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();
		builder.append("(");
		builder.append(Float.toString(fValue));
		builder.append(", ");
		builder.append(Long.toString(fTimestamp));
		builder.append(")");
		return builder.toString();
	}
}
