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
 * Indicates the type of device that input events come from such as a keyboard, touchscreen, gamepad, etc.
 * <p>
 * Instances of this class are immutable.
 * <p>
 * You cannot create instances of this class.
 * Instead, you use the pre-allocated objects from this class' static methods and fields.
 */
public class InputDeviceType {
	/** Unique integer ID matching a SOURCE constant in the the Android "InputDevice" class. */
	private int fAndroidSourceId;

	/** Corona's unique integer ID matching a constant in the C++ "InputDeviceType" class. */
	private int fCoronaIntegerId;

	/** Corona's unique string ID matching a constant in the C++ "InputDeviceType" class. */
	private String fCoronaStringId;

	/** Indicates that the device is of an unknown type. */
	public static final InputDeviceType UNKNOWN = new InputDeviceType(0, 0, "unknown");

	/** Indicates that the device is a keyboard and will provide key events. */
	public static final InputDeviceType KEYBOARD = new InputDeviceType(257, 1, "keyboard");

	/** Indicates that the device is a mouse and will provide mouse motion events. */
	public static final InputDeviceType MOUSE = new InputDeviceType(8194, 2, "mouse");

	/** Indicates that the device is a stylus and will provide touch motion events. */
	public static final InputDeviceType STYLUS = new InputDeviceType(16386, 3, "stylus");

	/** Indicates that the device is a trackball and will provide scroll motion events. */
	public static final InputDeviceType TRACKBALL = new InputDeviceType(65540, 4, "trackball");

	/**
	 * Indicates that the device is a touchpad and will provide analog motion events.
	 * <p>
	 * Note that the touch coordinates provided will not be in screen coordinates.
	 * An example of this kind of device is the touch surface at the top of an Apple Magic Mouse.
	 */
	public static final InputDeviceType TOUCHPAD = new InputDeviceType(1048584, 5, "touchpad");

	/** Indicates that the device is a touchscreen and will provide touch motion events. */
	public static final InputDeviceType TOUCHSCREEN = new InputDeviceType(4098, 6, "touchscreen");

	/** Indicates that the device is a joystick and will provide analog motion and key events. */
	public static final InputDeviceType JOYSTICK = new InputDeviceType(16777232, 7, "joystick");

	/** Indicates that the device is a gamepad and will provide analog motion and key events. */
	public static final InputDeviceType GAMEPAD = new InputDeviceType(1025, 8, "gamepad");

	/** Indicates that the device is a direction pad and will provide key events. */
	public static final InputDeviceType DPAD = new InputDeviceType(513, 9, "directionalPad");

	/** Stores a collection of all known input device types. */
	private static ReadOnlyInputDeviceTypeSet sTypeCollection = null;


	/** Static constructor. */
	static {
		// Create a read-only collection of all pre-allocated device type via reflection.
		InputDeviceTypeSet collection = new InputDeviceTypeSet();
		try {
			for (java.lang.reflect.Field field : InputDeviceType.class.getDeclaredFields()) {
				if (field.getType().equals(InputDeviceType.class)) {
					InputDeviceType deviceType = (InputDeviceType)field.get(null);
					if (deviceType != null) {
						collection.add(deviceType);
					}
				}
			}
		}
		catch (Exception ex) { }
		sTypeCollection = new ReadOnlyInputDeviceTypeSet(collection);
	}

	/**
	 * Creates a new input device type.
	 * @param androidSourceId Unique integer ID matching a SOURCE constant in Android class "InputDevice".
	 * @param coronaIntegerId Corona's unique integer ID for the input device.
	 * @param coronaStringId Corona's unique string ID for the input device.
	 */
	private InputDeviceType(int androidSourceId, int coronaIntegerId, String coronaStringId) {
		fAndroidSourceId = androidSourceId;
		fCoronaIntegerId = coronaIntegerId;
		fCoronaStringId = coronaStringId;
	}
	
	/**
	 * Gets the unique integer ID as defined by one of the SOURCE constants in Android class InputDevice.
	 * @return Returns a unique integer ID matching a SOURCE constant in Android class InputDevice.
	 */
	public int toAndroidSourceId() {
		return fAndroidSourceId;
	}

	/**
	 * Gets Corona's unique integer ID for this device type.
	 * @return Returns a unique integer ID used by Corona to identify the device type.
	 */
	public int toCoronaIntegerId() {
		return fCoronaIntegerId;
	}

	/**
	 * Gets Corona's unique string ID for this device type such as "keyboard", "mouse", "gamepad", etc.
	 * @return Returns a unique string ID used by Corona's input events such as "keyboard", "mouse", etc.
	 */
	public String toCoronaStringId() {
		return fCoronaStringId;
	}

	/**
	 * Gets Corona's unique string ID for this device type such as "keyboard", "mouse", "gamepad", etc.
	 * @return Returns a unique string ID used by Corona's input events in Lua.
	 */
	@Override
	public String toString() {
		return fCoronaStringId;
	}

	/**
	 * Gets an integer hash code for this object.
	 * @return Returns this object's hash code.
	 */
	@Override
	public int hashCode() {
		return fAndroidSourceId;
	}

	/**
	 * Gets a read-only collection of all known input device types.
	 * @return Returns a read-only collection of all known input device types.
	 */
	public static ReadOnlyInputDeviceTypeSet getCollection() {
		return sTypeCollection;
	}

