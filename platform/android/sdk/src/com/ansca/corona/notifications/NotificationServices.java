//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.notifications;


import android.content.pm.ApplicationInfo;

/**
 * Provides thread safe methods for posting and managing notifications.
 * <p>
 * Allows you to easily add, modify, and remove status bar notifications. Also allows you to
 * schedule notifications via the Android alarm manager to later appear in the status bar.
 * <p>
 * All notifications posted by this class will be persisted. This means that when this application
 * restarts, all last posted notifications will be redisplayed on the Android status bar and
 * rescheduled to the Android alarm manager. The intent is to match the iOS behavior where the
 * iOS operating system persists and restores local notifications.
 * @see ScheduledNotificationSettings
 * @see StatusBarNotificationSettings
 */
public final class NotificationServices extends com.ansca.corona.ApplicationContextProvider {
	/**
	 * The string this class tags to all posted status bar notifications by default.
	 * Used to uniquely identify notifications when posting them so that they will not
	 * conflict with 3rd party library status bar notifications.
	 */
	private static final String DEFAULT_CHANNEL_ID = "com.coronalabs.defaultChannel";


	/**
	 * Flag indicating if the Corona notification system has been initialized.
	 * <p>
	 * To be set true in the constructor only once after loading all notifications.
	 */
	private static boolean sWasInitialized = false;

	/** Stores all notification configurations that have been set up in Corona. */
	private static NotificationSettingsCollection<NotificationSettings> sNotificationCollection =
			new NotificationSettingsCollection<NotificationSettings>();

	/**
	 * Stores a collection of reserved unique notification IDs.
	 * <p>
	 * This collection is used to determine what unique integer IDs are reservered for Android
	 * status bar notifications and alarm manager scheduled notifications. This collection must
	 * contain the same notification IDs used in member variable "sNotificationCollection", but
	 * it can also contain additional IDs to be posted later.
	 */
	private static java.util.HashSet<Integer> sReservedNotificationIdSet = new java.util.HashSet<Integer>();


	/**
	 * Creates an object that provides access to the notification system.
	 * @param context Reference to an Android created context used to access the notification system.
	 *                <p>
	 *                Setting this to null will cause an exception to be thrown.
	 */
	public NotificationServices(android.content.Context context) {
		super(context);

		// Initialize the notification system if not done already.
		synchronized (NotificationServices.class) {
			if (sWasInitialized == false) {
				// Load all notifications from file to a temporary collection.
				NotificationSettingsCollection<NotificationSettings> loadedNotifications;
				loadedNotifications = new NotificationSettingsCollection<NotificationSettings>();
				loadSettingsTo(loadedNotifications);

				// Post all loaded notifications and store them to the main notification collection created above.
				// Note: The post() method will automatically update past due scheduled notifications to status bar
				//       notifications, store the updated notification to the main collection, and then save the
				//       changes to file.
				post(loadedNotifications);
				sWasInitialized = true;
			}
		}
	}

//	public static synchronized void setApplicationBadgeNumber(int value) {
//	}
//
//	public static synchronized int getApplicationBadgeNumber() {
//		return 0;
//	}

	/**
	 * Generates and reserves a unique notification ID to be used by a status bar or scheduled notification.
	 * <p>
	 * The returned ID is intended to be assigned to a StatusBarNotificationSettings or ScheduleNotificationSettings
	 * object to be posted by this class later. This ID will be "reserved", meaning that calling this method
	 * again will cause it to return a different ID.
	 * <p>
	 * If you need to use the Android "NotificationManager" class directly, then you should call this method
	 * so that this class will never use the same notification ID that you assign to the NotificationManager.
	 * <p>
	 * Note that removing a notification via this class will cause its associated notification ID to be
	 * unreserved and make its ID available again via this method.
	 * @return Returns a unique notification ID.
	 */
	public int reserveId() {
		synchronized (NotificationServices.class) {
			int id;
			for (id = 1; (id == 0) || (reserveId(id) == false); id++);
			return id;
		}
	}

	/**
	 * Attempts to reserve the given ID to be used by a status bar or scheduled notification.
	 * <p>
	 * The returned ID is intended to be assigned to a StatusBarNotificationSettings or ScheduleNotificationSettings
	 * object to be posted by this class later or via the Android "NotificationManager".
	 * <p>
	 * Note that removing a notification via this class will cause its associated notification ID to be
	 * unreserved and make its ID available again via this method.
	 * @param id The integer ID to attempt to reserve for a notification.
	 * @return Returns true if the ID was successfully reserved and can be used by a notification.
	 *         <p>
	 *         Returns false if the given ID has already been reserved by a status bar or scheduled notification
	 *         and is not unique. This means that the given ID should not be used by a new notification because
	 *         it will overwrite an existing one.
	 */
	public boolean reserveId(int id) {
		synchronized (NotificationServices.class) {
			return sReservedNotificationIdSet.add(Integer.valueOf(id));
		}
	}

	/**
	 * Determines if there are any notifications currently posted to the Android system.
	 * @return Returns true if there is at least one active notification.
	 *         <p>
	 *         Returns false if there are no notifications posted.
	 */
	public boolean hasNotifications() {
		synchronized (NotificationServices.class) {
			return (sNotificationCollection.size() > 0);
		}
	}

	/**
	 * Fetches a notification that was posted by this class by its unique integer ID.
	 * @param id The unique integer ID of the notification.
	 * @return Returns the requested notification's configuration.
	 *         <p>
	 *         Returns null if a notification having the given ID could not be found.
	 */
	public NotificationSettings fetchById(int id) {
		synchronized (NotificationServices.class) {
			return fetchById(NotificationSettings.class, id);
		}
	}

