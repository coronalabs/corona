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
 * Provides easy thread safe access to input devices for Android 2.2 (API Level 8) and higher operating systems.
 * <p>
 * Also allows plugin developers to add custom input devices to Corona that are not managed by Android,
 * which will provide device information and input events to the Lua side of Corona in the same manner
 * as Android managed input devices. This is needed for input devices that do not use the HID protocol.
 * You would add custom input devices by calling this class' add() method.
 */
public final class InputDeviceServices extends com.ansca.corona.ApplicationContextProvider {
	/** Stores information about every input device that has ever connected to the system. */
	private static InputDeviceInterfaceCollection sDeviceCollection = new InputDeviceInterfaceCollection();

	/** Provides unique IDs for every input device added to this class. */
	private static int sNextCoronaDeviceId = 1;

	/** Monitor for connection and configuration changes to Android managed input devices. */
	private static InputDeviceMonitor sDeviceMonitor = null;

	/**
	 * Dictionary of task dispatchers which uses CoronaRuntime references as the key.
	 * <p>
	 * Only one Corona runtime is expected to exist at a time, but this class will support
	 * multiple runtime in case support for this option is added in the future.
	 */
	private static java.util.HashMap<com.ansca.corona.CoronaRuntime, com.ansca.corona.CoronaRuntimeTaskDispatcher>
						sTaskDispatcherMap = new java.util.HashMap<com.ansca.corona.CoronaRuntime, com.ansca.corona.CoronaRuntimeTaskDispatcher>();


	/** Static constructor. */
	static {
		// Set up a Corona runtime listener. Used to do the following:
		// - Create/destroy CoronaRuntimeTaskDispatcher objects when Corona runtimes are created/destroyed.
		// - Stop/start device monitoring as this application is suspened/resumed.
		com.ansca.corona.CoronaEnvironment.addRuntimeListener(new CoronaRuntimeEventHandler());
	}

	/**
	 * Creates an object that provides access to the input device system.
	 * @param context Reference to an Android created context used to access the input device system.
	 *                <p>
	 *                Setting this to null will cause an exception to be thrown.
	 */
	public InputDeviceServices(android.content.Context context) {
		super(context);

		// Create and set up a new input device monitor, if not done already.
		// Will notify this class when Android managed devices have been added, removed, or reconfigured.
		if (sDeviceMonitor == null) {
			sDeviceMonitor = new InputDeviceMonitor(context);
			sDeviceMonitor.setListener(new InputDeviceMonitorEventHandler());
		}

		// Fetch all Android managed input devices, if not done already.
		if (android.os.Build.VERSION.SDK_INT >= 9) {
			if (sDeviceCollection.size() <= 0) {
				fetchAll();
			}
		}
	}

	/**
	 * Adds a custom input device to Corona that is not managed by Android.
	 * <p>
	 * This method is only to be called for devices that are not managed by Android, meaning the Android
	 * operating system will never provide input events for them. This includes devices that do not use
	 * the HID protocol. Adding custom devices to Corona via this method will expose them in Lua similarly
	 * to Android managed input devices, invoking the same input Lua input events.
	 * @param deviceSettings The configuration of the input device to be added to Corona and exposed
	 *                       to Lua via its input events.
	 * @return Returns a context object that the caller can use to update the device's configuration,
	 *         connection status, and send device input events to. Corona will raise Lua events for
	 *         all updates and input events that have been received by this context object.
	 *         <p>
	 *         Returns null if given a null argument.
	 */
	public InputDeviceContext add(InputDeviceSettings deviceSettings) {
		return InputDeviceServices.add(InputDeviceInfo.from(deviceSettings));
	}

