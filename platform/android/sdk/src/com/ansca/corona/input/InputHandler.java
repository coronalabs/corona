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
 * Dispatches input such as key events, touch events, motion events, etc. to Corona's Lua listeners.
 * <p>
 * This class is intended to be used by an activity or view for sending Android input events
 * to Corona manually, such as for unhandled input events. Normally you would use a
 * ViewInputHandler class instead, which automatically dispatches all input events received by
 * a view to Corona. This class is for views that want to intercept the input event first.
 */
public class InputHandler {
	/** Dispatcher used to send input events to Corona's Lua listeners. */
	private com.ansca.corona.CoronaRuntimeTaskDispatcher fTaskDispatcher;

	/** Collection of touch trackers which keep track of touch events from multiple devices and pointers. */
	private TouchTrackerCollection fTouchTrackers;

	/** Collection of trap trackers used to detect taps from multiple touch sources. */
	private TapTrackerCollection fTapTrackers;

	/** Used to check whether multitouch is enabled. **/
	private com.ansca.corona.Controller fController;

	/** Creates a new input handler for sending posted input events to Corona's Lua listeners. */
	public InputHandler(com.ansca.corona.Controller controller) {
		fTaskDispatcher = null;
		fTouchTrackers = new TouchTrackerCollection();
		fTapTrackers = new TapTrackerCollection();
		fController = controller;
	}

	/**
	 * Sets the dispatcher to be used to send input events to Corona's Lua listeners.
	 * @param dispatcher Reference to the dispatcher used to send events to a Corona runtime.
	 *                   <p>
	 *                   Set to null prevent this handler from sending input events.
	 */
	public void setDispatcher(com.ansca.corona.CoronaRuntimeTaskDispatcher dispatcher) {
		fTaskDispatcher = dispatcher;
	}

	/**
	 * Gets the dispatcher used to send input events to Corona's Lua listeners.
	 * @return Returns a reference to the dispatcher used by this handler.
	 *         <p>
	 *         Returns null if a dispatcher has not been assigned to this handler.
	 */
	public com.ansca.corona.CoronaRuntimeTaskDispatcher getDispatcher() {
		return fTaskDispatcher;
	}

	/**
	 * Posts a key event to Corona to be raised as a "key" event in Lua.
	 * @param event The key event to be processed by Corona.
	 * @return Returns true if Corona has handled the given event. Returns false if not.
	 */
	public boolean handle(android.view.KeyEvent event) {
		// Validate.
		if (event == null) {
			return false;
		}

		// Do not allow key repeat. The "down" event should only happen once per press.
		KeyPhase phase = KeyPhase.from(event);
		if ((phase == KeyPhase.DOWN) && (event.getRepeatCount() > 0)) {
			return event.getKeyCode() != android.view.KeyEvent.KEYCODE_VOLUME_UP && event.getKeyCode() != android.view.KeyEvent.KEYCODE_VOLUME_DOWN;
		}

		// Ignore the key event if it came from the Corona runtime.
		// This typically happens if Corona's Lua listeners do not handle it and its been sent back the activity.
		if (event instanceof CoronaKeyEvent) {
			return false;
		}

		// Fetch the input device that generated this key event.
		InputDeviceInterface inputDevice = null;
		if (event.getDeviceId() > 0) {
			android.content.Context context = com.ansca.corona.CoronaEnvironment.getApplicationContext();
			if (context != null) {
				InputDeviceServices deviceServices = new InputDeviceServices(context);
				inputDevice = deviceServices.fetchDeviceFrom(event);
			}
		}

		// Make sure that the input device is flagged as "connected" since we've received in put from it.
		if (inputDevice != null) {
			inputDevice.getContext().update(ConnectionState.CONNECTED);
		}

		// In Corona Cards this will be exit out because there is no Corona Activity.  We want to let the CC user handle
		// key events because if we try to dispatch the key event to the view again theres validation in the view which 
		// prevents that from happening. 
		com.ansca.corona.CoronaActivity activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();
		if (activity == null) {
			return false;
		}

		// Raise a "key" event in Corona if the key code is supported.
		// Note: This key event will typically come back if not handled by Corona's Lua listeners.
		if ((event.getKeyCode() != android.view.KeyEvent.KEYCODE_HOME) &&
		    (event.getKeyCode() != android.view.KeyEvent.KEYCODE_POWER))
		{
			if (fTaskDispatcher != null) {
				fTaskDispatcher.send(new RaiseKeyEventTask(inputDevice, event));
				return true;
			}
		}
		
		// Ignore the given key event.
		return false;
	}

