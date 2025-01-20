//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;


/** Provides information about a received local or push notification. */
public class NotificationReceivedTask implements com.ansca.corona.CoronaRuntimeTask {
	/** The unique string key representing the application state such as "active" or "background". */
	private String fApplicationStateName;

	/** The notification's main settings. */
	private com.ansca.corona.notifications.StatusBarNotificationSettings fStatusBarSettings;


	/** The unique name of this event. */
	public static final String NAME = "notification";


	/**
	 * Creates a new notification event to be sent to the native side of Corona.
	 * @param applicationStateName Unique name of this application's current state such as "active" or "inactive".
	 * @param settings The status bar notification that has been tapped/triggered.
	 */
	public NotificationReceivedTask(
		String applicationStateName,
		com.ansca.corona.notifications.StatusBarNotificationSettings settings)
	{
		// Validate arguments.
		if ((applicationStateName == null) || (settings == null)) {
			throw new NullPointerException();
		}

		// Store settings.
		fApplicationStateName = applicationStateName;
		fStatusBarSettings = settings;
	}
	
	/** Sends the event to the native side of Corona. */
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		// Fetch the Corona runtime's Lua state.
		com.naef.jnlua.LuaState luaState = null;
		if (runtime != null) {
			luaState = runtime.getLuaState();
		}
		if (luaState == null) {
			return;
		}

		// Push this event's information to a new Lua table.
		luaState.newTable();
		int luaTableStackIndex = luaState.getTop();
		luaState.pushString(NAME);
		luaState.setField(luaTableStackIndex, "name");
		luaState.pushString(getNotificationTypeString());
		luaState.setField(luaTableStackIndex, "type");
		luaState.pushString(fApplicationStateName);
		luaState.setField(luaTableStackIndex, "applicationState");
		luaState.pushString(fStatusBarSettings.getContentText());
		luaState.setField(luaTableStackIndex, "alert");
		if (fStatusBarSettings.getSourceData() != null) {
			fStatusBarSettings.getSourceData().pushTo(luaState);
			luaState.setField(luaTableStackIndex, getSourceDataName());
		}
		if (fStatusBarSettings.getData() != null) {
			fStatusBarSettings.getData().pushTo(luaState);
		}
		else {
			luaState.newTable();
		}
		luaState.setField(luaTableStackIndex, "custom");

		// Dispatch the event at the top of the Lua stack.
		com.ansca.corona.JavaToNativeShim.dispatchEventInLua(runtime);

		// Pop the event from the Lua stack.
		luaState.pop(1);
	}

	/**
	 * Creates and returns a bundle containing a copy of this event's settings.
	 * Expected to be passed to an Intent object as extra data.
	 * @return Returns a new Bundle object having a copy of this event's settings.
	 */
	public android.os.Bundle toBundle() {
		android.os.Bundle bundle = new android.os.Bundle();
		bundle.putString("name", NAME);
		bundle.putString("type", getNotificationTypeString());
		bundle.putString("applicationState", fApplicationStateName);
		bundle.putString("alert", fStatusBarSettings.getContentText());
		com.ansca.corona.CoronaData sourceData = fStatusBarSettings.getSourceData();
		if (sourceData != null) {
			bundle.putParcelable(getSourceDataName(), new com.ansca.corona.CoronaData.Proxy(sourceData));
		}
		com.ansca.corona.CoronaData.Table customData = fStatusBarSettings.getData();
		if (customData == null) {
			customData = new com.ansca.corona.CoronaData.Table();
		}
		bundle.putParcelable("custom", new com.ansca.corona.CoronaData.Proxy(customData));
		return bundle;
	}

	/**
	 * Gets the unique string identifying the type of notification being provided by this event.
	 * @return Returns a unique string such as "local" or "remote".
	 */
	private String getNotificationTypeString() {
		if (fStatusBarSettings.isSourceLocal()) {
			return com.ansca.corona.notifications.NotificationType.LOCAL.toInvariantString();
		}
		return com.ansca.corona.notifications.NotificationType.REMOTE.toInvariantString();
	}

	/**
	 * Gets a valid "source data" name for the notification, to be used as a string key in a table
	 * to store the notification's source data.
	 * @return Returns a valid source data name.
	 */
	private String getSourceDataName() {
		String sourceDataName = fStatusBarSettings.getSourceDataName();
		if (sourceDataName != null) {
			sourceDataName = sourceDataName.trim();
		}
		if ((sourceDataName == null) || (sourceDataName.length() <= 0)) {
			sourceDataName = "sourceData";
		}
		return sourceDataName;
	}
}
