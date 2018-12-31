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

package com.ansca.corona;


/**
 * Indicates the type of orientation displayed by a window such as PortraitUpright or LandscapeRight. 
 * <p>
 * Instances of this class are immutable.
 * <p>
 * You cannot create instances of this class.
 * Instead, you use the pre-allocated objects from this class' static methods and fields.
 */
public final class WindowOrientation {
	/** Unique integer ID matching an orientation constant in Corona C++ enum Rtt::DeviceOrientation. */
	private int fCoronaIntegerId;

	/** Unique string ID matching an orientation used in Corona's "build.settings" file. */
	private String fCoronaStringId;


	/**
	 * Creates a new orientation object with the given IDs.
	 * @param coronaIntegerId Unique integer ID matching a constant in Corona C++ enum Rtt:DeviceOrientation.
	 * @param coronaStringId Unique string ID to be passed to the Lua orientation "event.type" property.
	 */
	private WindowOrientation(int coronaIntegerId, String coronaStringId) {
		// Validate.
		if (coronaStringId == null) {
			throw new NullPointerException();
		}

		// Store the given values.
		fCoronaIntegerId = coronaIntegerId;
		fCoronaStringId = coronaStringId;
	}


	/** Indicates that the orientation is unknown. */
	public static final WindowOrientation UNKNOWN = new WindowOrientation(0, "unknown");

	/** Indicates that the window is being displayed portrait. */
	public static final WindowOrientation PORTRAIT_UPRIGHT = new WindowOrientation(1, "portrait");

	/** Indicates that the window is being displayed landscape right. */
	public static final WindowOrientation LANDSCAPE_RIGHT = new WindowOrientation(2, "landscapeRight");

	/** Indicates that the window is being displayed portrait upside down. */
	public static final WindowOrientation PORTRAIT_UPSIDE_DOWN = new WindowOrientation(3, "portraitUpsideDown");

	/** Indicates that the window is being displayed landscape left. */
	public static final WindowOrientation LANDSCAPE_LEFT = new WindowOrientation(4, "landscapeLeft");


	/**
	 * Determines if this orientation is portrait.
	 * @return Returns true if the orientation is portrait upright or upside down.
	 *         <p>
	 *         Returns false if the orientation is landscape right/left or is unknown.
	 */
	public boolean isPortrait() {
		return (this.equals(PORTRAIT_UPRIGHT) || this.equals(PORTRAIT_UPSIDE_DOWN));
	}

	/**
	 * Determines if this orientation is landscape.
	 * @return Returns true if the orientation is landscape right or left.
	 *         <p>
	 *         Returns false if the orientation is portrait or unknown.
	 */
	public boolean isLandscape() {
		return (this.equals(LANDSCAPE_RIGHT) || this.equals(LANDSCAPE_LEFT));
	}

	/**
	 * Determines if the given activity supports this orientation.
	 * @param activity The activity object to compare with. Will determine what orientations this activity
	 *                 supports by calling its getRequestedOrientation() method.
	 *                 <p>
	 *                 Cannot be null or else an exception will be thrown.
	 * @return Returns true if the given activity supports this orientation. Returns false if not.
	 */
	public boolean isSupportedBy(android.app.Activity activity) {
		// Validate.
		if (activity == null) {
			throw new NullPointerException();
		}

		// Determine if the given activity supports this orientation.
		boolean isSupported;
		switch (activity.getRequestedOrientation()) {
			case android.content.pm.ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED:
			case android.content.pm.ActivityInfo.SCREEN_ORIENTATION_SENSOR:
			case 10: // android.content.pm.ActivityInfo.SCREEN_ORIENTATION_FULL_SENSOR:
				isSupported = true;
				break;
			case android.content.pm.ActivityInfo.SCREEN_ORIENTATION_PORTRAIT:
			case 7:  // android.content.pm.ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT:
			case 9:  // android.content.pm.ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT:
				isSupported = isPortrait();
				break;
			case android.content.pm.ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE:
			case 6:  // android.content.pm.ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE:
			case 8:  // android.content.pm.ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE:
				isSupported = isLandscape();
				break;
			default:
				isSupported = false;
				break;
		}
		return isSupported;
	}

