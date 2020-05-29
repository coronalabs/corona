//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.notifications;


/** Stores the configuration for one status bar notification. */
public class StatusBarNotificationSettings extends NotificationSettings {
	/** Unique integer ID assigned to this notification. */
	private int fId;

	/** The date and time to show in the status bar. */
	private java.util.Date fTimestamp;

	/** The title to be shown in the notification. */
	private String fContentTitle;

	/** The message to be displayed under the notification's title. */
	private String fContentText;

	/** The alert message to be displayed when the notification first arrives. */
	private String fTickerText;

	/** The unique name of the icon resource. */
	private String fIconResourceName;

	/** The badge number to be displayed next to the notification. */
	private int fBadgeNumber;

	/** Set true to allow the end-user to remove the status bar item. */
	private boolean fIsRemovable;

	/** Optional path to the audio file to be played when the notification first arrives. */
	private android.net.Uri fSoundFileUri;

	/** The name of the system that generated this notifcation, such as "google". */
	private String fSourceName;

	/** Set true if the system that generated this notifcation was "local". Set false if "remote". */
	private boolean fIsSourceLocal;

	/** Optional name describing the "fSourceData" member variable. Used to identify how to interpret this data. */
	private String fSourceDataName;

	/** Optional object used to store the source data used to generate this notification. */
	private com.ansca.corona.CoronaData fSourceData;

	/** Table for storing optional data with the notification. */
	private com.ansca.corona.CoronaData.Table fData;


	/** The resource name of the default drawable to use for the notification's icon in the status bar. */
	public static final String DEFAULT_ICON_RESOURCE_NAME = "corona_statusbar_icon_default";


	/** Creates a new status bar notification settings object. */
	public StatusBarNotificationSettings() {
		super();

		fId = 0;
		fTimestamp = new java.util.Date();
		fContentTitle = "";
		fContentText = "";
		fTickerText = "";
		fIconResourceName = DEFAULT_ICON_RESOURCE_NAME;
		fBadgeNumber = 0;
		fIsRemovable = true;
		fSoundFileUri = null;
		fSourceName = "";
		fIsSourceLocal = true;
		fSourceDataName = "";
		fSourceData = null;
		fData = new com.ansca.corona.CoronaData.Table();
	}

	/**
	 * Creates a new copy of this object.
	 * @return Returns a copy of this object.
	 */
	@Override
	public StatusBarNotificationSettings clone() {
		StatusBarNotificationSettings clone = (StatusBarNotificationSettings)super.clone();
		clone.fTimestamp = getTimestamp();
		clone.fSourceData = (fSourceData != null) ? fSourceData.clone() : null;
		clone.fData = (fData != null) ? fData.clone() : null;
		return clone;
	}

	/**
	 * Copies the settings from the given object to this object.
	 * @param settings The object to copy settings from. Cannot be null.
	 */
	public void copyFrom(StatusBarNotificationSettings settings) {
		// Validate.
		if (settings == null) {
			throw new NullPointerException();
		}

		// Copy settings.
		setId(settings.getId());
		fTimestamp = settings.getTimestamp();
		fContentTitle = settings.fContentTitle;
		fContentText = settings.fContentText;
		fTickerText = settings.fTickerText;
		fIconResourceName = settings.fIconResourceName;
		fBadgeNumber = settings.fBadgeNumber;
		fIsRemovable = settings.fIsRemovable;
		fSoundFileUri = settings.fSoundFileUri;
		fSourceName = settings.fSourceName;
		fIsSourceLocal = settings.fIsSourceLocal;
		fSourceDataName = settings.fSourceDataName;
		fSourceData = (settings.fSourceData != null) ? settings.fSourceData.clone() : null;
		fData = (settings.fData != null) ? settings.fData.clone() : null;
	}

	/**
	 * Gets the unique integer ID assigned to this notification.
	 * @return Returns a unique integer ID.
	 */
	public int getId() {
		return fId;
	}

	/**
	 * Sets an integer ID used to uniquely identify this notification.
	 * @param value The unique integer ID to be assigned.
	 */
	public void setId(int value) {
		fId = value;
	}
	
