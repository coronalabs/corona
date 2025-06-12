//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

import java.io.File;

import android.content.Context;
import android.content.SharedPreferences;

/**
 * Provides simple methods for launching and accessing a {@link com.ansca.corona.CoronaActivity CoronaActivity} as well as setting up
 * listeners for receiving events from the {@link com.ansca.corona.CoronaRuntime CoronaRuntime}.
 * <p>
 * All methods in this class are thread safe and can be called from any thread.
 * <p>
 * You cannot make instances of this class. Access is provided via static methods.
 */
public final class CoronaEnvironment {
	private final static String CORONA_PREFERENCES_NAME = "Corona";
	private final static String CORONA_PREFERENCES_LAST_INSTALL_TIME_KEY = "lastInstallTime";

	/** Stores a reference to the application context. */
	private static android.content.Context sApplicationContext = null;
	
	/**
	 * Reference to a Corona activity that is currently running.
	 * Set to null if Corona activity is not running, not even in the background.
	 */
	private static CoronaActivity sCoronaActivity = null;
	
	/** Optional Lua error handler that can be used to override Corona's default Lua error handling. */
	private static com.naef.jnlua.JavaFunction sLuaErrorHandlerFunction = null;
	
	/** Stores a collection of listeners that will receive runtime events such as "onLoaded". */
	private static java.util.ArrayList<CoronaRuntimeListener> sRuntimeListeners = new java.util.ArrayList<CoronaRuntimeListener>();
	
	private static CoronaLuaErrorHandler sCoronaLuaErrorHandler;

	/** The constructor is made private to prevent instances from being made. */
	private CoronaEnvironment() {
	}
	
	/**
	 * Static constructor used to do the following:
	 * <ul>
	 *  <li> Set up the default Lua error handler.
	 *  <li> Set up internal event handlers that exist for the lifetime of this application.
	 * </ul>
	 */
	static {
		sCoronaLuaErrorHandler = new CoronaLuaErrorHandler();
		CoronaEnvironment.setLuaErrorHandler(sCoronaLuaErrorHandler);
		CoronaRuntime.addListener(new CoronaEnvironment.RuntimeEventHandler());
	}
	
	static void setController(Controller controller) {
		sCoronaLuaErrorHandler.setController(controller);
	}

	/**
	 * Gets the path to Corona's documents directory.
	 * <p>
	 * This is the directory that Lua property
	 * <a href="http://docs.coronalabs.com/daily/api/library/system/DocumentsDirectory.html">"system.DocumentsDirectory"</a> maps to.
	 * <p>
	 * This directory is located in internal storage.
	 * @param context <a href="http://developer.android.com/reference/android/content/Context.html">Context</a> needed to access this
	 *				  application's directories. Cannot be null.
	 * @return Returns a {@link java.io.File File} object set to the specified directory.
	 */
	public static java.io.File getDocumentsDirectory(android.content.Context context) {
		// Store the application context, if not done already.
		setApplicationContext(context);

		// Return the specified directory as a File object.
		return context.getDir("data", android.content.Context.MODE_PRIVATE);
	}

	/**
	 * Gets the path to Corona's applicationSupport directory.
	 * <p>
	 * This is the directory that Lua property
	 * <a href="http://docs.coronalabs.com/daily/api/library/system/ApplicationSupportDirectory.html">"system.ApplicationSupportDirectory"</a> maps to.
	 * <p>
	 * This directory is located in internal storage.
	 * @param context <a href="http://developer.android.com/reference/android/content/Context.html">Context</a> needed to access this
	 *				  application's directories. Cannot be null.
	 * @return Returns a {@link java.io.File File} object set to the specified directory.
	 */
	public static java.io.File getApplicationSupportDirectory(android.content.Context context) {
		// Store the application context, if not done already.
		setApplicationContext(context);

		// Return the specified directory as a File object.
		return context.getDir("support", android.content.Context.MODE_PRIVATE);
	}

	/**
	 * Gets the path to Corona's temporary directory.
	 * <p>
	 * This is the directory that Lua property
	 * <a href="http://docs.coronalabs.com/daily/api/library/system/TemporaryDirectory.html">"system.TemporaryDirectory"</a> maps to.
	 * <p>
	 * This directory is located in internal storage. Its files will be automatically deleted
	 * by the operating system if internal storage is low on space.
	 * @param context <a href="http://developer.android.com/reference/android/content/Context.html">Context</a> needed to access this
	 *				  application's directories. Cannot be null.
	 * @return Returns a {@link java.io.File File} object set to the specified directory.
	 */
	public static java.io.File getTemporaryDirectory(android.content.Context context) {
		// Store the application context, if not done already.
		setApplicationContext(context);

		// Return the specified directory as a File object.
		java.io.File directory = new java.io.File(context.getCacheDir(), "tmp");
		directory.mkdirs();
		return directory;
	}
	
