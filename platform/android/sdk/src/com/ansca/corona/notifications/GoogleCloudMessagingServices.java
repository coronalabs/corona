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
 * Provides thread safe methods for registering and unregistering with the Google Cloud Messaging service.
 * <p>
 * Also provides registration information after successfully registering for push notifications with Google.
 */
public final class GoogleCloudMessagingServices extends com.ansca.corona.ApplicationContextProvider {
	/** The name of the shared preferences used by this class to store settings. */
	private static final String CORONA_PREFERENCES_NAME = "Corona";

	/** The unique string key used to store the registration ID in the preferences file. */
	private static final String CORONA_PREFERENCE_REGISTRATION_ID_KEY = "google-cloud-messaging-registration-id";

	/** The unique string key used to store the registration ID in the preferences file. */
	private static final String CORONA_PREFERENCE_PROJECT_NUMBERS_KEY = "google-cloud-messaging-project-numbers";

	/** The name of the package to send Google Cloud Messaging intents to. */
	private static final String GOOGLE_SERVICE_FRAMEWORK_PACKAGE_NAME = "com.google.android.gsf";

	/** The unique name of the Intent extra data that stores the project number(s). Also known as Sender IDs. */
	private static final String INTENT_EXTRA_PROJECT_NUMBERS = "sender";

	/** The unique name of the Intent extra data that stores the pending intent to this application. */
	private static final String INTENT_EXTRA_PENDING_INTENT = "app";

	/** The unique name of the Intent extra data that stores the registration ID assigned to this application. */
	private static final String INTENT_EXTRA_REGISTRATION_ID = "registration_id";

	/** The unique name of the Intent extra data that stores a string indicating that this app was unregistered. */
	private static final String INTENT_EXTRA_UNREGISTERED = "unregistered";

	/** The unique name of the Intent extra data that stores an error string ID. */
	private static final String INTENT_EXTRA_ERROR_ID = "error";

	/** The unique name of the Intent extra data that stores a string indicating the type of message that was received. */
	private static final String INTENT_EXTRA_MESSAGE_TYPE = "message_type";

	/** The unique name of the Intent extra data that stores the number of messages deleted. */
	private static final String INTENT_EXTRA_TOTAL_DELETED = "total_deleted";

	/**
	 * The unique string ID assigned to this application after registering with the Google Cloud Messaging system.
	 * <p>
	 * Set to an empty string if this application is not registered.
	 */
	private static String sRegistrationId = null;

	/**
	 * String storing a comma separated list of project numbers that this application has been registered with.
	 * <p>
	 * Set to an empty string if this application is not registered.
	 */
	private static String sRegisteredProjectNumbers = null;

	/** Stores a queue of "register" and "unregister" operations to be executed. */
	private static java.util.LinkedList<Operation> sOperationQueue = new java.util.LinkedList<Operation>();

	/** Stores a reference to an executed operation that is waiting for a reply from Google. */
	private static Operation sPendingOperation = null;

	/**
	 * Creates an interface to the Google Cloud Messaging system.
	 * @param context Reference to an Android created context needed to access Google's system.
	 *                <p>
	 *                Setting this to null will cause an exception to be thrown.
	 */
	public GoogleCloudMessagingServices(android.content.Context context) {
		super(context);

		// Fetch this application's registration information from its preferences file, if not done already.
		synchronized (GoogleCloudMessagingServices.class) {
			if (sRegistrationId == null) {
				sRegistrationId = "";
				sRegisteredProjectNumbers = "";
				try {
					String stringBuffer;
					android.content.SharedPreferences preferencesReader;
					preferencesReader = getApplicationContext().getSharedPreferences(
							CORONA_PREFERENCES_NAME, android.content.Context.MODE_PRIVATE);
					stringBuffer = preferencesReader.getString(CORONA_PREFERENCE_REGISTRATION_ID_KEY, "");
					if (stringBuffer != null) {
						sRegistrationId = stringBuffer;
					}
					stringBuffer = preferencesReader.getString(CORONA_PREFERENCE_PROJECT_NUMBERS_KEY, "");
					if (stringBuffer != null) {
						sRegisteredProjectNumbers = stringBuffer;
					}
				}
				catch (Exception ex) {
					ex.printStackTrace();
				}
			}
		}
	}