	/**
	 * Posts a motion event to Corona to be raised as a "touch", "analog", or "mouse" event in Lua.
	 * @param event The motion event to be processed by Corona.
	 * @return Returns true if Corona has handled the given event. Returns false if not.
	 */
	public boolean handle(android.view.MotionEvent event) {
		// Validate.
		if (event == null) {
			return false;
		}
		
		// Handle the given motion event.
		boolean wasHandled = false;
		wasHandled |= handleAxisEvent(event);
		wasHandled |= handleMouseEvent(event);
		wasHandled |= handleTouchEvent(event);
		wasHandled |= handleTapEvent(event);

		// Returns true if this handler has consumed the given event.
		return wasHandled;
	}

	/**
	 * Posts analog axis events to Corona if the given motion event provides them.
	 * Axis events typically come from a joystick, analog trigger, mouse, touchpad, and touchscreen.
	 * @param event The motion event to be processed by Corona.
	 * @return Returns true if Corona has handled the given event. Returns false if not.
	 */
	private boolean handleAxisEvent(android.view.MotionEvent event) {
		// Validate.
		if (event == null) {
			return false;
		}

		// Do not continue if running on an Android OS that does not support axis input.
		if (android.os.Build.VERSION.SDK_INT < 12) {
			return false;
		}

		// Fetch the input device that generated this key event.
		InputDeviceInterface inputDevice = null;
		if (event.getDeviceId() > 0) {
			android.content.Context context = com.ansca.corona.CoronaEnvironment.getApplicationContext();
			if (context != null) {
				InputDeviceServices deviceServices = new InputDeviceServices(context);
				inputDevice = deviceServices.fetchDeviceFrom(event);
			}
		}
		if (inputDevice == null) {
			return false;
		}

		// Fetch data from all axis inputs and submit that data to the device context.
		// The device context will raise "axis" events if the values have changed.
		inputDevice.getContext().beginUpdate();
		ReadOnlyAxisInfoCollection axisCollection = inputDevice.getDeviceInfo().getAxes();
		for (int axisIndex = 0; axisIndex < axisCollection.size(); axisIndex++) {
			AxisInfo axisInfo = axisCollection.getByIndex(axisIndex);
			if (axisInfo != null) {
				try {
					float value = ApiLevel12.getAxisValueFrom(event, axisInfo.getType().toAndroidIntegerId());
					inputDevice.getContext().update(axisIndex, new AxisDataPoint(value, event.getEventTime()));
				}
				catch (Exception ex) { }
			}
		}
		inputDevice.getContext().endUpdate();

		// Do not consume the axis event by returning true.
		// Doing so would prevent a gamepad's d-pad or left thumbstick from providing arrow key events
		return false;
	}

