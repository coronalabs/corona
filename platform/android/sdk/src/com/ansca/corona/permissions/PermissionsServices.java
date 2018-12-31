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

package com.ansca.corona.permissions;


/**
 * Provides methods and fields used to easily work with 
 * <a href="http://developer.android.com/guide/topics/security/permissions.html">Android permissions</a>,
 * both in the <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a>,
 * and at <a href="http://developer.android.com/training/permissions/requesting.html">runtime on Android 6</a> and above devices.
 * <p>
 * All methods in this class are thread safe and can be called from any thread.
 * <p>
 * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2016/2828/">daily build 2016.2828</a></b>.
 * @see ApplicationContextProvider
 */
public class PermissionsServices extends com.ansca.corona.ApplicationContextProvider {

	/** 
	 * API Level independent constants for 
	 * <a href="http://developer.android.com/guide/topics/security/permissions.html#normal-dangerous">dangerous Android permissions</a> 
	 * supported by Corona, sorted by permission groups. <b>These constants can be used regardless of the API Level of the device.</b>
	 * <p>
	 * Documentation for each constant elaborates further on the documentation provided by Google.
	 * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2016/2828/">daily build 2016.2828</a></b>.
	 * @see <a href="http://developer.android.com/guide/topics/security/permissions.html#permission-groups">Permission Groups</a>
	 * and <a href="http://developer.android.com/reference/android/Manifest.permission.html">Manifest.permission</a>
	 */
	public static class Permission {
		/** 
		 * Calendar group 
		 */

		/** 
		 * Allows an application to read the user's calendar data.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#CALENDAR">Calendar</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.READ_CALENDAR"
		 */
		public static final String READ_CALENDAR = "android.permission.READ_CALENDAR"; // android.Manifest.permission.READ_CALENDAR

		/** 
		 * Allows an application to write the user's calendar data.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#CALENDAR">Calendar</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.WRITE_CALENDAR"
		 */
		public static final String WRITE_CALENDAR = "android.permission.WRITE_CALENDAR"; // android.Manifest.permission.WRITE_CALENDAR
		
		/**
		 * Camera group
		 */

		/** 
		 * Required to be able to access the camera device.
		 * <p>
		 * This will automatically enforce the 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-feature-element.html">&lt;uses-feature&gt;</a> 
		 * manifest element for <i>all</i> camera features. If you do not require all camera features or can properly operate if a camera is
		 * not available, then you must modify your manifest as appropriate in order to install on devices that don't support all 
		 * camera features.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#CAMERA">Camera</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.CAMERA"
		 */
		public static final String CAMERA = "android.permission.CAMERA"; // android.Manifest.permission.CAMERA
		
		/** 
		 * Contacts group
		 */

		/** 
		 * Allows an application to read the user's contacts data.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#CONTACTS">Contacts</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.READ_CONTACTS"
		 */
		public static final String READ_CONTACTS = "android.permission.READ_CONTACTS"; // android.Manifest.permission.READ_CONTACTS

		/** 
		 * Allows an application to write the user's contacts data.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#CONTACTS">Contacts</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.WRITE_CONTACTS"
		 */
		public static final String WRITE_CONTACTS = "android.permission.WRITE_CONTACTS"; // android.Manifest.permission.WRITE_CONTACTS

		/** 
		 * Allows access to the list of accounts in the Accounts Service.
		 * <p>
		 * <b>Note:</b> Beginning with 
		 * <a href="http://developer.android.com/about/versions/marshmallow/android-6.0-changes.html">Android 6.0 (API level 23)</a>,
		 * if an app shares the signature of the authenticator that manages an account, it does not need 
		 * <a href="http://developer.android.com/reference/android/Manifest.permission.html#GET_ACCOUNTS">"GET_ACCOUNTS"</a> 
		 * permission to read information about that account. On 
		 * <a href="http://developer.android.com/about/versions/android-5.1.html">Android 5.1</a> and lower, all apps need
		 * <a href="http://developer.android.com/reference/android/Manifest.permission.html#GET_ACCOUNTS">"GET_ACCOUNTS"</a>
		 * permission to read information about any account.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#CONTACTS">Contacts</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.GET_ACCOUNTS"
		 */
		public static final String GET_ACCOUNTS = "android.permission.GET_ACCOUNTS"; // android.Manifest.permission.GET_ACCOUNTS
		
		/**
		 * Location group
		 */

		/** 
		 * Allows an app to access precise location.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#LOCATION">Location</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.ACCESS_FINE_LOCATION"
		 */
		public static final String ACCESS_FINE_LOCATION = "android.permission.ACCESS_FINE_LOCATION"; // android.Manifest.permission.ACCESS_FINE_LOCATION

		/** 
		 * Allows an app to access approximate location.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#LOCATION">Location</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.ACCESS_COARSE_LOCATION"
		 */
		public static final String ACCESS_COARSE_LOCATION = "android.permission.ACCESS_COARSE_LOCATION"; // android.Manifest.permission.ACCESS_COARSE_LOCATION
		
		/**
		 * Microphone group
		 */

		/** 
		 * Allows an application to record audio.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#MICROPHONE">Microphone</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.RECORD_AUDIO"
		 */
		public static final String RECORD_AUDIO = "android.permission.RECORD_AUDIO"; // android.Manifest.permission.RECORD_AUDIO
		
		/**
		 * Phone group
		 */

		/** 
		 * Allows read only access to phone state.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#PHONE">Phone</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.READ_PHONE_STATE"
		 */
		public static final String READ_PHONE_STATE = "android.permission.READ_PHONE_STATE"; // android.Manifest.permission.READ_PHONE_STATE

		/** 
		 * Allows an application to initiate a phone call without going through the 
		 * Dialer user interface for the user to confirm the call.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#PHONE">Phone</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.CALL_PHONE"
		 */
		public static final String CALL_PHONE = "android.permission.CALL_PHONE"; // android.Manifest.permission.CALL_PHONE

		/** 
		 * Allows an application to read the user's call log.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#PHONE">Phone</a>
		 * &emsp;
		 * Added in <a href="http://developer.android.com/about/versions/android-4.1.html">API Level 16</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.READ_CALL_LOG"
		 */
		public static final String READ_CALL_LOG = "android.permission.READ_CALL_LOG"; // android.Manifest.permission.READ_CALL_LOG

		/** 
		 * Allows an application to write (but not read) the user's contacts data.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#PHONE">Phone</a>
		 * &emsp;
		 * Added in <a href="http://developer.android.com/about/versions/android-4.1.html">API Level 16</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.WRITE_CALL_LOG"
		 */
		public static final String WRITE_CALL_LOG = "android.permission.WRITE_CALL_LOG"; // android.Manifest.permission.WRITE_CALL_LOG

		/** 
		 * Allows an application to add voicemails into the system.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#PHONE">Phone</a>
		 * &emsp;
		 * Added in <a href="http://developer.android.com/about/versions/android-4.0.html">API Level 14</a>
		 * <p>
		 * &emsp; Constant Value: "com.android.voicemail.permission.ADD_VOICEMAIL"
		 */
		public static final String ADD_VOICEMAIL = "com.android.voicemail.permission.ADD_VOICEMAIL"; // android.Manifest.permission.ADD_VOICEMAIL

		/** 
		 * Allows an application to use SIP service.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#PHONE">Phone</a>
		 * &emsp;
		 * Added in <a href="http://developer.android.com/about/versions/android-2.3.html">API Level 9</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.USE_SIP"
		 */
		public static final String USE_SIP = "android.permission.USE_SIP"; // android.Manifest.permission.USE_SIP

		/** 
		 * Allows an application to see the number being dialed during an outgoing call with the 
		 * option to redirect the call to a different number or abort the call altogether.
		  * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#PHONE">Phone</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.PROCESS_OUTGOING_CALLS"
		 */
		public static final String PROCESS_OUTGOING_CALLS = "android.permission.PROCESS_OUTGOING_CALLS"; // android.Manifest.permission.PROCESS_OUTGOING_CALLS
		
		/**
		 * Sensors group
		 */