	/**
	 * Registers this application for push notifications from the Google Cloud Messaging system.
	 * @param projectNumber A unique string (also known as a Sender ID) used to associate this application
	 *                      with a Google API project.
	 *                      <p>
	 *                      The project number can be aquired from the "Google API Console"...
	 *                      <p>
	 *                      <a href="http://code.google.com/apis/console">http://code.google.com/apis/console</a>
	 */
	public void register(String projectNumber) {
		synchronized (GoogleCloudMessagingServices.class) {
			RegisterOperation operation;

			// Validate argument.
			if ((projectNumber == null) || (projectNumber.length() <= 0)) {
				return;
			}

			// Clear the operation queue of all registration requests, if any.
			sOperationQueue.clear();

			// Do not continue if this application is already registered with the given project numbers.
			if (isRegistered() && sRegisteredProjectNumbers.equals(projectNumber) && (sPendingOperation == null)) {
				return;
			}

			// Do not continue if we have already sent a registration request with the given project number(s).
			if (sPendingOperation instanceof RegisterOperation) {
				operation = (RegisterOperation)sPendingOperation;
				if (operation.getProjectNumbers().equals(projectNumber)) {
					return;
				}
			}

			// Unregister this application if it is registered/registering with different project numbers.
			if (sPendingOperation == null) {
				if (isRegistered() && (sRegisteredProjectNumbers.equals(projectNumber) == false)) {
					sOperationQueue.addLast(new UnregisterOperation());
				}
			}
			else if (sPendingOperation instanceof RegisterOperation) {
				sOperationQueue.addLast(new UnregisterOperation());
			}

			// Add a registration request to the queue and send it.
			operation = new RegisterOperation(projectNumber);
			sOperationQueue.addLast(operation);
			executeNextQueuedOperation();
		}
	}

	/**
	 * Registers this application for push notifications from the Google Cloud Messaging system.
	 * @param projectNumbers An array of unique strings (also known as Sender IDs) used to associate this
	 *                       application with multiple Google API projects.
	 *                       <p>
	 *                       The project numbers can be aquired from the "Google API Console"...
	 *                       <p>
	 *                       <a href="http://code.google.com/apis/console">http://code.google.com/apis/console</a>
	 */
	public void register(String[] projectNumbers) {
		// Validate argument.
		if ((projectNumbers == null) || (projectNumbers.length <= 0)) {
			return;
		}

		// Combine the array of project numbers into one comma separated string.
		StringBuilder builder = new StringBuilder();
		for (String nextProjectNumber : projectNumbers) {
			if ((nextProjectNumber != null) && (nextProjectNumber.length() > 0)) {
				if (builder.length() > 0) {
					builder.append(",");
				}
				builder.append(nextProjectNumber);
			}
		}
		String commaSeparatedProjectNumbers = builder.toString();

		// Register this application with all of the given project numbers.
		register(commaSeparatedProjectNumbers);
	}

	/**
	 * Unregisters this application for push notifications from the Google Cloud Messaging system.
	 */
	public void unregister() {
		// Clear the operation queue of all registration requests, if any.
		sOperationQueue.clear();

		// Do not continue if an unregister request has already been sent.
		if (sPendingOperation instanceof UnregisterOperation) {
			return;
		}

		// Do not continue if the application is not registered or actively registering.
		if ((sPendingOperation == null) && isUnregistered()) {
			return;
		}

		// Add a unregister request to the queue and send it.
		sOperationQueue.addLast(new UnregisterOperation());
		executeNextQueuedOperation();
	}