	/**
	 * Gets the date and time to be shown next to the notification.
	 * @return Returns the assigned timestamp.
	 */
	public java.util.Date getTimestamp() {
		return (java.util.Date)fTimestamp.clone();
	}

	/**
	 * Sets the date and time to be shown next to the notification.
	 * @param value The date and time to assign to the notification. Cannot be null.
	 */
	public void setTimestamp(java.util.Date value) {
		if (value == null) {
			throw new NullPointerException();
		}
		fTimestamp = (java.util.Date)value.clone();
	}

	/**
	 * Gets the title to be shown by the notification.
	 * @return Returns the notification's title.
	 *         <p>
	 *         Returns an empty string if a title has not been assigned.
	 */
	public String getContentTitle() {
		return fContentTitle;
	}

	/**
	 * Sets the title text to be shown by the notification.
	 * @param text The notification's title text. Can be set to null or empty string.
	 */
	public void setContentTitle(String text) {
		if (text == null) {
			text = "";
		}
		fContentTitle = text;
	}

	/**
	 * Gets the text to be shown under the notification's title.
	 * @return Returns the notification's content text.
	 *         <p>
	 *         Returns an empty string if text has not been assigned.
	 */
	public String getContentText() {
		return fContentText;
	}

	/**
	 * Sets the text to be shown under the notification's title.
	 * @param text The text to be shown under the title. Can be set to null or empty string.
	 */
	public void setContentText(String text) {
		if (text == null) {
			text = "";
		}
		fContentText = text;
	}

	/**
	 * Gets the alert text which is briefly shown on the status bar when the notification triggers.
	 * @return Returns the notification's alert text.
	 *         <p>
	 *         Returns an empty string if text has not been assigned.
	 */
	public String getTickerText() {
		return fTickerText;
	}

	/**
	 * Sets the alert text to be briefly shown on the status bar when the notification triggers.
	 * @param text The alert text to be shown.
	 *             <p>
	 *             Set to null or empty string to not show an alert text.
	 */
	public void setTickerText(String text) {
		if (text == null) {
			text = "";
		}
		fTickerText = text;
	}

	/**
	 * Gets the unique name of the drawable resource to use for this notification's icon.
	 * @return Returns the unique name of the drawable resource. This is the file's name without the extension.
	 */
	public String getIconResourceName() {
		return fIconResourceName;
	}

	/**
	 * Gets the R class' unique integer ID for the drawable resource to use as the notification's icon.
	 * @return Returns the drawable icon's unique integer ID in the R class.
	 *         <p>
	 *         Returns "com.ansca.corona.storage.ResourceServices.INVALID_RESOURCE_ID" if the resource name given
	 *         to the setIconResourceName() method could not find its associated drawable resource.
	 */
	public int getIconResourceId() {
		// Fetch the application context.
		android.content.Context context = com.ansca.corona.CoronaEnvironment.getApplicationContext();
		if (context == null) {
			return com.ansca.corona.storage.ResourceServices.INVALID_RESOURCE_ID;
		}

		// Return the icon's resource ID.
		com.ansca.corona.storage.ResourceServices resourceServices;
		resourceServices = new com.ansca.corona.storage.ResourceServices(context);
		return resourceServices.getDrawableResourceId(fIconResourceName);
	}

	/**
	 * Sets the drawable resource to use as the notification's icon in the status bar.
	 * @param resourceName The unique name of the drawable resource. This is the file's name without the extension.
	 */
	public void setIconResourceName(String resourceName) {
		if (resourceName == null) {
			resourceName = "";
		}
		fIconResourceName = resourceName;
	}

	/**
	 * Gets the badge number to be shown next to the notification.
	 * @return Returns the number to be shown next to the notification.
	 *         <p>
	 *         Returns zero if no number is to be shown.
	 */
	public int getBadgeNumber() {
		return fBadgeNumber;
	}

	/**
	 * Sets the badge number to be shown next to the notification.
	 * @param value The number to be shown next to the notification.
	 *              <p>
	 *              Set to a value less than or equal to zero to not display a badge number.
	 */
	public void setBadgeNumber(int value) {
		if (value < 0) {
			value = 0;
		}
		fBadgeNumber = value;
	}