		/** 
		 * Allows an application to access data from sensors that the user uses to 
		 * measure what is happening inside his/her body, such as heart rate.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#SENSORS">Sensors</a>
		 * &emsp;
		 * Added in <a href="http://developer.android.com/sdk/api_diff/20/changes.html">API Level 20</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.BODY_SENSORS"
		 */
		public static final String BODY_SENSORS = "android.permission.BODY_SENSORS"; // android.Manifest.permission.BODY_SENSORS
		
		/**
		 * SMS group
		 */

		/** 
		 * Allows an application to send SMS messages.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#SMS">SMS</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.SEND_SMS"
		 */
		public static final String SEND_SMS = "android.permission.SEND_SMS"; // android.Manifest.permission.SEND_SMS

		/** 
		 * Allows an application to receive SMS messages.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#SMS">SMS</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.RECEIVE_SMS"
		 */
		public static final String RECEIVE_SMS = "android.permission.RECEIVE_SMS"; // android.Manifest.permission.RECEIVE_SMS

		/** 
		 * Allows an application to read SMS messages.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#SMS">SMS</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.READ_SMS"
		 */
		public static final String READ_SMS = "android.permission.READ_SMS"; // android.Manifest.permission.READ_SMS

		/** 
		 * Allows an application to receive WAP push messages.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#SMS">SMS</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.RECEIVE_WAP_PUSH"
		 */
		public static final String RECEIVE_WAP_PUSH = "android.permission.RECEIVE_WAP_PUSH"; // android.Manifest.permission.RECEIVE_WAP_PUSH

		/** 
		 * Allows an application to monitor incoming MMS messages.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#SMS">SMS</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.RECEIVE_MMS"
		 */
		public static final String RECEIVE_MMS = "android.permission.RECEIVE_MMS"; // android.Manifest.permission.RECEIVE_MMS

		/** 
		 * Allows an application to read previously received cell broadcast messages and to register a content observer to get 
		 * notifications when a cell broadcast has been received and added to the database. For emergency alerts, the database is 
		 * updated immediately after the alert dialog and notification sound/vibration/speech are presented. The "read" column is then 
		 * updated after the user dismisses the alert. This enables supplementary emergency assistance apps to start loading additional 
		 * emergency information (if Internet access is available) when the alert is first received, and to delay presenting the info
		 * to the user until after the initial alert dialog is dismissed.
		 * <p>
		 * <b>WARNING!</b/> This permission is hidden in the Android source code. <b>Use at your own risk!</b>
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#SMS">SMS</a>
		 * &emsp;
		 * Hidden in 
		 * <a href="http://github.com/android/platform_frameworks_base/blob/marshmallow-release/core/res/AndroidManifest.xml#L458">Android source</a>. Added approximately in
		 * <a href="http://github.com/android/platform_frameworks_base/blob/jb-release/core/res/AndroidManifest.xml#L213">API Level 16</a>.
		 * <p>
		 * &emsp; Constant Value: "android.permission.READ_CELL_BROADCASTS"
		 */
		public static final String READ_CELL_BROADCASTS = "android.permission.READ_CELL_BROADCASTS";
		
		/**
		 * Storage group
		 */

		/** 
		 * Allows an application to read from external storage.
		 * <p>
		 * Any app that declares the 
		 * <a href="http://developer.android.com/reference/android/Manifest.permission.html#WRITE_EXTERNAL_STORAGE">WRITE_EXTERNAL_STORAGE</a> permission is implicitly granted this permission.
		 * <p>
		 * This permission is enforced starting in 
		 * <a href="http://developer.android.com/about/versions/android-4.4.html">API Level 19</a>. Before API level 19, this 
		 * permission is not enforced and all apps still have access to read from external storage. You can test your app with the 
		 * permission enforced by enabling <i>Protect USB storage</i> under Developer options in the Settings app on a device running 
		 * Android 4.1 or higher.
		 * <p>
		 * Also starting in API level 19, this permission is <i>not</i> required to read/write files in your application-specific 
		 * directories returned by <a href="http://developer.android.com/reference/android/content/Context.html#getExternalFilesDir(java.lang.String)">getExternalFilesDir(String)</a> 
		 * and <a href="http://developer.android.com/reference/android/content/Context.html#getExternalCacheDir()">getExternalCacheDir()</a>.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#STORAGE">STORAGE</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/about/versions/android-4.1.html">API Level 16</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.READ_EXTERNAL_STORAGE"
		 */
		public static final String READ_EXTERNAL_STORAGE = "android.permission.READ_EXTERNAL_STORAGE"; // android.Manifest.permission.READ_EXTERNAL_STORAGE

		/** 
		 * Allows an application to write to external storage.
		 * <p>
		 * Starting in <a href="http://developer.android.com/about/versions/android-4.4.html">API Level 19</a>, this permission is 
		 * <i>not</i> required to read/write files in your application-specific directories returned by 
		 * <a href="http://developer.android.com/reference/android/content/Context.html#getExternalFilesDir(java.lang.String)">getExternalFilesDir(String)</a> and <a href="http://developer.android.com/reference/android/content/Context.html#getExternalCacheDir()">getExternalCacheDir()</a>.
		 * <p>
		 * Permission Group:
		 * <a href="http://developer.android.com/reference/android/Manifest.permission_group.html#STORAGE">STORAGE</a>
		 * &emsp;
		 * Added in 
		 * <a href="http://developer.android.com/about/versions/android-1.6.html">API Level 4</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission.WRITE_EXTERNAL_STORAGE"
		 */
		public static final String WRITE_EXTERNAL_STORAGE = "android.permission.WRITE_EXTERNAL_STORAGE"; // android.Manifest.permission.WRITE_EXTERNAL_STORAGE
	}

	/** 
	 * API Level independent constants for supported permission-group names. 
	 * <b>These constants can be used regardless of the API Level of the device.</b>
	 * <p>
	 * Documentation for each constant elaborates further on the documentation provided by Google.
	 * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2016/2828/">daily build 2016.2828</a></b>. 
	 * @see <a href="http://developer.android.com/guide/topics/security/permissions.html#permission-groups">Permission Groups</a>
	 * and <a href="http://developer.android.com/reference/android/Manifest.permission_group.html">Manifest.permission_group</a>
	 */
	public static class PermissionGroup {
		/** 
		 * Used for runtime permissions related to user's calendar.
		 * <p>
		 * Added in 
		 * <a href="http://developer.android.com/about/versions/android-4.2.html">API Level 17</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission-group.CALENDAR"
		 */
		public static final String CALENDAR = "android.permission-group.CALENDAR"; // android.Manifest.permission_group.CALENDAR

		/** 
		 * Used for permissions that are associated with accessing camera or capturing images/video from the device.
		 * <p>
		 * Added in 
		 * <a href="http://developer.android.com/about/versions/android-4.2.html">API Level 17</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission-group.CAMERA"
		 */
		public static final String CAMERA = "android.permission-group.CAMERA"; // android.Manifest.permission_group.CAMERA

		/** 
		 * Used for runtime permissions related to contacts and profiles on this device.
		 * <p>
		 * Added in 
		 * <a href="http://developer.android.com/about/versions/marshmallow/android-6.0.html">API Level 23</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission-group.CONTACTS"
		 */
		public static final String CONTACTS = "android.permission-group.CONTACTS"; // android.Manifest.permission_group.CONTACTS

		/** 
		 * Used for permissions that allow accessing the device location.
		 * <p>
		 * Added in 
		 * <a href="http://developer.android.com/guide/topics/manifest/uses-sdk-element.html#ApiLevels">API Level 1</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission-group.LOCATION"
		 */
		public static final String LOCATION = "android.permission-group.LOCATION"; // android.Manifest.permission_group.LOCATION

		/** 
		 * Used for permissions that are associated with accessing microphone audio from the device. 
		 * Note that phone calls also capture audio but are in a separate (more visible) permission group.
		 * <p>
		 * Added in 
		 * <a href="http://developer.android.com/about/versions/android-4.2.html">API Level 17</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission-group.MICROPHONE"
		 */
		public static final String MICROPHONE = "android.permission-group.MICROPHONE"; // android.Manifest.permission_group.MICROPHONE

		/** 
		 * Used for permissions that are associated telephony features.
		 * <p>
		 * Added in 
		 * <a href="http://developer.android.com/about/versions/marshmallow/android-6.0.html">API Level 23</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission-group.PHONE"
		 */
		public static final String PHONE = "android.permission-group.PHONE"; // android.Manifest.permission_group.PHONE

