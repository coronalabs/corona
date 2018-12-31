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

package com.ansca.corona.notifications;


/**
 * Indicates the type of notification such as "local" or "remote".
 * Provides string IDs matching types used by the native side of Corona.
 * <p>
 * Instances of this class are immutable.
 * <p>
 * You cannot create instances of this class.
 * Instead, you use the pre-allocated objects from this class' static methods and fields.
 */
public class NotificationType {
	/** The unique string ID for this notification type. This string is never localized. */
	private String fInvariantName;


	/**
	 * Creates a new notification type object with the given string ID.
	 * @param invariantName Unique string ID matching a notification type from the native side of Corona.
	 */
	private NotificationType(String invariantName) {
		fInvariantName = invariantName;
	}


	/** Indicates a local/scheduled notification. */
	public static final NotificationType LOCAL = new NotificationType("local");

	/** Indicates a remote/push notification. */
	public static final NotificationType REMOTE = new NotificationType("remote");
	
	/** Indicates a setup notification for registering a remote notification. */
	public static final NotificationType REMOTE_REGISTRATION = new NotificationType("remoteRegistration");
	

	/**
	 * Gets the unique string ID matching a notification type from the native side of Corona.
	 * @return Returns this notification type's unqiue string ID.
	 */
	public String toInvariantString() {
		return fInvariantName;
	}

	/**
	 * Gets a notification type matching the given invariant string ID.
	 * @param value Unique string ID matching a notification type from the native side of Corona.
	 * @return Returns a notification type object matching the given invariant string.
	 *         <p>
	 *         Returns null if the given string ID is unknown.
	 */
	public static NotificationType fromInvariantString(String value) {
		// Return a pre-allocated object matching the given string ID via reflection.
		try {
			for (java.lang.reflect.Field field : NotificationType.class.getDeclaredFields()) {
				if (field.getType().equals(NotificationType.class)) {
					NotificationType notificationType = (NotificationType)field.get(null);
					if (notificationType.fInvariantName.equals(value)) {
						return notificationType;
					}
				}
			}
		}
		catch (Exception ex) { }
		
		// The given string ID is unknown.
		return null;
	}
}
