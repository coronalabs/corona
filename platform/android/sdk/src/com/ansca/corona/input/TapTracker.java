//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.input;


/** Tracker used to determine if tap events have occurred via touch events from one input device. */
public class TapTracker implements Cloneable {
	/** The unique integer ID of the device the touch event came from. */
	private int fDeviceId;

	/** The coordinates where the first "down" touch event occurred. */
	private TouchPoint fFirstTouchBeganPoint;

	/** The coordinates where the last "down" touch event occurred. */
	private TouchPoint fLastTouchBeganPoint;

	/**
	 * The last tap point's timestamp.
	 * Used to determine if the next tap should increment the tap count if received in time.
	 */
	private long fLastTapTimestamp;

	/** The coordinates of the last detected tap event. */
	private TouchPoint fTapPoint;

	/** Stores the number of consecutive taps that have occurred at the same coordinates. */
	private int fTapCount;


	/**
	 * Creates a new tracker used to determine if a tap event based on given touch events.
	 * @param deviceId Unique integer ID of the device the touch events are coming from.
	 *                 This typically comes from the MotionEvent.getDeviceId() method.
	 */
	public TapTracker(int deviceId) {
		fDeviceId = deviceId;
		reset();
	}

	/**
	 * Creates a new copy of this object.
	 * @return Returns a copy of this object.
	 */
	@Override
	public TapTracker clone() {
		TapTracker clone = null;
		try {
			clone = (TapTracker)super.clone();
		}
		catch (Exception ex) { }
		return clone;
	}

	/** Resets tap tracking. */
	public void reset() {
		fFirstTouchBeganPoint = null;
		fLastTouchBeganPoint = null;
		fLastTapTimestamp = 0;
		fTapPoint = null;
		fTapCount = 0;
	}

	/**
	 * Gets the unique integer ID assigned to the device that the touch/tap events are coming from.
	 * <p>
	 * This ID comes form the Android MotionEvent that raised the touch event.
	 * @return Returns the device's unique integer ID.
	 */
	public int getDeviceId() {
		return fDeviceId;
	}

	/**
	 * Determines if the last call to the updateWith() method has detected a tap event.
	 * @return Returns true if a tap event has occurred. In which case, methods getTapPoint()
	 *         and getTapCount() will provide information about the tap event.
	 *         <p>
	 *         Returns false if a tap event has not been detected.
	 */
	public boolean hasTapOccurred() {
		return (fTapPoint != null);
	}

	/**
	 * Gets the coordinates and timestamp of the last detected tap event.
	 * <p>
	 * This value gets updated after every call to the updateWith() method.
	 * @return Returns the coordinates of the last detected tap event.
	 *         <p>
	 *         Returns null if the last call to updateWith() did not detect a tap event.
	 */
	public TouchPoint getTapPoint() {
		return fTapPoint;
	}

	/**
	 * Gets the number of consecutive taps detected at the same point on the screen or touchpad.
	 * <p>
	 * This value gets updated after every call to the updateWith() method.
	 * @return Returns the number of taps detected at the same coordinate on the screen or touchpad.
	 *         <p>
	 *         Returns zero if the last call to updateWith() did not detect a tap event.
	 */
	public int getTapCount() {
		if (fTapPoint == null) {
			return 0;
		}
		return fTapCount;
	}

	/**
	 * Updates this tracker with the given touch data and determines if a tap event has occurred.
	 * <p>
	 * You are expected to call the hasTapOccurred() method after calling this method.
	 * If hasTapOccurred() returns true, then methods getTapPoint() and getTapCount() will
	 * provide information about the tap event that has just occurred.
	 * @param point The touch point that was received. Cannot be null.
	 * @param phase The current touch phase such as BEGAN, MOVED, or ENDED. Cannot be null.
	 */
	public void updateWith(TouchPoint point, TouchPhase phase) {
		// Validate arguments.
		if ((point == null) || (phase == null)) {
			throw new NullPointerException();
		}

		// Clear the last detected tap point.
		fTapPoint = null;

		// Handle the given touch data.
		if (phase == TouchPhase.BEGAN) {
			// Store the "down" position of this touch point.
			// Will be used to check if the pointer was released near the same position later.
			if (fFirstTouchBeganPoint == null) {
				fFirstTouchBeganPoint = point;
			}
			fLastTouchBeganPoint = point;
			
			// Reset the tap count if this touch point is nowhere near the last touch point.
			if (areCoordinatesWithinTapBounds(fLastTouchBeganPoint, fFirstTouchBeganPoint) == false) {
				fTapCount = 0;
				fFirstTouchBeganPoint = fLastTouchBeganPoint;
			}
		}
		else if ((fLastTouchBeganPoint != null) && (phase == TouchPhase.ENDED)) {
			// Register a "tap" event if pointer has been released at the same location it began.
			if (areCoordinatesWithinTapBounds(point, fLastTouchBeganPoint)) {
				// Set the tap count. Increment it if this tap came in within a half second of the last.
				if ((point.getTimestamp() - fLastTapTimestamp) > 500) {
					fTapCount = 1;
				}
				else if (fTapCount < Integer.MAX_VALUE) {
					fTapCount++;
				}

				// Store the tap point.
				fLastTapTimestamp = point.getTimestamp();
				fTapPoint = point;

				// Clear the last touch event.
				fLastTouchBeganPoint = null;
			}
		}
	}

