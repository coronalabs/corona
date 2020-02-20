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
 * Provides information about one axis from an input device such as its unique Android integer ID,
 * unique Android symbolic name, and Corona's unique string ID.
 * <p>
 * Instances of this class are immutable.
 * <p>
 * You cannot create instances of this class.
 */
public class AxisType {
	/** Stores a collection of all known axis types. */
	private static java.util.ArrayList<AxisType> sTypeCollection = null;

	/** Unique integer ID as defined by the axis constants in Android's "MotionEvent" class. */
	private int fAndroidIntegerId;

	/** Corona's unique integer ID matching a constant in the C++ "InputAxisType" class. */
	private int fCoronaIntegerId;

	/** Corona's unique string ID matching a constant in the C++ "InputAxisType" class. */
	private String fCoronaStringId;


	/**
	 * Creates a new axis type.
	 * @param androidIntegerId The axis' unique integer ID as defined by one of the AXIS constants
	 *                         in Android's "MotionEvent" class.
	 * @param coronaIntegerId Corona's unique integer ID for the axis type.
	 * @param coronaStringId Corona's unique string ID to be used by the "axis" event in Lua.
	 */
	private AxisType(int androidIntegerId, int coronaIntegerId, String coronaStringId) {
		fAndroidIntegerId = androidIntegerId;
		fCoronaIntegerId = coronaIntegerId;
		fCoronaStringId = coronaStringId;
	}


	/** Indicates that the axis type is unknown. */
	public static final AxisType UNKNOWN = new AxisType(-1, 0, "unknown");

	/** Indicates that the device provides x-axis input, such as from a joystick. */
	public static final AxisType X = new AxisType(0, 1, "x");

	/** Indicates that the device provides y-axis input, such as from a joystick. */
	public static final AxisType Y = new AxisType(1, 2, "y");

	/** Indicates that the device provides z-axis input, such as from a joystick. */
	public static final AxisType Z = new AxisType(11, 3, "z");

	/** Indicates that the device provides rotation input around the x-axis. */
	public static final AxisType ROTATION_X = new AxisType(12, 4, "rotationX");

	/** Indicates that the device provides rotation input around the y-axis. */
	public static final AxisType ROTATION_Y = new AxisType(13, 5, "rotationY");

	/** Indicates that the device provides rotation input around the z-axis. */
	public static final AxisType ROTATION_Z = new AxisType(14, 6, "rotationZ");

	/** Indicates that the device provides x-axis input from the left thumbstick on a gamepad. */
	public static final AxisType LEFT_X = new AxisType(-1, 7, "leftX");

	/** Indicates that the device provides y-axis input from the left thumbstick on a gamepad. */
	public static final AxisType LEFT_Y = new AxisType(-1, 8, "leftY");

	/** Indicates that the device provides x-axis input from the right thumbstick on a gamepad. */
	public static final AxisType RIGHT_X = new AxisType(-1, 9, "rightX");

	/** Indicates that the device provides y-axis input from the right thumbstick on a gamepad. */
	public static final AxisType RIGHT_Y = new AxisType(-1, 10, "rightY");

	/** Indicates that the device provides x-axis input from a joystick's hat switch. */
	public static final AxisType HAT_X = new AxisType(15, 11, "hatX");

	/** Indicates that the device provides y-axis input from a joystick's hat switch. */
	public static final AxisType HAT_Y = new AxisType(16, 12, "hatY");

	/** Indicates that the device provides left trigger input from a gamepad. */
	public static final AxisType LEFT_TRIGGER = new AxisType(17, 13, "leftTrigger");

	/** Indicates that the device provides right trigger input from a gamepad. */
	public static final AxisType RIGHT_TRIGGER = new AxisType(18, 14, "rightTrigger");

	/** Indicates that the device provides gas pedal input or right trigger input from a gamepad. */
	public static final AxisType GAS = new AxisType(22, 15, "gas");

	/** Indicates that the device provides brake pedal input or left trigger input from a gamepad. */
	public static final AxisType BRAKE = new AxisType(23, 16, "brake");

	/** Indicates that the device provides steering wheel input. */
	public static final AxisType WHEEL = new AxisType(21, 17, "wheel");

	/** Indicates that the device provides rudder input. */
	public static final AxisType RUDDER = new AxisType(20, 18, "rudder");