	/**
	 * Fetches a notification that was posted by this class by its unique integer ID and notification type.
	 * @param type The type of notification to fetch.
	 *             Must be set to one of the following:
	 *             <ul>
	 *              <li> ScheduledNotificationSettings.class
	 *              <li> StatusBarNotificationSettings.class
	 *             </ul>
	 * @param id The unique integer ID of the notification.
	 * @return Returns the requested notification's configuration.
	 *         <p>
	 *         Returns null if a notification of the given type and ID could not be found.
	 */
	public <T extends NotificationSettings> T fetchById(Class<T> type, int id) {
		synchronized (NotificationServices.class) {
			if (type != null) {
				NotificationSettings settings = sNotificationCollection.getById(id);
				if (type.isInstance(settings)) {
					return (T)settings.clone();
				}
			}
			return null;
		}
	}

	/**
	 * Fetches a scheduled notification that was posted by this class by its ID.
	 * @param id The unique integer ID assigned to the scheduled notification.
	 * @return Returns the configuration of the specified scheduled notification.
	 *         <p>
	 *         Returns null if the a scheduled notification having the given ID could not be found.
	 *         Will also return null if the scheduled notification has been triggered and it has become
	 *         a status bar notification.
	 */
	public ScheduledNotificationSettings fetchScheduledNotificationById(int id) {
		synchronized (NotificationServices.class) {
			return fetchById(ScheduledNotificationSettings.class, id);
		}
	}

	/**
	 * Fetches a status bar notification that was posted by this class by its ID.
	 * @param id The unique integer ID assigned to the status bar notification.
	 * @return Returns the configuration of the specified status bar notification.
	 *         <p>
	 *         Returns null if a status bar notification having the given ID could not be found.
	 */
	public StatusBarNotificationSettings fetchStatusBarNotificationById(int id) {
		synchronized (NotificationServices.class) {
			return fetchById(StatusBarNotificationSettings.class, id);
		}
	}

	/**
	 * Fetches all scheduled notifications that have been posted by this class and have not triggered yet.
	 * @return Returns a collection of scheduled notifications that have not triggered yet.
	 *         Note that notifications that have been scheduled outside of this class via the Android
	 *         "AlarmManager" class will not be in this collection.
	 *         <p>
	 *         Returns an empty collection if no pending scheduled notifications could be found.
	 */
	public NotificationSettingsCollection<ScheduledNotificationSettings> fetchScheduledNotifications() {
		synchronized (NotificationServices.class) {
			return fetchNotificationsByType(ScheduledNotificationSettings.class);
		}
	}

	/**
	 * Fetches all status bar notifications that have been posted by this class, including scheduled notifications
	 * that have been triggered and have become status bar notifications.
	 * @return Returns a collection of status bar notifications that have been posted by this class
	 *         and are currently shown on the status bar. Note that notifications displayed on the status bar
	 *         outside of this class via the Android "NotificationManager" class will not be in this collection.
	 *         <p>
	 *         Returns an empty collection if no status bar notifications were found.
	 */
	public NotificationSettingsCollection<StatusBarNotificationSettings> fetchStatusBarNotifications() {
		synchronized (NotificationServices.class) {
			return fetchNotificationsByType(StatusBarNotificationSettings.class);
		}
	}

	/**
	 * Fetches all notifications that were posted by this class by the given type.
	 * @param type The type of notifications to fetch.
	 *             Must be set to one of the following:
	 *             <ul>
	 *              <li> NotificationSettings.class
	 *              <li> ScheduledNotificationSettings.class
	 *              <li> StatusBarNotificationSettings.class
	 *             </ul>
	 * @return Returns a collection of active notifications that were posted by this class.
	 *         <p>
	 *         Returns an empty collection of notification of the given type could not be found.
	 */
	private <T extends NotificationSettings> NotificationSettingsCollection<T> fetchNotificationsByType(Class<T> type) {
		synchronized (NotificationServices.class) {
			NotificationSettingsCollection<T> collection;

			collection = new NotificationSettingsCollection<T>();
			if (type != null) {
				for (NotificationSettings settings : sNotificationCollection) {
					if (type.isInstance(settings)) {
						collection.add((T)settings.clone());
					}
				}
			}
			return collection;
		}
	}

	/**
	 * Adds a new notification or updates an existing notification with the given settings in the Android system.
	 * <p>
	 * Notifications posted by this class will be persisted. This means that when this application is restarted,
	 * all previously posted notifications will be restored (rescheduled and displayed in the status bar) to
	 * match the iOS notification behavior.
	 * <p>
	 * The given notification settings must be assigned a unique ID, which can be acquired by calling
	 * the reserveId() method. This ID is assigned to the notification that is posted to the Android
	 * system and is used to modify and remove the notification.
	 * <p>
	 * If the given notification has been posted before, then posting it again will cause its existing
	 * notification settings to be updated with the given settings.
	 * @param settings The notification settings to use when creating or updating an Android notification.
	 *                 <p>
	 *                 Settings this to null will cause this method to do nothing.
	 * @see ScheduledNotificationSettings
	 * @see StatusBarNotificationSettings
	 */
	public void post(NotificationSettings settings) {
		synchronized (NotificationServices.class) {
			// Validate.
			if (settings == null) {
				return;
			}

			// Post the notification to the Android system and store it in the main collection.
			postAndUpdateCollectionWith(settings);

			// Update the configuration file.
			saveSettings(sNotificationCollection);
		}
	}

