//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

/**
 * Receives a broadcast when the Android devices has been booted up.
 * <p>
 * This allows this application process to be automatically started on boot up provided that
 * the "android.permission.RECEIVE_BOOT_COMPLETED" permission has been set in the manifest file.
 * <p>
 * An instance of this class will be automatically created by the Android OS when a broadcasted
 * message is sent to this app. This is done via the "receiver" entry in the application's
 * AndroidManifest.xml file. You should never create an instance of this class yourself.
 */
public class SystemStartupBroadcastReceiver extends android.content.BroadcastReceiver {
    /**
     * Called right after the Android device has been booted up.
     * @param context The context this receiver belongs to.
     * @param intent An empty intent object.
     */
    @Override
    public void onReceive(android.content.Context context, android.content.Intent intent) {
        // Post all notifications to the status bar and reschedule all pending notifications.
        // If there is at least one active notification, then start the Corona service in the
        // background to keep this application process alive. We must do this otherwise the
        // Android system will cancel all notifications when this application shuts down.
        try {
            com.ansca.corona.notifications.NotificationServices notificationServices;
            notificationServices = new com.ansca.corona.notifications.NotificationServices(context);
            if (notificationServices.hasNotifications()) {
                context.startService(new android.content.Intent(context, CoronaService.class));
            }
        }
        catch (Exception ex){
            android.util.Log.e("CORONA", "Exception onReceive in SystemStartupBroadcastReceiver", ex);
        }
    }
}