	/**
	 * Fetches the type of device that the given input event came from.
	 * @param event The motion event received from an activity or view's onTouchEvent(), onGenericMotionEvent(),
	 *              onTrackballevent(), or similar method.
	 * @return Returns the type of input device the event came from, such as KEYBOARD or TOUCHSCREEN.
	 *         <p>
	 *         Returns null if given a null argument.
	 *         <p>
	 *         Returns the UNKNOWN type if the event's input source is not known by Corona or Android.
	 */
	public static InputDeviceType from(android.view.MotionEvent event) {
		// Validate argument.
		if (event == null) {
			return null;
		}

		// Fetch the device type from the given input event.
		InputDeviceType deviceType;
		if (android.os.Build.VERSION.SDK_INT >= 9) {
			// Fetch the device type by the event's source ID.
			int sourceId = ApiLevel9.getSourceIdFrom(event);
			deviceType = InputDeviceType.fromAndroidSourceId(sourceId);
		}
		else {
			// For older operating systems, we have to assume that motion events come from a touchscreen.
			deviceType = InputDeviceType.TOUCHSCREEN;
		}
		return deviceType;
	}

	/**
	 * Fetches the type of device that the given input event came from.
	 * <p>
	 * Note that buttons on a gamepad or joystick are typically received on Android as key events.
	 * @param event The key event received from an activity or view's onKeyEvent() method.
	 * @return Returns the type of input device the event came from, such as KEYBOARD or GAMEPAD.
	 *         <p>
	 *         Returns null if given a null argument.
	 *         <p>
	 *         Returns the UNKNOWN type if the event's input source is not known by Corona or Android.
	 */
	public static InputDeviceType from(android.view.KeyEvent event) {
		// Validate argument.
		if (event == null) {
			return null;
		}

		// Fetch the device type from the given input event.
		InputDeviceType deviceType;
		if (android.os.Build.VERSION.SDK_INT >= 9) {
			// Fetch the device type by the event's source ID.
			int sourceId = ApiLevel9.getSourceIdFrom(event);
			deviceType = InputDeviceType.fromAndroidSourceId(sourceId);
		}
		else {
			// For older operating systems, we have to assume that key events come from a keyboard.
			deviceType = InputDeviceType.KEYBOARD;
		}
		return deviceType;
	}

	/**
	 * Fetches the input device type for the given Android source ID.
	 * @param sourceId Unique integer ID matching a SOURCE constant in Android class "InputDevice".
	 *                 <p>
	 *                 This ID can also come from a MotionEvent or KeyEvent object's getSource() method.
	 *                 However, you should never call that method unless you are running API Level 9 or higher.
	 *                 For lower API Levels, it is better to call this class' other static from() methods instead.
	 * @return Returns an input device type object matching the given source ID.
	 *         <p>
	 *         Returns UNKNOWN if the given ID is not recognized by Corona or Android.
	 */
	public static InputDeviceType fromAndroidSourceId(int sourceId) {
		// Fetch a pre-allocated object matching the given ID.
		// Note: A source ID can be made up of multiple device types.
		//       In this case, select a device type having the highest ID value.
		//       This way, a device that is both a GAMEPAD and a KEYBOARD will be returned as a GAMEPAD.
		InputDeviceType deviceType = InputDeviceType.UNKNOWN;
		for (InputDeviceType nextDeviceType : sTypeCollection) {
			if ((nextDeviceType != InputDeviceType.UNKNOWN) &&
			    ((sourceId & nextDeviceType.toAndroidSourceId()) == nextDeviceType.toAndroidSourceId()) &&
			    (nextDeviceType.toAndroidSourceId() > deviceType.toAndroidSourceId()))
			{
				deviceType = nextDeviceType;
			}
		}
		return deviceType;
	}

	/**
	 * Fetches a collection of input device types extracted from the given Android sources bit field.
	 * <p>
	 * Note that a sources bit field can flag multiple device types. This sources bit field is returned
	 * by the Android InputDevice.getSources() method. It can also come from the KeyEvent.getSource()
	 * and MotionEvent.getSource() methods, but those methods typically (but not always) return only
	 * one device type.
	 * @param sources Bit field matching SOURCE constants in Android class "InputDevice".
	 * @return Returns a collection of device types found in the given Android sources bit field.
	 *         <p>
	 *         This collection will never be empty. If the given argument specifies a device type this
	 *         class does not recognize, then the collection will contain one UNKNOWN type.
	 */
	public static InputDeviceTypeSet collectionFromAndroidSourcesBitField(int sources) {
		// Add pre-allocated device types to the collection matching the bits found in the given source ID.
		InputDeviceTypeSet collection = new InputDeviceTypeSet();
		for (InputDeviceType deviceType : sTypeCollection) {
			if ((deviceType != InputDeviceType.UNKNOWN) &&
			    ((sources & deviceType.toAndroidSourceId()) == deviceType.toAndroidSourceId()))
			{
				collection.add(deviceType);
			}
		}

		// If the collection is empty, then add an UNKNOWN device type to it.
		if (collection.size() <= 0) {
			collection.add(InputDeviceType.UNKNOWN);
		}

		// Return the collection of device types.
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
		 * Fetches the given event's source ID matching a SOURCE constant in Android class InputDevice.
		 * @param event Reference to the input event to retrieve the source ID from via the getSource() method.
		 *              <p>
		 *              Cannot be null or else an exception will be thrown.
		 * @return Returns a unique ID such as SOURCE_TOUCHSCREEN, SOURCE_MOUSE, SOURCE_GAMEPAD, etc.
		 */
		public static int getSourceIdFrom(android.view.InputEvent event) {
			if (event == null) {
				throw new NullPointerException();
			}
			return event.getSource();
		}
	}
}