	/**
	 * Gets the path to Corona's caches directory.
	 * <p>
	 * This is the directory that Lua property
	 * <a href="http://docs.coronalabs.com/daily/api/library/system/CachesDirectory.html">"system.CachesDirectory"</a> maps to.
	 * <p>
	 * This directory is located in internal storage. Its files will be automatically deleted
	 * by the operating system if internal storage is low on space.
	 * @param context <a href="http://developer.android.com/reference/android/content/Context.html">Context</a> needed to access this
	 *				  application's directories. Cannot be null.
	 * @return Returns a {@link java.io.File File} object set to the specified directory.
	 */
	public static java.io.File getCachesDirectory(android.content.Context context) {
		// Store the application context, if not done already.
		setApplicationContext(context);

		// Return the specified directory as a File object.
		java.io.File directory = new java.io.File(context.getCacheDir(), "Caches");
		directory.mkdirs();
		return directory;
	}
	
	/**
	 * Gets the path to Corona's internal caches directory.
	 * <p>
	 * This directory is not made available in Lua. It is a directory used by Corona to
	 * store information for its internal features such as analytics.
	 * <p>
	 * This directory is located in internal storage. Its files will be automatically deleted
	 * by the operating system if internal storage is low on space.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 * @param context Context needed to access this application's directories. Cannot be null.
	 * @return Returns a File object set to the specified directory.
	 */
	static java.io.File getInternalCachesDirectory(android.content.Context context) {
		// Store the application context, if not done already.
		setApplicationContext(context);

		// Return the specified directory as a File object.
		java.io.File directory = new java.io.File(context.getCacheDir(), ".system");
		directory.mkdirs();
		return directory;
	}

	/**
	 * Gets the path to Corona's internal temporary directory.
	 * <p>
	 * This directory is not made available in Lua. It is a directory used by Corona to store
	 * temporary files that will be deleted every time the Corona activity gets launched.
	 * <p>
	 * This directory is located in internal storage. Its files will be automatically deleted
	 * by the operating system if internal storage is low on space.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 * @param context Context needed to access this application's directories. Cannot be null.
	 * @return Returns a File object set to the specified directory.
	 */
	static java.io.File getInternalTemporaryDirectory(android.content.Context context) {
		// Store the application context, if not done already.
		setApplicationContext(context);

		// Return the specified directory as a File object.
		java.io.File directory = new java.io.File(getInternalCachesDirectory(context), "temp");
		directory.mkdirs();
		return directory;
	}

	/**
	 * Gets the path to Corona's internal resource cache directory.
	 * <p>
	 * This directory is not made available in Lua. It is a directory used by Corona to store
	 * temporary resource files for quick access and to make resource files available to HTML.
	 * <p>
	 * This directory will be deleted every time a new version of the app has been installed so that
	 * updated resources can replace old resources.
	 * <p>
	 * This directory is located in internal storage. Its files will be automatically deleted
	 * by the operating system if internal storage is low on space.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 * @param context Context needed to access this application's directories. Cannot be null.
	 * @return Returns a File object set to the specified directory.
	 */

	// WARNING: This is used in com.ansca.corona.maps.MapView via reflection so check there before
	// making any changes to this method
	static java.io.File getInternalResourceCachesDirectory(android.content.Context context) {
		// Store the application context, if not done already.
		setApplicationContext(context);

		// Return the specified directory as a File object.
		java.io.File directory = new java.io.File(getInternalCachesDirectory(context), "resources");
		directory.mkdirs();
		return directory;
	}

	/**
	 * Gets the path to Corona's internal web cache directory.
	 * <p>
	 * This directory is not made available in Lua. It is a directory used by Corona to cache
	 * web pages, cookies, and other web resources.
	 * <p>
	 * This directory is located in internal storage. Its files will be automatically deleted
	 * by the operating system if internal storage is low on space.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 * @param context Context needed to access this application's directories. Cannot be null.
	 * @return Returns a File object set to the specified directory.
	 */

	// WARNING: This is used in com.ansca.corona.maps.MapView via reflection so check there before
	// making any changes to this method
	static java.io.File getInternalWebCachesDirectory(android.content.Context context) {
		// Store the application context, if not done already.
		setApplicationContext(context);

		// Return the specified directory as a File object.
		java.io.File directory = new java.io.File(getInternalCachesDirectory(context), "web");
		directory.mkdirs();
		return directory;
	}