	/** Indicates that the device provides throttle or slider input. */
	public static final AxisType THROTTLE = new AxisType(19, 19, "throttle");

	/** Indicates that the device provides vertical scroll input, such as from a mouse's scroll wheel. */
	public static final AxisType VERTICAL_SCROLL = new AxisType(9, 20, "verticalScroll");

	/** Indicates that the device provides horizontal scroll input, such as from a mouse's scroll wheel. */
	public static final AxisType HORIZONTAL_SCROLL = new AxisType(10, 21, "horizontalScroll");

	/** Indicates that the device provides the direction of a stylus or finger relative to a surface. */
	public static final AxisType ORIENTATION = new AxisType(8, 22, "orientation");

	/** Indicates that the device provides the distance that a stylus or finger is from the screen. */
	public static final AxisType HOVER_DISTANCE = new AxisType(24, 23, "hoverDistance");

	/**
	 * Indicates that the device provides the length of the major axis of the stylus or finger
	 * that is above the touchscreen or touchpad.
	 */
	public static final AxisType HOVER_MAJOR = new AxisType(6, 24, "hoverMajor");

	/**
	 * Indicates that the device provides the length of the minor axis of the stylus or finger
	 * that is above the touchscreen or touchpad.
	 */
	public static final AxisType HOVER_MINOR = new AxisType(7, 25, "hoverMinor");

	/** Indicates that the device provides the surface area that is in contact of the touchscreen or touchpad. */
	public static final AxisType TOUCH_SIZE = new AxisType(3, 26, "touchSize");

	/**
	 * Indicates that the device provides the length of the major axis of the stylus or finger
	 * that is in contact with the touchscreen or touchpad.
	 */
	public static final AxisType TOUCH_MAJOR = new AxisType(4, 27, "touchMajor");

	/**
	 * Indicates that the device provides the length of the minor axis of the stylus or finger
	 * that is in contact with the touchscreen or touchpad.
	 */
	public static final AxisType TOUCH_MINOR = new AxisType(5, 28, "touchMinor");

	/**
	 * Indicates that the device provides the amount of pressure applied to the touchscreen or touchpad.
	 * Can also be used by a mouse or trackball to indicate that a button was pressed.
	 */
	public static final AxisType PRESSURE = new AxisType(2, 29, "pressure");

	/** Indicates that the device provides the angle that a stylus is being held in relative to a surface. */
	public static final AxisType TILT = new AxisType(25, 30, "tilt");

	/** Generic axis input 1 belonging to a device. */
	public static final AxisType GENERIC_1 = new AxisType(32, 31, "generic1");

	/** Generic axis input 2 belonging to a device. */
	public static final AxisType GENERIC_2 = new AxisType(33, 32, "generic2");

	/** Generic axis input 3 belonging to a device. */
	public static final AxisType GENERIC_3 = new AxisType(34, 33, "generic3");

	/** Generic axis input 4 belonging to a device. */
	public static final AxisType GENERIC_4 = new AxisType(35, 34, "generic4");

	/** Generic axis input 5 belonging to a device. */
	public static final AxisType GENERIC_5 = new AxisType(36, 35, "generic5");

	/** Generic axis input 6 belonging to a device. */
	public static final AxisType GENERIC_6 = new AxisType(37, 36, "generic6");

	/** Generic axis input 7 belonging to a device. */
	public static final AxisType GENERIC_7 = new AxisType(38, 37, "generic7");

	/** Generic axis input 8 belonging to a device. */
	public static final AxisType GENERIC_8 = new AxisType(39, 38, "generic8");

	/** Generic axis input 9 belonging to a device. */
	public static final AxisType GENERIC_9 = new AxisType(40, 39, "generic9");

	/** Generic axis input 10 belonging to a device. */
	public static final AxisType GENERIC_10 = new AxisType(41, 40, "generic10");

	/** Generic axis input 11 belonging to a device. */
	public static final AxisType GENERIC_11 = new AxisType(42, 41, "generic11");

	/** Generic axis input 12 belonging to a device. */
	public static final AxisType GENERIC_12 = new AxisType(43, 42, "generic12");

	/** Generic axis input 13 belonging to a device. */
	public static final AxisType GENERIC_13 = new AxisType(44, 43, "generic13");

	/** Generic axis input 14 belonging to a device. */
	public static final AxisType GENERIC_14 = new AxisType(45, 44, "generic14");

