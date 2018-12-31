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
 * Stores information about one input device.
 * <p>
 * Instances of this class are immutable.
 * <p>
 * Instances of this class can only be created via the static from() method.
 */
public class InputDeviceInfo implements Cloneable {
	/** The type of input device this is such as KEYBOARD, MOUSE, JOYSTICK, etc. */
	private InputDeviceType fDeviceType;

	/** The device's unique integer ID assigned by Android. */
	private int fAndroidDeviceId;

	/** The device's unique string ID that persists after rebooting the Android device. */
	private String fPermanentStringId;

	/** The input device's product name as assigned by the manufacturer. */
	private String fProductName;

	/** The device's alias/display name. Typically set via Android's "Bluetooth" settings activity. */
	private String fDisplayName;

	/** Set to true if the device supports vibration/rumble. */
	private boolean fCanVibrate;

	/** The player/controller number assigned to the device. Set to zero if not assigned a player number. */
	private int fPlayerNumber;

	/** Stores a collection of input sources this device gets input events from. */
	private InputDeviceTypeSet fInputSources;

	/** Read-only collection which wraps the fInputSources member variable. */
	private ReadOnlyInputDeviceTypeSet fReadOnlyInputSources;

	/** Stores a collection of axis inputs. */
	private AxisInfoCollection fAxisCollection;

	/** Read-only collection which wraps the fAxisCollection member variable. */
	private ReadOnlyAxisInfoCollection fReadOnlyAxisCollection;


	/** Creates a new input device information object. */
	private InputDeviceInfo() {
		fDeviceType = InputDeviceType.UNKNOWN;
		fAndroidDeviceId = -1;
		fPermanentStringId = null;
		fProductName = "";
		fDisplayName = "";
		fCanVibrate = false;
		fPlayerNumber = 0;
		fInputSources = new InputDeviceTypeSet();
		fReadOnlyInputSources = new ReadOnlyInputDeviceTypeSet(fInputSources);
		fAxisCollection = new AxisInfoCollection();
		fReadOnlyAxisCollection = new ReadOnlyAxisInfoCollection(fAxisCollection);
	}

	/**
	 * Returns a copy of this object.
	 * <br>
	 * Since this object is immutable, this will return this object's reference.
	 * This clone() method is overriden so that a copy is not unnecessarily created.
	 * @return Returns this object's reference since it is immutable.
	 */
	@Override
	public InputDeviceInfo clone() {
		return this;
	}

	/**
	 * Creates a copy of this input device information, but with the player number set to zero.
	 * @return Returns a new device information object with the player number set to zero.
	 *         <br>
	 *         Will return this object's reference if this object's player number is already zero.
	 */
	public InputDeviceInfo cloneWithoutPlayerNumber() {
		InputDeviceInfo clone = this;
		if (hasPlayerNumber()) {
			try {
				clone = (InputDeviceInfo)super.clone();
				clone.fPlayerNumber = 0;
			}
			catch (Exception ex) { }
		}
		return clone;
	}

	/**
	 * Gets the kind of input device this info object represents.
	 * @return Returns the type of input device such as "KEYBOARD", "MOUSE", "JOYSTICK", etc.
	 */
	public InputDeviceType getType() {
		return fDeviceType;
	}

	/**
	 * Indicates if this input device has a unique integer ID assigned by the Android OS.
	 * If so, then this indicates that the getAndroidDeviceId() method will return a valid value
	 * matching an Android managed InputDevice object.
	 * @return Returns true if the getAndroidDeviceId() will return a valid Android assigned device ID.
	 *         <p>
	 *         Returns false if this device is not managed by Android and is instead a custom device
	 *         that was added to Corona.
	 */
	public boolean hasAndroidDeviceId() {
		return (fAndroidDeviceId >= 0);
	}

	/**
	 * Gets the unique integer ID assigned to this device by the Android OS.
	 * @return Returns a unique integer ID assigned to the device by the Android OS.
	 *         <p>
	 *         Returns -1 if this device does not assigned an ID by Android, which indicates that
	 *         this is a non-HID protocol device that was added to Corona.
	 */
	public int getAndroidDeviceId() {
		return fAndroidDeviceId;
	}