	/** Attempts to run the next operation in the queue if any. */
	private void executeNextQueuedOperation() {
		synchronized (GoogleCloudMessagingServices.class) {
			// Do not continue if there is a pending operation waiting for a response.
			if (sPendingOperation != null) {
				return;
			}

			// Do not continue if there are no operations in the queue.
			if (sOperationQueue.isEmpty()) {
				return;
			}

			// Pop off the next operation in the queue and execute it.
			sPendingOperation = sOperationQueue.removeFirst();
			sPendingOperation.run();
		}
	}

	/**
	 * Determines if this application is currently registered for push notifications from Google.
	 * @return Returns true if this application is currently registered. Returns false if not.
	 */
	public boolean isRegistered() {
		return (sRegistrationId.length() > 0);
	}

	/**
	 * Determines if this application is not currently registered for push notifications from Google.
	 * @return Returns true if this application is not registered. Returns false if it is.
	 */
	public boolean isUnregistered() {
		return !isRegistered();
	}

	/**
	 * Gets the unique string ID assigned to this application if currently registered for push notifications.
	 * @return Returns a unique string ID assigned to this application. This ID is needed to send notifications.
	 *         <p>
	 *         Returns an empty string if this application is not registered for push notifications.
	 */
	public String getRegistrationId() {
		return sRegistrationId;
	}

	/**
	 * Gets the project numbers this application used during registration as a comma separated string.
	 * @return Returns a string containing a list of comma seprated project numbers.
	 *         <p>
	 *         Returns an empty string if this application is not registered for push notifications.
	 */
	public String getCommaSeparatedRegisteredProjectNumbers() {
		synchronized (GoogleCloudMessagingServices.class) {
			if (sRegisteredProjectNumbers == null) {
				return "";
			}
			return sRegisteredProjectNumbers;
		}
	}

	/**
	 * Gets the project numbers this application used during registration.
	 * @return Returns an array of project number strings.
	 *         <p>
	 *         Returns null if this application is not registered for push notifications.
	 */
	public String[] getRegisteredProjectNumbers() {
		synchronized (GoogleCloudMessagingServices.class) {
			if ((sRegisteredProjectNumbers == null) || (sRegisteredProjectNumbers.length() <= 0)) {
				return null;
			}
			return sRegisteredProjectNumbers.split(",");
		}
	}

	/**
	 * Stores the given registration information.
	 * <p>
	 * This is a private method that stores the given information to static member variables and also saves
	 * them to this application's preferences file to be loaded when the application restarts.
	 * @param registrationId Unique string ID received from Google after successful registration.
	 *                       <p>
	 *                       Set to null or empty string to indicate this application has been unregistered.
	 * @param projectNumbers A string providing a comma separated list of unique project numbers that this
	 *                       application was registered under.
	 *                       <p>
	 *                       Set to null or empty string to indicate this application has been unregistered.
	 */
	private void saveRegistrationInformation(String registrationId, String projectNumbers) {
		synchronized (GoogleCloudMessagingServices.class) {
			// Do not allow the given settings to be null.
			if (registrationId == null) {
				registrationId = "";
			}
			if (projectNumbers == null) {
				projectNumbers = "";
			}

			// Update member variables.
			sRegistrationId = registrationId;
			sRegisteredProjectNumbers = projectNumbers;

			// Save the given values to this application's preferences file.
			try {
				android.content.SharedPreferences preferencesReader;
				preferencesReader = getApplicationContext().getSharedPreferences(
						CORONA_PREFERENCES_NAME, android.content.Context.MODE_PRIVATE);
				android.content.SharedPreferences.Editor preferencesWriter = preferencesReader.edit();
				preferencesWriter.putString(CORONA_PREFERENCE_REGISTRATION_ID_KEY, registrationId);
				preferencesWriter.putString(CORONA_PREFERENCE_PROJECT_NUMBERS_KEY, projectNumbers);
				preferencesWriter.commit();
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}
		}
	}