	/**
	 * Determines if this status bar item can be removed by the end-user without tapping it.
	 * @return Returns true if the end-user can remove the status bar item by tapping the status bar's clear button
	 *         or by swiping the status bar item off of the notificaiton list.
	 *         <p>
	 *         Returns false if the status bar item may only be removed by the application.
	 */
	public boolean isRemovable() {
		return fIsRemovable;
	}

	/**
	 * Sets whether or not the end-user is able to remove the status bar item without tapping it.
	 * @param value Set true to allow the end-user to remove the status bar item by tapping the status bar's
	 *              clear button or by swiping it off of the status bar notification list. In this case the
	 *              application will never receive the status bar notification's event.
	 *              <p>
	 *              Set false to only allow this application to remove the status bar item.
	 */
	public void setRemovable(boolean value) {
		fIsRemovable = value;
	}

	/**
	 * Gets the path to the sound file to be played when the notification first arrives.
	 * @return Returns a Uri providing the path and file name to the sound file.
	 *         <p>
	 *         Returns null if a sound file was not assigned.
	 */
	public android.net.Uri getSoundFileUri() {
		return fSoundFileUri;
	}

	/**
	 * Sets a path to the sound file to be played when the notification first arrives.
	 * @param uri The path to the sound file to be played.
	 *            <p>
	 *            Set to null to use the system's default notification sound.
	 */
	public void setSoundFileUri(android.net.Uri uri) {
		fSoundFileUri = uri;
	}

	/**
	 * Gets the name of the system that generated this notification, such as "google" for push notifications.
	 * @return Returns the name of the system that generated this notification.
	 *         <p>
	 *         Returns an empty string if the source is unknown.
	 */
	public String getSourceName() {
		return fSourceName;
	}

	/**
	 * Sets the name of the system that generated this notification.
	 * @param name The name of the system, such as "google" if it came from Google's push notification system.
	 *             <p>
	 *             Set to null or empty string if the source is unknown.
	 */
	public void setSourceName(String name) {
		if (name == null) {
			name = "";
		}
		fSourceName = name;
	}

	/**
	 * Determines if the system that generated this notification was local or remote.
	 * @return Returns true if this notification came from the device itself, such as a scheduled notification.
	 *         <p>
	 *         Returns false if this notification came from the network, such as a Google push notification.
	 */
	public boolean isSourceLocal() {
		return fIsSourceLocal;
	}

	/**
	 * Sets a flag indicating if this notification came from a local or remote source.
	 * @param value Set true if this notification came from this device, such as a scheduled notification.
	 *              <p>
	 *              Set false if this notification came from the network, such as a Google push notification.
	 */
	public void setSourceLocal(boolean value) {
		fIsSourceLocal = value;
	}

	/**
	 * Gets a name describing this object's optional "source data" returned by the getSourceData() method.
	 * <p>
	 * The source data is a copy of the data used to generate this notification.
	 * @return Returns a string describing this object's "source data" used to generate this notification.
	 *         <p>
	 *         Returns an empty string if a name was not provided or unknown.
	 */
	public String getSourceDataName() {
		return fSourceDataName;
	}

	/**
	 * Sets a name describing this object's optional "source data" returned by the getSourceData() method.
	 * <p>
	 * The source data is a copy of the data used to generate this notification.
	 * @param name String desscribing this notification's source data.
	 *             <p>
	 *             Set to null or empty string if no source data has been provided.
	 */
	public void setSourceDataName(String name) {
		if (name == null) {
			name = "";
		}
		fSourceDataName = name;
	}

	/**
	 * Gets an optional used to store the data that was used to generate this notification.
	 * <p>
	 * For example, it can be used to store a copy of the Android bundle received by a GCM remote notification.
	 * @return Returns the data used to generate this notification.
	 *         <p>
	 *         Returns null if source data was not provided or if the source was unknown.
	 */
	public com.ansca.corona.CoronaData getSourceData() {
		return fSourceData;
	}

	/**
	 * Assigns this object the source data used to generate this notification, if applicable.
	 * <p>
	 * For example, it can be used to store a copy of the Android bundle received by a GCM remote notification.
	 * @param sourceData Copy of the data used to generate this notification.
	 *                   <p>
	 *                   Set to null if the source is unknown or was not provided.
	 */
	public void setSourceData(com.ansca.corona.CoronaData sourceData) {
		fSourceData = sourceData;
	}

