//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.notifications;


/** Stores the configuration for one scheduled notification to be triggered in the future. */
public class ScheduledNotificationSettings extends NotificationSettings {
	/** The date and time this notification should be triggered. */
	private java.util.Date fEndTime;

	/** Settings for the status bar notification to be displayed when this notification is triggered. */
	private StatusBarNotificationSettings fStatusBarSettings;


	/** Creates a new scheduled notification settings object. */
	public ScheduledNotificationSettings() {
		this(new StatusBarNotificationSettings());
	}

	/**
	 * Creates a new scheduled notification settings object.
	 * @param statusBarSettings Settings for the status bar item to be displayed when this notification triggers.
	 */
	private ScheduledNotificationSettings(StatusBarNotificationSettings statusBarSettings) {
		super();

		if (statusBarSettings == null) {
			statusBarSettings = new StatusBarNotificationSettings();
		}
		fEndTime = new java.util.Date();
		fStatusBarSettings = statusBarSettings;
	}

	/**
	 * Creates a new copy of this object.
	 * @return Returns a copy of this object.
	 */
	@Override
	public ScheduledNotificationSettings clone() {
		ScheduledNotificationSettings clone = (ScheduledNotificationSettings)super.clone();
		clone.fEndTime = getEndTime();
		clone.fStatusBarSettings = fStatusBarSettings.clone();
		return clone;
	}

	/**
	 * Copies the settings from the given object to this object.
	 * @param settings The object to copy settings from. Cannot be null.
	 */
	public void copyFrom(ScheduledNotificationSettings settings) {
		// Validate.
		if (settings == null) {
			throw new NullPointerException();
		}

		// Copy settings.
		setId(settings.getId());
		fEndTime = settings.getEndTime();
		fStatusBarSettings.copyFrom(settings.getStatusBarSettings());
	}

	/**
	 * Gets the unique integer ID assigned to this notification.
	 * @return Returns a unique integer ID.
	 */
	public int getId() {
		return fStatusBarSettings.getId();
	}

	/**
	 * Sets an integer ID used to uniquely identify this notification.
	 * @param value The unique integer ID to be assigned.
	 */
	public void setId(int value) {
		fStatusBarSettings.setId(value);
	}

	/**
	 * Gets the date and time this notification will be triggered.
	 * @return Returns the date and time the notification will be triggered.
	 */
	public java.util.Date getEndTime() {
		return (java.util.Date)fEndTime.clone();
	}

	/**
	 * Sets the date and time this notification will be triggered.
	 * @param value The date and time this notification will trigger. Cannot be null.
	 */
	public void setEndTime(java.util.Date value) {
		if (value == null) {
			throw new NullPointerException();
		}
		fEndTime = (java.util.Date)value.clone();
	}

	/**
	 * Gets the settings for the status bar item to be displayed when this notification triggers.
	 * @return Returns this notification's status bar settings.
	 */
	public StatusBarNotificationSettings getStatusBarSettings() {
		return fStatusBarSettings;
	}

	/**
	 * Creates a new scheduled notification settings object using the information stored in a Lua table.
	 * @param context Context need to fetch application information. Cannot be null.
	 * @param luaState The Lua state to copy notification settings from.
	 * @param luaStackIndex Index to the Lua table in the Lua stack to read notification settings from.
	 *                      This table must be formatted according to the documentation specified by
	 *                      Corona's system.scheduleNotification() Lua API.
	 * @return Returns a new scheduled notification settings object containing a copy of the settings
	 *         stored in the referenced Lua table.
	 */
	public static ScheduledNotificationSettings from(
		android.content.Context context, com.naef.jnlua.LuaState luaState, int luaStackIndex)
	{
		StatusBarNotificationSettings statusBarSettings;
		statusBarSettings = StatusBarNotificationSettings.from(context, luaState, luaStackIndex);
		return new ScheduledNotificationSettings(statusBarSettings);
	}
}
