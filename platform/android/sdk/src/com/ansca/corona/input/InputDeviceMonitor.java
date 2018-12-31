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
 * Monitors Android managed input devices and detects when device have connected to the system,
 * disconnected from the system, and when devices have been reconfigured.
 * <p>
 * Note that device monitoring is noly supported on Android 2.3 and higher operating systems.
 * This monitor will do nothing on Android 2.2 since that operating system version does not
 * provide access to input devices.
 */
public class InputDeviceMonitor {
	/** Reference to the application context needed to create the API Level 16 event handler. */
	private android.content.Context fContext;

	/** The handler that will receive input device status events and send them to this monitor. */
	private EventHandler fEventHandler;

	/** Listener which receives this monitor's input device events. */
	private InputDeviceMonitor.Listener fListener;

	/** Set to true if this monitor has been started and is listening for device events. */
	private boolean fIsRunning;


	/**
	 * Listener used to receive an InputDeviceMonitor object's events.
	 * <p>
	 * Will receive events when passed to the InputDeviceMonitor.setListener() metohd.
	 */
	public interface Listener {
		/**
		 * Called when a new device has been connected to the system or when an existing device
		 * has been re-connected to the system.
		 * @param androidDeviceId Unique integer ID assigned to the device by Android.
		 *                        Expected to be given to the Android InputDevice.getDevice(id) method.
		 */
		public void onInputDeviceConnected(int androidDeviceId);

		/**
		 * Called when an existing device has been disconnected.
		 * @param androidDeviceId Unique integer ID assigned to the device by Android.
		 *                        Matches the ID returned by the Android InputDevice.getId() method.
		 */
		public void onInputDeviceDisconnected(int androidDeviceId);
		
		/**
		 * Called when an existing device has been reconfigured,
		 * such as when axis inputs have been added or removed.
		 * @param androidDeviceId Unique integer ID assigned to the device by Android.
		 *                        Expected to be given to the Android InputDevice.getDevice(id) method.
		 */
		public void onInputDeviceReconfigured(int androidDeviceId);
	}


	/**
	 * Creates a new monitor which detects input device configuration and connection changes.
	 * @param context Reference to an Android created context used to access the input device system.
	 *                <p>
	 *                Setting this to null will cause an exception to be thrown.
	 */
	public InputDeviceMonitor(android.content.Context context) {
		// Validate.
		if (context == null) {
			throw new NullPointerException();
		}

		// Initialize member variables.
		fContext = context.getApplicationContext();
		fListener = null;
		fIsRunning = false;

		// Create the input device event handler based on the OS version we're running on.
		// This must be done after initializing the above member variables.
		if (android.os.Build.VERSION.SDK_INT >= 16) {
			fEventHandler = ApiLevel16.createEventHandlerWith(this);
		}
		else if (android.os.Build.VERSION.SDK_INT >= 9) {
			fEventHandler = ApiLevel9.createEventHandlerWith(this);
		}
		else {
			fEventHandler = null;
		}
	}

	/**
	 * Gets a reference to the application context used by this device monitor.
	 * @return Returns a reference to the application context.
	 */
	public android.content.Context getContext() {
		return fContext;
	}

	/**
	 * Sets the listener that will receive this monitor's input device events.
	 * @param listener Reference to the listener that will receive input device events.
	 *                 <p>
	 *                 Set to null to remove the listener.
	 */
	public void setListener(InputDeviceMonitor.Listener listener) {
		fListener = listener;
	}

	/**
	 * Gets the listener that this monitor will send input device events to.
	 * @return Returns the listener reference that was given to this monitor via the setListener() method.
	 *         <p>
	 *         Returns null if this monitor has not been assigned a listener.
	 */
	public InputDeviceMonitor.Listener getListener() {
		return fListener;
	}

