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
 * Provides information about one particular key such as its unique Android integer ID,
 * unique Corona string ID, its symbolic name, etc.
 * <p>
 * Instances of this class are immutable.
 * <p>
 * You cannot create instances of this class.
 * Instead, you acquire KeyCode objects via this class' static from() methods.
 */
public class KeyCode {
	/** Unique integer ID of the key code as defined by Android. */
	private int fAndroidNumericId;

	/**
	 * Unique string ID of the key code as defined by the C++ side of Corona.
	 * This string constant is retrieved on demand from the "AndroidKeyServices" C++ class.
	 */
	private String fCoronaStringId;


	/**
	 * Creates a new key code object.
	 * @param androidNumericId Android's unique integer ID matching a KEYCODE constant in the KeyEvent class.
	 */
	private KeyCode(int androidNumericId) {
		fAndroidNumericId = androidNumericId;
		fCoronaStringId = null;
	}


	/**
	 * Gets Android's unique integer ID for this key code.
	 * @return Returns a unique integer ID matching an Android constant in the KeyEvent class.
	 */
	public int toAndroidKeyCode() {
		return fAndroidNumericId;
	}

	/**
	 * Gets Android's unique name for this key code.
	 * <p>
	 * This is the string received from Android's KeyEvent.keyCodeToString() method.
	 * @return Returns the unique symbolic name of the key code.
	 */
	public String toAndroidSymbolicName() {
		String symbolicName;

		if (android.os.Build.VERSION.SDK_INT >= 12) {
			symbolicName = ApiLevel12.getSymbolicNameFromKeyCode(fAndroidNumericId);
		}
		else if ((fAndroidNumericId > android.view.KeyEvent.KEYCODE_UNKNOWN) &&
		         (fAndroidNumericId <= android.view.KeyEvent.getMaxKeyCode())) {
			symbolicName = Integer.toString(fAndroidNumericId);
		}
		else {
			symbolicName = "KEYCODE_UNKNOWN";
		}
		return symbolicName;
	}

	/**
	 * Gets Corona's unique string ID for this key code.
	 * @return Returns a unique string ID used by Corona's key event in Lua.
	 */
	public String toCoronaStringId() {
		if (fCoronaStringId == null) {
			fCoronaStringId = com.ansca.corona.JavaToNativeShim.getKeyNameFromKeyCode(fAndroidNumericId);
			if (fCoronaStringId == null) {
				fCoronaStringId = "unknown";
			}
		}
		return fCoronaStringId;
	}
	
	/**
	 * Gets Corona's unique string ID for this key code.
	 * @return Returns a unique string ID used by Corona's key event in Lua.
	 */
	@Override
	public String toString() {
		return toCoronaStringId();
	}

	/**
	 * Gets the key code from the given Android key event object.
	 * @param event The Android key event object providing a key code.
	 *              <p>
	 *              Setting this to null will cause an exception to occur.
	 * @return Returns the given key event's key code information.
	 */
	public static KeyCode from(android.view.KeyEvent event) {
		if (event == null) {
			throw new NullPointerException();
		}
		return KeyCode.fromAndroidKeyCode(event.getKeyCode());
	}

	/**
	 * Gets a key code object from the given Android key code integer ID.
	 * @param value The key code's unique integer ID matching a KEYCODE constant in Android class KeyEvent.
	 * @return Returns the given key code's information.
	 */
	public static KeyCode fromAndroidKeyCode(int value) {
		return new KeyCode(value);
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
		 * Gets a string that represents the symbolic name of the specified key code.
		 * This is retrieved by calling Android's KeyEvent.keyCodeToString() method.
		 * @param keyCode Unique integer ID matching an Android KEYCODE constant in the KeyEvent class.
		 * @return Returns the symbolic name such as "KEYCODE_A" or an equivalent numeric constant such as "1001".
		 *         <p>
		 *         Returns "KEYCODE_UNKNOWN" if given an unknown key code.
		 */
		public static String getSymbolicNameFromKeyCode(int keyCode) {
			return android.view.KeyEvent.keyCodeToString(keyCode);
		}
	}
}