	/**
	 * Adds the given device to Corona's collection, to be exposed to the Lua side of Corona.
	 * @param deviceInfo The configuration of the input device to be added to Corona and exposed
	 *                   to Lua via its input events.
	 * @return Returns a context object that the caller can use to update the device's configuration,
	 *         connection status, and send device input events to. Corona will raise Lua events for
	 *         all updates and input events that have been received by this context object.
	 *         <p>
	 *         Returns null if given a null argument.
	 */
	private static InputDeviceContext add(InputDeviceInfo deviceInfo) {
		synchronized (InputDeviceServices.class) {
			// Validate.
			if (deviceInfo == null) {
				return null;
			}

			// Do not add the device if it already exists in the collection.
			if (deviceInfo.hasAndroidDeviceId()) {
				InputDeviceInterface deviceInterface = sDeviceCollection.getByAndroidDeviceIdAndType(
											deviceInfo.getAndroidDeviceId(), deviceInfo.getType());
				if (deviceInterface != null) {
					return null;
				}
			}

			// Generate a unique ID for the given device.
			int coronaDeviceId = sNextCoronaDeviceId;
			sNextCoronaDeviceId++;

			// Create a context for the given device and add its interface to the main collection.
			InputDeviceContext deviceContext = new InputDeviceContext(coronaDeviceId, deviceInfo);
			sDeviceCollection.add(new InputDeviceInterface(deviceContext));

			// Add event listeners to the new device context.
			deviceContext.addListener(InputDeviceContextEventHandler.INSTANCE);
			if (deviceInfo.hasAndroidDeviceId()) {
				deviceContext.setVibrateRequestHandler(InputDeviceContextEventHandler.INSTANCE);
			}

			// Return the device context.
			return deviceContext;
		}
	}

	/**
	 * Fetches all input devices that have ever been connected to the system.
	 * @return Returns a collection of all input devices that have ever been connected.
	 *         Note that some device in this collection may be flagged as disconnected.
	 */
	public InputDeviceInterfaceCollection fetchAll() {
		// Fetch all input device information from Android and update this collection.
		// This is to make sure this class is in sync with Android's device list.
		updateCollection();

		// Return a clone of the device collection.
		InputDeviceInterfaceCollection deviceCollection = null;
		synchronized (InputDeviceServices.class) {
			deviceCollection = sDeviceCollection.clone();
		}
		if (deviceCollection == null) {
			deviceCollection = new InputDeviceInterfaceCollection();
		}
		return deviceCollection;
	}

	/**
	 * Fetches all input devices having the given Android device ID.
	 * <p>
	 * Note that Corona may create multiple DeviceInterface objects for the same Android input device
	 * if it has multiple sets of axis inputs, such as the Ouya game controller which is has
	 * Joystick axis inputs and mouse/touchpad axis inputs.
	 * @param id Unique integer ID assigned to the input device by Android.
	 *           This is the ID that is returnd by Android's InputDevice.getId() method.
	 * @return Returns a collection of devices having the given Android device ID.
	 *         <p>
	 *         Returns an empty collection if the given ID was not found.
	 */
	public InputDeviceInterfaceCollection fetchByAndroidDeviceId(int id) {
		// Update the device collection with the newest information about the specified Android input device.
		// This will add the input device to the collection if it doesn't contain it yet.
		updateCollectionWithAndroidDeviceId(id);

		// Return a collection of devices having the given Android device ID.
		InputDeviceInterfaceCollection collection = new InputDeviceInterfaceCollection();
		synchronized (InputDeviceServices.class) {
			for (InputDeviceInterface deviceInterface : sDeviceCollection) {
				if (deviceInterface.getDeviceInfo().hasAndroidDeviceId() &&
				    (deviceInterface.getDeviceInfo().getAndroidDeviceId() == id))
				{
					collection.add(deviceInterface);
				}
			}
		}
		return collection;
	}

	/**
	 * Fetches the input device that generated the given key event.
	 * @param event The key event to fetch device information from.
	 * @return Returns a reference to the requested input device, if found.
	 *         <p>
	 *         Returns null if the event's device could not be found or if given a null argument.
	 */
	public InputDeviceInterface fetchDeviceFrom(android.view.KeyEvent event) {
		// Validate.
		if (event == null) {
			return null;
		}

		// Fetch the event's input device information.
		return fetchByAndroidDeviceIdAndType(event.getDeviceId(), InputDeviceType.from(event));
	}