	/**
	 * Determines if it is possible to monitor Android managed input devices on the currently
	 * running Android operating system version.
	 * @return Returns true if device monitoring is supported. This means this monitor can provide
	 *         events when devices are connected, disconnected, reconfigured, etc.
	 *         <p>
	 *         Returns false if the operating system does not support monitoring input devices.
	 *         This will always be the case for Android 2.2 (API Level 8) or older operating systems.
	 *         In this case, calling this monitor's start() method will do nothing and device events
	 *         will never be raised.
	 */
	public boolean canMonitorDevices() {
		return (fEventHandler != null);
	}

	/**
	 * Determines if this monitor has been started and is actively listening for device events.
	 * @return Returns true if this monitor has been started and is listening for events.
	 *         <p>
	 *         Returns false if this monitor has been stopped.
	 */
	public boolean isRunning() {
		return fIsRunning;
	}

	/** Starts monitoring for input device configuration and connection changes. */
	public void start() {
		// Do not continue if already running.
		if (fIsRunning) {
			return;
		}

		// Start monitoring by subscribing to the system's device events.
		if (fEventHandler != null) {
			fEventHandler.subscribe();
			fIsRunning = true;
		}
	}

	/**
	 * Stops monitoring for input device events.
	 * <p>
	 * Expected to be called when an activity has been paused or stopped.
	 */
	public void stop() {
		// Do not continue if already stopped.
		if (fIsRunning == false) {
			return;
		}

		// Stop monitoring by unsubscribing from the system's device events.
		if (fEventHandler != null) {
			fEventHandler.unsubscribe();
			fIsRunning = false;
		}
	}

	/**
	 * Called by the monitor's event handler when a new device has connected
	 * or when an existing device has been re-connected to the system.
	 * @param androidDeviceId Unique integer ID assigned to the device by Android.
	 *                        Expected to be given to the Android InputDevice.getDevice(id) method.
	 */
	private void onInputDeviceConnected(int androidDeviceId) {
		InputDeviceMonitor.Listener listener = fListener;
		if (listener != null) {
			listener.onInputDeviceConnected(androidDeviceId);
		}
	}

	/**
	 * Called by the monitor's event handler when an existing device has been disconnected.
	 * @param androidDeviceId Unique integer ID assigned to the device by Android.
	 *                        Matches the ID returned by the Android InputDevice.getId() method.
	 */
	private void onInputDeviceDisconnected(int androidDeviceId) {
		InputDeviceMonitor.Listener listener = fListener;
		if (listener != null) {
			listener.onInputDeviceDisconnected(androidDeviceId);
		}
	}

	/**
	 * Called by the monitor's event handler when an existing device has been reconfigured,
	 * such as when axis inputs have been added or removed.
	 * @param androidDeviceId Unique integer ID assigned to the device by Android.
	 *                        Expected to be given to the Android InputDevice.getDevice(id) method.
	 */
	private void onInputDeviceReconfigured(int androidDeviceId) {
		InputDeviceMonitor.Listener listener = fListener;
		if (listener != null) {
			listener.onInputDeviceReconfigured(androidDeviceId);
		}
	}


	/** Abstract class from which the device monitor's event handler must derive from. */
	private static abstract class EventHandler {
		private InputDeviceMonitor fDeviceMonitor;

		/**
		 * Creates a new input device status event handler.
		 * @param deviceMonitor Reference to the device monitor to send events to. Cannot be null.
		 */
		public EventHandler(InputDeviceMonitor deviceMonitor) {
			if (deviceMonitor == null) {
				throw new NullPointerException();
			}
			fDeviceMonitor = deviceMonitor;
		}

		/**
		 * Gets a reference to the device monitor that this handler will notify when
		 * device changes have been detected.
		 * @return Returns a reference to the device monitor that owns this handler.
		 */
		public InputDeviceMonitor getDeviceMonitor() {
			return fDeviceMonitor;
		}

		/**
		 * Subscribes to input device events.
		 * <p>
		 * Expected to be called when the input device monitor has been started.
		 */
		public abstract void subscribe();

		/**
		 * Unsubscribes from input device events.
		 * <p>
		 * Expected to be called when the input device monitor has been stopped.
		 */
		public abstract void unsubscribe();
	}