	/**
	 * Posts all of the given notifications to the Android system.
	 * <p>
	 * Notifications posted by this class will be persisted. This means that when this application is restarted,
	 * all previously posted notifications will be restored (rescheduled and displayed in the status bar) to
	 * match the iOS notification behavior.
	 * <p>
	 * The given notification settings must be assigned a unique ID, which can be acquired by calling
	 * the reserveId() method. This ID is assigned to the notification that is posted to the Android
	 * system and is used to modify and remove the notification.
	 * <p>
	 * If the given notification has been posted before, then posting it again will cause its existing
	 * notification settings to be updated with the given settings.
	 * @param collection A collection of notification settings to be posted to the Android system.
	 *                   <p>
	 *                   Settings this to null will cause this method to do nothing.
	 * @see ScheduledNotificationSettings
	 * @see StatusBarNotificationSettings
	 */
	public <T extends NotificationSettings> void post(Iterable<T> collection) {
		synchronized (NotificationServices.class) {
			// Validate.
			if (collection == null) {
				return;
			}

			// Post all given notifications.
			for (NotificationSettings settings : collection) {
				postAndUpdateCollectionWith(settings);
			}

			// Update the configuration file.
			saveSettings(sNotificationCollection);
		}
	}

	/**
	 * Posts the given notification to the Android system and then adds it to this class' main collection.
	 * <p>
	 * Note that this method will not save to file. The intent is for this method to be called several times
	 * by this class when post multiple notifications at the same time.
	 * @param settings Reference to the notification configuration to be posted. If the notification has
	 *                 never been posted before, then the notification will be added to the system.
	 *                 If the notification has already been posted to the Android system, then its
	 *                 configuration in the Android system will be updated with the given settings.
	 */
	private void postAndUpdateCollectionWith(NotificationSettings settings) {
		synchronized (NotificationServices.class) {
			// Validate.
			if (settings == null) {
				return;
			}

			// If the given configuration is for a scheduled notification that is past due,
			// then don't bother scheduling it and post it as a status bar notification instead.
			if (settings instanceof ScheduledNotificationSettings) {
				ScheduledNotificationSettings scheduledSettings = (ScheduledNotificationSettings)settings;
				java.util.Date currentTime = new java.util.Date();
				if (scheduledSettings.getEndTime().compareTo(currentTime) <= 0) {
					settings = scheduledSettings.getStatusBarSettings();
				}
			}

			// Post the notification to the Android system.
			postSystemNotification(settings);

			// Update the collection with the given notification settings.
			NotificationSettings originalSettings = sNotificationCollection.getById(settings.getId());
			if (originalSettings == null) {
				// This is a new notification configuration. Add it to the collection.
				sReservedNotificationIdSet.add(Integer.valueOf(settings.getId()));
				sNotificationCollection.add(settings.clone());
			}
			else if ((originalSettings instanceof ScheduledNotificationSettings) &&
					(settings instanceof ScheduledNotificationSettings)) {
				// Notification ID exists. Copy the given settings to the existing settings.
				((ScheduledNotificationSettings)originalSettings).copyFrom((ScheduledNotificationSettings)settings);
			}
			else if ((originalSettings instanceof StatusBarNotificationSettings) &&
					(settings instanceof StatusBarNotificationSettings)) {
				// Notification ID exists. Copy the given settings to the existing settings.
				((StatusBarNotificationSettings)originalSettings).copyFrom((StatusBarNotificationSettings)settings);
			}
			else  {
				// Notification ID exists, but the types do not match.
				// Replace the old notification settings with the new one.
				removeSystemNotification(originalSettings);
				sNotificationCollection.remove(originalSettings);
				sNotificationCollection.add(settings.clone());
			}

			// Raise an event if we're posting a status bar notification and the Corona runtime is running.
			if ((settings instanceof StatusBarNotificationSettings)) {
				StatusBarNotificationSettings statusBarSettings = (StatusBarNotificationSettings)settings;

				for (com.ansca.corona.CoronaRuntime runtime : com.ansca.corona.CoronaRuntimeProvider.getAllCoronaRuntimes()) {
					if (runtime.isRunning()) {
						runtime.getTaskDispatcher().send(new com.ansca.corona.events.NotificationReceivedTask("active", statusBarSettings));
					}
				}
			}
		}
	}

