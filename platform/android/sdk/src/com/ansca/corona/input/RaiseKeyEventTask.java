//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.input;


import android.util.Log;

/**
 * Corona runtime task used to raise a "key" event to be received by a Lua listener.
 * <p>
 * Instances of this class are expected to be passed to the CoronaRuntimeTaskDisplatcher.send() method.
 * The dispatcher will then have this task executed by the Corona runtime in a thread safe manner.
 * @see com.ansca.corona.CoronaRuntimeTaskDispatcher
 */
public class RaiseKeyEventTask implements com.ansca.corona.CoronaRuntimeTask {
	/** Reference to the device that generated this event. */
	private InputDeviceInterface fDevice;

	/** Stores a reference to the key event that was received. */
	private android.view.KeyEvent fKeyEvent;


	/**
	 * Creates a new Corona runtime task used to raise a "key" event to be received by a Lua listener.
	 * @param device Reference to the device that generated this input event.
	 * @param event Reference to the key event that was received. Cannot be null.
	 */
	public RaiseKeyEventTask(InputDeviceInterface device,android.view.KeyEvent event) {
		if (event == null) {
			throw new NullPointerException();
		}
		fDevice = device;
		fKeyEvent = event;
	}

	/**
	 * Called by the Corona runtime just before it renders a frame.
	 * <p>
	 * Raises a "key" event in the Corona runtime to be received by a Lua listener.
	 * <p>
	 * Warning! This method is not called on the main thread.
	 * @param runtime Reference to the CoronaRuntime object that is currently running.
	 *                Provides access to its LuaState object.
	 */
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		// Validate.
		if (fKeyEvent == null) {
			return;
		}
		
		// Send the key event to Lua and determine if it is to be overriden.
		boolean isCommandPressed = false;
		boolean isCtrlPressed = false;
		boolean isShiftPressed = fKeyEvent.isShiftPressed();
		if (android.os.Build.VERSION.SDK_INT >= 11) {
			isCommandPressed = ApiLevel11.isMetaPressedFor(fKeyEvent);
			isCtrlPressed = ApiLevel11.isCtrlPressedFor(fKeyEvent);
			isShiftPressed |= ApiLevel11.isCapsLockOnFor(fKeyEvent);
		}
		boolean wasKeyOverridden = com.ansca.corona.JavaToNativeShim.keyEvent(runtime, 
										fDevice,
										com.ansca.corona.input.KeyPhase.from(fKeyEvent),
										fKeyEvent.getKeyCode(),
										isShiftPressed,
										fKeyEvent.isAltPressed(),
										isCtrlPressed,
										isCommandPressed);

		// Fetch the activity, if still shown.  In CoronaCards this will return null and we won't try to dispatch the event back to the activity
		com.ansca.corona.CoronaActivity activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();
		if (activity == null) {
			return;
		}

		// If the key was not overridden, then resend the key back to the Activity to perform the default handling.
		if (!wasKeyOverridden && !(fKeyEvent instanceof CoronaKeyEvent))  {
			activity.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					try {
						// Fetch the activity, if still shown.
						com.ansca.corona.CoronaActivity activity;
						activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();
						if (activity == null) {
							return;
						}

						// Do not continue if the activity is being destroyed.
						if (activity.isFinishing()) {
							return;
						}

						// Re-send the key event to the to most view.
						activity.dispatchKeyEvent(new CoronaKeyEvent(fKeyEvent));
					}
					catch (Exception e) {
						Log.e("Corona", "Exception was caught in activity.dispatchKeyEvent", e);
					}
				}
			});
		}
	}


	/**
	 * Provides access to API Level 11 (Honeycomb) features.
	 * Should only be accessed if running on an operating system matching this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * Instead, you are expected to call its static methods instead.
	 */
	private static class ApiLevel11 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel11() {}

		/**
		 * Determines if the caps lock was on/off at the time the given key event occurred.
		 * @param event Reference to the key event to check. Cannot be null.
		 * @return Returns true if the Caps Lock is currently on. Returns false if off.
		 */
		public static boolean isCapsLockOnFor(android.view.KeyEvent event) {
			return event.isCapsLockOn();
		}

		/**
		 * Determines if the Ctrl key was down/up at the time the given key event occurred.
		 * @param event Reference to the key event to check. Cannot be null.
		 * @return Returns true if the Ctrl key was down. Returns false if up.
		 */
		public static boolean isCtrlPressedFor(android.view.KeyEvent event) {
			return event.isCtrlPressed();
		}

		/**
		 * Determines if the Command/Windows key was pressed/released at the time the given key event occurred.
		 * @param event Reference to the key event to check. Cannot be null.
		 * @return Returns true if the Command/Windows key was down. Returns false if up.
		 */
		public static boolean isMetaPressedFor(android.view.KeyEvent event) {
			return event.isMetaPressed();
		}
	}
}
