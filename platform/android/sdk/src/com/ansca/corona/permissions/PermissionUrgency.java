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