	/**
	 * Determines if this input device has a permanent string ID assigned to it.
	 * <p>
	 * This is an ID which remains the same after rebooting the Android device and typically
	 * comes from Android's InputDevice.getDescriptor() method. However, that method is only
	 * supported on Android 4.1 (API Level 16).
	 * @return Returns true if this device has a valid permanent ID assigned to it, meaning that
	 *         the getPermanentStringId() method will return a valid value.
	 *         <p>
	 *         Returns false if this device does not have a permanent ID.
	 */
	public boolean hasPermanentStringId() {
		return ((fPermanentStringId != null) && (fPermanentStringId.length() > 0));
	}

	/**
	 * Gets the device's unique string ID which persists after rebooting the Android device.
	 * This ID typically comes from Android's InputDevice.getDescriptor() method.
	 * @return Returns the device's unique string ID.
	 *         <p>
	 *         Returns null if this device does not have a permanent ID.
	 */
	public String getPermanentStringId() {
		return fPermanentStringId;
	}

	/**
	 * Gets the device's product name as set by the manufacturer.
	 * @return Returns the device's product name.
	 */
	public String getProductName() {
		return fProductName;
	}

	/**
	 * Gets the device's alias/display name, which typically comes from Android's InputDevice.getName() method.
	 * <p>
	 * This name is typically the product name unless the end-user changes it via the Bluetooth
	 * settings screen on the Android device.
	 * @return Returns the device's display name.
	 */
	public String getDisplayName() {
		return fDisplayName;
	}

	/**
	 * Determines if the device supports vibration/rumble feedback.
	 * @return Returns true if the device can be vibrated. Returns false if not.
	 */
	public boolean canVibrate() {
		return fCanVibrate;
	}

	/**
	 * Determines if a player/controller number is assigned to the device.
	 * This is an Xbox style player number light on an Android gamepad.
	 * @return Returns true if a player/controller number is assigned to the device.
	 *         This means that the getPlayerNumber() method will return a valid 1-based value.
	 *         <p>
	 *         Returns false if a player number was not assigned to the device by the system.
	 */
	public boolean hasPlayerNumber() {
		return (fPlayerNumber > 0);
	}

	/**
	 * Gets the player/controller number assigned to the device by the system.
	 * This is an Xbox style player number light on an Android gamepad.
	 * @return Returns a 1-based value that is the player/controller number assigned to the device.
	 *         <p>
	 *         Returns zero if a player number was not assigned to the device by the system.
	 */
	public int getPlayerNumber() {
		return fPlayerNumber;
	}

	/**
	 * Gets a collection of input sources that this device will receive input events from
	 * such as a JOYSTICK, GAMEPAD, TOUCHPAD, etc.
	 * <p>
	 * This is only useful if this input device represents a managed Android device, because
	 * the returned collection would mostly match what is returned by the Android
	 * InputDevice.getSources() method.
	 * @return Returns a collection of input source types.
	 */
	public ReadOnlyInputDeviceTypeSet getInputSources() {
		return fReadOnlyInputSources;
	}

	/**
	 * Gets a collection of all axis inputs this device has.
	 * @return Returns a read-only collection of axis input configurations.
	 *         <p>
	 *         The returned collection will be empty if this device does not have any axis inputs.
	 */
	public ReadOnlyAxisInfoCollection getAxes() {
		return fReadOnlyAxisCollection;
	}

