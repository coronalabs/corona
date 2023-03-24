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
 * Monitors the current state of the Android device and listens for broadcasted system events such as screen off/on.
 */
public class SystemMonitor {
	/** The context that owns this object. Needed to register for broadcast events. */
	private android.content.Context fContext;
	
	/** Object used to listen for broadcasted system events and reports current state of the system to this object. */
	private SystemEventHandler fSystemEventHandler;
	
	/** Set true if the screen is on. Set false if the screen is off. */
	private boolean fIsScreenOn;
	
	/** Set true if the system is low on memory. Set false if not. */
	private boolean fIsLowOnMemory;

	/** Set true if the ringer's silent mode is currently enabled. */
	private boolean fIsSilentModeEnabled;

	/** Reference to a Corona runtime associated with this monitor. Can be null. */
	private CoronaRuntime fCoronaRuntime;
	
	/** Listener to be invoked when system events have been received, such as a screen change. Can be null. */
	private CoronaApiListener fListener;


	/**
	 * Creates a new monitor object which listens for system events.
	 * @param context The context object that owns this monitor and used to register for broadcasted system events. Cannot be null.
	 */
	public SystemMonitor(CoronaRuntime runtime, android.content.Context context) {
		// Validate.
		if (context == null) {
			throw new NullPointerException();
		}
		
		// Initialize member variables.
		fContext = context;
		fSystemEventHandler = null;
		fIsScreenOn = true;
		fIsLowOnMemory = false;
		fIsSilentModeEnabled = false;
		fCoronaRuntime = runtime;
		fListener = null;
	}

	/**
	 * Sets the listener to be invoked when a system event has been received, such as onScreenLockStateChanged().
	 * @param listener The listener to be invoked. Can be null.
	 */
	public void setCoronaApiListener(CoronaApiListener listener) {
		fListener = listener;
	}
	
	/** Starts monitoring for system events. */
	public void start() {
		// Do not continue if this is CoronaCards.
		if ((fCoronaRuntime != null) && fCoronaRuntime.isCoronaKit()) {
			return;
		}

		// Start monitoring, if not already started.
		if (fSystemEventHandler == null ) {
			fSystemEventHandler = new SystemEventHandler(this);
		}
	}
	
	/** Stops monitoring for system events. */
	public void stop() {
		// Do not continue if already stopped.
		if (fSystemEventHandler == null) {
			return;
		}

		// Dispose of the system event handler.
		fSystemEventHandler.dispose();
		fSystemEventHandler = null;

//TODO: Temporarily removed mute handling until we find a solution that Google Play will accept.
//		// Enable audio output in case it is currently muted.
//		// We do this because this monitor is no longer handling the ringer's silent mode.
//		android.media.AudioManager audioManager;
//		audioManager = (android.media.AudioManager)fActivity.getSystemService(android.content.Context.AUDIO_SERVICE);
//		audioManager.setStreamMute(android.media.AudioManager.STREAM_MUSIC, false);
		fIsSilentModeEnabled = false;
	}
	
	/**
	 * Forces this object to check the status of the system now and sends events to Corona Lua listeners.
	 */
	public void update() {
//		isLowOnMemory();
	}
	
	/**
	 * Determines if this system monitor object has been started and is currently listening for system events.
	 * @return Returns true if the system monitor has been started. Returns false if it is stopped.
	 */
	public boolean isRunning() {
		return (fSystemEventHandler != null);
	}
	
	private android.content.Context getContext() {
		return fContext;
	}

	/**
	 * Determines if the screen is currently on and it is not showing the screen-lock window.
	 * @return Returns true if the screen is on. Returns false if the screen is powered off or the screen-lock window is shown.
	 */
	public boolean isScreenOn() {
		return fIsScreenOn;
	}
	
	/**
	 * Determines if the screen is currently powered off or if the screen-lock window is shown.
	 * @return Returns true if the screen is powered off or the screen-lock window is shown. Returns false if the screen is shown.
	 */
	public boolean isScreenOff() {
		return !fIsScreenOn;
	}
	