	/**
	 * Gets this orientation's unique integer ID matching a constant in Corona C++ enum Rtt::DeviceOrientation.
	 * @return Returns this orientation's unique Corona integer ID.
	 */
	public int toCoronaIntegerId() {
		return fCoronaIntegerId;
	}

	/**
	 * Gets this orientation's unique string ID passed to the Lua orientation "event.type" property.
	 * @return Returns this orientation's unique string ID.
	 */
	public String toCoronaStringId() {
		return fCoronaStringId;
	}

	/**
	 * Gets the name of this orientation.
	 * @return Returns this orientation's name.
	 */
	@Override
	public String toString() {
		return fCoronaStringId;
	}

	/**
	 * Fetches the application's current orientation from the window manager.
	 * @param context Context needed to access the "window manager" system service. Cannot be null.
	 * @return Returns the application window's current orientation.
	 */
	public static WindowOrientation fromCurrentWindowUsing(android.content.Context context) {
		// Validate.
		if (context == null) {
			throw new NullPointerException();
		}

		// Fetch the display's current orientation in degrees.
		int displayAngle;
		android.view.WindowManager windowManager =
				(android.view.WindowManager)context.getSystemService(android.content.Context.WINDOW_SERVICE);
		switch (windowManager.getDefaultDisplay().getRotation()) {
			case android.view.Surface.ROTATION_180:
				displayAngle = 180;
				break;
			case android.view.Surface.ROTATION_270:
				displayAngle = 270;
				break;
			case android.view.Surface.ROTATION_90:
				displayAngle = 90;
				break;
			default:
				displayAngle = 0;
				break;
		}

		// Return the current orientation.
		return fromDegrees(context, displayAngle);
	}

	/**
	 * Gets an orientation based on the display's angle in degrees relative to the device's natural orientation.
	 * @param context Context needed to determine the device's natural orientation. Cannot be null.
	 * @param degrees The display angle in degrees retrieved by the Display.getRotation() method or
	 *                the OrientationEventListener.onOrientationChanged() method.
	 *                <p>
	 *                Must be a value between 0 and 359, where 0 represents the natural orientation
	 *                of the device and it rotates in the counter-clockwise direction.
	 * @return Returns an orientation matching the given display angle.
	 */
	public static WindowOrientation fromDegrees(android.content.Context context, int degrees) {
		// Validate.
		if (context == null) {
			throw new NullPointerException();
		}
		
		// Determine the natural orientation of the device.
		// For phones and small tablets, this is typically portrait.
		// For larget tablets, this is typically landscape.
		boolean isNaturalOrientationPortrait = true;
		android.view.WindowManager windowManager =
				(android.view.WindowManager)context.getSystemService(android.content.Context.WINDOW_SERVICE);
		android.view.Display display = windowManager.getDefaultDisplay();
		switch (display.getRotation()) {
			case android.view.Surface.ROTATION_0:
			case android.view.Surface.ROTATION_180:
				isNaturalOrientationPortrait = (display.getWidth() < display.getHeight());
				break;
			case android.view.Surface.ROTATION_90:
			case android.view.Surface.ROTATION_270:
				isNaturalOrientationPortrait = (display.getWidth() > display.getHeight());
				break;
		}

		// If the device's natural orientation is landscape, then offset given orientation value to portrait.
		if (isNaturalOrientationPortrait == false) {
			degrees = (degrees + 90) % 360;
		}
		
		// Determine the orientation based on the given angle in degrees.
		WindowOrientation orientation;
		if ((degrees >= 45) && (degrees < 135)) {
			orientation = LANDSCAPE_RIGHT;
		}
		else if ((degrees >= 135) && (degrees < 225)) {
			orientation = PORTRAIT_UPSIDE_DOWN;
		}
		else if ((degrees >= 225) && (degrees < 315)) {
			orientation = LANDSCAPE_LEFT;
		}
		else {
			orientation = PORTRAIT_UPRIGHT;
		}
		return orientation;
	}
}
