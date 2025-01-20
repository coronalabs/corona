//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.notifications;


/**
 * Receives broadcasts from the Android alarm manager service when scheduled notifications have triggered.
 * This receiver will automatically display a status bar notification for these triggered events.
 * <p>
 * An instance of this class will be automatically created by the Android OS when a broadcasted
 * message is sent to this app. This is done via the "receiver" entry in the application's
 * AndroidManifest.xml file. You should never create an instance of this class yourself.
 */
public class AlarmManagerBroadcastReceiver extends android.content.BroadcastReceiver {
	/** The unique name of the Intent extra data that stores the notification ID. */
	private static final String INTENT_EXTRA_ID_NAME = "id";


	/**
	 * Called when a scheduled notification has been triggered by the Android alarm manager service.
	 * @param context The context this receiver belongs to.
	 * @param intent Information about the scheduled notification.
	 */
	@Override
	public void onReceive(android.content.Context context, android.content.Intent intent) {
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
		ScheduledNotificationSettings notificationSettings;
		NotificationServices notificationServices = new NotificationServices(context);
		notificationSettings = notificationServices.fetchScheduledNotificationById(notificationId);
		if (notificationSettings == null) {
			return;
		}

		// Display a status bar notification.
		// This also removes the scheduled notification from the system since they share the same ID.
		notificationServices.post(notificationSettings.getStatusBarSettings());
	}

	/**
	 * Creates a new Intent object that will call this broadcast receiver's onReceive() method with
	 * information about the given notification.
	 * <p>
	 * Intended to be wrapped by a PendingIntent object and given to the Android AlarmManager service
	 * to be invoked when a scheduled notification triggers.
	 * @param context Context object needed to create the Intent object.
	 *                 <p>
	 *                 Cannot be null or else an exception will be thrown.
	 * @param settings Provides the notification ID to be applied to the intent so that the broadcast receiver
	 *                 can identify which notification was triggered.
	 *                 <p>
	 *                 Cannot be null or else an exception will be thrown.
	 * @return Returns a new Intent object for the given notification settings.
	 */
	public static android.content.Intent createIntentFrom(
		android.content.Context context, ScheduledNotificationSettings settings)
	{
		// Validate.
		if ((context == null) || (settings == null)) {
			throw new NullPointerException();
		}

		// Create and return the intent.
		android.content.Intent intent = new android.content.Intent(context, AlarmManagerBroadcastReceiver.class);
		intent.setData(android.net.Uri.parse("notification://alarm?id=" + Integer.toString(settings.getId())));
		intent.putExtra(INTENT_EXTRA_ID_NAME, settings.getId());
		return intent;
	}
}