	/**
	 * Processes an intent received from the Google Cloud Messaging system.
	 * <p>
	 * Will finalize registration, unregistration, or post a notification depending on the
	 * information provided by the given intent.
	 * <p>
	 * This is an internal method that is only expected to be called by the
	 * GoogleCloudMessagingBroadcastReceiver class when it receives notifications.
	 * @param intent The intent object received from Google Cloud Messaging.
	 */
	void process(android.content.Intent intent) {
		// Validate.
		if (intent == null) {
			return;
		}

		// Fetch the intent's action string.
		String actionName = intent.getAction();
		if ((actionName == null) || (actionName.length() <= 0)) {
			return;
		}

		// Handle the received message.
		if (actionName.equals("com.google.android.c2dm.intent.REGISTRATION")) {
			// This application has been successfully registered for push notifications.
			String registrationId = intent.getStringExtra(INTENT_EXTRA_REGISTRATION_ID);
			String unregisteredMessage = intent.getStringExtra(INTENT_EXTRA_UNREGISTERED);
			String errorId = intent.getStringExtra(INTENT_EXTRA_ERROR_ID);
			if ((registrationId != null) && (registrationId.length() > 0)) {
				// This application has been successfully registered for push notifications.
				String projectNumbers = "";
				synchronized (GoogleCloudMessagingServices.class) {
					if (sPendingOperation instanceof RegisterOperation) {
						projectNumbers = ((RegisterOperation)sPendingOperation).getProjectNumbers();
					}
					sPendingOperation = null;
				}
				onRegisteredWith(registrationId, projectNumbers);
				executeNextQueuedOperation();
			}
			else if (unregisteredMessage != null) {
				// This application has been unregistered and will no longer receive push notifications.
				synchronized (GoogleCloudMessagingServices.class) {
					sPendingOperation = null;
				}
				onUnregistered();
				executeNextQueuedOperation();
			}
			else if ((errorId != null) && (errorId.length() > 0)) {
				synchronized (GoogleCloudMessagingServices.class) {
					// An error response has been received.
					android.util.Log.i("Corona", "ERROR: Google Cloud Messaging Registration Error: " + errorId);

					// Handle the error.
					if ((sPendingOperation instanceof RegisterOperation) && (sOperationQueue.size() > 0)) {
						// The queue contains a register or unregister operation.
						// Give up on the existing operation and execute the next one.
						sPendingOperation = null;
						executeNextQueuedOperation();
					}
					else if (errorId.equals("SERVICE_NOT_AVAILABLE") && (sPendingOperation != null)) {
						// Re-send the last sent register/unregister request since there was a connection error.
						android.os.Handler handler = new android.os.Handler(getApplicationContext().getMainLooper());
						handler.postDelayed(sPendingOperation, 60000);
					}
				}
			}
		}
		else if (actionName.equals("com.google.android.c2dm.intent.RECEIVE")) {
			// A notification has been received.
			String messageTypeName = intent.getStringExtra(INTENT_EXTRA_MESSAGE_TYPE);
			if ((messageTypeName != null) && (messageTypeName.length() > 0)) {
				// A special message from Google's server has been received.
				if (messageTypeName.equals("deleted_messages")) {
					// Notifications have been deleted from the server.
					android.util.Log.v("Corona", "Google Cloud Messaging has deleted messages.");
				}
				else {
					// An unknown message was received. Log it.
					android.util.Log.v("Corona", "Received unkown message type '" +
								messageTypeName + "' from Google Cloud Messaging.");
				}
			}
			else {
				// A push notification has been received.
				onReceivedNotification(intent.getExtras());
			}
		}
	}