	/**
	 * Fetches the input device that generated the given motion event.
	 * @param event The motion event to fetch device information from.
	 * @return Returns a reference to the requested input device, if found.
	 *         <p>
	 *         Returns null if the event's device could not be found or if given a null argument.
	 */
	public InputDeviceInterface fetchDeviceFrom(android.view.MotionEvent event) {
		// Validate.
		if (event == null) {
			return null;
		}

		// Fetch the event's input device information.
		return fetchByAndroidDeviceIdAndType(event.getDeviceId(), InputDeviceType.from(event));
	}

	/**
	 * Fetches an input device by its Android assigned ID and device type.
	 * @param id Unique integer ID assigned to the input device by Android.
	 *           This is the ID that is returned by Android's InputDevice.getId() method.
	 * @param type The type of device to fetch such as KEYBOARD, MOUSE, JOYSTICK, etc.
	 * @return Returns a reference to the requested input device, if found.
	 *         <p>
	 *         Returns null if a device having the given ID and type could not be found.
	 */
	public InputDeviceInterface fetchByAndroidDeviceIdAndType(int id, InputDeviceType type) {
		// Validate.
		if (type == null) {
			return null;
		}

		// Update the device collection with the newest information about the specified Android input device.
		// This will add the input device to the collection if it doesn't contain it yet.
		updateCollectionWithAndroidDeviceId(id);

		// Fetch the requested input device from the main collection.
		InputDeviceInterface deviceInterface;
		synchronized (InputDeviceServices.class) {
			deviceInterface = sDeviceCollection.getByAndroidDeviceIdAndType(id, type);
		}
		return deviceInterface;
	}

	/**
	 * Fetches an input device by its Corona assigned ID.
	 * @param id Unique integer ID assigned to the input device by Corona.
	 * @return Returns a reference to the requested input device matching the given ID.
	 *         <p>
	 *         Returns null if the given ID was not found.
	 */
	public InputDeviceInterface fetchByCoronaDeviceId(int id) {
		synchronized (InputDeviceServices.class) {
			return sDeviceCollection.getByCoronaDeviceId(id);
		}
	}

	/**
	 * Updates the device collection with all input device information retrieved from the Android system.
	 * <p>
	 * This method will raise Corona "inputDeviceStatus" events if any of the input devices
	 * connection states or configurations have changed.
	 */
	private static void updateCollection() {
		// Do not continue if running on an Android 2.2 or older operating systems.
		// Android does not support access to input devices on these older systems.
		if (android.os.Build.VERSION.SDK_INT < 9) {
			return;
		}
		
		// Create a clone of the main device connection.
		// This allows us to easily iterate it in a thread safe manner below.
		InputDeviceInterfaceCollection deviceCollection = null;
		synchronized (InputDeviceServices.class) {
			deviceCollection = sDeviceCollection.clone();
		}
		if (deviceCollection == null) {
			return;
		}

		// Fetch IDs for all input devices that are currently connected to the system.
		int[] androidDeviceIdArray = ApiLevel9.fetchAndroidDeviceIds();

		// Flag devices as disconnected if their IDs are no longer returned by the system.
		for (InputDeviceInterface deviceInterface : deviceCollection) {
			if (deviceInterface.getDeviceInfo().hasAndroidDeviceId()) {
				boolean wasFound = false;
				for (int androidDeviceId : androidDeviceIdArray) {
					if (deviceInterface.getDeviceInfo().getAndroidDeviceId() == androidDeviceId) {
						wasFound = true;
						break;
					}
				}
				if (wasFound == false) {
					deviceInterface.getContext().beginUpdate();
					deviceInterface.getContext().update(ConnectionState.DISCONNECTED);
					if (deviceInterface.getDeviceInfo().getPlayerNumber() > 0) {
						deviceInterface.getContext().update(
								deviceInterface.getDeviceInfo().cloneWithoutPlayerNumber());
					}
					deviceInterface.getContext().endUpdate();
				}
			}
		}

		// For the IDs that were returned by the system, update the main collection with
		// their newest device information and flag them as connected.
		for (int androidDeviceId : androidDeviceIdArray) {
			updateCollectionWithAndroidDeviceId(androidDeviceId);
		}
	}

