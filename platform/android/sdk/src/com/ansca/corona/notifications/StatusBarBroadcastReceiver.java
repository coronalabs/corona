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
 * Receives broadcasts from the Android status bar when a status notifications has been tapped or removed.
 * Will automatically remove the notification from Corona and display the activity if tapped.
 * <p>
 * An instance of this class will be automatically created by the Android OS when a broadcasted
 * message is sent to this app. This is done via the "receiver" entry in the application's
 * AndroidManifest.xml file. You should never create an instance of this class yourself.
 */
public class StatusBarBroadcastReceiver extends android.content.BroadcastReceiver {
	/** The unique name of the Intent extra data that stores the notification ID. */
	private static final String INTENT_EXTRA_ID_NAME = "id";


	/**
	 * Called when a status bar notification has been tapped or removed.
	 * @param context The context this receiver belongs to.
	 * @param intent Information about the status bar notification.
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
			intent = new android.content.Intent(context, com.ansca.corona.CoronaActivity.class);
			intent.addFlags(android.content.Intent.FLAG_ACTIVITY_NEW_TASK);
			intent.addFlags(android.content.Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
			intent.putExtra(com.ansca.corona.events.NotificationReceivedTask.NAME, event.toBundle());
			context.startActivity(intent);
		}
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
		android.content.Intent intent = new android.content.Intent(context, StatusBarBroadcastReceiver.class);
		intent.setData(android.net.Uri.parse("notification://statusbar?id=" + Integer.toString(settings.getId())));
		intent.setAction(android.content.Intent.ACTION_DELETE);
		intent.putExtra(INTENT_EXTRA_ID_NAME, settings.getId());
		return intent;
	}
}
