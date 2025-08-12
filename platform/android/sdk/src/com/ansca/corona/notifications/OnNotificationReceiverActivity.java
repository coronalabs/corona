//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


package com.ansca.corona.notifications;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;

public class OnNotificationReceiverActivity extends Activity {
	private static final String INTENT_EXTRA_ID_NAME = "id";

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		handleNotification(this, getIntent());
		finish();
	}

	@Override
	protected void onNewIntent(Intent intent) {
		super.onNewIntent(intent);
		handleNotification(this, intent);
		finish();
	}


	/**
	 * Creates a new Intent object that will call this broadcast receiver's onReceive() method with
	 * information about the given notification.
	 * <p>
	 * Intended to be wrapped by a PendingIntent object and given to an Android "Notification" object
	 * for its touch/tap listener.
	 * @param context Context object needed to create the Intent object.
	 *                 <p>
	 *                 Cannot be null or else an exception will be thrown.
	 * @param settings Provides the notification ID to be applied to the intent so that the broadcast receiver
	 *                 can identify which notification was invoked.
	 *                 <p>
	 *                 Cannot be null or else an exception will be thrown.
	 * @return Returns a new Intent object for the given notification settings.
	 */
	public static android.content.Intent createContentIntentFrom(
			android.content.Context context, StatusBarNotificationSettings settings)
	{
		android.content.Intent intent = createDeleteIntentFrom(context, settings);
		intent.setAction(android.content.Intent.ACTION_VIEW);
		return intent;
	}

	/**
	 * Creates a new Intent object that will call this broadcast receiver's onReceive() method with
	 * information about the given notification.
	 * <p>
	 * Intended to be wrapped by a PendingIntent object and given to an Android "Notification" object
	 * for its delete listener.
	 * @param context Context object needed to create the Intent object.
	 *                 <p>
	 *                 Cannot be null or else an exception will be thrown.
	 * @param settings Provides the notification ID to be applied to the intent so that the broadcast receiver
	 *                 can identify which notification was invoked.
	 *                 <p>
	 *                 Cannot be null or else an exception will be thrown.
	 * @return Returns a new Intent object for the given notification settings.
	 */
	public static android.content.Intent createDeleteIntentFrom(
			android.content.Context context, StatusBarNotificationSettings settings)
	{
		// Validate.
		if ((context == null) || (settings == null)) {
			throw new NullPointerException();
		}

		// Create and return the intent.
		android.content.Intent intent = new android.content.Intent(context, OnNotificationReceiverActivity.class);
		intent.setData(android.net.Uri.parse("notification://statusbar?id=" + Integer.toString(settings.getId())));
		intent.setAction(android.content.Intent.ACTION_DELETE);
		intent.putExtra(INTENT_EXTRA_ID_NAME, settings.getId());
		return intent;
	}
	private static void handleNotification(Context context, Intent intent) {
		// Validate.
		if (intent == null) {
			return;
		}

		// Fetch the notification ID from the intent.
		if (intent.hasExtra(INTENT_EXTRA_ID_NAME) == false) {
			return;
		}
		int notificationId = intent.getIntExtra(INTENT_EXTRA_ID_NAME, -1);

		// Fetch the notification object by ID.
		StatusBarNotificationSettings notificationSettings;
		NotificationServices notificationServices = new NotificationServices(context);
		notificationSettings = notificationServices.fetchStatusBarNotificationById(notificationId);
		if (notificationSettings == null) {
			return;
		}

		// Remove the notification settings from Corona.
		// This way the status bar item will not reappear when restarting this application.
		notificationServices.removeById(notificationSettings.getId());

		// If the notification was tapped on, then display the Corona activity and send a notification event.
		if (android.content.Intent.ACTION_VIEW.equals(intent.getAction())) {
			String applicationStateName = "inactive";

			// Create the notification event.
			com.ansca.corona.events.NotificationReceivedTask event = new com.ansca.corona.events.NotificationReceivedTask(
					applicationStateName, notificationSettings);

			for (com.ansca.corona.CoronaRuntime runtime : com.ansca.corona.CoronaRuntimeProvider.getAllCoronaRuntimes()) {
				applicationStateName = runtime.isRunning() ? "active" : "inactive";
				event = new com.ansca.corona.events.NotificationReceivedTask(
						applicationStateName, notificationSettings);
				runtime.getTaskDispatcher().send(event);
			}

			// Display the Corona activity window if not already visible.
			Class<?> activityClass = com.ansca.corona.CoronaActivity.class;
			try {
				android.content.pm.PackageInfo pi = context.getPackageManager().getPackageInfo(context.getPackageName(), android.content.pm.PackageManager.GET_ACTIVITIES);
				for (android.content.pm.ActivityInfo ai : pi.activities) {
					try {
						Class<?> c = Class.forName(ai.name);
						if (com.ansca.corona.CoronaActivity.class.isAssignableFrom(c)) {
							activityClass = c;
							break;
						}
					} catch (Throwable ignore) {
					}
				}
			}
			catch (Throwable ignore) {
			}
			intent = new android.content.Intent(context, activityClass);
			intent.addFlags(android.content.Intent.FLAG_ACTIVITY_NEW_TASK);
			intent.addFlags(android.content.Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
			intent.putExtra(com.ansca.corona.events.NotificationReceivedTask.NAME, event.toBundle());
			context.startActivity(intent);
	}
}
}