	/**
	 * Posts the given notification to the Android system.
	 * <p>
	 * A scheduled notification will be posted via the Android "AlarmManager" class.
	 * <p>
	 * A status bar notification will be posted via the Android "NotificationManager" class.
	 * @param settings Reference to the notification configuration to be posted. If the notification has
	 *                 never been posted before, then the notification will be added to the system.
	 *                 If the notification has already been posted to the Android system, then its
	 *                 configuration in the Android system will be updated with the given settings.
	 */
	private void postSystemNotification(NotificationSettings settings) {
		synchronized (NotificationServices.class) {
			// Validate.
			if (settings == null) {
				return;
			}

			// Post the given notification to the system.
			android.content.Context context = getApplicationContext();
			try {
				if (settings instanceof ScheduledNotificationSettings) {
					// *** Schedule the given notification to be triggered in the future via the AlarmManager. ***
					ScheduledNotificationSettings scheduledSettings = (ScheduledNotificationSettings)settings;

					// Set up a pending intent that uniquely identifies the notification by ID and to tell
					// the alarm manager to send the notification to the AlarmManagerBroadcastReceiver class.
					android.app.PendingIntent pendingIntent = android.app.PendingIntent.getBroadcast(
							context, 0, AlarmManagerBroadcastReceiver.createIntentFrom(context, scheduledSettings), android.app.PendingIntent.FLAG_IMMUTABLE);


					// Schedule the notification using the Android alarm manager.
					android.app.AlarmManager alarmManager;
					String serviceName = android.content.Context.ALARM_SERVICE;
					alarmManager = (android.app.AlarmManager)context.getSystemService(serviceName);
					if (android.os.Build.VERSION.SDK_INT >= 23) {
						NotificationServices.ApiLevel23.alarmManagerSetExactAndAllowWhileIdle(
								alarmManager,
								android.app.AlarmManager.RTC_WAKEUP,
								scheduledSettings.getEndTime().getTime(),
								pendingIntent);
					}
					else if (android.os.Build.VERSION.SDK_INT >= 19) {
						NotificationServices.ApiLevel19.alarmManagerSetExact(
								alarmManager,
								android.app.AlarmManager.RTC_WAKEUP,
								scheduledSettings.getEndTime().getTime(),
								pendingIntent);
					}
					else {
						alarmManager.set(
								android.app.AlarmManager.RTC_WAKEUP,
								scheduledSettings.getEndTime().getTime(),
								pendingIntent);
					}
				}
				else if (settings instanceof StatusBarNotificationSettings) {
					// *** Update the Android status bar with the given notification. ***
					StatusBarNotificationSettings statusBarSettings = (StatusBarNotificationSettings)settings;

					// Create and set up the status bar notification object.
					// Note: Android 3.x and higher versions requires us to use the the "Notification.Builder"
					//       class because many of the "Notification" class' methods and fields have been deprecated.
					android.app.Notification notification = null;
					if (android.os.Build.VERSION.SDK_INT >= 16) {
						notification = NotificationServices.ApiLevel16.createNotificationFrom(context, statusBarSettings);
					}
					else if (android.os.Build.VERSION.SDK_INT >= 11) {
						notification = NotificationServices.ApiLevel11.createNotificationFrom(context, statusBarSettings);
					}
					else {
						notification = NotificationServices.ApiLevel1.createNotificationFrom(context, statusBarSettings);
					}

					// Update the Android system's status bar.
					android.app.NotificationManager notificationManager;
					String serviceName = android.content.Context.NOTIFICATION_SERVICE;
					notificationManager = (android.app.NotificationManager)context.getSystemService(serviceName);
					notificationManager.notify(statusBarSettings.getId(), notification);
				}
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}
		}
	}

	/**
	 * Removes a scheduled notification or status bar notification that was once posted by this class
	 * by its unique integer ID.
	 * <p>
	 * Note that removing a notification will unreserve its associated ID and make it available
	 * again by the reserveId() methods.
	 * @param id The unique integer ID of the notification to be removed.
	 * @return Returns true if the notification was successfully removed.
	 *         <p>
	 *         Returns false if a notification having the given ID could not be found.
	 */
	public boolean removeById(int id) {
		synchronized (NotificationServices.class) {
			// Fetch the notification settings.
			NotificationSettings settings = sNotificationCollection.getById(id);
			if (settings == null) {
				return false;
			}

			// Remove the notification from the system.
			removeSystemNotification(settings);

			// Remove the notification from the collection.
			sReservedNotificationIdSet.remove(Integer.valueOf(id));
			sNotificationCollection.remove(settings);

			// Update the configuration file.
			saveSettings(sNotificationCollection);
			return true;
		}
	}

	/**
	 * Removes all scheduled notifications and status bar notifications that were posted by this class.
	 */
	public void removeAll() {
		synchronized (NotificationServices.class) {
			// Remove all notifications from the system.
			for (NotificationSettings settings : sNotificationCollection) {
				removeSystemNotification(settings);
				sReservedNotificationIdSet.remove(Integer.valueOf(settings.getId()));
			}

			// Remove all notification configurations.
			sNotificationCollection.clear();
			saveSettings(sNotificationCollection);
		}
	}

	/**
	 * Removes the given notification from the Android system.
	 * @param settings The configuration of the scheduled notification or status bar notification.
	 */
	private void removeSystemNotification(NotificationSettings settings) {
		synchronized (NotificationServices.class) {
			// Validate.
			if (settings == null) {
				return;
			}

			// Remove the notification from the Android system.
			android.content.Context context = getApplicationContext();
			try {
				if (settings instanceof ScheduledNotificationSettings) {
					// Cancel the scheduled notification.
					android.app.AlarmManager alarmManager;
					String serviceName = android.content.Context.ALARM_SERVICE;
					alarmManager = (android.app.AlarmManager)context.getSystemService(serviceName);
					ScheduledNotificationSettings scheduledSettings = (ScheduledNotificationSettings)settings;
					android.app.PendingIntent pendingIntent = android.app.PendingIntent.getBroadcast(
							context, 0, AlarmManagerBroadcastReceiver.createIntentFrom(context, scheduledSettings), android.app.PendingIntent.FLAG_IMMUTABLE);
					alarmManager.cancel(pendingIntent);
				}
				else if (settings instanceof StatusBarNotificationSettings) {
					// Remove the status bar notification.
					android.app.NotificationManager notificationManager;
					String serviceName = android.content.Context.NOTIFICATION_SERVICE;
					notificationManager = (android.app.NotificationManager)context.getSystemService(serviceName);
					notificationManager.cancel(settings.getId());
				}
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}
		}
	}