		/** 
		 * Used for permissions that are associated with accessing camera or capturing images/video from the device.
		 * <!-- TODO: Actually have this link to the constant! -->
		 * Corresponds to the 
		 * <a href="http://docs.coronalabs.com/daily/api/library/native/showPopup.html">"BodySensors" platform-agnostic type</a> 
		 * in Corona.
		 * <p>
		 * Added in 
		 * <a href="http://developer.android.com/about/versions/marshmallow/android-6.0.html">API Level 23</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission-group.SENSORS"
		 */
		public static final String SENSORS = "android.permission-group.SENSORS"; // android.Manifest.permission_group.SENSORS

		/** 
		 * Used for runtime permissions related to user's SMS messages.
		 * <p>
		 * Added in 
		 * <a href="http://developer.android.com/about/versions/marshmallow/android-6.0.html">API Level 23</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission-group.SMS"
		 */
		public static final String SMS = "android.permission-group.SMS"; // android.Manifest.permission_group.SMS

		/** 
		 * Used for runtime permissions related to the shared external storage.
		 * <p>
		 * Added in 
		 * <a href="http://developer.android.com/about/versions/android-1.6.html">API Level 4</a>
		 * <p>
		 * &emsp; Constant Value: "android.permission-group.STORAGE"
		 */
		public static final String STORAGE = "android.permission-group.STORAGE"; // android.Manifest.permission_group.STORAGE
	}

	/** All dangerous-level permissions groups our permissions framework supports */
	private static String[] sDangerousPermissionGroups = new String[] {
			PermissionGroup.CALENDAR,
			PermissionGroup.CAMERA,
			PermissionGroup.CONTACTS,
			PermissionGroup.LOCATION,
			PermissionGroup.MICROPHONE,
			PermissionGroup.PHONE,
			PermissionGroup.SENSORS,
			PermissionGroup.SMS,
			PermissionGroup.STORAGE
	};

	/** Contains all the <uses-permission> tags in the AndroidManifest.xml */
	private static String[] sRequestedPermissions = null;

	/** Contains all of the permissions this app has previously requested. 
	 * Is built from SharedPreferences and only available on API Level 11+ */
	private static java.util.HashSet<String> sPreviouslyRequestedPermissions = null;

	/** SharedPreferences file for PermissionsServices */
	private static android.content.SharedPreferences sPermissionsPreferences = null;

	// TODO: Move all these permission maps to overrides of the android permissionInfo and permissionGroupInfo classes.

	/** Contains a mapping between platform-agnostic app permission names and the permission group names. */
	private static java.util.HashMap<String, String> sPlatformAgnosticPermissionGroupMap = new java.util.HashMap<String, String>();

	/** Contains a mapping between permission group names and platform-agnostic app permission names names. */
	private static java.util.HashMap<String, String> sPermissionGroupPlatformAgnosticMap = new java.util.HashMap<String, String>();

	/** Contains a mapping between all dangerous permissions and their permission groups as of Android 6. */
	private static java.util.HashMap<String, String> sMarshmallowPermissionToPermissionGroupMap = new java.util.HashMap<String, String>();

	/** Contains a mapping between all supported dangerous permission groups and the permissions in them as of Android 6. */
	private static java.util.HashMap<String, java.util.ArrayList<String>> sMarshmallowPermissionGroupToPermissionsMap = new java.util.HashMap<String, java.util.ArrayList<String>>();
	
	// /** Contains a cache of Permission -> Permission Group Lookups. */
	// private static java.util.Hashmap<String, String> fPermissionGroupLookupCache = new java.util.HashMap<String, String>();

	// /**
	//  * Creates an object that provides easy access to the application's resources, using the Application Context.
	//  * <p>
	//  * Use this for anything that could be used in Corona Cards.
	//  * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2016/2831/">daily build 2016.2831</a></b>.
	//  * <p>
	//  */
	// public PermissionsServices() {
	// 	this(com.ansca.corona.CoronaEnvironment.getApplicationContext());
	// }

	/**
	 * Creates an object that provides easy access to the application's permissions.
	 * @param context Reference to an Android created context used to access the input device system.
	 *                <b>Note:</b> Setting this to null will cause an exception to be thrown.
	 */
	public PermissionsServices(android.content.Context context) {
		super(context);
		synchronized(PermissionsServices.class) {
			if (sPreviouslyRequestedPermissions == null 
				&& android.os.Build.VERSION.SDK_INT >= 11) {
				ApiLevel11.initializePreviouslyRequestedPermissions(context);
			}
			if (sPlatformAgnosticPermissionGroupMap.isEmpty()) {
				
				// Populate the App Permission - Permission-Group Map.
				// The map uses all lower-case so we can handle simple typos developers may make.
				sPlatformAgnosticPermissionGroupMap.put("bodysensors", PermissionGroup.SENSORS);
				sPlatformAgnosticPermissionGroupMap.put("calendar", PermissionGroup.CALENDAR);
				sPlatformAgnosticPermissionGroupMap.put("calendars", PermissionGroup.CALENDAR);
				sPlatformAgnosticPermissionGroupMap.put("camera", PermissionGroup.CAMERA);
				sPlatformAgnosticPermissionGroupMap.put("contacts", PermissionGroup.CONTACTS);
				sPlatformAgnosticPermissionGroupMap.put("location", PermissionGroup.LOCATION);
				sPlatformAgnosticPermissionGroupMap.put("microphone", PermissionGroup.MICROPHONE);
				// These names are currently Android-only, but may exist on other platforms at some point.
				sPlatformAgnosticPermissionGroupMap.put("phone", PermissionGroup.PHONE);
				sPlatformAgnosticPermissionGroupMap.put("sms", PermissionGroup.SMS);
				sPlatformAgnosticPermissionGroupMap.put("storage", PermissionGroup.STORAGE);

			}
			if (sPermissionGroupPlatformAgnosticMap.isEmpty()) {

				// Populate the Permission-Group - App-Permission Map
				sPermissionGroupPlatformAgnosticMap.put(PermissionGroup.SENSORS, "BodySensors");
				sPermissionGroupPlatformAgnosticMap.put(PermissionGroup.CALENDAR, "Calendars");
				sPermissionGroupPlatformAgnosticMap.put(PermissionGroup.CAMERA, "Camera");
				sPermissionGroupPlatformAgnosticMap.put(PermissionGroup.CONTACTS, "Contacts");
				sPermissionGroupPlatformAgnosticMap.put(PermissionGroup.LOCATION, "Location");
				sPermissionGroupPlatformAgnosticMap.put(PermissionGroup.MICROPHONE, "Microphone");
				// These names are currently Android-only, but may exist on other platforms at some point.
				sPermissionGroupPlatformAgnosticMap.put(PermissionGroup.PHONE, "Phone");
				sPermissionGroupPlatformAgnosticMap.put(PermissionGroup.SMS, "SMS");
				sPermissionGroupPlatformAgnosticMap.put(PermissionGroup.STORAGE, "Storage");
			}
			if (sMarshmallowPermissionToPermissionGroupMap.isEmpty()) {

				// Populate the Marshmallow Dangerous Permissions - Permission Group Map
				// Calendar group
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.READ_CALENDAR, PermissionGroup.CALENDAR);
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.WRITE_CALENDAR, PermissionGroup.CALENDAR);

				// Camera group
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.CAMERA, PermissionGroup.CAMERA);