	/**
	 * Called when this application has been successfully registered for push notifications with Google.
	 * <p>
	 * Stores the given registration information and raises a "remoteRegistration" event in the Corona runtime.
	 * @param registrationId Unique string ID assigned to this application by Google. Cannot be null or empty.
	 * @param projectNumbers A string containing a comma separated list of project numbers this application
	 *                       has been registered under.
	 */
	private void onRegisteredWith(String registrationId, String projectNumbers) {
		// Validate.
		if ((registrationId == null) || (registrationId.length() <= 0)) {
			return;
		}

		// Store the given registration information.
		saveRegistrationInformation(registrationId, projectNumbers);

		// Queue the registration event to be sent to the Corona runtime, but only if it is currently running.
		com.ansca.corona.events.NotificationRegistrationTask event = new com.ansca.corona.events.NotificationRegistrationTask(registrationId);
		for (com.ansca.corona.CoronaRuntime runtime : com.ansca.corona.CoronaRuntimeProvider.getAllCoronaRuntimes()) {
			runtime.getTaskDispatcher().send(event);
		}
	}

	/**
	 * Called when this application has been successfully unregistered from Google's push notification system.
	 * <p>
	 * Clears the application's saved registration information.
	 */
	private void onUnregistered() {
		saveRegistrationInformation("", "");
	}

	/**
	 * Called when a notification has been received.
	 * <p>
	 * Posts a notification to the status bar using the settings from the given bundle.
	 * @param bundle Bundle expected to come from a "com.google.android.c2dm.intent.RECEIVE" intent.
	 */
	private void onReceivedNotification(android.os.Bundle bundle) {
		// Validate.
		if (bundle == null) {
			return;
		}

		// Set up a new status bar notification.
		NotificationServices notificationServices = new NotificationServices(getApplicationContext());
		StatusBarNotificationSettings settings = new StatusBarNotificationSettings();
		settings.setId(notificationServices.reserveId());
		settings.setSourceName("google");
		settings.setSourceLocal(false);
		settings.setSourceDataName("androidGcmBundle");
		settings.setSourceData(com.ansca.corona.CoronaData.from(bundle));

		// Set the title to the application name by default.
		String applicationName = com.ansca.corona.CoronaEnvironment.getApplicationName();
		settings.setContentTitle(applicationName);
		settings.setTickerText(applicationName);

		// Copy the alert information from the bundle.
		Object value = bundle.get("alert");
		if (value instanceof String) {
			// If the alert string is a JSON table, then fetch its fields.
			String alertString = (String)value;
			boolean isJson = false;
			try {
				org.json.JSONObject jsonObject = new org.json.JSONObject(alertString);
				value = jsonObject.opt("title");
				if (value instanceof String) {
					settings.setContentTitle((String)value);
				}
				value = jsonObject.opt("body");
				if (value instanceof String) {
					settings.setContentText((String)value);
					settings.setTickerText((String)value);
				}
				else {
					value = jsonObject.opt("text");
					if (value instanceof String) {
						settings.setContentText((String)value);
						settings.setTickerText((String)value);
					}
				}
				value = jsonObject.opt("number");
				if (value instanceof Number) {
					settings.setBadgeNumber(((Number)value).intValue());
				}
				isJson = true;
			}
			catch (Exception ex) { }

			// If the alert string is not JSON, then accept the string as is.
			if (isJson == false) {
				settings.setContentText(alertString);
				settings.setTickerText(alertString);
			}
		}
		else if (value == null) {
			// If an alert field was not provided, then check the bundle itself.
			// Note: This is how Fuse provides notifications.
			value = bundle.get("title");
			if (value instanceof String) {
				settings.setContentTitle((String)value);
			}
			value = bundle.get("body");
			if (value instanceof String) {
				settings.setContentText((String)value);
				settings.setTickerText((String)value);
			}
			else {
				value = bundle.get("text");
				if (value instanceof String) {
					settings.setContentText((String)value);
					settings.setTickerText((String)value);
				}
			}
			value = bundle.get("number");
			if (value instanceof Number) {
				settings.setBadgeNumber(((Number)value).intValue());
			}
		}

		// Set the path to a custom sound file, if provided.
		value = bundle.get("sound");
		if (value instanceof String) {
			android.net.Uri uri = null;
			try {
				uri = com.ansca.corona.storage.FileContentProvider.createContentUriForFile(
							getApplicationContext(), ((String)value).trim());
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}
			settings.setSoundFileUri(uri);
		}

		// Copy the bundle's custom data, if provided.
		com.ansca.corona.CoronaData.Table customData = null;
		value = bundle.get("custom");
		if (value instanceof String) {
			try {
				customData = com.ansca.corona.CoronaData.Table.from(new org.json.JSONObject((String)value));
			}
			catch (Exception ex) { }
		}
		else if (value instanceof android.os.Bundle) {
			customData = com.ansca.corona.CoronaData.Table.from((android.os.Bundle)value);
		}
		if (customData != null) {
			settings.setData(customData);
		}

		// Post the notification to the status bar.
		notificationServices.post(settings);
	}


