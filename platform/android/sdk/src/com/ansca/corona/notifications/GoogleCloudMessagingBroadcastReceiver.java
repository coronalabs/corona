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