	/**
	 * Determines if the given device information exactly matches this object's device information.
	 * @param deviceInfo Reference to the device information to be compared with this object.
	 * @return Returns true if the given device information exactly matches this object's information.
	 *         <p>
	 *         Returns false if given a null argument or if the given device information does not
	 *         match this object's device information.
	 */
	public boolean equals(InputDeviceInfo deviceInfo) {
		// Not equal if given a null reference.
		if (deviceInfo == null) {
			return false;
		}

		// Compare permanent string IDs.
		if (((fPermanentStringId == null) && (deviceInfo.fPermanentStringId != null)) ||
		    ((fPermanentStringId != null) && (deviceInfo.fPermanentStringId == null)))
		{
			return false;
		}
		else if ((fPermanentStringId != null) &&
		         (fPermanentStringId.equals(deviceInfo.fPermanentStringId) == false))
		{
			return false;
		}

		// Compare all other member variables.
		if ((deviceInfo.fDeviceType != fDeviceType) ||
		    (deviceInfo.fAndroidDeviceId != fAndroidDeviceId) ||
		    (deviceInfo.fProductName.equals(fProductName) == false) ||
		    (deviceInfo.fDisplayName.equals(fDisplayName) == false) ||
		    (deviceInfo.fCanVibrate != fCanVibrate) ||
		    (deviceInfo.fPlayerNumber != fPlayerNumber) ||
		    (deviceInfo.fInputSources.equals(fInputSources) == false))
		{
			return false;
		}

		// Compare axis collections.
		if (deviceInfo.fAxisCollection.size() != fAxisCollection.size()) {
			return false;
		}
		for (int index = 0; index < fAxisCollection.size(); index++) {
			AxisInfo axisInfo1 = deviceInfo.fAxisCollection.getByIndex(index);
			AxisInfo axisInfo2 = fAxisCollection.getByIndex(index);
			if (axisInfo1.equals(axisInfo2) == false) {
				return false;
			}
		}

		// The given device information matches this information.
		return true;
	}

	/**
	 * Determines if the given object matches this object.
	 * @param value Reference to the object to compare against this object.
	 * @return Returns true if the given object is of type InputDeviceInfo and its information
	 *         exactly matches this object's device information.
	 *         <p>
	 *         Returns false if given a null reference, an object that is not of type InputDeviceInfo,
	 *         or if the device information given does not match this object's device information.
	 */
	@Override
	public boolean equals(Object value) {
		if ((value instanceof InputDeviceInfo) == false) {
			return false;
		}
		return equals((InputDeviceInfo)value);
	}

	/**
	 * Creates a new immutable InputDeviceInfo object using the information copied from the given settings.
	 * @param settings The device settings to be copied by the new device info object.
	 * @return Returns a new input device object with the given settings.
	 *         <p>
	 *         Returns null if given a null argument.
	 */
	public static InputDeviceInfo from(InputDeviceSettings settings) {
		// Validate.
		if (settings == null) {
			return null;
		}

		// Create a new device info object and copy the given device settings to it.
		InputDeviceInfo deviceInfo = new InputDeviceInfo();
		deviceInfo.fDeviceType = settings.getType();
		deviceInfo.fInputSources.add(deviceInfo.fDeviceType);
		deviceInfo.fPermanentStringId = settings.getPermanentStringId();
		deviceInfo.fProductName = settings.getProductName();
		deviceInfo.fDisplayName = settings.getDisplayName();
		for (AxisSettings axisSettings : settings.getAxes()) {
			deviceInfo.fAxisCollection.add(AxisInfo.from(axisSettings));
		}

		// Return the new device info object.
		return deviceInfo;
	}

	/**
	 * Creates a collection of immutable InputDeviceInfo objects from one Android InputDevice object.
	 * <p>
	 * Note that an InputDeviceInfo object will be created for every input source
	 * (ie: keyboard, mouse, gamepad, etc.) found in the one Android InputDevice object.
	 * This makes input device handling simpler in Corona and matches Windows' DirectInput
	 * handling of hybrid devices.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 * @param id The unique integer ID assigned to an Android InputDevice object.
	 * @return Returns a collection of InputDeviceInfo objects providing a copy of the information
	 *         stored in Android's InputDevice object.
	 *         <p>
	 *         Returns an empty collection if an input device for the given ID was not found.
	 */
	static java.util.List<InputDeviceInfo> collectionFromAndroidDeviceId(int id) {
		java.util.List<InputDeviceInfo> collection;

		if (android.os.Build.VERSION.SDK_INT >= 9) {
			collection = ApiLevel9.fetchDeviceInfoFromAndroidDeviceId(id);
		}
		else {
			collection = new java.util.ArrayList<InputDeviceInfo>();
		}
		return collection;
	}