	/**
	 * Posts mouse related events to Corona if the given motion event provides mouse input.
	 * @param event The motion event to be processed by Corona.
	 * @return Returns true if Corona has handled the given event. Returns false if not.
	 */
	private boolean handleMouseEvent(android.view.MotionEvent event) {
		// Validate.
		if (event == null) {
			return false;
		}

		// Do not continue if Corona "mouse" events cannot be produced from the given motion event.
		InputDeviceType deviceType = InputDeviceType.from(event);
		if (deviceType != InputDeviceType.MOUSE) {
			return false;
		}

		// Ignore the mouse event if invalid.
		if (event.getPointerCount() <= 0) {
			return false;
		}

		// Fetch the mouse cursor's state.
		int actionState = event.getAction() & android.view.MotionEvent.ACTION_MASK;

		// Fetch the mouse cursor's position.
		TouchPoint mousePoint = new TouchPoint(event.getX(), event.getY(), event.getEventTime());

		// Fetch the mouse's button states.
		boolean isPrimaryButtonPressed = false;
		boolean isSecondaryButtonPressed = false;
		boolean isMiddleButtonPressed = false;
		if (android.os.Build.VERSION.SDK_INT >= 14) {
			isPrimaryButtonPressed = ApiLevel14.isPrimaryMouseButtonPressedFor(event);
			isSecondaryButtonPressed = ApiLevel14.isSecondaryMouseButtonPressedFor(event);
			isMiddleButtonPressed = ApiLevel14.isMiddleMouseButtonPressedFor(event);
		}
		else if ((actionState == android.view.MotionEvent.ACTION_DOWN) ||
		         (actionState == android.view.MotionEvent.ACTION_MOVE)) {
			isPrimaryButtonPressed = true;
		}

		// Fetch the mouse's scroll wheel deltas in pixels, if provided.
		float scrollX = 0;
		float scrollY = 0;
		final int ACTION_SCROLL = 8;	// = android.view.MotionEvent.ACTION_SCROLL
		if ((android.os.Build.VERSION.SDK_INT >= 12) && (actionState == ACTION_SCROLL)) {
			// Fetch the normalized scroll wheel values. (Ranges between -1.0 to 1.0)
			final int AXIS_VSCROLL = 9;
			final int AXIS_HSCROLL = 10;
			scrollX = ApiLevel12.getAxisValueFrom(event, AXIS_HSCROLL);
			scrollY = ApiLevel12.getAxisValueFrom(event, AXIS_VSCROLL);

			// Scroll in pixels based on the system's default font size.
			// Note: This is a slightly smaller distance used by Google's internal View.getVerticalScrollFactor().
			scrollX *= fController.getDefaultFontSize();
			scrollY *= fController.getDefaultFontSize();

			// Invert the y scroll value so that scrolling down heads towards the positive direction.
			if (scrollY != 0) {
				scrollY *= -1.0f;
			}
		}

		// Raise a "mouse" event.
		if (fTaskDispatcher != null) {
			RaiseMouseEventTask task;
			task = new RaiseMouseEventTask(
							mousePoint.getX(), mousePoint.getY(),
							scrollX, scrollY,
							mousePoint.getTimestamp(),
							isPrimaryButtonPressed,
							isSecondaryButtonPressed,
							isMiddleButtonPressed);
			fTaskDispatcher.send(task);
		}

		// Fetch the mouse's touch tracker.
		int deviceId = event.getDeviceId();
		int pointerId = event.getPointerId(0);
		TouchTracker touchTracker = fTouchTrackers.getByDeviceAndPointerId(deviceId, pointerId);
		if (touchTracker == null) {
			touchTracker = new TouchTracker(deviceId, pointerId);
			fTouchTrackers.add(touchTracker);
		}

		// Update the mouse's touch tracker which starts when the primary/left mouse button is pressed.
		boolean hasReceivedTouchEvent = false;
		if (isPrimaryButtonPressed && (actionState != android.view.MotionEvent.ACTION_CANCEL)) {
			if (touchTracker.hasNotStarted()) {
				// The mouse's primary button has just been pressed down.
				touchTracker.updateWith(mousePoint, TouchPhase.BEGAN);
				hasReceivedTouchEvent = true;
			}
			else {
				// The mouse cursor's status has been updated while the primary button was down.
				// Raise a move event, but only if the cursor has actually moved.
				float deltaX = Math.abs(mousePoint.getX() - touchTracker.getLastPoint().getX());
				float deltaY = Math.abs(mousePoint.getY() - touchTracker.getLastPoint().getY());
				if ((deltaX >= 1.0f) || (deltaY >= 1.0f)) {
					touchTracker.updateWith(mousePoint, TouchPhase.MOVED);
					hasReceivedTouchEvent = true;
				}
			}
		}
		else if (touchTracker.hasStarted()) {
			// The mouse's primary button has just been released.
			if (actionState == android.view.MotionEvent.ACTION_CANCEL) {
				touchTracker.updateWith(mousePoint, TouchPhase.CANCELED);
			}
			else {
				touchTracker.updateWith(mousePoint, TouchPhase.ENDED);
			}
			hasReceivedTouchEvent = true;
		}

		// Raise a "touch" event, if occurred.
		if (hasReceivedTouchEvent && (fTaskDispatcher != null)) {
			if (fController.isMultitouchEnabled()) {
				TouchTrackerCollection trackerCollection = new TouchTrackerCollection();
				trackerCollection.add(touchTracker);
				fTaskDispatcher.send(new RaiseMultitouchEventTask(trackerCollection));
			}
			else {
				fTaskDispatcher.send(new RaiseTouchEventTask(touchTracker));
			}
		}

		// Reset the mouse's touch tracker if ended.
		// This must be done after posting the "touch" event up above.
		if ((touchTracker.getPhase() == TouchPhase.ENDED) ||
		    (touchTracker.getPhase() == TouchPhase.CANCELED)) {
			touchTracker.reset();
		}

		// Consume the event by returning true.
		return true;
	}

