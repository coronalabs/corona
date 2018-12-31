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