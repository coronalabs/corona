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


/** Base implementation for one notification configuration object. */
public abstract class NotificationSettings implements Cloneable {
	/** Creates a new notification settings object. */
	public NotificationSettings() {
	}

	/**
	 * Creates a new copy of this object.
	 * @return Returns a copy of this object.
	 */
	@Override
	public NotificationSettings clone() {
		NotificationSettings clone = null;
		try {
			clone = (NotificationSettings)super.clone();
		}
		catch (Exception ex) { }
		return clone;
	}

	/**
	 * Gets the unique integer ID assigned to this notification.
	 * @return Returns a unique integer ID.
	 */
	public abstract int getId();

	/**
	 * Sets an integer ID used to uniquely identify this notification.
	 * @param value The unique integer ID to be assigned.
	 */
	public abstract void setId(int value);
}