	/**
	 * Determines if the screen is currently locked and does not allow user interaction. This includes when the screen is off.
	 * @return Returns true if screen is locked. Returns false if screen is unlocked and allows user interaction.
	 */
	public boolean isScreenLocked() {
		android.app.KeyguardManager keyguardManager;
		keyguardManager = (android.app.KeyguardManager)fContext.getSystemService(android.content.Context.KEYGUARD_SERVICE);
		//Use new/updated method if supported
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP_MR1) {
     return keyguardManager.isDeviceLocked();
	 }else{
		 return keyguardManager.inKeyguardRestrictedInputMode();
	 }

	}
	/**
	 * Determines if the screen is currently unlocked and allows user interaction.
	 * @return Returns true if screen is unlocked. Returns false if screen is locked or powered off.
	 */
	public boolean isScreenUnlocked() {
		return !isScreenLocked();
	}
	
	/**
	 * Determines if the system is currently low on memory.
	 * Automatically sends a "memoryWarning" event to a Corona Lua listener if low memory was detected.
	 * @return Returns true if system is low on memory. Returns false if not.
	 */
	public boolean isLowOnMemory() {
		// Fetch system memory status.
		// Note: We have to poll for this warning because the activity's onLowMemory() method never gets called based on my testing.
		android.app.ActivityManager activityManager;
		android.app.ActivityManager.MemoryInfo memoryInfo = new android.app.ActivityManager.MemoryInfo();
		activityManager = (android.app.ActivityManager)fContext.getSystemService(android.content.Context.ACTIVITY_SERVICE);
		activityManager.getMemoryInfo(memoryInfo);
		
		// Send an event to a Lua listener if the state has changed.
		if (fIsLowOnMemory != memoryInfo.lowMemory) {
			fIsLowOnMemory = memoryInfo.lowMemory;
			if (fCoronaRuntime != null) {
				com.ansca.corona.events.EventManager eventManager = fCoronaRuntime.getController().getEventManager();
				if (fIsLowOnMemory && (eventManager != null)) {
					eventManager.addEvent(new com.ansca.corona.events.RunnableEvent(new java.lang.Runnable() {
						@Override
						public void run() {
							if (fCoronaRuntime.getController() != null) {
								JavaToNativeShim.memoryWarningEvent(fCoronaRuntime);
							}
						}
					}));
				}
			}
		}
		
		// Return the low memory state.
		return fIsLowOnMemory;
	}

	/**
	 * Determines if the ringer's silent mode is currently enabled.
	 * @return Returns true if the device is in silent mode, which means it is in "mute" or "vibrate" mode.
	 *         <p>
	 *         Returns false if the device's ringer is not muted.
	 */
	public boolean isSilentModeEnabled() {
		// Determine if the ringer's silent mode is enabled.
		android.media.AudioManager audioManager;
		audioManager = (android.media.AudioManager)fContext.getSystemService(android.content.Context.AUDIO_SERVICE);
		boolean isMuted = (audioManager.getRingerMode() != android.media.AudioManager.RINGER_MODE_NORMAL);

		// Enable/disable audio output if silent mode has changed.
		// Note: The setStreamMute() method counts the number of times audio has been muted by this process.
		//       This means that you have to un-mute the same number of times in order to hear audio again.
		//       Because of this behavior, you should never mute the audio more than once.
		if (isMuted != fIsSilentModeEnabled) {
//TODO: Temporarily removed mute handling until we find a solution that Google Play will accept.
//			audioManager.setStreamMute(android.media.AudioManager.STREAM_MUSIC, isMuted);
			fIsSilentModeEnabled = isMuted;
		}

		// Return the device's silent mode state.
		return fIsSilentModeEnabled;
	}
	
	
	/**
	 * Private class only used by the SystemMonitor class used to listen for broadcasted system events.
	 */
	private static class SystemEventHandler extends android.content.BroadcastReceiver {
		/** The SystemMonitor object that owns this object. */
		private SystemMonitor fMonitor;

		/**
		 * Creates a new event handler for broadcasted system events.
		 * @param monitor The SystemMonitor object that will receive events from this object. Cannot be null.
		 */
		public SystemEventHandler(SystemMonitor monitor) {
			super();
			
			// Validate.
			if (monitor == null) {
				throw new NullPointerException();
			}
			
			// Initialize member variables.
			fMonitor = monitor;
			
			// Register this broadcast receiver with the monitor's activity.
			android.content.IntentFilter intentFilter = new android.content.IntentFilter();
			intentFilter.addAction(android.content.Intent.ACTION_SCREEN_OFF);
			intentFilter.addAction(android.content.Intent.ACTION_SCREEN_ON);
			intentFilter.addAction(android.content.Intent.ACTION_USER_PRESENT);
			intentFilter.addAction(android.media.AudioManager.RINGER_MODE_CHANGED_ACTION);
			fMonitor.getContext().registerReceiver(this, intentFilter);
		}
		
		/**
		 * Unregisters and disables this broadcast receiver.
		 * Must be called before exiting the application.
		 */
		public void dispose() {
			fMonitor.getContext().unregisterReceiver(this);
		}

		/**
		 * This method is called when a system event is being broadcasted.
		 * @param context The context in which the receiver is running.
		 * @param intent The intent being received. This contains the event.
		 */
		@Override
		public void onReceive(android.content.Context context, android.content.Intent intent) {
			// Validate.
			if (intent == null) {
				return;
			}
			
			// Fetch the event unique string key.
			String actionName = intent.getAction();
			if ((actionName == null) || (actionName.length() <= 0)) {
				return;
			}
			
			// Handle the system event.
			CoronaApiListener listener = fMonitor.fListener;
			if (actionName.equals(android.content.Intent.ACTION_SCREEN_OFF)) {
				fMonitor.fIsScreenOn = false;
				if (listener != null) {
					listener.onScreenLockStateChanged(true);
				}
			}
			else if (actionName.equals(android.content.Intent.ACTION_SCREEN_ON)) {
				fMonitor.fIsScreenOn = true;
				if (listener != null) {
					listener.onScreenLockStateChanged(fMonitor.isScreenLocked());
				}
			}
			else if (actionName.equals(android.content.Intent.ACTION_USER_PRESENT)) {
				if (listener != null) {
					listener.onScreenLockStateChanged(false);
				}
			}
			else if (actionName.equals(android.media.AudioManager.RINGER_MODE_CHANGED_ACTION)) {
				fMonitor.isSilentModeEnabled();
			}
		}
	}
}
