//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