	/**
	 * Loads the last saved notifications to the given collection.
	 * <p>
	 * This method is expected to be called upon application startup to restore the last saved/posted notifications.
	 * @param collection The collection that will receive the loaded notification configurations. Cannot be null.
	 */
	private void loadSettingsTo(NotificationSettingsCollection<NotificationSettings> collection) {
		synchronized (NotificationServices.class) {
			// Validate.
			if (collection == null) {
				return;
			}

			// Empty the given collection.
			collection.clear();

			// Get the path to the XML file to read notification settings from.
			java.io.File filePath = new java.io.File(getApplicationContext().getCacheDir(), ".system");
			filePath = new java.io.File(filePath, "NotificationSettings.xml");

			// Do not continue if the XML file does not exist.
			// This means that there are no active notifications for this class to manage.
			if (filePath.exists() == false) {
				return;
			}

			// Load all notification settings from file.
			java.io.FileReader fileReader = null;
			try {
				// Set up an XML reader to parse the file.
				org.xmlpull.v1.XmlPullParser xmlReader = android.util.Xml.newPullParser();
				fileReader = new java.io.FileReader(filePath);
				xmlReader.setInput(fileReader);

				// Fetch all notification settings from the XML file.
				int xmlEventType = xmlReader.getEventType();
				while (xmlEventType != org.xmlpull.v1.XmlPullParser.END_DOCUMENT) {
					if (xmlEventType == org.xmlpull.v1.XmlPullParser.START_TAG) {
						NotificationSettings settings = loadSettingsFrom(xmlReader);
						if (settings != null) {
							collection.add(settings);
						}
					}
					xmlEventType = xmlReader.next();
				}
			}
			catch (Exception ex) {
				// An error occurred while parsing the file. Log the error and stop parsing the file.
				ex.printStackTrace();
			}
			finally {
				// Close the file.
				if (fileReader != null) {
					try { fileReader.close(); }
					catch (Exception ex) { }
				}
			}
		}
	}

	/**
	 * Extracts one notification configuration from the given XML reader.
	 * @param xmlReader Reader used to extract one notification settings object from XML.
	 *                  The reader must be referencing the start tag of a scheduled or status bar configuration.
	 * @return Returns the extract notification setting that was read via the given XML reader.
	 *         <p>
	 *         Returns null if failed to read notification settings from XML.
	 */
	private NotificationSettings loadSettingsFrom(org.xmlpull.v1.XmlPullParser xmlReader) {
		synchronized (NotificationServices.class) {
			// Validate.
			if (xmlReader == null) {
				return null;
			}

			// Read one notification configuration from the given XML reader.
			try {
				// Do not continue if the XML reader is not currently referencing a start tag.
				int xmlEventType = xmlReader.getEventType();
				if (xmlEventType != org.xmlpull.v1.XmlPullParser.START_TAG) {
					return null;
				}

				// Extract notification settings from the XML tag.
				if ("scheduled".equals(xmlReader.getName())) {
					ScheduledNotificationSettings settings = new ScheduledNotificationSettings();
					settings.setEndTime(new java.util.Date(Long.parseLong(xmlReader.getAttributeValue("", "endTime"))));
					xmlEventType = xmlReader.nextTag();
					if (xmlReader.getEventType() == org.xmlpull.v1.XmlPullParser.START_TAG) {
						if ("statusBar".equals(xmlReader.getName())) {
							NotificationSettings statusBarSettings = loadSettingsFrom(xmlReader);
							if (statusBarSettings instanceof StatusBarNotificationSettings) {
								settings.getStatusBarSettings().copyFrom((StatusBarNotificationSettings)statusBarSettings);
								return settings;
							}
						}
					}
				}
				else if ("statusBar".equals(xmlReader.getName())) {
					StatusBarNotificationSettings settings = new StatusBarNotificationSettings();
					settings.setId(Integer.parseInt(xmlReader.getAttributeValue("", "id")));
					settings.setTimestamp(new java.util.Date(Long.parseLong(xmlReader.getAttributeValue("", "timestamp"))));
					settings.setContentTitle(xmlReader.getAttributeValue("", "contentTitle"));
					settings.setContentText(xmlReader.getAttributeValue("", "contentText"));
					settings.setTickerText(xmlReader.getAttributeValue("", "tickerText"));
					settings.setIconResourceName(xmlReader.getAttributeValue("", "iconResourceName"));
					settings.setBadgeNumber(Integer.parseInt(xmlReader.getAttributeValue("", "badgeNumber")));
					String stringValue = xmlReader.getAttributeValue("", "soundFileUri");
					if ((stringValue != null) && (stringValue.length() > 0)) {
						settings.setSoundFileUri(android.net.Uri.parse(stringValue));
					}
					settings.setSourceName(xmlReader.getAttributeValue("", "sourceName"));
					stringValue = xmlReader.getAttributeValue("", "isSourceLocal");
					if ((stringValue != null) && (stringValue.length() > 0)) {
						settings.setSourceLocal(Boolean.valueOf(stringValue));
					}
					while (true) {
						xmlEventType = xmlReader.nextTag();
						if (xmlReader.getEventType() == org.xmlpull.v1.XmlPullParser.START_TAG) {
							if ("sourceData".equals(xmlReader.getName())) {
								settings.setSourceDataName(xmlReader.getAttributeValue("", "name"));
								settings.setSourceData(com.ansca.corona.CoronaData.from(xmlReader));
							}
							else if ("data".equals(xmlReader.getName())) {
								com.ansca.corona.CoronaData data = com.ansca.corona.CoronaData.from(xmlReader);
								if (data instanceof com.ansca.corona.CoronaData.Table) {
									settings.setData((com.ansca.corona.CoronaData.Table)data);
								}
							}
						}
						else if ((xmlReader.getEventType() == org.xmlpull.v1.XmlPullParser.END_TAG) &&
								"statusBar".equals(xmlReader.getName())) {
							break;
						}
					}
					return settings;
				}
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}

			// Failed to extract or find valid notification settings.
			return null;
		}
	}