	/**
	 * Updates this class' device collection with information retrieved from the specified
	 * Android managed input device. If the device does not exist in this collection, then
	 * it will be added.
	 * <p>
	 * This method will raise Corona "inputDeviceStatus" events if the specified device's
	 * connection state or configuration has changed.
	 * @param id The unique ID assigned to the input device by Android.
	 *           Matches an ID returned by Android's InputDevice.getDeviceIds() method.
	 */
	private static void updateCollectionWithAndroidDeviceId(int id) {
		// Do not continue if running on an Android 2.2 or older operating systems.
		// Android does not support access to input devices on these older systems.
		if (android.os.Build.VERSION.SDK_INT < 9) {
			return;
		}

		// Fetch all device information for the given Android device ID.
		java.util.List<InputDeviceInfo> collection = InputDeviceInfo.collectionFromAndroidDeviceId(id);

		// Update this class' main device collection with the retrieved device information.
		for (InputDeviceInfo deviceInfo : collection) {
			InputDeviceInterface deviceInterface = null;
			InputDeviceContext deviceContext = null;

			// Fetch the device's context from the main collection, if it exists.
			synchronized (InputDeviceServices.class) {
				// First, attempt to fetch it by its Android device ID.
				deviceInterface = sDeviceCollection.getByAndroidDeviceIdAndType(id, deviceInfo.getType());

				// If not found, then attempt to fetch it by its permanent ID and display name.
				// Note: A device that is disconnected and re-connected will typically be assigned a
				//       new Android device ID. Corona should treat this as a configuration change.
				if ((deviceInterface == null) && deviceInfo.hasPermanentStringId()) {
					deviceInterface = sDeviceCollection.getByPermanentStringIdAndTypeAndDisplayName(
												deviceInfo.getPermanentStringId(),
												deviceInfo.getType(), deviceInfo.getDisplayName());
				}

				// If still not found, then attempt to fetch a disconnected device having the same display name.
				// Note: This is needed on Android 4.0 and older systems where Android does not assign devices
				//       a permanent string ID. Devices that are disconnected and re-connected are assigned
				//       new Android device IDs. So, we assume the last disconnected device is the same device.
				if ((deviceInterface == null) && (android.os.Build.VERSION.SDK_INT < 16)) {
					deviceInterface = sDeviceCollection.getBy(ConnectionState.DISCONNECTED).getByTypeAndDisplayName(
												deviceInfo.getType(), deviceInfo.getDisplayName());
				}
			}
			if (deviceInterface != null) {
				deviceContext = deviceInterface.getContext();
			}

			// If the device context was not found, then assume it is a new input device that has just
			// connected to the system. Add it to this class' main collection.
			if (deviceContext == null) {
				deviceContext = add(deviceInfo);
				if (deviceContext == null) {
					continue;
				}
			}

			// Flag that the device is connected and update its configuration.
			// This will raise a Corona "inputDeviceStatus" event in Lua if the device status has changed.
			deviceContext.beginUpdate();
			deviceContext.update(ConnectionState.CONNECTED);
			deviceContext.update(deviceInfo);
			deviceContext.endUpdate();
		}
	}