	/**
	 * Updates this tracker with the given touch data and determines if a tap event has occurred.
	 * <p>
	 * You are expected to call the hasTapOccurred() method after calling this method.
	 * If hasTapOccurred() returns true, then methods getTapPoint() and getTapCount() will
	 * provide information about the tap event that has just occurred.
	 * @param event The touch data that was just received.
	 *              <p>
	 *              If this touch event's device ID does not match this tracker's device ID,
	 *              then its touch data will be ignored and this tracker will not be updated.
	 *              <p>
	 *              Settings this to null will cause an exception to occur.
	 */
	public void updateWith(android.view.MotionEvent event) {
		// Validate argument.
		if (event == null) {
			throw new NullPointerException();
		}

		// Clear the last detected tap point.
		fTapPoint = null;

		// Ignore the given event if it comes from a different device.
		if (event.getDeviceId() != fDeviceId) {
			return;
		}

		// If 2 or more pointers are on the screen/touchpad at the same time,
		// then ignore the event and do not raise a tap event when they are released.
		if (event.getPointerCount() > 1) {
			reset();
			return;
		}

		// If the given event was generated by a mouse, then only generate a tap event
		// if its primary mouse button was clicked to match desktop application behavior.
		// Ignore the given event if any other mouse button was clicked.
		if ((android.os.Build.VERSION.SDK_INT >= 14) && (InputDeviceType.from(event) == InputDeviceType.MOUSE)) {
			if (ApiLevel14.isNonPrimaryMouseButtonDownFor(event)) {
				reset();
				return;
			}
		}

		// Fetch the "touch" phase from the given event's "action" code.
		// If a touch phase could not be extracted, then this motion event is not a touch event.
		// Note: This typically happens for mouse hover events.
		TouchPhase phase = TouchPhase.from(event);
		if (phase == null) {
			return;
		}

		// Update this tracker with the given touch event.
		TouchPoint point = new TouchPoint(event.getX(), event.getY(), event.getEventTime());
		updateWith(point, phase);
	}

	/**
	 * Determines if the given touch coordinates are close enough to be considered a tap event.
	 * @param point1 The coordinates of the first touch point.
	 * @param point2 The coordinates of the second touch point.
	 * @return Returns true if the coordinates are close enough to be considered a tap event.
	 *         <p>
	 *         Returns false if the two coordinates are too far apart.
	 */
	private boolean areCoordinatesWithinTapBounds(TouchPoint point1, TouchPoint point2) {
		// Validate arguments.
		if ((point1 == null) || (point2 == null)) {
			return false;
		}

		// Determine if the two coordinates are close enough to be considered a tap event.
		float deltaX = Math.abs(point2.getX() - point1.getX());
		float deltaY = Math.abs(point2.getY() - point1.getY());
		return ((deltaX <= 40.0f) && (deltaY <= 40.0f));
	}


	/**
	 * Provides access to API Level 14 (Ice Cream Sandwich) features.
	 * Should only be accessed if running on an operating system matching this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * You are expected to call its static methods instead.
	 */
	private static class ApiLevel14 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel14() { }

		/**
		 * Determines if any mouse button other than the primary button is down for the given motion event.
		 * <p>
		 * Note that the primary mouse button is typically the left mouse button.
		 * @param event The mouse motion event providing mouse button states.
		 *              Mouse button status will be retrieved by calling its getButtonState() method.
		 * @return Returns true if any mouse other than the primary is down. This would include the
		 *         secondary/right button, tertiary/middle button, back button, and forward button.
		 *         <p>
		 *         Returns false if no other button is pressed down.
		 */
		public static boolean isNonPrimaryMouseButtonDownFor(android.view.MotionEvent event) {
			if (event == null) {
				return false;
			}
			return ((event.getButtonState() & ~android.view.MotionEvent.BUTTON_PRIMARY) != 0);
		}
	}
}