	/**
	 * Creates a collection of immutable InputDeviceInfo objects for all Android  managed input devices
	 * currently connected and available to the system.
	 * <p>
	 * Note that an InputDeviceInfo object will be created for every input source
	 * (ie: keyboard, mouse, gamepad, etc.) found in the one Android InputDevice object.
	 * This makes input device handling simpler in Corona and matches Windows' DirectInput
	 * handling of hybrid devices.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 * @return Returns a collection of InputDeviceInfo objects providing a copy of the information
	 *         stored in each Android InputDevice object.
	 *         <p>
	 *         Returns an empty collection if there are no input devices currenty available.
	 */
	static java.util.List<InputDeviceInfo> collectionFromAllAndroidDevices() {
		java.util.ArrayList<InputDeviceInfo> collection = new java.util.ArrayList<InputDeviceInfo>();
		if (android.os.Build.VERSION.SDK_INT >= 9) {
			for (int androidDeviceId : ApiLevel9.fetchAndroidDeviceIds()) {
				collection.addAll(ApiLevel9.fetchDeviceInfoFromAndroidDeviceId(androidDeviceId));
			}
		}
		return collection;
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

		/**
		 * Fetches a collection of InputDeviceInfo objects containing a copy of information
		 * from one Android InputDevice object having the given ID.
		 * @param id The unique integer ID assigned to an Android InputDevice object.
		 * @return Returns a collection of InputDeviceInfo objects for the given ID.
		 *         <p>
		 *         Returns an empty collection if an input device for the given ID was not found.
		 */
		public static java.util.List<InputDeviceInfo> fetchDeviceInfoFromAndroidDeviceId(int id) {
			// Create a collection to store the fetched devices in.
			java.util.ArrayList<InputDeviceInfo> collection;
			collection = new java.util.ArrayList<InputDeviceInfo>();

			// Do not continue if given an invalid ID.
			// An ID of 0 or -1 is typically assigned to key events when coming from the virtual keyboard.
			if (id <= 0) {
				return collection;
			}

			// Fetch the requested input device from Android.
			android.view.InputDevice inputDevice = android.view.InputDevice.getDevice(id);
			if (inputDevice == null) {
				return collection;
			}

			// Copy the input device's information to new InputDeviceInfo objects.
			// Will create a InputDeviceInfo object for every axis input source,
			// like how it works with DirectInput on Windows.
			int sources = inputDevice.getSources();
			if (sources != 0) {
				// Fetch all input sources from the input device.
				InputDeviceTypeSet primaryInputSources =
							InputDeviceType.collectionFromAndroidSourcesBitField(sources);

				// Fetch all axis input sources from the input device.
				// This should be a subset of the primary input sources above.
				// Note: Axis input is only supported on API Level 12 or higher systems.
				InputDeviceTypeSet remainingAxisInputSources = null;
				if (android.os.Build.VERSION.SDK_INT >= 12) {
					remainingAxisInputSources = ApiLevel12.getAxisSourcesFrom(inputDevice);
					primaryInputSources.addAll(remainingAxisInputSources);
				}
				else {
					remainingAxisInputSources = new InputDeviceTypeSet();
				}

				// Fetch the primary input source from the device.
				// This source will be used as the device type for one Corona device object made up of
				// multiple Android BUTTON class input sources, but will only support on axis input source.
				InputDeviceType primaryDeviceType = null;
				if (remainingAxisInputSources.size() > 1) {
					// Multiple axis input sources exist. Select the primary source as the device type.
					int axisSourcesBitField = remainingAxisInputSources.toAndroidSourcesBitField();
					primaryDeviceType = InputDeviceType.fromAndroidSourceId(axisSourcesBitField);
					remainingAxisInputSources.remove(primaryDeviceType);
				}
				else if (remainingAxisInputSources.size() == 1) {
					// Only one axis input source exists. Use it as the primary device type.
					primaryDeviceType = remainingAxisInputSources.iterator().next();
					remainingAxisInputSources.clear();
				}
				if (primaryDeviceType == null) {
					// Device does not have any axis inputs.
					// Select the primary input source out of all sources as the device type.
					primaryDeviceType = InputDeviceType.fromAndroidSourceId(sources);
				}
				primaryInputSources.removeAll(remainingAxisInputSources);

				// Create a Corona device info object for a keyboard if the input device has alpha-numeric keys.
				// Note: Gamepads and joysticks are considered keyboards as well, but are not alpha-numeric.
				//       For those devices, Corona should not call them keyboards to match other platforms.
				if (primaryInputSources.contains(InputDeviceType.KEYBOARD) &&
				    (inputDevice.getKeyboardType() == android.view.InputDevice.KEYBOARD_TYPE_ALPHABETIC))
				{
					primaryInputSources.remove(InputDeviceType.KEYBOARD);
					collection.add(createDeviceInfoFor(inputDevice, InputDeviceType.KEYBOARD));
				}

				// Create a Corona device info object for the primary input source.
				InputDeviceInfo deviceInfo = createDeviceInfoFor(inputDevice, primaryDeviceType);
				deviceInfo.fInputSources.addAll(primaryInputSources);
				if (android.os.Build.VERSION.SDK_INT >= 12) {
					for (InputDeviceType axisInputSource : primaryInputSources) {
						deviceInfo.fAxisCollection.addAll(
								ApiLevel12.getAxisInfoFrom(inputDevice, axisInputSource));
					}
				}
				collection.add(deviceInfo);

				// Create a Corona device info object for all remaining axis input sources.
				for (InputDeviceType axisInputSource : remainingAxisInputSources) {
					deviceInfo = createDeviceInfoFor(inputDevice, axisInputSource);
					if (android.os.Build.VERSION.SDK_INT >= 12) {
						deviceInfo.fAxisCollection.addAll(
								ApiLevel12.getAxisInfoFrom(inputDevice, axisInputSource));
					}
					collection.add(deviceInfo);
				}
			}
			else {
				// An input source of zero indicates that the device type is unkown.
				// We can only create one input device info object in this case.
				InputDeviceInfo deviceInfo = createDeviceInfoFor(inputDevice, InputDeviceType.UNKNOWN);
				collection.add(deviceInfo);
			}

			// Return the fetched device information.
			return collection;
		}

		/**
		 * Creates one InputDeviceInfo object for the given Android InputDevice object.
		 * @param inputDevice Reference to the Android input device to copy information from.
		 * @param deviceType The input source to copy information from such as KEYBOARD, MOUSE, etc.
		 * @return Returns device info copied from the given Android input device object.
		 *         <p>
		 *         Returns null if given null arguments.
		 */
		private static InputDeviceInfo createDeviceInfoFor(
			android.view.InputDevice inputDevice, InputDeviceType deviceType)
		{
			// Validate.
			if ((inputDevice == null) || (deviceType == null)) {
				return null;
			}

			// Create a new device info object and copy Android's input device infomration to it.
			InputDeviceInfo deviceInfo = new InputDeviceInfo();
			deviceInfo.fAndroidDeviceId = inputDevice.getId();
			deviceInfo.fDeviceType = deviceType;
			deviceInfo.fInputSources.add(deviceType);
			deviceInfo.fProductName = inputDevice.getName();
			deviceInfo.fDisplayName = inputDevice.getName();
			if (android.os.Build.VERSION.SDK_INT >= 16) {
				deviceInfo.fPermanentStringId = ApiLevel16.getPermanentStringIdFrom(inputDevice);
				deviceInfo.fCanVibrate = ApiLevel16.isVibrationSupportedFor(inputDevice);
			}
			if (android.os.Build.VERSION.SDK_INT >= 19) {
				deviceInfo.fPlayerNumber = ApiLevel19.getControllerNumberFrom(inputDevice);
			}

			// Return the new device info object.
			return deviceInfo;
		}
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
		 * Fetches all axis input sources from the given input device.
		 * <p>
		 * This is done by traversing all motion ranges for the given input device by calling
		 * its InputDevice.getMotionRanges() method.
		 * @param inputDevice Reference to the input device to fetch axis sources from.
		 * @return Returns a collection of axis input sources retrieved from the given device.
		 *         <p>
		 *         Returns an empty collection if the given device does not have any axis input sources
		 *         or if the argument is null.
		 */
		public static InputDeviceTypeSet getAxisSourcesFrom(android.view.InputDevice inputDevice) {
			InputDeviceTypeSet sourceSet = new InputDeviceTypeSet();

			if (inputDevice != null) {
				for (android.view.InputDevice.MotionRange motionRange : inputDevice.getMotionRanges()) {
					sourceSet.add(InputDeviceType.fromAndroidSourceId(motionRange.getSource()));
				}
			}
			return sourceSet;
		}

		/**
		 * Fetches all axis input information from given input device and input source.
		 * @param inputDevice Reference to the input device to fetch axis information from.
		 * @param inputSource Type of input source to fetch axis information from such as JOYSTICK,
		 *                    MOUSE, TOUCHSCREEN, TOUCHPAD, etc.
		 * @return Returns a collection of axis input information retrieved from the given input device.
		 *         <p>
		 *         Returns an empty collection if not axis inputs were found or if given null arguments.
		 */
		public static AxisInfoCollection getAxisInfoFrom(
			android.view.InputDevice inputDevice, InputDeviceType inputSource)
		{
			AxisInfoCollection axisCollection = new AxisInfoCollection();

			if ((inputDevice != null) && (inputSource != null)) {
				AxisSettings axisSettings = new AxisSettings();
				for (android.view.InputDevice.MotionRange motionRange : inputDevice.getMotionRanges()) {
					if (motionRange.getSource() == inputSource.toAndroidSourceId()) {
						// Fetch the axis type.
						AxisType axisType = AxisType.fromAndroidIntegerId(motionRange.getAxis());

						// Determine if the axis provides absolute or relative values.
						boolean isProvidingAbsoluteValues = true;
						if ((axisType == AxisType.VERTICAL_SCROLL) || (axisType == AxisType.HORIZONTAL_SCROLL)) {
							isProvidingAbsoluteValues = false;
						}
						else if (inputSource == InputDeviceType.TRACKBALL) {
							if ((axisType == AxisType.X) || (axisType == AxisType.Y)) {
								isProvidingAbsoluteValues = false;
							}
						}

						// Add the axis' settings to the collection.
						axisSettings.setType(AxisType.fromAndroidIntegerId(motionRange.getAxis()));
						axisSettings.setMinValue(motionRange.getMin());
						axisSettings.setMaxValue(motionRange.getMax());
						axisSettings.setAccuracy(motionRange.getFuzz());
						axisSettings.setIsProvidingAbsoluteValues(isProvidingAbsoluteValues);
						axisCollection.add(AxisInfo.from(axisSettings));
					}
				}
			}
			return axisCollection;
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
		 * Fetches the input device's permanent string ID.
		 * <p>
		 * This ID is returned by the InputDevice.getDescriptor() method.
		 * @param inputDevice Reference to the input device to fetch the ID from.
		 * @return Returns the input device's permanent string ID.
		 *         <p>
		 *         Returns null if the input device does not have a permanent string ID
		 *         or if given a null argument.
		 */
		public static String getPermanentStringIdFrom(android.view.InputDevice inputDevice) {
			// Fetch the given input device's permanent ID.
			String stringId = null;
			if (inputDevice != null) {
				stringId = inputDevice.getDescriptor();
			}

			// If the string ID is empty, then make it null.
			if ((stringId != null) && (stringId.length() <= 0)) {
				stringId = null;
			}

			// Return the requested ID.
			return stringId;
		}

		/**
		 * Determines if the given input device support vibration/rumble feedback.
		 * @param inputDevice Reference the input device to check.
		 * @return Returns true if the given input device supports vibration/rumble feedback.
		 *         <p>
		 *         Returns false if the input device does not or if given a null argument.
		 */
		public static boolean isVibrationSupportedFor(android.view.InputDevice inputDevice) {
			if (inputDevice != null) {
				android.os.Vibrator vibrator = inputDevice.getVibrator();
				if (vibrator != null) {
					return vibrator.hasVibrator();
				}
			}
			return false;
		}
	}

	/**
	 * Provides access to API Level 19 (Android 4.4 KitKat) features.
	 * Should only be accessed if running on an operating system matching this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * You are expected to call its static methods instead.
	 */
	private static class ApiLevel19 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel19() { }

		/**
		 * Fetches the controller number (aka: player number) assigned to the input device by the system.
		 * This is an Xbox style player number light on an Android game controller.
		 * @param inputDevice Reference to the input to fetch the controller number from.
		 * @return Returns a 1-based number indicating the controller number assigned to the device.
		 *         <p>
		 *         Returns zero if the device is not assigned a controller number of if given null.
		 */
		public static int getControllerNumberFrom(android.view.InputDevice inputDevice) {
			if (inputDevice == null) {
				return 0;
			}
			return inputDevice.getControllerNumber();
		}
	}
}
