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