	/**
	 * Gets a table of data that can be optionally assigned to this notification.
	 * <p>
	 * This data will be made available to the notification listener in Lua via the "event.custom" property.
	 * @return Returns this notification's table of data.
	 *         <p>
	 *         Returns null or an empty table if no data was assigned.
	 */
	public com.ansca.corona.CoronaData.Table getData() {
		return fData;
	}

	/**
	 * Replaces this notification's table of data with the one given.
	 * <p>
	 * This data will be made available to the notification listener in Lua via the "event.custom" property.
	 * @param data The table of data to be assigned to this notification.
	 *             <p>
	 *             Set to null to not assign any data to this notification. The notification listener in Lua
	 *             will receive an empty table via the "event.custom" property in this case.
	 */
	public void setData(com.ansca.corona.CoronaData.Table data) {
		fData = data;
	}

	/**
	 * Creates a new status bar notification settings object using the information stored in a Lua table.
	 * @param context Context need to fetch application information. Cannot be null.
	 * @param luaState The Lua state to copy notification settings from.
	 * @param luaStackIndex Index to the Lua table in the Lua stack to read notification settings from.
	 *                      This table must be formatted according to the documentation specified by
	 *                      Corona's system.scheduleNotification() Lua API.
	 * @return Returns a new status bar notification settings object containing a copy of the settings
	 *         stored in the referenced Lua table.
	 */
	public static StatusBarNotificationSettings from(
		android.content.Context context, com.naef.jnlua.LuaState luaState, int luaStackIndex)
	{
		// Validate.
		if (context == null) {
			throw new NullPointerException();
		}

		// Create the status bar configuration object.
		StatusBarNotificationSettings settings = new StatusBarNotificationSettings();

		// Set the status bar notification's title to the application name by default.
		android.content.pm.ApplicationInfo applicationInfo = context.getApplicationInfo();
		String applicationName = (String)context.getPackageManager().getApplicationLabel(applicationInfo);
		settings.setContentTitle(applicationName);
		settings.setTickerText(applicationName);

		// Fetch the settings stored in the Lua table. (This is optional.)
		if ((luaState != null) && luaState.isTable(luaStackIndex)) {
			// Set up the status bar notification messages.
			luaState.getField(luaStackIndex, "alert");
			if (luaState.isString(-1)) {
				String text = luaState.toString(-1);
				settings.setContentText(text);
				settings.setTickerText(text);
			}
			else if (luaState.isTable(-1)) {
				luaState.getField(-1, "title");
				if (luaState.isString(-1)) {
					settings.setContentTitle(luaState.toString(-1));
				}
				luaState.pop(1);
				luaState.getField(-1, "body");
				if (luaState.isString(-1)) {
					String text = luaState.toString(-1);
					settings.setContentText(text);
					settings.setTickerText(text);
				}
				luaState.pop(1);
				luaState.getField(-1, "number");
				if (luaState.isNumber(-1)) {
					settings.setBadgeNumber(luaState.toInteger(-1));
				}
				luaState.pop(1);
			}
			luaState.pop(1);

			// Set the path to a custom sound file, if provided.
			luaState.getField(luaStackIndex, "sound");
			if (luaState.isString(-1)) {
				android.net.Uri uri = null;
				try {
					String filePath = luaState.toString(-1).trim();
					uri = com.ansca.corona.storage.FileContentProvider.createContentUriForFile(context, filePath);
				}
				catch (Exception ex) {
					ex.printStackTrace();
				}
				settings.setSoundFileUri(uri);
			}
			luaState.pop(1);

			// Tag the notification with custom data, if provided.
			luaState.getField(luaStackIndex, "custom");
			if (luaState.isTable(-1)) {
				com.ansca.corona.CoronaData customData = com.ansca.corona.CoronaData.from(luaState, -1);
				if (customData instanceof com.ansca.corona.CoronaData.Table) {
					settings.setData((com.ansca.corona.CoronaData.Table)customData);
				}
			}
			luaState.pop(1);
		}

		// Return the notification settings object.
		return settings;
	}
}