	/**
	 * Saves the given notification settings to be loaded later.
	 * <p>
	 * This method is expected to be called everytime a notification has been posted, updated, or removed.
	 * The intent is to persist this information so that notifications can be loaded and restored upon
	 * application startup.
	 * @param collection The collection of notification configuration to be saved. Cannot be null.
	 */
	private void saveSettings(NotificationSettingsCollection<NotificationSettings> collection) {
		synchronized (NotificationServices.class) {
			// Validate.
			if (collection == null) {
				return;
			}

			// Get the path to the XML file to write notification settings to.
			java.io.File filePath = new java.io.File(getApplicationContext().getCacheDir(), ".system");
			filePath = new java.io.File(filePath, "NotificationSettings.xml");

			// If the collection is empty, then delete the existing XML file.
			if (collection.size() <= 0) {
				try {
					if (filePath.exists()) {
						filePath.delete();
					}
				}
				catch (Exception ex) {
					ex.printStackTrace();
				}
				return;
			}

			// Write all notification settings to file.
			java.io.FileWriter fileWriter = null;
			try {
				// Set up an XML writer.
				org.xmlpull.v1.XmlSerializer xmlWriter = android.util.Xml.newSerializer();
				fileWriter = new java.io.FileWriter(filePath);
				xmlWriter.setOutput(fileWriter);

				// Write all notification settings to the XML file.
				xmlWriter.startDocument("UTF-8", true);
				xmlWriter.startTag("", "notifications");
				for (NotificationSettings settings : collection) {
					saveSettings(settings, xmlWriter);
				}
				xmlWriter.endTag("", "notifications");
				xmlWriter.endDocument();
				xmlWriter.flush();
			}
			catch (Exception ex) {
				// An error occurred while writing to file. Log it and then give up.
				ex.printStackTrace();
			}
			finally {
				// Close the file.
				if (fileWriter != null) {
					try { fileWriter.flush(); }
					catch (Exception ex) { }
					try { fileWriter.close(); }
					catch (Exception ex) { }
				}
			}
		}
	}

	/**
	 * Writes one notification settings object to the given XML writer.
	 * @param settings The notification configuration to output as XML.
	 * @param xmlWriter The object used to output given notification as XML. Expected to write to an XML file.
	 */
	private void saveSettings(NotificationSettings settings, org.xmlpull.v1.XmlSerializer xmlWriter)
	{
		synchronized (NotificationServices.class) {
			// Validate.
			if ((settings == null) || (xmlWriter == null)) {
				return;
			}

			// Write the given settings to xml.
			try {
				if (settings instanceof ScheduledNotificationSettings) {
					ScheduledNotificationSettings scheduledSettings = (ScheduledNotificationSettings)settings;
					xmlWriter.startTag("", "scheduled");
					xmlWriter.attribute("", "endTime", Long.toString(scheduledSettings.getEndTime().getTime()));
					saveSettings(scheduledSettings.getStatusBarSettings(), xmlWriter);
					xmlWriter.endTag("", "scheduled");
				}
				else if (settings instanceof StatusBarNotificationSettings) {
					StatusBarNotificationSettings statusBarSettings = (StatusBarNotificationSettings)settings;
					xmlWriter.startTag("", "statusBar");
					xmlWriter.attribute("", "id", Integer.toString(statusBarSettings.getId()));
					xmlWriter.attribute("", "timestamp", Long.toString(statusBarSettings.getTimestamp().getTime()));
					xmlWriter.attribute("", "contentTitle", statusBarSettings.getContentTitle());
					xmlWriter.attribute("", "contentText", statusBarSettings.getContentText());
					xmlWriter.attribute("", "tickerText", statusBarSettings.getTickerText());
					xmlWriter.attribute("", "iconResourceName", statusBarSettings.getIconResourceName());
					xmlWriter.attribute("", "badgeNumber", Integer.toString(statusBarSettings.getBadgeNumber()));
					if (statusBarSettings.getSoundFileUri() != null) {
						xmlWriter.attribute("", "soundFileUri", statusBarSettings.getSoundFileUri().toString());
					}
					xmlWriter.attribute("", "sourceName", statusBarSettings.getSourceName());
					xmlWriter.attribute("", "isSourceLocal", Boolean.toString(statusBarSettings.isSourceLocal()));
					if (statusBarSettings.getSourceData() != null) {
						xmlWriter.startTag("", "sourceData");
						xmlWriter.attribute("", "name", statusBarSettings.getSourceDataName());
						statusBarSettings.getSourceData().writeTo(xmlWriter);
						xmlWriter.endTag("", "sourceData");
					}
					if (statusBarSettings.getData() != null) {
						xmlWriter.startTag("", "data");
						statusBarSettings.getData().writeTo(xmlWriter);
						xmlWriter.endTag("", "data");
					}
					xmlWriter.endTag("", "statusBar");
				}
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}
		}
	}

	/**
	 * Provides access to API Level 1 features.
	 * Should only be accessed if running on an operating system with at least this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * You are expected to call its static methods instead.
	 */
	private static class ApiLevel1 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel1() {
		}