	/**
	 * Posts touchscreen related events to Corona if the given motion event provides touch input.
	 * @param event The motion event to be processed by Corona.
	 * @return Returns true if Corona has handled the given event. Returns false if not.
	 */
	private boolean handleTouchEvent(android.view.MotionEvent event) {
		TouchTracker touchTracker;
		TouchPoint touchPoint;

		// Validate.
		if (event == null) {
			return false;
		}

		// Do not continue if Corona "touch" events cannot be produced from the given motion event.
		InputDeviceType deviceType = InputDeviceType.from(event);
		if ((deviceType != InputDeviceType.TOUCHSCREEN) &&
		    (deviceType != InputDeviceType.STYLUS)) {
			return false;
		}

		// If multitouch is enabled, then create a touch collection for batching all touch events.
		TouchTrackerCollection multitouchTracker = null;
		if (fController.isMultitouchEnabled()) {
			multitouchTracker = new TouchTrackerCollection();
		}

		// Fetch the touch event's current phase such as began, moved, or ended.
		TouchPhase touchPhase = TouchPhase.from(event);

		// Handle the received touch event.
		switch (event.getAction() & android.view.MotionEvent.ACTION_MASK) {
			case android.view.MotionEvent.ACTION_DOWN:
				// A touch event from only one pointer has started.
				// Remove all previous touch trackers from that pointer's device.
				fTouchTrackers.removeByDeviceId(event.getDeviceId());

				// Start tracking the device's touch events.
				touchTracker = new TouchTracker(event.getDeviceId(), event.getPointerId(0));
				touchPoint = new TouchPoint(event.getX(), event.getY(), event.getEventTime());
				touchTracker.updateWith(touchPoint, TouchPhase.BEGAN, event.getPressure());
				fTouchTrackers.add(touchTracker);

				// Prepare to raise the received touch event.
				if (multitouchTracker != null) {
					// Batch the received touch event as a single multitouch event, to be raised later.
					multitouchTracker.add(touchTracker);
				}
				else if (fTaskDispatcher != null) {
					// Raise the touch event now.
					fTaskDispatcher.send(new RaiseTouchEventTask(touchTracker));
				}
				break;

			case android.view.MotionEvent.ACTION_MOVE:
			case android.view.MotionEvent.ACTION_UP:
			case android.view.MotionEvent.ACTION_CANCEL:
				// Single or multiple touch movements have been received.
				int pointerCount = event.getPointerCount();
				for (int pointerIndex = 0; pointerIndex < pointerCount; pointerIndex++) {
					// Fetch the touch pointer's tracker.
					int deviceId = event.getDeviceId();
					int pointerId = event.getPointerId(pointerIndex);
					touchTracker = fTouchTrackers.getByDeviceAndPointerId(deviceId, pointerId);
					if ((touchTracker == null) || touchTracker.hasNotStarted()) {
						continue;
					}

					// Ignore the pointer's touch event if it has been flagged moved, but has not actually moved.
					float xValue = event.getX(pointerIndex);
					float yValue = event.getY(pointerIndex);
					if (touchPhase == TouchPhase.MOVED) {
						float deltaX = Math.abs(xValue - touchTracker.getLastPoint().getX());
						float deltaY = Math.abs(yValue - touchTracker.getLastPoint().getY());
						if ((deltaX < 1.0f) && (deltaY < 1.0f)) {
							continue;
						}
					}

					// Update the pointer's touch tracker.
					touchPoint = new TouchPoint(xValue, yValue, event.getEventTime());
					touchTracker.updateWith(touchPoint, touchPhase, event.getPressure(pointerIndex));

					// Prepare to raise the received touch event.
					if (multitouchTracker != null) {
						// Batch the received touch event as a single multitouch event, to be raised later.
						multitouchTracker.add(touchTracker);
					}
					else if (fTaskDispatcher != null) {
						// Raise the touch event now.
						fTaskDispatcher.send(new RaiseTouchEventTask(touchTracker));
					}
				}

				// Remove the device's touch trackers if touch tracking has ended.
				if ((touchPhase == TouchPhase.ENDED) || (touchPhase == TouchPhase.CANCELED)) {
					fTouchTrackers.removeByDeviceId(event.getDeviceId());
				}
				break;

			case android.view.MotionEvent.ACTION_POINTER_DOWN:
				// A multitouch event has started.
				// Only handle it if multitouch has been activated.
				if (multitouchTracker != null) {
					// Remove the last touch tracker having the same pointer ID.
					int pointerIndex = event.getActionIndex();
					int pointerId = event.getPointerId(pointerIndex);
					fTouchTrackers.removeByDeviceAndPointerId(event.getDeviceId(), pointerId);

					// Create a new touch tracker with the given touch data.
					touchPoint = new TouchPoint(
									event.getX(pointerIndex), event.getY(pointerIndex), event.getEventTime());
					touchTracker = new TouchTracker(event.getDeviceId(), pointerId);
					touchTracker.updateWith(touchPoint, touchPhase, event.getPressure(pointerIndex));
					fTouchTrackers.add(touchTracker);

					// Batch the received touch event as a single multitouch event, to be raised later.
					multitouchTracker.add(touchTracker);
				}
				break;

			case android.view.MotionEvent.ACTION_POINTER_UP:
				// A pointer has been released from the screen/touchpad.
				// Fetch its touch tracker.
				int pointerIndex = event.getActionIndex();
				int pointerId = event.getPointerId(pointerIndex);
				touchTracker = fTouchTrackers.getByDeviceAndPointerId(event.getDeviceId(), pointerId);
				if (touchTracker != null) {
					// Update the pointer's touch tracker.
					touchPoint = new TouchPoint(
									event.getX(pointerIndex), event.getY(pointerIndex), event.getEventTime());
					touchTracker.updateWith(touchPoint, touchPhase, event.getPressure(pointerIndex));

					// Handle the received touch event.
					if (multitouchTracker != null) {
						// Batch the received touch event as a single multitouch event, to be raised later.
						multitouchTracker.add(touchTracker);

						// Stop tracking this pointer's touch events.
						fTouchTrackers.removeByDeviceAndPointerId(event.getDeviceId(), pointerId);
					}
					else {
						// Raise the touch event now.
						if (fTaskDispatcher != null) {
							fTaskDispatcher.send(new RaiseTouchEventTask(touchTracker));
						}

						// Stop tracking this device's touch events.
						fTouchTrackers.removeByDeviceId(event.getDeviceId());
					}
				}
				break;
		}

		// Raise a multitouch event, if enabled.
		if ((multitouchTracker != null) && (multitouchTracker.size() > 0) && (fTaskDispatcher != null)) {
			fTaskDispatcher.send(new RaiseMultitouchEventTask(multitouchTracker));
		}

		// Consume the event by returning true.
		return true;
    }