	/**
	 * Displays the {@link com.ansca.corona.CoronaActivity CoronaActivity} window on screen.
	 * If the {@link com.ansca.corona.CoronaActivity CoronaActivity} is currently suspended in the background, then this method will
	 * bring it to the foreground.
	 * <p>
	 * <b>DO NOT USE THIS API IN A <a href="http://docs.coronalabs.com/daily/coronacards/android/index.html">CORONACARDS FOR ANDROID</a>
	 * PROJECT!</b> There's no {@link com.ansca.corona.CoronaActivity CoronaActivity} in
	 * <a href="http://docs.coronalabs.com/daily/coronacards/android/index.html">CoronaCards for Android</a> so invoking this API will
	 * <b>crash your app</b>!
	 * @param context The <a href="http://developer.android.com/reference/android/content/Context.html">context</a> derived object
	 * 		  needed to display the {@link com.ansca.corona.CoronaActivity CoronaActivity} via
	 *		  <a href="http://developer.android.com/reference/android/content/Intent.html">intents</a>. Cannot be null.
	 */
	public static void showCoronaActivity(android.content.Context context) {
		// Validate.
		if (context == null) {
			return;
		}
		
		// Store the application context if not done already.
		sApplicationContext = context.getApplicationContext();
		
		// Display the Corona activity.
		android.content.Intent intent = new android.content.Intent(context, CoronaActivity.class);
		intent.addFlags(android.content.Intent.FLAG_ACTIVITY_NEW_TASK);
		intent.addFlags(android.content.Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
		context.startActivity(intent);
	}
	
	/**
	 * Stores a reference to the given Corona activity, making it available to the rest of the application.
	 * This is an internal method to be called by the CoronaActivity.onCreate() method.
	 * @param activity Reference to the CoronaActivity object that has just been created.
	 */
	static void setCoronaActivity(CoronaActivity activity) {
		// Store the application context if not done already.
		if (activity != null) {
			sApplicationContext = activity.getApplicationContext();
		}
		
		// Store the given activity reference.
		sCoronaActivity = activity;
	}
	
	/**
	 * Gets a reference to the currently active {@link com.ansca.corona.CoronaActivity CoronaActivity}.
	 * @return Returns a reference to the currently active {@link com.ansca.corona.CoronaActivity CoronaActivity} object.
	 *         Returns null if the {@link com.ansca.corona.CoronaActivity CoronaActivity} was never created, has been destroyed, or we're
	 *         using <a href="http://docs.coronalabs.com/daily/coronacards/android/index.html">CoronaCards for Android</a>.
	 */
	public static CoronaActivity getCoronaActivity() {
		return sCoronaActivity;
	}
	
	/**
	 * Assigns this class the application context.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 * @param context The context from which the application context will be extracted from. Cannot be null.
	 */
	static void setApplicationContext(android.content.Context context) {
		// Validate the given context.
		if (context == null) {
			throw new NullPointerException();
		}

		// Fetch the application context from the given context.
		context = context.getApplicationContext();
		if (context == null) {
			throw new NullPointerException();
		}

		// Store the application context.
		sApplicationContext = context;
	}

	/**
	 * Provides easy access to this application's
	 * <a href="http://developer.android.com/reference/android/content/Context.html">context</a>.
	 * @return Returns an instance of the application
	 *		   <a href="http://developer.android.com/reference/android/content/Context.html">context</a> object.
	 *         Returns null if this application has not been fully initialized yet.
	 */
	public static android.content.Context getApplicationContext() {
		return sApplicationContext;
	}

	/**
	 * Provides easy access to the application's name that is displayed to the end user.
	 * Is the same name returned by
	 * <a href="http://docs.coronalabs.com/daily/api/library/system/getInfo.html">system.getInfo("appName")</a> in Corona.
	 * <p>
	 * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2016/2869/">daily build 2016.2869</a></b>.
	 * @return Returns the application's name that is displayed to the end user.
	 *         <p>
	 *         Returns an empty {@link java.lang.String String} if
	 *         {@link com.ansca.corona.CoronaEnvironment#getApplicationContext() CoronaEnvironment.getApplicationContext()} is null.
	 */
	public static String getApplicationName() {
		String applicationName = "";
		
		if (sApplicationContext == null) {
			return applicationName;
		}

		android.content.pm.ApplicationInfo applicationInfo = sApplicationContext.getApplicationInfo();
		android.content.pm.PackageManager packageManager = sApplicationContext.getPackageManager();
		CharSequence value = packageManager.getApplicationLabel(applicationInfo);
		if (value != null) {
			applicationName = value.toString();
		}
		if (applicationName == null) {
			applicationName = "";
		}

		return applicationName;
	}
	
	/**
	 * Gets the custom Lua error handler that was given to the
	 * {@link com.ansca.corona.CoronaEnvironment#setLuaErrorHandler(com.naef.jnlua.JavaFunction) CoronaEnvironment.setLuaErrorHandler()}
	 * method.
	 * @return Returns a reference to a custom Lua error handler.
	 *         This is a Lua function that is implemented in Java.
	 *         <p>
	 *         Returns null if a custom error handler was not set.
	 *         In this case, Corona will use its default error handling.
	 */
	public static com.naef.jnlua.JavaFunction getLuaErrorHandler() {
		return sLuaErrorHandlerFunction;
	}
	
	/**
	 * Sets a custom handler for Lua errors, such as syntax or runtime errors.
	 * @param handler Reference to the custom Lua error handler.
	 *                This is a Lua function that is implemented in Java.
	 *                <p>
	 *                Set to null to use Corona's default error handler.
	 */
	public static void setLuaErrorHandler(com.naef.jnlua.JavaFunction handler) {
		// Do not continue if the handler has not changed.
		if (handler == sLuaErrorHandlerFunction) {
			return;
		}
		
		// Store the new handler.
		sLuaErrorHandlerFunction = handler;
		
		// Enable/disable custom error handling in the native side of Corona.
		if (handler != null) {
			JavaToNativeShim.useJavaLuaErrorHandler();
		}
		else {
			JavaToNativeShim.useDefaultLuaErrorHandler();
		}
	}
	
	/**
	 * Called by the C++ side of Corona when a Lua error occurs.
	 * Should only be invoked if JavaToNativeShim.useDefaultLuaErrorHandler() has been called,
	 * which sets up the core code to use a custom error handler implemented on the Java side.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 * @param luaStateMemoryAddress The memory address (ie: pointer) to the Lua state that the error occurred in.
	 * @return Returns the number of values returned by the Lua error handler.
	 */
	static int invokeLuaErrorHandler(long luaStateMemoryAddress) {
		// Validate argument.
		if (luaStateMemoryAddress == 0) {
			return 1;
		}
		
		// Copy the Lua error handler reference in case it suddenly changes in another thread.
		// This prevents possible race conditions.
		com.naef.jnlua.JavaFunction luaErrorHandlerFunction = sLuaErrorHandlerFunction;
		
		// Do not continue if we do not have an error handler.
		if (luaErrorHandlerFunction == null) {
			return 1;
		}
		
		com.naef.jnlua.LuaState luaState = new com.naef.jnlua.LuaState(luaStateMemoryAddress);
		
		// Invoke the Lua error handler.
		return luaErrorHandlerFunction.invoke(luaState);
	}
	
	/**
	 * Adds a listener for receiving events from the {@link com.ansca.corona.CoronaRuntime CoronaRuntime}.
	 * @param listener The listener that will receive events. Cannot be null.
	 * @see CoronaRuntimeListener
	 */
	public static void addRuntimeListener(CoronaRuntimeListener listener) {
		synchronized (sRuntimeListeners) {
			// Validate.
			if (listener == null) {
				return;
			}
			
			// Do not continue if the given listener already exists in the collection.
			if (sRuntimeListeners.indexOf(listener) >= 0) {
				return;
			}
			
			// Add the given listener to the collection.
			sRuntimeListeners.add(listener);
		}
	}
	
	/**
	 * Removes the given listener that was added via the
	 * {@link com.ansca.corona.CoronaEnvironment#addRuntimeListener(com.ansca.corona.CoronaRuntimeListener) addRuntimeListener()} method.
	 * @param listener The listener object to be removed by reference.
	 * @see CoronaRuntimeListener
	 */
	public static void removeRuntimeListener(CoronaRuntimeListener listener) {
		synchronized (sRuntimeListeners) {
			// Validate.
			if (listener == null) {
				return;
			}
			
			// Remove the given listener from the collection.
			sRuntimeListeners.remove(listener);
		}
	}
	
	
	/** Private class that is only used by the CoronaEnvironment class for handling Corona runtime events. */
	private static class RuntimeEventHandler implements CoronaRuntimeListener {
		/**
		 * Creates and returns a shallow copy of the CoronaRuntimeListener collection "sRuntimeListeners".
		 * <p>
		 * This is needed by this class' event raising methods such as onLoaded() to access the collection in
		 * a thread safe manner and to allow a called listener to add/remove listeners from this collection
		 * without invalidating the iterator which would cause an exception.
		 * @return Returns a shallow copy of the CoronaRuntimeListener collection.
		 */
		private java.util.ArrayList<CoronaRuntimeListener> cloneListenerCollection() {
			synchronized (sRuntimeListeners) {
				return (java.util.ArrayList<CoronaRuntimeListener>)sRuntimeListeners.clone();
			}
		}
		
		/**
		 * Called after the Corona runtime has been created and just before executing the "main.lua" file.
		 * @param runtime Reference to the CoronaRuntime object that has just been loaded/initialized.
		 */
		public void onLoaded(CoronaRuntime runtime) {
			for (CoronaRuntimeListener listener : cloneListenerCollection()) {
				if (listener != null) {
					listener.onLoaded(runtime);
				}
			}
		}
		
		/**
		 * Called just after the Corona runtime has executed the "main.lua" file.
		 * @param runtime Reference to the CoronaRuntime object that has just been started.
		 */
		public void onStarted(CoronaRuntime runtime) {
			for (CoronaRuntimeListener listener : cloneListenerCollection()) {
				if (listener != null) {
					listener.onStarted(runtime);
				}
			}
		}
		
		/**
		 * Called just after the Corona runtime has been suspended.
		 * @param runtime Reference to the CoronaRuntime object that has just been suspended.
		 */
		public void onSuspended(CoronaRuntime runtime) {
			for (CoronaRuntimeListener listener : cloneListenerCollection()) {
				if (listener != null) {
					listener.onSuspended(runtime);
				}
			}
		}
		
		/**
		 * Called just after the Corona runtime has been resumed after a suspend.
		 * @param runtime Reference to the CoronaRuntime object that has just been resumed.
		 */
		public void onResumed(CoronaRuntime runtime) {
			for (CoronaRuntimeListener listener : cloneListenerCollection()) {
				if (listener != null) {
					listener.onResumed(runtime);
				}
			}
		}
		
		/**
		 * Called just before the Corona runtime terminates.
		 * @param runtime Reference to the CoronaRuntime object that is being terminated.
		 */
		public void onExiting(CoronaRuntime runtime) {
			for (CoronaRuntimeListener listener : cloneListenerCollection()) {
				if (listener != null) {
					listener.onExiting(runtime);
				}
			}
		}
	}

	/**
	 * Checks to see if the current apk of Corona is a new install.  This does NOT write the new install time.
	 * @param context The Context
	 */
	static boolean isNewInstall(Context context) {
		try {
			SharedPreferences preferencesReader = context.getSharedPreferences(CORONA_PREFERENCES_NAME, Context.MODE_PRIVATE);
			if (preferencesReader != null) {
				File apkFile = new File(context.getPackageCodePath());
				long currentTimestamp = apkFile.lastModified();
				long lastTimestamp = preferencesReader.getLong(CORONA_PREFERENCES_LAST_INSTALL_TIME_KEY, 0L);
				return currentTimestamp != lastTimestamp;
			}
		} catch (Exception ex) { }
		return true;
	}

	/**
	 * Writes the current install time of the apk into the preferences so it can be checked against next time the app is run.
	 * This will overwrite the old install time with the new install time.  This will also delete the coronaResources directory
	 * which is where the resources directory extracts to.  This is done to ensure that any updated resource files are properly updated.
	 * @param context The Context
	 */
	static void onNewInstall(Context context) {
		try {
			SharedPreferences preferencesReader = context.getSharedPreferences(CORONA_PREFERENCES_NAME, Context.MODE_PRIVATE);
			if (preferencesReader != null) {
				// Write the current install time to the preferences
				File apkFile = new File(context.getPackageCodePath());
				long currentTimestamp = apkFile.lastModified();
				SharedPreferences.Editor preferencesWriter = preferencesReader.edit();
				preferencesWriter.putLong(CORONA_PREFERENCES_LAST_INSTALL_TIME_KEY, currentTimestamp);
				preferencesWriter.commit();

				deleteDirectoryTree(context.getFileStreamPath("coronaResources"));
			}
		} catch (Exception ex) { }		
	}

	static void deleteTempDirectory(Context context) {
		// Delete Corona's internal temporary directory every time we start up this activity.
		try { deleteDirectoryTree(CoronaEnvironment.getInternalTemporaryDirectory(context)); }
		catch (Exception ex) { }
	}

	/**
	 * Deletes the target directory's files and subdirectories.
	 * @param target Reference to a directory or file to be deleted. Cannot be null.
	 */
	private static void deleteDirectoryTree(java.io.File target) {
		if (target.isDirectory()) {
			for (java.io.File file : target.listFiles()) {
				deleteDirectoryTree(file);
			}
		}
		target.delete();
	}
}
