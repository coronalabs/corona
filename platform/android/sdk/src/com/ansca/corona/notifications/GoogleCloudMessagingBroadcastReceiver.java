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
 * Receives broadcasts from Google Cloud Messaging such as push notifications and registration messages.
 * This receiver will automatically post notifications to the status bar via the NotificationServices class.
 * <p>
 * An instance of this class will be automatically created by the Android OS when a broadcasted
 * message is sent to this app. This is done via the "receiver" entry in the application's
 * AndroidManifest.xml file. You should never create an instance of this class yourself.
 */
public class GoogleCloudMessagingBroadcastReceiver extends android.content.BroadcastReceiver {
	/**
	 * Called when a notification has been received from the Google Cloud Messaging system.
	 * @param context The context this receiver belongs to.
	 * @param intent Information about the notification.
	 */
	@Override
	public void onReceive(android.content.Context context, android.content.Intent intent) {
		GoogleCloudMessagingServices gcmServices = new GoogleCloudMessagingServices(context);
		gcmServices.process(intent);
	}
}