	/**
	 * Provides access to API Level 9 (Gingerbread) features.
	 * Should only be accessed if running on an operating system matching this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * You are expected to call its static methods instead.
	 */
	private static class ApiLevel9 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel9() { }

		/**
		 * Creates a new EventHandler object that takes advantage of API Level 9's features.
		 * @param deviceMonitor Reference to the device monitor to send events to. Cannot be null.
		 */
		public static InputDeviceMonitor.EventHandler createEventHandlerWith(InputDeviceMonitor deviceMonitor) {
			return new ApiLevel9.EventHandler(deviceMonitor);
		}

		/**
		 * Periodically polls for the status of all input devices and notifies the InputDeviceMonitor
		 * which owns this handler when status change has been detected.
		 * <p>
		 * This event handler should only be used on Android OS versions 2.3 to 4.0.
		 * <p>
		 * This class is private and is only expected to be used by the InputDeviceMonitor class.
		 */
		private static class EventHandler extends InputDeviceMonitor.EventHandler
			implements com.ansca.corona.MessageBasedTimer.Listener
		{
			/** Timer used to poll for input device status. */
			private com.ansca.corona.MessageBasedTimer fTimer;

			/**
			 * Stores an array of Android device IDs that was last retrieved from the system.
			 * <p>
			 * This is used by the timer to detect if the input device IDs retrieved from the
			 * system is different than the retrieved IDs.
			 */
			private int[] fLastAndroidDeviceIdArray;


			/**
			 * Creates a new input device status event handler.
			 * @param deviceMonitor Reference to the device monitor to send events to. Cannot be null.
			 */
			public EventHandler(InputDeviceMonitor deviceMonitor) {
				super(deviceMonitor);

				// Initialize member variables.
				fTimer = new com.ansca.corona.MessageBasedTimer();
				fTimer.setHandler(new android.os.Handler(android.os.Looper.getMainLooper()));
				fTimer.setInterval(com.ansca.corona.TimeSpan.fromSeconds(1));
				fTimer.setListener(this);
				fLastAndroidDeviceIdArray = new int[] {};
			}

			/**
			 * Subscribes to input device events.
			 * <p>
			 * Expected to be called when the input device monitor has been started.
			 */
			@Override
			public void subscribe() {
				// Fetch all Android device IDs currently available.
				// This way we won't trigger any connected/disconnected events since monitoring has stopped.
				fLastAndroidDeviceIdArray = android.view.InputDevice.getDeviceIds();
				if (fLastAndroidDeviceIdArray == null) {
					fLastAndroidDeviceIdArray = new int[] {};
				}

				// Start monitoring for input device changes.
				fTimer.start();
			}

			/**
			 * Unsubscribes from input device events.
			 * <p>
			 * Expected to be called when the input device monitor has been stopped.
			 */
			@Override
			public void unsubscribe() {
				fTimer.stop();
			}

			/**
			 * Called when this event handler's timer elapses.
			 * <p>
			 * Polls for all input devices currently connected to the system and determines if
			 * any new devices have been connected or existing devices have been disconnected.
			 */
			@Override
			public void onTimerElapsed() {
				// Fetch all Android device IDs from the system.
				int[] receivedAndroidDeviceIdArray = android.view.InputDevice.getDeviceIds();
				if (receivedAndroidDeviceIdArray == null) {
					receivedAndroidDeviceIdArray = new int[] {};
				}

				// Check if any devices have been added.
				for (int receivedDeviceId : receivedAndroidDeviceIdArray) {
					boolean isNewDevice = true;
					for (int existingDeviceId : fLastAndroidDeviceIdArray) {
						if (receivedDeviceId == existingDeviceId) {
							// This is a known device. Ignore it.
							isNewDevice = false;
							break;
						}
					}
					if (isNewDevice) {
						// A new device has been found. Notify the system by raising an event.
						getDeviceMonitor().onInputDeviceConnected(receivedDeviceId);
					}
				}

				// Check if any device have been removed.
				for (int existingDeviceId : fLastAndroidDeviceIdArray) {
					boolean wasDeviceRemoved = true;
					for (int receivedDeviceId : receivedAndroidDeviceIdArray) {
						if (receivedDeviceId == existingDeviceId) {
							// This device is still available.
							wasDeviceRemoved = false;
							break;
						}
					}
					if (wasDeviceRemoved) {
						// A new device has been removed. Notify the system by raising an event.
						getDeviceMonitor().onInputDeviceDisconnected(existingDeviceId);
					}
				}

				// Store the recently fetched device IDs for the next time we poll the system.
				fLastAndroidDeviceIdArray = receivedAndroidDeviceIdArray;
			}
		}
	}


	/**
	 * Provides access to API Level 16 (Jelly Bean) features.
	 * Should only be accessed if running on an operating system matching this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * You are expected to call its static methods instead.
	 */
	private static class ApiLevel16 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel16() { }


		/**
		 * Creates a new EventHandler object that takes advantage of API Level 16's features.
		 * @param deviceMonitor Reference to the device monitor to send events to. Cannot be null.
		 */
		public static InputDeviceMonitor.EventHandler createEventHandlerWith(InputDeviceMonitor deviceMonitor) {
			return new ApiLevel16.EventHandler(deviceMonitor);
		}

		/**
		 * Detects input device changes via the Android InputManager class and notifies the
		 * device monitor when these changes occur.
		 * <p>
		 * This event handler should only be used on Android 4.1 or newer OS versions.
		 * <p>
		 * This class is private and is only expected to be used by the InputDeviceMonitor class.
		 */
		private static class EventHandler extends InputDeviceMonitor.EventHandler
			implements android.hardware.input.InputManager.InputDeviceListener
		{
			/** Handler needed to receive events from the Android InputManager class. */
			private android.os.Handler fHandler;


			/**
			 * Creates a new input device status event handler.
			 * @param deviceMonitor Reference to the device monitor to send events to. Cannot be null.
			 */
			public EventHandler(InputDeviceMonitor deviceMonitor) {
				super(deviceMonitor);

				fHandler = new android.os.Handler(android.os.Looper.getMainLooper());
			}

			/**
			 * Subscribes to input device events.
			 * <p>
			 * Expected to be called when the input device monitor has been started.
			 */
			@Override
			public void subscribe() {
				android.hardware.input.InputManager inputManager = getInputManager();
				if (inputManager != null) {
					inputManager.registerInputDeviceListener(this, fHandler);
				}
			}

			/**
			 * Unsubscribes from input device events.
			 * <p>
			 * Expected to be called when the input device monitor has been stopped.
			 */
			@Override
			public void unsubscribe() {
				android.hardware.input.InputManager inputManager = getInputManager();
				if (inputManager != null) {
					inputManager.unregisterInputDeviceListener(this);
				}
			}

			/**
			 * Fetches the Android input manager from the application context.
			 * @return Returns a reference to the Android input manager.
			 */
			private android.hardware.input.InputManager getInputManager() {
				String serviceName = android.content.Context.INPUT_SERVICE;
				android.content.Context context = getDeviceMonitor().getContext();
				return (android.hardware.input.InputManager)(context.getSystemService(serviceName));
			}

			/**
			 * Called when a new input device has connected to the system.
			 * @param device Unique integer ID assigned to the input device by Android.
			 */
			@Override
			public void onInputDeviceAdded(int deviceId) {
				getDeviceMonitor().onInputDeviceConnected(deviceId);
			}

			/**
			 * Called when a input device has been disconnected from the system.
			 * @param device Unique integer ID assigned to the input device by Android.
			 */
			@Override
			public void onInputDeviceRemoved(int deviceId) {
				getDeviceMonitor().onInputDeviceDisconnected(deviceId);
			}
			
			/**
			 * Called when an input device has been reconfigured, such as when axis inputs
			 * have been or removed from it.
			 * @param device Unique integer ID assigned to the input device by Android.
			 */
			@Override
			public void onInputDeviceChanged(int deviceId) {
				getDeviceMonitor().onInputDeviceReconfigured(deviceId);
			}
		}
	}
}