		/**
		 * Gets the resource Id for the notification icon using the given settings.
		 * @param context Context needed to access ResourceServices. Cannot be null.
		 * @param settings The status bar settings containing the notification icon resource.
		 * @return Returns the resource Id for the notification icon using the given settings.
		 *         <p>
		 *         Returns null if given a null "context" or "settings" object.
		 */
		public static Integer getIconResourceId(
				android.content.Context context, StatusBarNotificationSettings settings)
		{
			// Validate.
			if ((context == null) || (settings == null)) {
				return null;
			}

			// Fetch the icon's resource ID.
			// If not found, then use Corona's default notification icon.
			int iconResourceId = settings.getIconResourceId();
			if (iconResourceId == com.ansca.corona.storage.ResourceServices.INVALID_RESOURCE_ID) {
				com.ansca.corona.storage.ResourceServices resourceServices;
				resourceServices = new com.ansca.corona.storage.ResourceServices(context);
				iconResourceId = resourceServices.getDrawableResourceId(
						StatusBarNotificationSettings.DEFAULT_ICON_RESOURCE_NAME);
			}

			return iconResourceId;
		}

		/**
		 * Creates an Android "Notification" object with the given settings.
		 * <p>
		 * The notification object is made using the "Notification" class.
		 * @param context Context needed to set up the notification. Cannot be null.
		 * @param settings The status bar settings used to build the notification object.
		 * @return Returns a new Android "Notification" object using the given settings.
		 *         <p>
		 *         Returns null if given a null "context" or "settings" object.
		 */
		public static android.app.Notification createNotificationFrom(
				android.content.Context context, StatusBarNotificationSettings settings)
		{
			Integer iconResourceId = ApiLevel1.getIconResourceId(context, settings);
			if (iconResourceId == null) {
				return null;
			}

			// Set up the notification object.
			android.app.Notification notification = new android.app.Notification(
					iconResourceId, settings.getTickerText(),
					settings.getTimestamp().getTime());
			android.content.Intent intent =
					StatusBarBroadcastReceiver.createContentIntentFrom(context, settings);

			// Invoke the "setLatestEventInfo" method is available.
			// It was removed from API Level 23, so we get it by reflection.
			try {
				// notification.setLatestEventInfo(
				// 	context, settings.getContentTitle(),
				// 	settings.getContentText(),
				// 	android.app.PendingIntent.getBroadcast(context, 0, intent, 0));
				java.lang.reflect.Method setLatestEventInfoMethod = notification.getClass().getMethod("setLatestEventInfo",
						android.content.Context.class, CharSequence.class, CharSequence.class, android.app.PendingIntent.class);
				setLatestEventInfoMethod.invoke(notification, context, settings.getContentTitle(),
						settings.getContentText(), android.app.PendingIntent.getBroadcast(context, 0, intent, 0));
			} catch (Exception e) {
				// TODO: Print some warning to the developer telling them to use the ApiLevel16 class if they get here.
				return null;
			}

			intent = StatusBarBroadcastReceiver.createDeleteIntentFrom(context, settings);
			notification.deleteIntent = android.app.PendingIntent.getBroadcast(context, 0, intent, 0);
			notification.number = settings.getBadgeNumber();
			if (settings.getSoundFileUri() != null) {
				notification.sound = settings.getSoundFileUri();
			}
			else {
				notification.defaults = android.app.Notification.DEFAULT_SOUND;
			}
			notification.flags = android.app.Notification.FLAG_ONLY_ALERT_ONCE;
			if (settings.isRemovable()) {
				notification.flags |= android.app.Notification.FLAG_AUTO_CANCEL;
			}
			else {
				notification.flags |= android.app.Notification.FLAG_ONGOING_EVENT;
				notification.flags |= android.app.Notification.FLAG_NO_CLEAR;
			}

			return notification;
		}
	}


	/**
	 * Provides access to API Level 11 features.
	 * Should only be accessed if running on an operating system with at least this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * You are expected to call its static methods instead.
	 */
	private static class ApiLevel11 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel11() {
		}

		/**
		 * Creates an Android "Notification.Builder" object configured with the given settings.
		 * This builder object can then be used to create a "Notification" object.
		 * @param context Context needed to set up the notification builder. Cannot be null.
		 * @param settings The status bar settings to be copied to the notification builder object.
		 * @return Returns a new Android "Notification.Builder" object configured with the given settings.
		 *         <p>
		 *         Returns null if given a null "context" or "settings" object.
		 */
		public static android.app.Notification.Builder createNotificationBuilderFrom(
				android.content.Context context, StatusBarNotificationSettings settings)
		{

			Integer iconResourceId = ApiLevel1.getIconResourceId(context, settings);
			if (iconResourceId == null) {
				return null;
			}

			// Set up the notification object.
			android.app.Notification.Builder builder = new android.app.Notification.Builder(context);
			builder.setContentTitle(settings.getContentTitle());
			builder.setContentText(settings.getContentText());
			builder.setTicker(settings.getTickerText());
			builder.setWhen(settings.getTimestamp().getTime());
			builder.setSmallIcon(iconResourceId);
			builder.setNumber(settings.getBadgeNumber());
			if (settings.getSoundFileUri() != null) {
				builder.setSound(settings.getSoundFileUri());
			}
			else {
				builder.setDefaults(android.app.Notification.DEFAULT_SOUND);
			}
			builder.setOnlyAlertOnce(true);
			builder.setAutoCancel(settings.isRemovable());
			builder.setOngoing(settings.isRemovable() == false);

			// Set an intent to be invoked when the notification has been tapped.
			android.content.Intent intent;
			intent = OnNotificationReceiverActivity.createContentIntentFrom(context, settings);
			builder.setContentIntent(android.app.PendingIntent.getActivity(context, 0, intent, android.app.PendingIntent.FLAG_IMMUTABLE));

			// Set an intent to be invoked when the notification has been cleared/removed.
			intent = StatusBarBroadcastReceiver.createDeleteIntentFrom(context, settings);
			builder.setDeleteIntent(android.app.PendingIntent.getActivity(context, 0, intent, android.app.PendingIntent.FLAG_IMMUTABLE));

			// Return the notification builder object.
			return builder;
		}

		/**
		 * Creates an Android "Notification" object with the given settings.
		 * <p>
		 * The notification object is made using the "Notification.Builder" class.
		 * @param context Context needed to set up the notification. Cannot be null.
		 * @param settings The status bar settings used to build the notification object.
		 * @return Returns a new Android "Notification" object using the given settings.
		 *         <p>
		 *         Returns null if given a null "context" or "settings" object.
		 */
		public static android.app.Notification createNotificationFrom(
				android.content.Context context, StatusBarNotificationSettings settings)
		{
			// Set up a notification builder.
			android.app.Notification.Builder builder = ApiLevel11.createNotificationBuilderFrom(context, settings);
			if (builder == null) {
				return null;
			}

			// Create and return the notification object.
			return builder.getNotification();
		}
	}

	/**
	 * Provides access to API Level 16 features.
	 * Should only be accessed if running on an operating system with at least this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * You are expected to call its static methods instead.
	 */
	private static class ApiLevel16 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel16() {
		}

		/**
		 * Creates an Android "Notification" object with the given settings.
		 * <p>
		 * The notification object is made using the "Notification.Builder" class.
		 * @param context Context needed to set up the notification. Cannot be null.
		 * @param settings The status bar settings used to build the notification object.
		 * @return Returns a new Android "Notification" object using the given settings.
		 *         <p>
		 *         Returns null if given a null "context" or "settings" object.
		 */
		public static android.app.Notification createNotificationFrom(
				android.content.Context context, StatusBarNotificationSettings settings)
		{
			// Set up a notification builder.
			android.app.Notification.Builder builder = ApiLevel11.createNotificationBuilderFrom(context, settings);

			if (android.os.Build.VERSION.SDK_INT >= 26) {
				String serviceName = android.content.Context.NOTIFICATION_SERVICE;
				android.app.NotificationManager notificationManager;
				notificationManager = (android.app.NotificationManager)context.getSystemService(serviceName);
				android.app.NotificationChannel mChannel;
				ApplicationInfo applicationInfo = context.getApplicationInfo();
				int stringId = applicationInfo.labelRes;
				String applicationName = stringId == 0 ? applicationInfo.nonLocalizedLabel.toString() : context.getString(stringId);

				try {
					mChannel = notificationManager.getNotificationChannel(DEFAULT_CHANNEL_ID);
					if (mChannel == null)
						throw new Exception();
				} catch (Exception e) {
					mChannel = new android.app.NotificationChannel(DEFAULT_CHANNEL_ID, applicationName, android.app.NotificationManager.IMPORTANCE_DEFAULT);
					mChannel.setDescription("Default notification channel");
					mChannel.enableLights(true);
					notificationManager.createNotificationChannel(mChannel);
				}
				if(!applicationName.equals(mChannel.getName())) {
					mChannel.setName(applicationName);
				}
				builder.setChannelId(DEFAULT_CHANNEL_ID);
			}
			if (builder == null) {
				return null;
			}

			// Create and return the notification object.
			return builder.build();
		}
	}

	/**
	 * Provides access to API Level 19 features.
	 * Should only be accessed if running on an operating system with at least this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * You are expected to call its static methods instead.
	 */
	private static class ApiLevel19 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel19() { }

		/**
		 * Calls the given alarm manager's setExact() method with the given arguments.
		 * @param alarmManager The alarm manager to call setExact() on. This method will do nothing if set to null.
		 * @param type Set to either ELAPSED_REALTIME, ELAPSED_REALTIME_WAKEUP, RTC, or RTC_WAKEUP.
		 * @param triggerAtMilliseconds Time in milliseconds from now to trigger the alarm.
		 * @param pendingIntent The intent to be invoked when the alarm triggers.
		 */
		public static void alarmManagerSetExact(
				android.app.AlarmManager alarmManager,
				int type, long triggerAtMilliseconds, android.app.PendingIntent pendingIntent)
		{
			if (alarmManager != null) {
				alarmManager.setExact(type, triggerAtMilliseconds, pendingIntent);
			}
		}
	}

	/**
	 * Provides access to API Level 23 features.
	 * Should only be accessed if running on an operating system with at least this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * You are expected to call its static methods instead.
	 */
	private static class ApiLevel23 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel23() { }

		/**
		 * Calls the given alarm manager's setExactAndAllowWhileIdle() method with the given arguments.
		 * @param alarmManager The alarm manager to call setExactAndAllowWhileIdle() on. This method will do nothing if set to null.
		 * @param type Set to either ELAPSED_REALTIME, ELAPSED_REALTIME_WAKEUP, RTC, or RTC_WAKEUP.
		 * @param triggerAtMilliseconds Time in milliseconds from now to trigger the alarm.
		 * @param pendingIntent The intent to be invoked when the alarm triggers.
		 */
		public static void alarmManagerSetExactAndAllowWhileIdle(
				android.app.AlarmManager alarmManager,
				int type, long triggerAtMilliseconds, android.app.PendingIntent pendingIntent)
		{
			if (alarmManager != null) {
				alarmManager.setExactAndAllowWhileIdle(type, triggerAtMilliseconds, pendingIntent);
			}
		}
	}
}