	/**
	 * Posts tap events to Corona if the given motion event provides them.
	 * @param event The motion event to be processed by Corona.
	 * @return Returns true if Corona has handled the given event. Returns false if not.
	 */
	private boolean handleTapEvent(android.view.MotionEvent event) {
		// Validate.
		if (event == null) {
			return false;
		}

		// Do not continue if Corona "tap" events cannot be produced from the given motion event.
		InputDeviceType deviceType = InputDeviceType.from(event);
		if ((deviceType != InputDeviceType.TOUCHSCREEN) &&
		    (deviceType != InputDeviceType.STYLUS) &&
		    (deviceType != InputDeviceType.MOUSE)) {
			return false;
		}

		// Fetch the device's tap tracker.
		TapTracker tapTracker = fTapTrackers.getByDeviceId(event.getDeviceId());
		if (tapTracker == null) {
			tapTracker = new TapTracker(event.getDeviceId());
			fTapTrackers.add(tapTracker);
		}

		// Update the tap tracker with the given touch event data.
		tapTracker.updateWith(event);

		// Raise a tap event, if occurred.
		if (tapTracker.hasTapOccurred() && (fTaskDispatcher != null)) {
			fTaskDispatcher.send(new RaiseTapEventTask(tapTracker.getTapPoint(), tapTracker.getTapCount()));
		}

		// Consume the event by returning true.
		return true;
	}