	/**
	 * Handles events received from the Corona runtime.
	 * <p>
	 * This class is in charge of creating CoronaRuntimeTaskDispatcher objects for every Corona runtime
	 * that gets created. This class is also in charge of stopping/starting device monitoring when the
	 * Corona runtime gets suspended/resumed.
	 * <p>
	 * This class is only expected to be used by the InputDeviceServices class.
	 */
	private static class CoronaRuntimeEventHandler implements com.ansca.corona.CoronaRuntimeListener {
		/** Creates a new Corona runtime event handler. */
		public CoronaRuntimeEventHandler() { }

		/**
		 * Called after the Corona runtime has been created and just before executing the "main.lua" file.
		 * This is the application's opportunity to register custom APIs into Lua.
		 * <p>
		 * Warning! This method is not called on the main thread.
		 * @param runtime Reference to the CoronaRuntime object that has just been loaded/initialized.
		 *                Provides a LuaState object that allows the application to extend the Lua API.
		 */
		public void onLoaded(com.ansca.corona.CoronaRuntime runtime) {
			// Create a new Corona task dispatcher and add it to the hash map.
			synchronized (InputDeviceServices.sTaskDispatcherMap) {
				InputDeviceServices.sTaskDispatcherMap.put(
						runtime, new com.ansca.corona.CoronaRuntimeTaskDispatcher(runtime));
			}
		}
		
		/**
		 * Called just after the Corona runtime has executed the "main.lua" file.
		 * <p>
		 * Warning! This method is not called on the main thread.
		 * @param runtime Reference to the CoronaRuntime object that has just been started.
		 */
		public void onStarted(com.ansca.corona.CoronaRuntime runtime) {
			InputDeviceServices.sDeviceMonitor.start();
		}
		
		/**
		 * Called just after the Corona runtime has been suspended which pauses all rendering, audio, timers,
		 * and other Corona related operations. This can happen when another Android activity (ie: window) has
		 * been displayed, when the screen has been powered off, or when the screen lock is shown.
		 * <p>
		 * Warning! This method is not called on the main thread.
		 * @param runtime Reference to the CoronaRuntime object that has just been suspended.
		 */
		public void onSuspended(com.ansca.corona.CoronaRuntime runtime) {
			InputDeviceServices.sDeviceMonitor.stop();
		}
		
		/**
		 * Called just after the Corona runtime has been resumed after a suspend.
		 * <p>
		 * Warning! This method is not called on the main thread.
		 * @param runtime Reference to the CoronaRuntime object that has just been resumed.
		 */
		public void onResumed(com.ansca.corona.CoronaRuntime runtime) {
			InputDeviceServices.sDeviceMonitor.start();
		}
		
		/**
		 * Called just before the Corona runtime terminates.
		 * <p>
		 * This happens when the Corona activity is being destroyed which happens when the user presses the Back button
		 * on the activity, when the native.requestExit() method is called in Lua, or when the activity's finish()
		 * method is called. This does not mean that the application is exiting.
		 * <p>
		 * Warning! This method is not called on the main thread.
		 * @param runtime Reference to the CoronaRuntime object that is being terminated.
		 */
		public void onExiting(com.ansca.corona.CoronaRuntime runtime) {
			synchronized (InputDeviceServices.sTaskDispatcherMap) {
				InputDeviceServices.sTaskDispatcherMap.remove(runtime);
			}
			InputDeviceServices.sDeviceMonitor.stop();
		}
	}


	/**
	 * Handles events received from an InputDeviceMonitor.
	 * <p>
	 * Detects when Android managed input devices have been connected, disconnected, and reconfigured.
	 * Will add newly connected input devices to the InputDeviceServices class' main collection
	 * and raise the received events as Corona "inputDeviceStatus" events to be received by Lua listeners.
	 * <p>
	 * This class is only expected to be used by the InputDeviceServices class.
	 */
	private static class InputDeviceMonitorEventHandler implements InputDeviceMonitor.Listener {
		/** Creates a new handler for an InputDeviceMonitor object's events. */
		public InputDeviceMonitorEventHandler() { }

