//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

import android.app.Activity;

// TODO: REFACTOR SEVERAL OTHER INFORMATION PIECES INTO THIS CLASS SO CORONA CARDS CAN USE THEM!
// BE SURE TO WRAP THESE IN THE CORONA ACTIVITY CLASS HOWEVER AS TO NOT BREAK BACKWARD-COMPATIBILITY!

/** Gets information on an activity. */
public class CoronaActivityInfo {
	private boolean fSupportsOrientationChanges;
	private Activity fActivity;

	public CoronaActivityInfo(Activity activity) {
		fActivity = activity;
	}

	/**
	 * <!-- TODO: MAKE THIS PUBLIC AGAIN AFTER IT WAS REMOVED FROM 2013.2109 BY ACCIDENT -->
	 * Determines if this activity supports multiple orientations.
	 * @return Returns true if this activity's view can change orientations. Returns false if it has a fixed orientation.
	 */
	private boolean supportsOrientationChanges() {
		boolean supportsOrientationChanges;
		
		// If the auto-rotate setting is disabled, then we have a fixed orientation.
		if (fActivity == null || 
			android.provider.Settings.System.getInt(fActivity.getContentResolver(), 
 				android.provider.Settings.System.ACCELEROMETER_ROTATION, 0) == 0) {
			return false;
		}

		switch (fActivity.getRequestedOrientation()) {
			case android.content.pm.ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED:
			case android.content.pm.ActivityInfo.SCREEN_ORIENTATION_SENSOR:
			case android.content.pm.ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE:
			case 11: // android.content.pm.ActivityInfo.SCREEN_ORIENTATION_USER_LANDSCAPE:
			case android.content.pm.ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT:
			case 12: // android.content.pm.ActivityInfo.SCREEN_ORIENTATION_USER_PORTRAIT:
			case android.content.pm.ActivityInfo.SCREEN_ORIENTATION_FULL_SENSOR:
			case 13: // android.content.pm.ActivityInfo.SCREEN_ORIENTATION_FULL_USER:
				supportsOrientationChanges = true;
				break;
			default:
				supportsOrientationChanges = false;
				break;
		}
		return supportsOrientationChanges;
	}

	public boolean hasFixedOrientation() {
		return !supportsOrientationChanges();
	}
}