	/** Base class representing a single operation to be executed. */
	private static abstract class Operation implements Runnable {
		/** Creates a new operation object. */
		public Operation() {
		}

		/** Executes this operation. */
		@Override
		public abstract void run();
	}

	/** Sends a registration request to Google when the run method is called. */
	private static class RegisterOperation extends GoogleCloudMessagingServices.Operation {
		/** String storing a comma separated list of project numbers to register with. */
		private String fProjectNumbers;

		/**
		 * Creates a new operation that will register this application for push notifications.
		 * @param projectNumbers String containing a comma separated list of project numbers. Cannot be null.
		 */
		public RegisterOperation(String projectNumbers) {
			super();

			if (projectNumbers == null) {
				throw new NullPointerException();
			}
			fProjectNumbers = projectNumbers;
		}

		/**
		 * Gets a string of project numbers that is used during registration.
		 * @return Returns a string containing a comma separated list of project numbers.
		 */
		public String getProjectNumbers() {
			return fProjectNumbers;
		}

		/** Executes this operation. */
		@Override
		public void run() {
			// Fetch the application context.
			android.content.Context context = com.ansca.corona.CoronaEnvironment.getApplicationContext();
			if (context == null) {
				return;
			}

			// Send the registration request.
//			android.content.Intent intent = new android.content.Intent("com.google.android.c2dm.intent.REGISTER");
//			intent.setPackage(GOOGLE_SERVICE_FRAMEWORK_PACKAGE_NAME);
//			intent.putExtra(
//					INTENT_EXTRA_PENDING_INTENT,
//					android.app.PendingIntent.getBroadcast(context, 0, new android.content.Intent(), 0));
//			intent.putExtra(INTENT_EXTRA_PROJECT_NUMBERS, fProjectNumbers);
//			context.startService(intent);
		}
	}

	/** Sends an unregister request to Google when the run() method is called. */
	private static class UnregisterOperation extends GoogleCloudMessagingServices.Operation {
		/** Creates a new operation that will unregister this application from push notifications. */
		public UnregisterOperation() {
			super();
		}

		/** Executes this operation. */
		@Override
		public void run() {
			// Fetch the application context.
			android.content.Context context = com.ansca.corona.CoronaEnvironment.getApplicationContext();
			if (context == null) {
				return;
			}

			// Send the unregistration request if we have permission to do so.
			com.ansca.corona.permissions.PermissionsServices permissionsServices = 
				new com.ansca.corona.permissions.PermissionsServices(context);
			if (permissionsServices.getPermissionStateFor("com.google.android.c2dm.permission.RECEIVE") 
					== com.ansca.corona.permissions.PermissionState.GRANTED) {
//				android.content.Intent intent = new android.content.Intent("com.google.android.c2dm.intent.UNREGISTER");
//				intent.setPackage(GOOGLE_SERVICE_FRAMEWORK_PACKAGE_NAME);
//				intent.putExtra(
//						INTENT_EXTRA_PENDING_INTENT,
//						android.app.PendingIntent.getBroadcast(context, 0, new android.content.Intent(), 0));
//				context.startService(intent);
			}
		}
	}
}