		/**
		 * Called when a new device has been connected to the system or when an existing device
		 * has been re-connected to the system.
		 * @param androidDeviceId Unique integer ID assigned to the device by Android.
		 *                        Expected to be given to the Android InputDevice.getDevice(id) method.
		 */
		public void onInputDeviceConnected(int androidDeviceId) {
			InputDeviceServices.updateCollectionWithAndroidDeviceId(androidDeviceId);
		}

		/**
		 * Called when an existing device has been disconnected.
		 * @param androidDeviceId Unique integer ID assigned to the device by Android.
		 *                        Matches the ID returned by the Android InputDevice.getId() method.
		 */
		public void onInputDeviceDisconnected(int androidDeviceId) {
			// Create a clone of the main device connection.
			InputDeviceInterfaceCollection deviceCollection = null;
			synchronized (InputDeviceServices.class) {
				deviceCollection = InputDeviceServices.sDeviceCollection.clone();
			}
			if (deviceCollection == null) {
				return;
			}

			// Update the connection state for all devices having the given Android device ID.
			// This will raise a Corona "inputDeviceStatus" event in Lua if the state has changed.
			for (InputDeviceInterface deviceInterface : deviceCollection) {
				if (deviceInterface.getDeviceInfo().getAndroidDeviceId() == androidDeviceId) {
					deviceInterface.getContext().beginUpdate();
					deviceInterface.getContext().update(ConnectionState.DISCONNECTED);
					if (deviceInterface.getDeviceInfo().getPlayerNumber() > 0) {
						deviceInterface.getContext().update(
								deviceInterface.getDeviceInfo().cloneWithoutPlayerNumber());
					}
					deviceInterface.getContext().endUpdate();
				}
			}
		}
		
		/**
		 * Called when an existing device has been reconfigured,
		 * such as when axis inputs have been added or removed.
		 * @param androidDeviceId Unique integer ID assigned to the device by Android.
		 *                        Expected to be given to the Android InputDevice.getDevice(id) method.
		 */
		public void onInputDeviceReconfigured(int androidDeviceId) {
			InputDeviceServices.updateCollectionWithAndroidDeviceId(androidDeviceId);
		}
	}


