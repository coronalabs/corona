//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.permissions;

/**
 * Enum depicting the urgency of a permissions request.
 */
public enum PermissionUrgency {
	/**
	 * Permission is only for something that's not very important, but nice to have.
	 * <p>
	 * For example, a Camera app asking for Location permission to geotag photos would be low urgency.
	 */
	LOW,

	/**
	 * Permission is needed to do something important with the app.
	 * <p>
	 * For example, a Camera app asking for Microphone permission to take video would be normal urgency.
	 */
	NORMAL,

	/**
	 * Permission is absolutely critical to running the app.
	 * <p>
	 * For example, a Camera app asking for Camera permission would be critical urgency since it can't function without it.
	 */
	CRITICAL;

	/**
	 * Gets a PermissionUrgency from a {@link java.lang.String String}.
	 */
	public static PermissionUrgency from(String urgencyString) {

		// Validate
		if( urgencyString == null ) {
			return PermissionUrgency.LOW;
		}

		urgencyString = urgencyString.toLowerCase();

		if ( urgencyString.equals("critical") ) return PermissionUrgency.CRITICAL;
		else if ( urgencyString.equals("normal") ) return PermissionUrgency.NORMAL;
		else return PermissionUrgency.LOW;
	}
}