	/** Generic axis input 15 belonging to a device. */
	public static final AxisType GENERIC_15 = new AxisType(46, 45, "generic15");

	/** Generic axis input 16 belonging to a device. */
	public static final AxisType GENERIC_16 = new AxisType(47, 46, "generic16");


	/**
	 * Determines if this axis type is natively supported by Android and has a matching AXIS constant ID
	 * in the "MotionEvent" Android class.
	 * @return Returns true if this axis type is supported by Android and that the toAndroidIntegerId()
	 *         will return a valid ID that can be used by Android's "MotionEvent" class' axis methods.
	 *         <p>
	 *         Returns false if this is a Corona defined axis type that is not defined by Android.
	 */
	public boolean hasAndroidIntegerId() {
		return (fAndroidIntegerId >= 0);
	}

	/**
	 * Gets a unique integer ID for this axis type as defined by an AXIS constant in Android's "MotionEvent" class.
	 * @return Returns Android's unique integer ID for this axis type.
	 *         <p>
	 *         Returns -1 if this axis type is UNKNOWN or is a Corona axis type that does not exist in Android.
	 */
	public int toAndroidIntegerId() {
		return fAndroidIntegerId;
	}

	/**
	 * Gets Android's unique name for this axis type.
	 * <p>
	 * This is the string received from Android's MotionEvent.axisToString() method.
	 * @return Returns the unique symbolic name for the axis type.
	 */
	public String toAndroidSymbolicName() {
		String symbolicName;

		if (android.os.Build.VERSION.SDK_INT >= 12) {
			symbolicName = ApiLevel12.getSymbolicNameFromAndroidIntegerId(fAndroidIntegerId);
		}
		else {
			symbolicName = Integer.toString(fAndroidIntegerId);
		}
		return symbolicName;
	}

	/**
	 * Gets Corona's unique integer ID for this axis type.
	 * @return Returns a unique integer ID used by Corona to identify the axis type.
	 */
	public int toCoronaIntegerId() {
		return fCoronaIntegerId;
	}

	/**
	 * Gets Corona's unique string ID for this axis type.
	 * @return Returns a unique string ID used by Corona's "axis" events in Lua.
	 */
	public String toCoronaStringId() {
		return fCoronaStringId;
	}

	/**
	 * Gets Corona's unique string ID for this axis type.
	 * @return Returns a unique string ID used by Corona's "axis" events in Lua.
	 */
	@Override
	public String toString() {
		return fCoronaStringId;
	}

	/**
	 * Fetches the axis type for the given Android axis integer ID.
	 * @param value The integer ID matching an AXIS constant in Android's "MotionEvent" class.
	 * @return Returns an axis type object matching the given ID.
	 *         <p>
	 *         Returns UNKNOWN if the given ID is not recognized by Corona.
	 */
	public static AxisType fromAndroidIntegerId(int value) {
		// Add the pre-allocated axis types to a collection via reflection, if not done already.
		// This allows for fast retrieval for future calls to this method.
		if (sTypeCollection == null) {
			try {
				sTypeCollection = new java.util.ArrayList<AxisType>();
				for (java.lang.reflect.Field field : AxisType.class.getDeclaredFields()) {
					if (field.getType().equals(AxisType.class)) {
						AxisType axisType = (AxisType)field.get(null);
						if (axisType != null) {
							sTypeCollection.add(axisType);
						}
					}
				}
			}
			catch (Exception ex) { }
			if (sTypeCollection == null) {
				return AxisType.UNKNOWN;
			}
		}

		// Return a pre-allocated object matching the given ID.
		for (AxisType axisType : sTypeCollection) {
			if (axisType.toAndroidIntegerId() == value) {
				return axisType;
			}
		}

		// The given ID is unknown. Return an unknown axis type.
		return AxisType.UNKNOWN;
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
		private ApiLevel12() {}

		/**
		 * Gets a string that represents the symbolic name for the given Android axis ID.
		 * This is retrieved by calling Android's MotionEvent.axisToString() method.
		 * @param value Unique integer ID matching an Android AXIS constant in the "MotionEvent" class.
		 * @return Returns the symbolic name such as "X_AXIS".
		 *         <p>
		 *         If the given ID is unknown, then returns a string containing the ID's numeric value.
		 */
		public static String getSymbolicNameFromAndroidIntegerId(int value) {
			return android.view.MotionEvent.axisToString(value);
		}
	}
}