	/**
	 * Handles events received from an InputDeviceContext object such as device status changes,
	 * received axis data, and vibration/rumble feedback requests.
	 * <p>
	 * This class is only expected to be used by the InputDeviceServices class.
	 */
	private static class InputDeviceContextEventHandler
		implements InputDeviceContext.OnStatusChangedListener,
		           InputDeviceContext.OnAxisDataReceivedListener,
		           InputDeviceContext.VibrateRequestHandler
	{
		/** Pre-allocated instance of this class. */
		public static InputDeviceContextEventHandler INSTANCE = new InputDeviceContextEventHandler();


		/** Creates a new device context event handler. */
		public InputDeviceContextEventHandler() { }

		/**
		 * Called when an input device's status has been changed, such as from connected to disconnected
		 * or if the device has been reconfigured.
		 * @param context Reference to the input device that has had its status changed.
		 * @param eventInfo Provides information about what has changed.
		 */
		public void onStatusChanged(InputDeviceContext context, InputDeviceStatusEventInfo eventInfo) {
			// Validate.
			if ((context == null) || (eventInfo == null)) {
				return;
			}

			// Do not raise an event if the Corona runtime is currently suspended.
			// The input device monitor will be disabled while suspended.
			if (InputDeviceServices.sDeviceMonitor.isRunning() == false) {
				return;
			}

			// Fetch the context's device interface.
			InputDeviceInterface deviceInterface = null;
			synchronized (InputDeviceServices.class) {
				deviceInterface = sDeviceCollection.getByCoronaDeviceId(context.getCoronaDeviceId());
			}
			if (deviceInterface == null) {
				return;
			}

			// Create a task that will raise an "inputDeviceStatus" event to be received by a Lua listener.
			RaiseInputDeviceStatusChangedEventTask task;
			task = new RaiseInputDeviceStatusChangedEventTask(deviceInterface, eventInfo);

			// Send the above task to all Corona runtimes.
			synchronized (InputDeviceServices.sTaskDispatcherMap) {
				for (com.ansca.corona.CoronaRuntimeTaskDispatcher taskDispatcher :
				     InputDeviceServices.sTaskDispatcherMap.values())
				{
					taskDispatcher.send(task);
				}
			}
		}

		/**
		 * Called when an input device has received new axis data.
		 * @param context Reference to the input device that has received new axis data.
		 * @param eventInfo Provides the received data and information about its axis.
		 */
		public void onAxisDataReceived(InputDeviceContext context, AxisDataEventInfo eventInfo) {
			// Validate.
			if ((context == null) || (eventInfo == null)) {
				return;
			}

			// Do not raise an event if the Corona runtime is currently suspended.
			// The input device monitor will be disabled while suspended.
			if (InputDeviceServices.sDeviceMonitor.isRunning() == false) {
				return;
			}

			// Fetch the context's device interface.
			InputDeviceInterface deviceInterface = null;
			synchronized (InputDeviceServices.class) {
				deviceInterface = sDeviceCollection.getByCoronaDeviceId(context.getCoronaDeviceId());
			}
			if (deviceInterface == null) {
				return;
			}

			// Create a task that will raise an "axis" event to be received by a Lua listener.
			RaiseAxisEventTask task = new RaiseAxisEventTask(deviceInterface, eventInfo);

			// Send the above task to all Corona runtimes.
			synchronized (InputDeviceServices.sTaskDispatcherMap) {
				for (com.ansca.corona.CoronaRuntimeTaskDispatcher taskDispatcher :
				     InputDeviceServices.sTaskDispatcherMap.values())
				{
					taskDispatcher.send(task);
				}
			}
		}

		/**
		 * Called when an input device's vibrate() method has been called.
		 * <p>
		 * This signals the handler to vibrate/rumble the requested device.
		 * @param context Reference to the input device that has been requested to be vibrated.
		 * @param settings Provides instructions on how to vibrate the input device.
		 */
		public void onHandleVibrateRequest(InputDeviceContext context, VibrationSettings settings) {
			// Validate.
			if (context == null) {
				return;
			}

			// Vibrate the input device, if supported.
			if ((android.os.Build.VERSION.SDK_INT >= 16) && context.getDeviceInfo().hasAndroidDeviceId()) {
				ApiLevel16.vibrateInputDeviceUsing(context.getDeviceInfo().getAndroidDeviceId(), settings);
			}
		}
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
		 * Fetches the Android device IDs for all connected input devices managed by Android.
		 * <p>
		 * This is done by calling the Android InputDevice.getDeviceIds() method.
		 * @return Returns an array of Android device IDs that are currently connected to the system.
		 *         <p>
		 *         Returns an empty array if their are no input devices available.
		 */
		public static int[] fetchAndroidDeviceIds() {
			int[] deviceIds = android.view.InputDevice.getDeviceIds();
			if (deviceIds == null) {
				deviceIds = new int[] {};
			}
			return deviceIds;
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
		 * Vibrates an Android managed input devices.
		 * @param androidDeviceId Unique integer ID of the Android InputDevice object to vibrate.
		 * @param settings Provides instructions on how to vibrate the input device.
		 */
		public static void vibrateInputDeviceUsing(int androidDeviceId, VibrationSettings settings) {
			// Fetch the input device.
			android.view.InputDevice inputDevice = android.view.InputDevice.getDevice(androidDeviceId);
			if (inputDevice == null) {
				return;
			}

			// Vibrate the controller, if possible.
			android.os.Vibrator vibrator = inputDevice.getVibrator();
			if ((vibrator != null) && vibrator.hasVibrator()) {
				vibrator.vibrate(100);
			}
		}
	}
}