				// Contacts group
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.READ_CONTACTS, PermissionGroup.CONTACTS);
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.WRITE_CONTACTS, PermissionGroup.CONTACTS);
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.GET_ACCOUNTS, PermissionGroup.CONTACTS);

				// Location group
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.ACCESS_FINE_LOCATION, PermissionGroup.LOCATION);
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.ACCESS_COARSE_LOCATION, PermissionGroup.LOCATION);

				// Microphone group
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.RECORD_AUDIO, PermissionGroup.MICROPHONE);

				// Phone group
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.READ_PHONE_STATE, PermissionGroup.PHONE);
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.CALL_PHONE, PermissionGroup.PHONE);
				// Exclude these permissions from the Phone group if they don't exist on the API Level of the device we're running on.
				if (android.os.Build.VERSION.SDK_INT >= 16) {
					sMarshmallowPermissionToPermissionGroupMap.put(Permission.READ_CALL_LOG, PermissionGroup.PHONE);
					sMarshmallowPermissionToPermissionGroupMap.put(Permission.WRITE_CALL_LOG, PermissionGroup.PHONE);
				}
				if (android.os.Build.VERSION.SDK_INT >= 14) {
					sMarshmallowPermissionToPermissionGroupMap.put(Permission.ADD_VOICEMAIL, PermissionGroup.PHONE);
				}
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.USE_SIP, PermissionGroup.PHONE);
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.PROCESS_OUTGOING_CALLS, PermissionGroup.PHONE);

				// Body Sensors group
				// Exclude these permissions from the Body Sensors group if they don't exist on the API Level of the device we're running on.
				if (android.os.Build.VERSION.SDK_INT >= 20) {
					sMarshmallowPermissionToPermissionGroupMap.put(Permission.BODY_SENSORS, PermissionGroup.SENSORS);
				}

				// SMS group
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.SEND_SMS, PermissionGroup.SMS);
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.RECEIVE_SMS, PermissionGroup.SMS);
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.READ_SMS, PermissionGroup.SMS);
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.RECEIVE_WAP_PUSH, PermissionGroup.SMS);
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.RECEIVE_MMS, PermissionGroup.SMS);
				if (android.os.Build.VERSION.SDK_INT >= 16) {
					// WARNING: This permission is hidden in the Android source code!
					sMarshmallowPermissionToPermissionGroupMap.put(Permission.READ_CELL_BROADCASTS, PermissionGroup.SMS);
				}

				// Storage group
				// Exclude these permissions from the Storage group if they don't exist on the API Level of the device we're running on.
				if (android.os.Build.VERSION.SDK_INT >= 16) {
					sMarshmallowPermissionToPermissionGroupMap.put(Permission.READ_EXTERNAL_STORAGE, PermissionGroup.STORAGE);
				}
				sMarshmallowPermissionToPermissionGroupMap.put(Permission.WRITE_EXTERNAL_STORAGE, PermissionGroup.STORAGE);
			}
			if (sMarshmallowPermissionGroupToPermissionsMap.isEmpty()) {
				// Populate the Marshmallow Dangerous Permission Group - Permissions Map.
				// Calendar group
				sMarshmallowPermissionGroupToPermissionsMap.put(PermissionGroup.CALENDAR, new java.util.ArrayList<String>());
				java.util.ArrayList<String> calendarGroupPermissions = 
					sMarshmallowPermissionGroupToPermissionsMap.get(PermissionGroup.CALENDAR);
				calendarGroupPermissions.add(Permission.READ_CALENDAR);
				calendarGroupPermissions.add(Permission.WRITE_CALENDAR);

				// Camera group
				sMarshmallowPermissionGroupToPermissionsMap.put(PermissionGroup.CAMERA, new java.util.ArrayList<String>());
				java.util.ArrayList<String> cameraGroupPermissions = 
					sMarshmallowPermissionGroupToPermissionsMap.get(PermissionGroup.CAMERA);
				cameraGroupPermissions.add(Permission.CAMERA);

				// Contacts group
				sMarshmallowPermissionGroupToPermissionsMap.put(PermissionGroup.CONTACTS, new java.util.ArrayList<String>());
				java.util.ArrayList<String> contactsGroupPermissions = 
					sMarshmallowPermissionGroupToPermissionsMap.get(PermissionGroup.CONTACTS);
				contactsGroupPermissions.add(Permission.READ_CONTACTS);
				contactsGroupPermissions.add(Permission.WRITE_CONTACTS);
				contactsGroupPermissions.add(Permission.GET_ACCOUNTS);

				// Location group
				sMarshmallowPermissionGroupToPermissionsMap.put(PermissionGroup.LOCATION, new java.util.ArrayList<String>());
				java.util.ArrayList<String> locationGroupPermissions = 
					sMarshmallowPermissionGroupToPermissionsMap.get(PermissionGroup.LOCATION);
				locationGroupPermissions.add(Permission.ACCESS_COARSE_LOCATION);
				locationGroupPermissions.add(Permission.ACCESS_FINE_LOCATION);

				// Microphone group
				sMarshmallowPermissionGroupToPermissionsMap.put(PermissionGroup.MICROPHONE, new java.util.ArrayList<String>());
				java.util.ArrayList<String> microphoneGroupPermissions = 
					sMarshmallowPermissionGroupToPermissionsMap.get(PermissionGroup.MICROPHONE);
				microphoneGroupPermissions.add(Permission.RECORD_AUDIO);

				// Phone group
				sMarshmallowPermissionGroupToPermissionsMap.put(PermissionGroup.PHONE, new java.util.ArrayList<String>());
				java.util.ArrayList<String> phoneGroupPermissions = 
					sMarshmallowPermissionGroupToPermissionsMap.get(PermissionGroup.PHONE);
				phoneGroupPermissions.add(Permission.READ_PHONE_STATE);
				phoneGroupPermissions.add(Permission.CALL_PHONE);
				if (android.os.Build.VERSION.SDK_INT >= 16) {
					phoneGroupPermissions.add(Permission.READ_CALL_LOG);
					phoneGroupPermissions.add(Permission.WRITE_CALL_LOG);
				}
				if (android.os.Build.VERSION.SDK_INT >= 14) {
					phoneGroupPermissions.add(Permission.ADD_VOICEMAIL);
				}
				phoneGroupPermissions.add(Permission.USE_SIP);
				phoneGroupPermissions.add(Permission.PROCESS_OUTGOING_CALLS);

				// Body Sensors group
				if (android.os.Build.VERSION.SDK_INT >= 20) {
					sMarshmallowPermissionGroupToPermissionsMap.put(PermissionGroup.SENSORS, new java.util.ArrayList<String>());
					java.util.ArrayList<String> sensorsGroupPermissions = 
						sMarshmallowPermissionGroupToPermissionsMap.get(PermissionGroup.SENSORS);
					sensorsGroupPermissions.add(Permission.BODY_SENSORS);
				}

				// SMS group
				sMarshmallowPermissionGroupToPermissionsMap.put(PermissionGroup.SMS, new java.util.ArrayList<String>());
				java.util.ArrayList<String> smsGroupPermissions = 
					sMarshmallowPermissionGroupToPermissionsMap.get(PermissionGroup.SMS);
				smsGroupPermissions.add(Permission.SEND_SMS);
				smsGroupPermissions.add(Permission.RECEIVE_SMS);
				smsGroupPermissions.add(Permission.READ_SMS);
				smsGroupPermissions.add(Permission.RECEIVE_WAP_PUSH);
				smsGroupPermissions.add(Permission.RECEIVE_MMS);
				if (android.os.Build.VERSION.SDK_INT >= 16) {
					// WARNING: This permission is hidden in the Android source code!
					smsGroupPermissions.add(Permission.READ_CELL_BROADCASTS);
				}

				// Storage group
				sMarshmallowPermissionGroupToPermissionsMap.put(PermissionGroup.STORAGE, new java.util.ArrayList<String>());
				java.util.ArrayList<String> storageGroupPermissions = 
					sMarshmallowPermissionGroupToPermissionsMap.get(PermissionGroup.STORAGE);
				if (android.os.Build.VERSION.SDK_INT >= 16) {
					storageGroupPermissions.add(Permission.READ_EXTERNAL_STORAGE);
				}
				storageGroupPermissions.add(Permission.WRITE_EXTERNAL_STORAGE);			
			}
		}
	}

	/**
	 * Searches the <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a>
	 * for all permissions within the desired permission group.
	 * @param permissionGroup The permission group we're looking for the presence of.
	 * @return Returns all permissions in the 
	 * 		   <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a> 
	 * 		   that match the desired permission group.
	 *         <p>
	 *         Returns null if no permission in the desired permission group is in the 
	 *		   <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a> 
	 *		   or we're in an invalid environment.
	 */
	public String[] findAllPermissionsInManifestForGroup(String permissionGroup) {
		// TODO: HAVE THIS BE COMPLIANT WITH ANDROID 6 GROUPS ON LEGACY THINGS IF NEEDED!
		// Validate environment.
		android.content.Context context = getApplicationContext();
		if (context == null) return null;

		android.content.pm.PackageManager packageManager = context.getPackageManager();
		if (packageManager == null) return null;

		// Grab all <uses-permission> tags in the AndroidManifest.xml.
		String[] requestedPermissions = getRequestedPermissions();

		if (requestedPermissions != null && requestedPermissions.length > 0) {
			java.util.ArrayList<String> allPermissionsInGroup = new java.util.ArrayList<String>();

			// For each permission used:
			for (int permissionIdx = 0; permissionIdx < requestedPermissions.length; permissionIdx++) {
				// Get the permission info object if possible.
				android.content.pm.PermissionInfo permissionInfo = null;
				try {
					permissionInfo = packageManager.getPermissionInfo(requestedPermissions[permissionIdx], 0);
				} catch (Exception ex) {
					// This permission doesn't exist on this device. Move on.
				}

				// Check the permission group attribute:
				// if part of the desired permission group. return the permission!
				if (permissionInfo != null && permissionInfo.group != null && permissionInfo.group.equals(permissionGroup)) {
					allPermissionsInGroup.add(requestedPermissions[permissionIdx]);
				}
			}

			// Return null if no permission in the desired group were found.
			if (allPermissionsInGroup.isEmpty()) {
				return null;
			} else {
				return allPermissionsInGroup.toArray(new String[0]);
			}
		}

		// No permissions found in the desired group.
		return null;		
	}

	/**
	 * Searches the <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a>
	 * for a permission within the desired permission group.
	 * @param permissionGroup The permission group we're looking for the presence of.
	 * @return Returns the first permission in the 
	 * 		   <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a> 
	 * 		   that matches the desired permission group.
	 *         <p>
	 *         Returns null if no permission in the desired permission group is in the 
	 *		   <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a> 
	 *		   or we're in an invalid environment.
	 */
	public String findPermissionInManifestForGroup(String permissionGroup) {
		// TODO: HAVE THIS BE COMPLIANT WITH ANDROID 6 GROUPS ON LEGACY THINGS IF NEEDED!
		String[] allPermissionsInGroup = findAllPermissionsInManifestForGroup(permissionGroup);
		return (allPermissionsInGroup == null) ? null : allPermissionsInGroup[0];
	}

	/**
	 * Determines the current state of the desired permission and returns it.
	 * @param permission The permission we want the current state of.
	 * @return Returns a {@link com.ansca.corona.permissions.PermissionState PermissionState enum} 
	 *		   depicting what state the desired permission is in.
	 * 		   <p>
	 * 		   Returns null if we are in an invalid environemnt.
	 */
	public PermissionState getPermissionStateFor(String permission) {
		// Validate environment.
		android.content.Context context = getApplicationContext();
		if (context == null) return null;

		// First we check the AndroidManifest.xml for the permission.
		if (isPermissionInManifest(permission)) {
			// If it's in the manifest, then we see if it's granted or denied.
			return checkCallingAndSelfPermission(permission);
		}

		return PermissionState.MISSING;
	}

	/**
	 * Determines the current state of the desired permission group supported by Corona and returns it.
	 * @param permissionGroup The permission group we want the current state of.
	 * @return Returns a {@link com.ansca.corona.permissions.PermissionState PermissionState enum} 
	 *		   depicting what state the desired permission group is in.
	 * 		   <p>
	 *		   Will return {@link com.ansca.corona.permissions.PermissionState#GRANTED PermissionState.GRANTED}
	 * 		   if all requested permissions in that group are granted.
	 *		   <p>
	 *		   If any of the requested permissions in the desired permission group are denied, 
	 *		   then this method will return {@link com.ansca.corona.permissions.PermissionState#DENIED PermissionState.DENIED}.
	 *		   <p>
	 *		   Will return {@link com.ansca.corona.permissions.PermissionState#MISSING PermissionState.MISSING} 
	 *		   if this group isn't supported by Corona or for this version of Android.
	 *		   <p>
	 * 		   Returns null if we are in an invalid environemnt.
	 */
	public PermissionState getPermissionStateForSupportedGroup(String permissionGroup) {
		synchronized(PermissionsServices.class) {
			// Validate environment.
			android.content.Context context = getApplicationContext();
			if (context == null) return null;

			if (!isSupportedPermissionGroupName(permissionGroup)) return PermissionState.MISSING;

			// Grab the permissions for this group.
			java.util.ArrayList<String> desiredGroupPermissions = null;
			String[] desiredGroupPermissionsArray = null;
			desiredGroupPermissions = sMarshmallowPermissionGroupToPermissionsMap.get(permissionGroup);
			if (desiredGroupPermissions == null) {
				// This permission group isn't supported on this version of Android.
				return PermissionState.MISSING;
			} else {
				desiredGroupPermissionsArray = desiredGroupPermissions.toArray(new String[0]);
			}

			// Grab permission state for each permission in this group.
			int requestedPermissionCount = desiredGroupPermissionsArray.length;
			int grantedPermissionCount = 0;
			for (int permissionIdx = 0; permissionIdx < desiredGroupPermissionsArray.length; permissionIdx++) {
				switch (getPermissionStateFor(desiredGroupPermissionsArray[permissionIdx])) {
					case DENIED:
						return PermissionState.DENIED;
					case MISSING:
						requestedPermissionCount--;
						break;
					default:
						// Permission is granted!
						grantedPermissionCount++;
						break;
				} 
			}

			// Now, if requested and granted permission counts match, 
			// we know that all requested permissions have been granted!
			if (requestedPermissionCount == grantedPermissionCount) {
				return PermissionState.GRANTED;
			}

			// Something went wrong in our environment, return null!
			return null;
		}
	}

	/**
	 * Gets the permission group for the desired permission.
	 * @param permission The permission whose group we want.
	 * @return Returns the permission group for the desired permission.
	 *		   <p>
	 *		   Returns null if this permission doesn't belong to a group, is not defined on this device,
	 *		   or we're in an invalid environment.
	 */
	public String getPermissionGroupFromPermission(String permission) {
		// Validate environment.
		android.content.Context context = getApplicationContext();
		if (context == null) return null;

		android.content.pm.PackageManager packageManager = context.getPackageManager();
		if (packageManager == null) return null;

		try {
			android.content.pm.PermissionInfo permissionInfo = packageManager.getPermissionInfo(permission, 0);
			return permissionInfo.group;
		} catch (Exception ex) {
			// This permission doesn't exist on this device. Move on.
		}

		return null;
	}

	/**
	 * <!-- TODO: Link to doc explaining what platform-agnostic app permission names vs regular names is -->
	 * Gets the Permission Group name for the given Platform-Agnostic App Permission name.
	 * @param pAAppPermissionName The Platform-Agnostic App Permission name to convert to a Permission Group name.
	 * @return Returns the Permission Group name for the given Platform-Agnostic App Permission.
	 *		   <p>
	 *		   Returns null if there's no permission group for this Platform-Agnostic App Permission.
	 */
	public String getPermissionGroupFromPAAppPermissionName(String pAAppPermissionName) {
		synchronized(PermissionsServices.class) {
			String lowerCaseName = pAAppPermissionName.toLowerCase();
			return sPlatformAgnosticPermissionGroupMap.get(lowerCaseName);
		}
	}

	/**
	 * <!-- TODO: Link to doc explaining what platform-agnostic app permission names vs regular names is -->
	 * Gets the Platform-Agnostic App Permission name for the given Permission Group name.
	 * @param permissionGroup The Permission Group name to convert to a Platform-Agnostic App Permission name.
	 * @return Returns the Platform-Agnostic App Permission name for the given Permission Group name.
	 *		   <p>
	 *		   Returns null if there's no Platform-Agnostic App Permission name for this Permission Group name.
	 */
	public String getPAAppPermissionNameFromPermissionGroup(String permissionGroup) {
		synchronized(PermissionsServices.class) {
			return sPermissionGroupPlatformAgnosticMap.get(permissionGroup);
		}
	}

	/**
	 * <!-- TODO: Link to doc explaining what platform-agnostic app permission names vs regular names is -->
	 * Gets the Platform-Agnostic App Permission name for the given Android permission.
	 * @param permission The Android permission to convert to a Platform-Agnostic App Permission name.
	 * @return Returns the Platform-Agnostic App Permission name for the given Android permission.
 	 *		   <p>
	 *		   Returns null if there's no Platform-Agnostic App Permission for this Android permission.
	 */
	public String getPAAppPermissionNameFromAndroidPermission(String permission) {
		synchronized(PermissionsServices.class) {
			String permissionGroupForPermission = null;
			if (android.os.Build.VERSION.SDK_INT >= 23) {
				permissionGroupForPermission = getPermissionGroupFromPermission(permission);
			} else {
				// For legacy Android versions, the Permission -> Permission Group map is different,
				// so we consult the advertised mapping.
				permissionGroupForPermission = sMarshmallowPermissionToPermissionGroupMap.get(permission);
			}
			return getPAAppPermissionNameFromPermissionGroup(permissionGroupForPermission);
		}
	}

	/**
	 * <!-- TODO: Not overload the term "requested permissions" here -->
	 * Grabs all the requested permissions in the 
	 * <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a>. 
	 * These are all the <uses-permission> tags.
	 * @return Returns a {@link java.lang.String String} array with the name of each <uses-permission> found in the 
	 * 		   <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a>.
	 *         <p>
	 *         Returns null if no <uses-permission> tags are in the
	 *		   <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a> 
	 *		   or we're in an invalid environment.
	 *         <p>
	 */
	public String[] getRequestedPermissions() {
		synchronized(PermissionsServices.class) {
			// Validate environment.
			android.content.Context context = getApplicationContext();
			if (context == null) return null;

			android.content.pm.PackageManager packageManager = context.getPackageManager();
			if (packageManager == null) return null;

			try {
				if (sRequestedPermissions == null) {
					// GetPackageInfo will return null in the event of no permissions requested!
					sRequestedPermissions = packageManager.getPackageInfo(
						context.getPackageName(), android.content.pm.PackageManager.GET_PERMISSIONS).requestedPermissions;
				}
			} catch (Exception ex) {
				ex.printStackTrace();
			}

			if (sRequestedPermissions != null) {
				// Return a deep copy of the requested permissions!
				// Since strings are immutable, just calling clone() is ok.
				return sRequestedPermissions.clone();
			}

			return null;
		}
	}

	/**
	 * Gets all the requested permissions in the 
	 * <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a> in the desired state.
	 * <p>
	 * <b>Note:</b> "Requested Permissions" are those defined in the 
	 * <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a>. As such, calling this 
	 * method with the argument {@link com.ansca.corona.permissions.PermissionState#MISSING "PermissionState.MISSING"} is invalid.
	 * @param permissionState A {@link com.ansca.corona.permissions.PermissionState} corresponding to the state of the requested 
	 * 						   permissions we wish to fetch.
	 * @return Returns a {@link java.lang.String String} array containing all the requested permissions in the desired state.
	 * 		   <p>
	 * 		   Returns null if given invalid arguments, in an invalid environemnt, or unable to get 
	 *		   <a href="http://developer.android.com/reference/android/content/pm/PackageInfo.html">PackageInfo</a> 
	 *		   from the Android 
	 *		   <a href="http://developer.android.com/reference/android/content/pm/PackageManager.html">PackageManager</a>.
	 */
	public String[] getRequestedPermissionsInState(PermissionState permissionState) {
		// Validate arguments.
		if (permissionState == PermissionState.MISSING) {
			android.util.Log.i("Corona", "ERROR: PermissionServices.getRequestedPermissionsInState(): Requested Permissions cannot by in the state, MISSING!");
			return null;
		}

		// Validate environment.
		android.content.Context context = getApplicationContext();
		if (context == null) return null;

		android.content.pm.PackageManager packageManager = context.getPackageManager();
		if (packageManager == null) return null;

		java.util.ArrayList<String> grantedPermissions = new java.util.ArrayList<String>();
		java.util.ArrayList<String> deniedPermissions = new java.util.ArrayList<String>();

		// Grab list of all requested permissions; the <uses-permission> tags in AndroidManifest.xml.
		String[] requestedPermissions = getRequestedPermissions();

		// Filter the granted and denied permissions.
		// Permissions can be granted or denied. Find out the status.
		try {
			// We have to manually check each requested permission.
			for (int permissionIdx = 0; permissionIdx < requestedPermissions.length; permissionIdx++) {
				
				if (checkCallingAndSelfPermission(requestedPermissions[permissionIdx]) == PermissionState.GRANTED) {
					grantedPermissions.add(requestedPermissions[permissionIdx]);
	        	} else {
					deniedPermissions.add(requestedPermissions[permissionIdx]);
	        	}
			}

			// Return the desired data
			if (permissionState == PermissionState.GRANTED) {
				return grantedPermissions.toArray(new String[0]);
			} else {
				return deniedPermissions.toArray(new String[0]);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return null;
	}

	/**
	 * Gets all the requested permissions in the 
	 * <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a> 
	 * in the desired state for the desired Permission Group.
	 * <p>
	 * <b>Note:</b> "Requested Permissions" are those defined in the 
	 * <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a>. As such, calling this 
	 * method with the argument {@link com.ansca.corona.permissions.PermissionState#MISSING "PermissionState.MISSING"} is invalid.
	 * @param permissionState A {@link com.ansca.corona.permissions.PermissionState} corresponding to the state of the requested 
	 * 						   permissions we wish to fetch.
	 * @param permissionGroup The Permission Group for which we want to get all the Android permissions in the 
	 *						  <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a>.
	 * @return Returns a {@link java.lang.String String} array containing all the requested permissions in the 
	 *		   desired state for the desired Permission Group.
	 * 		   <p>
	 * 		   Returns null if given invalid arguments, in an invalid environemnt, or unable to get 
	 *		   <a href="http://developer.android.com/reference/android/content/pm/PackageInfo.html">PackageInfo</a> 
	 *		   from the Android 
	 *		   <a href="http://developer.android.com/reference/android/content/pm/PackageManager.html">PackageManager</a>.
	 */
	public String[] getRequestedPermissionsInStateForGroup(PermissionState permissionState, String permissionGroup) {
		// Validate arguments.
		if (permissionState == PermissionState.MISSING) {
			android.util.Log.i("Corona", "ERROR: PermissionServices.getRequestedPermissionsInStateForGroup(): Requested Permissions cannot by in the state, MISSING!");
			return null;
		}

		if (!isSupportedPermissionGroupName(permissionGroup)) {
			android.util.Log.i("Corona", "ERROR: PermissionServices.getRequestedPermissionsInStateForGroup(): The supplied PermissionGroup name is not supported.");
			return null;
		}

		// Validate environment.
		android.content.Context context = getApplicationContext();
		if (context == null) {
			return null;
		}

		android.content.pm.PackageManager packageManager = context.getPackageManager();
		if (packageManager == null) {
			return null;
		}

		java.util.ArrayList<String> grantedPermissions = new java.util.ArrayList<String>();
		java.util.ArrayList<String> deniedPermissions = new java.util.ArrayList<String>();

		// Grab list of all requested permissions; the <uses-permission> tags in AndroidManifest.xml.
		String[] requestedPermissions = findAllPermissionsInManifestForGroup(permissionGroup);

		// Filter the granted and denied permissions.
		// Permissions can be granted or denied. Find out the status.
		try {
			// We have to manually check each requested permission.
			for (int permissionIdx = 0; permissionIdx < requestedPermissions.length; permissionIdx++) {
				
				if (checkCallingAndSelfPermission(requestedPermissions[permissionIdx]) == PermissionState.GRANTED) {
					grantedPermissions.add(requestedPermissions[permissionIdx]);
	        	} else {
					deniedPermissions.add(requestedPermissions[permissionIdx]);
	        	}
			}

			// Return the desired data
			if (permissionState == PermissionState.GRANTED) {
				return grantedPermissions.toArray(new String[0]);
			} else {
				return deniedPermissions.toArray(new String[0]);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return null;


	}
	/**
	 * Returns a {@link java.lang.String String} array of all dangerous-level permissions groups supported by Corona.
	 */
	public String[] getSupportedPermissionGroups() {
		synchronized(PermissionsServices.class) {
			return sDangerousPermissionGroups;
		}
	}

	/**
	 * <!-- TODO: Link to doc explaining what platform-agnostic app permission names vs regular names is -->
	 * Determines if this Android permission is part of a Platform-Agnostic App Permission.
	 * @param permission The Android permission to check.
	 * @return Returns whether this Android permission is part of a Platform-Agnostic App Permission.
	 */
	public boolean isPartOfPAAppPermission(String permission) {
		synchronized(PermissionsServices.class) {
			String permissionGroupForPermission = null;
			if (android.os.Build.VERSION.SDK_INT >= 23) {
				permissionGroupForPermission = getPermissionGroupFromPermission(permission);
			} else {
				// For legacy Android versions, the Permission -> Permission Group map is different,
				// so we consult the advertised mapping.
				permissionGroupForPermission = sMarshmallowPermissionToPermissionGroupMap.get(permission);
			}
			return sPermissionGroupPlatformAgnosticMap.containsKey(permissionGroupForPermission);
		}
	}

	/**
	 * <!-- TODO: Link to doc explaining what platform-agnostic app permission names are -->
	 * Determines if this name corresponds to the name of a Platform-Agnostic App Permission.
	 * @param name The name we're checking.
	 * @return Returns true if this name corresponds to the name of a Platform-Agnostic App Permission.
	 */
	public boolean isPAAppPermissionName(String name) {
		synchronized(PermissionsServices.class) {
			String lowerCaseName = name.toLowerCase();
			return sPlatformAgnosticPermissionGroupMap.containsKey(lowerCaseName);
		}
	}

	/**
	 * Determines if this name corresponds to the name of a dangerous-level Android Permission Group supported by Corona.
	 * @param name The name we're checking.
	 * @return Returns true if this name corresponds to the name of a dangerous-level Android Permission Group supported by Corona.
	 */
	public boolean isSupportedPermissionGroupName(String name) {
		synchronized(PermissionsServices.class) {
			for (int groupIdx = 0; groupIdx < sDangerousPermissionGroups.length; groupIdx++) {
				if (sDangerousPermissionGroups[groupIdx].equals(name)) return true;
			}
			return false;
		}
	}

	/**
	 * Determines if the desired Android permission is in the 
	 * <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a>.
	 * @param permission The Android permission we're looking for.
	 * @return Returns true if the desired permission was found in the 
	 * <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a>; false otherwise.
	 */
	public boolean isPermissionInManifest(String permission) {
		String[] requestedPermissions = getRequestedPermissions();
		if (requestedPermissions != null) {
			for (int permissionIdx = 0; permissionIdx < requestedPermissions.length; permissionIdx++) {
				if (requestedPermissions[permissionIdx].equals(permission)) return true;
			}
		}

		return false;
	}

	/**
	 * Requests the desired permissions, gathering data about which permissions are requested.
	 * <p>
	 * Wraps <a href="http://developer.android.com/reference/android/app/Activity.html#requestPermissions(java.lang.String[], int)">Activity.requestPermissions()</a>.
	 * @param settings The {@link com.ansca.corona.permissions.PermissionsSettings PermissionsSettings} used for this request.
	 *				   Cannot be null!
	 * @param resultHandler The 
	 *						{@link com.ansca.corona.CoronaActivity.OnRequestPermissionsResultHandler OnRequestPermissionsResultHandler} 
	 * 						that will be used to handle this permissions request.
	 */
	public final void requestPermissions(PermissionsSettings settings, 
		com.ansca.corona.CoronaActivity.OnRequestPermissionsResultHandler resultHandler) {
		
		// Validate.
		if (android.os.Build.VERSION.SDK_INT < 23) {
			android.util.Log.i("Corona", "ERROR: PermissionServices.requestPermissions(): Permissions can only be requested for Android 6 and above!");
			return;
		}

		// Request the desired permissions!
		ApiLevel23.requestPermissions(settings, resultHandler, this);

	}

	/**
	 * Requests all permissions that are currently denied in the desired permission group.
	 * @param permissionGroup The Permission Group we want to request all denied permissions for.
	 * @param resultHandler The 
	 *						{@link com.ansca.corona.CoronaActivity.OnRequestPermissionsResultHandler OnRequestPermissionsResultHandler} 
	 * 						that will be used to handle this permissions request.
	 */
	public void requestAllDeniedPermissionsInGroup(String permissionGroup, 
		com.ansca.corona.CoronaActivity.OnRequestPermissionsResultHandler resultHandler) {
		// Get the status of each permission in this group.
		// Gather up all the denied ones.
		// Queue up a permission request for all the denied ones.
		// Request all the denied ones.

		// Build up a set of all permissions to request for the sync.
		java.util.LinkedHashSet<String> permissionsSet = new java.util.LinkedHashSet<String>();

		// Get all requested permissions in the AndroidManifest.xml for this group.
		String[] allPermissionsInGroup = findAllPermissionsInManifestForGroup(permissionGroup);

		if (allPermissionsInGroup != null && allPermissionsInGroup.length > 0) {
			// Check for descrepencies in permission state.
			for (int permissionIdx = 0; permissionIdx < allPermissionsInGroup.length; permissionIdx++) {
				PermissionState currentPermissionState = getPermissionStateFor(allPermissionsInGroup[permissionIdx]);
				if (currentPermissionState == PermissionState.DENIED) {
					permissionsSet.add(allPermissionsInGroup[permissionIdx]);
				}
			}
		} else {
			android.util.Log.v("Corona", "PermissionsServices.requestAllDeniedPermissionInGroup(): No permissions in group!");
		}

		if (!permissionsSet.isEmpty()) {
			// Request all desired permissions for sync!
			requestPermissions(new PermissionsSettings(permissionsSet), resultHandler);
		} else {
			android.util.Log.v("Corona", "PermissionsServices.requestAllDeniedPermissionInGroup(): Didn't find any permissions to request!");
		}
	}

	/** <!-- TODO: Add gotcha regarding using the native permission APIs not being compatible with this -->
	 * Determines if the user has requested the desired permission never be asked for again.
	 * @param permission The Android permission we want to know if it should never be requested again.
	 * @return Returns true if the user has requested the desired permission never be asked for 
	 *		   again, we're not on an Android 6 and above device, or we're in an invalid environment. False otherwise.
	 */
	public boolean shouldNeverAskAgain(String permission) {
		if (android.os.Build.VERSION.SDK_INT < 23) {
			// You shouldn't be asking for permissions below Android 6 in the first place.
			return true;
		}
		return ApiLevel23.shouldNeverAskAgain(permission, this);
		
	}

	/**
	 * Displays a native alert stating that this permission is missing from the 
	 * <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a>.
	 * @param permission The name of the permission that's missing.
	 * @param message A message to explain why the action can't be performed.
	 */
	public void showPermissionMissingFromManifestAlert(String permission, String message) {
		final com.ansca.corona.CoronaActivity activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();
		if (activity == null) {
			android.util.Log.v("Corona", "PermissionServices.showPermissionMissingFromManifestAlert(): CoronaActivity is null!");
			return;
		}

		activity.showPermissionMissingFromManifestAlert(permission, message);
	}

	/**
	 * Displays a native alert stating that the 
	 * <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a>
	 * doesn't contain any permissions from the desired permission group.
	 * @param permissionGroup The name of the permission group that's there's no permissions for.
	 */
	public void showPermissionGroupMissingFromManifestAlert(String permissionGroup) {
		final com.ansca.corona.CoronaActivity activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();
		if (activity == null) {
			android.util.Log.v("Corona", "PermissionServices.showPermissionGroupMissingFromManifestAlert(): CoronaActivity is null!");
			return;
		}

		activity.showPermissionGroupMissingFromManifestAlert(permissionGroup);
	}

	/**
	 * Checks if a permission has been granted, providing explanation for why it's needed or requesting it if needed.
	 * @param permission The name of the permission being checked.
	 * @return Returns PermissionState.GRANTED if this permission is granted and safe from permission-leak.
	 *		   <p>
	 *		   PermissionState.DENIED if this permission is denied.
	 *		   <p>
	 *		   Returns null if we're in an invalid environment.
	 */
	private PermissionState checkCallingAndSelfPermission(String permission) {
		// Mimic Android behavior with validating arguments.
		if (permission == null) {
            throw new IllegalArgumentException("permission is null");
        }

        android.content.Context context = getApplicationContext();
		if (context == null) return null;

        // Determine who the caller is.
        int callingPid = android.os.Binder.getCallingPid();
        boolean callingPidIsSelf = callingPid == android.os.Process.myPid();

        if (callingPidIsSelf) {
        	int permissionState = context.checkCallingOrSelfPermission(permission);
        	if (permissionState == android.content.pm.PackageManager.PERMISSION_GRANTED) {
        		return PermissionState.GRANTED;
        	} else {
        		return PermissionState.DENIED;
        	}
        } else {
        	// We actually have a separate calling IPC to consider.
        	// if caller & self are granted, then yay. Otherwise, boo!
        	int clientPermissionState = context.checkCallingPermission(permission);
        	int selfPermissionState = context.checkPermission(permission, android.os.Process.myPid(), android.os.Process.myUid());

        	if (clientPermissionState == selfPermissionState) {
        		if (selfPermissionState == android.content.pm.PackageManager.PERMISSION_GRANTED) {
	        		return PermissionState.GRANTED;
	        	} else {
	        		return PermissionState.DENIED;
	        	}
        	} else if (clientPermissionState == android.content.pm.PackageManager.PERMISSION_DENIED
        		&& selfPermissionState == android.content.pm.PackageManager.PERMISSION_GRANTED) {
        		// Another app is trying to exploit our apps leaking of customer permissions. Don't let them!
        		throw new SecurityException(
						"The app with process id: " + android.os.Binder.getCallingPid() + 
						" is trying to piggy back off of this app's " 
						+ permission + " permission!" );
        	} else {
        		return PermissionState.DENIED;
        	}
        }
	}

	/**
	 * Provides access to API Level 11 (Android 3.0 Honeycomb) features.
	 * Should only be accessed if running on an operating system matching this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * Instead, you are expected to call its static methods instead.
	 */
	private static class ApiLevel11 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel11() {}

		/**
		 * Initializes our cache of previously requested permissions.
		 * @param context Reference to an Android created context used to access the input device system.
	 	 *                <p>
	 	 *                Setting this to null will cause an exception to be thrown.
		 */
		public static void initializePreviouslyRequestedPermissions(android.content.Context context) {
			synchronized(ApiLevel11.class) {
				// Grab our SharedPreferences instance.
				sPermissionsPreferences = context.getSharedPreferences("CoronaPermissionsState", android.content.Context.MODE_PRIVATE);

				if (sPermissionsPreferences != null) {
					// Grab our Shared data!
					sPreviouslyRequestedPermissions = (java.util.HashSet<String>)sPermissionsPreferences.getStringSet(
						"PreviouslyRequestedPermissions", null);

					// Make a local copy of the set from SharedPreferences so we can modify it.
					if (sPreviouslyRequestedPermissions != null) {
						sPreviouslyRequestedPermissions = new java.util.HashSet<String>(sPreviouslyRequestedPermissions);
					} else {
						sPreviouslyRequestedPermissions = new java.util.HashSet<String>();
					}
				}
			}
		}

		/**
		 * Updates our cache of previously requested permissions based on info from the provided list of permissions.
		 * @param permissionsToAdd The new permissions being added to the cache.
		 */
		public static void updatePreviouslyRequestedPermissions(java.util.HashSet<String> permissionsToAdd) {
			synchronized(ApiLevel11.class) {
				// Log the permissions we're about to add.
				for (String permission : permissionsToAdd) {
					if (!sPreviouslyRequestedPermissions.contains(permission)) {
						sPreviouslyRequestedPermissions.add(permission);
					}
				}

				// Add the updated list to SharedPreferences if possible.
				if (sPermissionsPreferences != null) {
					android.content.SharedPreferences.Editor permissionsPreferenceEditor = sPermissionsPreferences.edit();
					permissionsPreferenceEditor.putStringSet("PreviouslyRequestedPermissions", sPreviouslyRequestedPermissions);
					permissionsPreferenceEditor.apply();
				}
			}
		}
	}

	/**
	 * Provides access to API Level 23 (Android 6.0 Marshmallow) features.
	 * Should only be accessed if running on an operating system matching this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * Instead, you are expected to call its static methods instead.
	 */
	private static class ApiLevel23 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel23() {}

		/**
		 * From http://developer.android.com/reference/android/app/Activity.html#requestPermissions(java.lang.String[], int):
		 *
		 * Requests permissions to be granted to this application. These permissions must be requested in your manifest, 
		 * they should not be granted to your app, and they should have protection level #PROTECTION_DANGEROUS dangerous, 
		 * regardless whether they are declared by the platform or a third-party app.
		 *
		 * @param settings The PermissionsSettings used for this request. Cannot be null!
	 	 * @param resultHandler The OnRequestPermissionsResultHandler that will be used to handle this permissions request. Cannot be null!
	 	 * @param services The PermissionsServices instance used for this request. Cannot be null!
		 */
		public static void requestPermissions(PermissionsSettings settings, 
			com.ansca.corona.CoronaActivity.OnRequestPermissionsResultHandler resultHandler,
			PermissionsServices services) {
			
			// Validate arguments!
			if (settings == null || resultHandler == null || services == null) {
				android.util.Log.i("Corona", "WARNING: PermissionServices.ApiLevel23.requestPermissions(): Invalid arguments!");
				return;
			}

			// Validate environment!
			final com.ansca.corona.CoronaActivity activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();
			if (activity == null) {
				android.util.Log.v("Corona", "PermissionServices.ApiLevel23.requestPermissions(): Cannot request permissions with null CoronaActivity!");
				return;
			}

			java.util.HashSet<String> permissionsToRequest = settings.getPermissions();
			if (permissionsToRequest == null || permissionsToRequest.isEmpty()) {
				// Empty permission request, just return!
				android.util.Log.v("Corona", "PermissionServices.ApiLevel23.requestPermissions(): No permissions to request!");
				return;
			}

			// Only request the permissions that are actually denied!
			java.util.HashSet<String> filteredPermissionsToRequest = new java.util.HashSet<String>();
			for (String permissionToRequest : permissionsToRequest) {
				switch(services.getPermissionStateFor(permissionToRequest)) {
					case MISSING:
						// Compose message about this app missing a permission it's trying to request!
						String applicationName = com.ansca.corona.CoronaEnvironment.getApplicationName();
						activity.showPermissionMissingFromManifestAlert(permissionToRequest, applicationName + " cannot request " + permissionToRequest + " because it's missing from the build.settings/AndroidManifest.xml!");
						return;
					case DENIED:
						filteredPermissionsToRequest.add(permissionToRequest);
						break;
					default:
						// Permission is already granted, so don't bother requesting it.
						break;
				}
			}

			// Only continue if something passed through the filter.
			if (filteredPermissionsToRequest.isEmpty()) {
				android.util.Log.v("Corona", "PermissionServices.ApiLevel23.requestPermissions(): All permissions that were requested have already been granted!");
				return;
			}

			ApiLevel11.updatePreviouslyRequestedPermissions(filteredPermissionsToRequest);

			activity.requestPermissions(filteredPermissionsToRequest.toArray(new String[0]),
				activity.registerRequestPermissionsResultHandler(resultHandler, settings));
		}

		/**
		 * Determines if the user has requested the desired permission never be asked for again.
		 * @param permission The permission we want to know if it should never be requested again.
		 * @param services The PermissionsServices instance to use.
		 * @return Returns true if the user has requested the desired permission never be asked for 
		 *		   again, we're not on an Android 6+ device, or we're in an invalid environment. False otherwise.
		 */
		public static boolean shouldNeverAskAgain(String permission, PermissionsServices services) {
			synchronized(ApiLevel23.class) {
				final com.ansca.corona.CoronaActivity activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();
				if (activity == null) {
					android.util.Log.v("Corona", "PermissionServices.ApiLevel23.shouldNeverAskAgain(): "
						+ "CoronaActivity is null!");
					return true;
				} else if (sPreviouslyRequestedPermissions == null) {
					android.util.Log.v("Corona", "PermissionServices.ApiLevel23.shouldNeverAskAgain(): Requested Permissions cache hasn't been initialized!");
					return true;
				}

				return sPreviouslyRequestedPermissions.contains(permission) &&
					services.getPermissionStateFor(permission) == PermissionState.DENIED &&
					!activity.shouldShowRequestPermissionRationale(permission);
			}
		}
	}
}