	/**
	 * Provides access to API Level 12 (Honeycomb) features.
	 * Should only be accessed if running on an operating system matching this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * You are expected to call its static methods instead.
	 */
	private static class ApiLevel12 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel12() { }

		/**
		 * Fetches an axis value from the given motion event.
		 * <p>
		 * This method will throw an exception if the given event does not provide data for
		 * the given "androidAxisId".
		 * @param event The event provided by Android's onGenericMotion() listener. Cannot be null.
		 * @param androidAxisId Unique ID of the axis to fetch data from. Must match an "AXIS" constant
		 *                      from the Android MotionEvent class such as AXIS_X, AXIS_Y, etc.
		 * @return Returns the requested axis value.
		 */
		public static float getAxisValueFrom(android.view.MotionEvent event, int androidAxisId) {
			if (event == null) {
				throw new NullPointerException();
			}
			return event.getAxisValue(androidAxisId);
		}
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
		 * Determines if the primary mouse button was pressed down for the given motion event.
		 * <p>
		 * Note that the primary mouse button is typically the left mouse button.
		 * @param event The mouse motion event providing mouse button states.
		 *              Mouse button status will be retrieved by calling its getButtonState() method.
		 * @return Returns true if the primary button was held down at the time the event was recorded.
		 *         <p>
		 *         Returns false if the button was not held down or if given an invalid event.
		 */
		public static boolean isPrimaryMouseButtonPressedFor(android.view.MotionEvent event) {
			if (event == null) {
				return false;
			}
			return ((event.getButtonState() & android.view.MotionEvent.BUTTON_PRIMARY) != 0);
		}

		/**
		 * Determines if the secondary mouse button was pressed down for the given motion event.
		 * <p>
		 * Note that the secondary mouse button is typically the right mouse button.
		 * @param event The mouse motion event providing mouse button states.
		 *              Mouse button status will be retrieved by calling its getButtonState() method.
		 * @return Returns true if the secondary button was held down at the time the event was recorded.
		 *         <p>
		 *         Returns false if the button was not held down or if given an invalid event.
		 */
		public static boolean isSecondaryMouseButtonPressedFor(android.view.MotionEvent event) {
			if (event == null) {
				return false;
			}
			return ((event.getButtonState() & android.view.MotionEvent.BUTTON_SECONDARY) != 0);
		}

		/**
		 * Determines if the middle/tertiary mouse button was pressed down for the given motion event.
		 * @param event The mouse motion event providing mouse button states.
		 *              Mouse button status will be retrieved by calling its getButtonState() method.
		 * @return Returns true if the middle button was held down at the time the event was recorded.
		 *         <p>
		 *         Returns false if the button was not held down or if given an invalid event.
		 */
		public static boolean isMiddleMouseButtonPressedFor(android.view.MotionEvent event) {
			if (event == null) {
				return false;
			}
			return ((event.getButtonState() & android.view.MotionEvent.BUTTON_TERTIARY) != 0);
		}
	}
}
