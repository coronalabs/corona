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
 * Activity used to play a video file or stream via Corona's media.playVideo() function.
 * Example usage:
 *    android.content.Intent myIntent = new android.content.Intent(myActivity, VideoActivity.class);
 *    myIntent.putExtra("video_uri", "http://www.mydomain.com/myvideo.mp3" );
 *    myIntent.putExtra("video_id", uniqueIntegerIdForThisActivity);
 *    myIntent.putExtra("media_controls_enabled", true);
 *    myActivity.startActivity(myIntent);
 */
public class VideoActivity extends android.app.Activity {

	/** View used to display the video. */
	private CoronaVideoView fVideoView;
	
	/** Unique integer ID assigned to this activity by this application's MediaManager class. */
	private long fVideoId;

	/** Set true if the activity's onPause() was called. Set false if currently active. */
	private boolean fWasActivityPaused;

	/** Set true if the video view is currently suspended. Set false if running/resumed. */
	private boolean fWasVideoSuspended;

	/** Broadcast receiver which receives and handles screen related events. */
	private ScreenEventHandler fScreenEventHandler;

	/** Animated spinner indicating that something is loading. */
	private android.widget.ProgressBar fLoadingIndicatorView;


	/**
	 * Initializes this activity's UI and member variables.
	 */
	@Override
	protected void onCreate(android.os.Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		// Wrap this activity context with a new context using the newest theme available on the system.
		// This way, child views can use the newest theme and this activity can use the fullscreen theme.
		int themeId;
		if (android.os.Build.VERSION.SDK_INT >= 14) {
			themeId = 16974120;		// android.R.style.Theme_DeviceDefault
		}
		else if (android.os.Build.VERSION.SDK_INT >= 11) {
			themeId = 16973931;		// android.R.style.Theme_Holo
		}
		else {
			themeId = android.R.style.Theme_Dialog;
		}
		android.view.ContextThemeWrapper themedContextWrapper;
		themedContextWrapper = new android.view.ContextThemeWrapper(this, themeId);

		// Listen for when the screen turns off/on and when the screen lock is hidden/shown.
		// We need this to avoid playing a video while the screen lock is covering up the activity.
		fScreenEventHandler = new ScreenEventHandler(this);

		// Create the video view and center it within the window.
		android.widget.FrameLayout layout = new android.widget.FrameLayout(themedContextWrapper);
		layout.setBackgroundColor(android.graphics.Color.BLACK);
		setContentView(layout);
		android.widget.FrameLayout.LayoutParams layoutParams;
		layoutParams = new android.widget.FrameLayout.LayoutParams(
				android.widget.FrameLayout.LayoutParams.FILL_PARENT,
				android.widget.FrameLayout.LayoutParams.FILL_PARENT,
				android.view.Gravity.CENTER_HORIZONTAL | android.view.Gravity.CENTER_VERTICAL);
		fVideoView = new CoronaVideoView(themedContextWrapper);
		layout.addView(fVideoView, layoutParams);

		// Set up the volume controls to adjust the "media" volume instead of the default "ringer" volume.
		setVolumeControlStream(android.media.AudioManager.STREAM_MUSIC);
		
		// Create an loading indicator and place it on top of the video view.
		// Will be shown while the video is loading.
		fLoadingIndicatorView = new android.widget.ProgressBar(themedContextWrapper);
		fLoadingIndicatorView.setLayoutParams(new android.widget.FrameLayout.LayoutParams(
				android.widget.FrameLayout.LayoutParams.WRAP_CONTENT,
				android.widget.FrameLayout.LayoutParams.WRAP_CONTENT,
				android.view.Gravity.CENTER));
		layout.addView(fLoadingIndicatorView);

		// Set up listeners to show/hide the loading indicator while the video is loading/loaded.
		fVideoView.setOnPreparedListener(new android.media.MediaPlayer.OnPreparedListener() {
			@Override
			public void onPrepared(android.media.MediaPlayer mediaPlayer) {
				// *** A new media player was created. ***

				// Hide the loading indicator. The video is usuallly loaded when a new player has been created.
				fLoadingIndicatorView.setVisibility(android.view.View.GONE);

				// Set up info listeners on the media player to know when it is buffering, such as during a seek.
				// Note: There is a bug on some devices (ex: LG Nexus 4) where buffering events will never be received.
				mediaPlayer.setOnInfoListener(new android.media.MediaPlayer.OnInfoListener() {
					@Override
					public boolean onInfo(android.media.MediaPlayer mediaPlayer, int what, int extra) {
						fLoadingIndicatorView.setVisibility(android.view.View.GONE);
						switch (what) {
							case android.media.MediaPlayer.MEDIA_INFO_BUFFERING_START:
								// Show the loading indicating while the video is buffering. It can't play right now.
								fLoadingIndicatorView.setVisibility(android.view.View.VISIBLE);
								break;
							case android.media.MediaPlayer.MEDIA_INFO_BUFFERING_END:
								// Hide the loading indicating when the video has finished buffering,
								// indicating that the video can now play at its current position.
								fLoadingIndicatorView.setVisibility(android.view.View.GONE);
								break;
						}
						return false;
					}
				});
			}
		});

		// Set up this activity to automatically close once the video ends or if an error has been encountered.
		fVideoView.setOnCompletionListener(new android.media.MediaPlayer.OnCompletionListener() {
			@Override
			public void onCompletion(android.media.MediaPlayer player) {
				for (com.ansca.corona.CoronaRuntime runtime : com.ansca.corona.CoronaRuntimeProvider.getAllCoronaRuntimes()) {
					if (!runtime.wasDisposed()) {
						runtime.getTaskDispatcher().send(new com.ansca.corona.events.VideoEndedTask(fVideoId));
					}
					MediaManager mediaManager = runtime.getController().getMediaManager();
					if (mediaManager != null) {
						mediaManager.resumeAll();
					}
				}
				
				finish();
			}
		});
		
		// Set up the video view and start playing.
		if (getIntent().getExtras().getBoolean("media_controls_enabled")) {
			fVideoView.setMediaController(new android.widget.MediaController(themedContextWrapper));
		}
		fVideoView.setVideoURIUsingCoronaProxy(android.net.Uri.parse(getIntent().getExtras().getString("video_uri")));
		fVideoId = getIntent().getExtras().getLong("video_id");
		fWasActivityPaused = false;
		fWasVideoSuspended = false;
		fVideoView.start();
	}

	/**
	 * Closes this activity.
	 * Overriden to prevent this activity from doing a slide-out animation.
	 */
	@Override
	public void finish() {
		// Have the base class close this window.
		super.finish();
		
		// If this activity is flagged to not show transition animations, then disable the slide-out animation.
		if ((getIntent().getFlags() & android.content.Intent.FLAG_ACTIVITY_NO_ANIMATION) != 0) {
			overridePendingTransition(0, 0);
		}
	}

	/** Called when the back key has been pressed. */
	@Override
	public void onBackPressed() {
		// Let the base class finish/close the activity.
		super.onBackPressed();

		// Traverse all active Corona runtimes.
		for (CoronaRuntime runtime : CoronaRuntimeProvider.getAllCoronaRuntimes()) {
			// Dispatch a Lua completion event.
			if (!runtime.wasDisposed()) {
				runtime.getTaskDispatcher().send(new com.ansca.corona.events.VideoEndedTask(fVideoId));
			}

			// Resume all other media content belonging to the runtime.
			MediaManager mediaManager = runtime.getController().getMediaManager();
			if (mediaManager != null) {
				mediaManager.resumeAll();
			}
		}
	}
	
	/** Called when the activity is no longer visible or active. */
	@Override
	protected void onPause() {
		super.onPause();

		// Suspend video playback.
		fWasActivityPaused = true;
		updateSuspendResumeState();
	}

	/**
	 * Called when the activity has been brought to the foreground.
	 * Note: The lock screen might be covering up the activity during a resume.
	 */
	@Override
	protected void onResume() {
		super.onResume();

		// Resume video back to its last playback state.
		fWasActivityPaused = false;
		updateSuspendResumeState();
	}

	/** Called just before the activity has been closed and destroyed. */
	@Override
	protected void onDestroy() {
		// Unregister the following event handler from the system.
		if (fScreenEventHandler != null) {
			fScreenEventHandler.dispose();
			fScreenEventHandler = null;
		}
		
		// Destroy this activity.
		super.onDestroy();
	}

	/** Suspends or resumes the video view based on the current state of the activity and screen lock. */
	private void updateSuspendResumeState() {
		if (fWasActivityPaused) {
			// The activity is currently paused.
			// Suspend the video if not done already.
			if (fWasVideoSuspended == false) {
				fWasVideoSuspended = true;
				fVideoView.suspend();
			}
		}
		else if (isScreenLocked() == false) {
			// The activity is currently active and the screen is unlocked.
			// Resume the video if not done already.
			// Note: Many devices will call the activity's onResume() method while the screen lock is still displayed.
			//       We should never resume the video while the screen lock is shown, because the video can't be seen.
			if (fWasVideoSuspended) {
				fWasVideoSuspended = false;
				fVideoView.resume();
			}
		}
	}

	/**
	 * Determines if the screen is currently locked and does not allow user interaction. This includes when the screen is off.
	 * @return Returns true if screen is locked. Returns false if screen is unlocked and allows user interaction.
	 */
	private boolean isScreenLocked() {
		android.app.KeyguardManager keyguardManager;
		keyguardManager = (android.app.KeyguardManager)getSystemService(android.content.Context.KEYGUARD_SERVICE);
		return keyguardManager.inKeyguardRestrictedInputMode();
	}


	/** Private class used to listen for broadcasted screen events. */
	private static class ScreenEventHandler extends android.content.BroadcastReceiver {
		/** The video activity that owns this object. */
		private VideoActivity fVideoActivity;

		/**
		 * Creates a new event handler for broadcasted screen events.
		 * @param videoActivity The video activity that owns this object. Cannot be null.
		 */
		public ScreenEventHandler(VideoActivity videoActivity) {
			super();
			
			// Validate.
			if (videoActivity == null) {
				throw new NullPointerException();
			}
			
			// Initialize member variables.
			fVideoActivity = videoActivity;
			
			// Register this broadcast receiver.
			android.content.IntentFilter intentFilter = new android.content.IntentFilter();
			intentFilter.addAction(android.content.Intent.ACTION_SCREEN_OFF);
			intentFilter.addAction(android.content.Intent.ACTION_SCREEN_ON);
			intentFilter.addAction(android.content.Intent.ACTION_USER_PRESENT);
			fVideoActivity.registerReceiver(this, intentFilter);
		}
		
		/**
		 * Unregisters and disables this broadcast receiver.
		 * Must be called by the video activity's onDestroy() method.
		 */
		public void dispose() {
			fVideoActivity.unregisterReceiver(this);
		}

		/**
		 * This method is called when a screen event is being broadcasted.
		 * @param context The context in which the receiver is running.
		 * @param intent The intent being received. This contains the event.
		 */
		@Override
		public void onReceive(android.content.Context context, android.content.Intent intent) {
			// Validate.
			if (intent == null) {
				return;
			}
			
			// Fetch the event's unique string key.
			String actionName = intent.getAction();
			if ((actionName == null) || (actionName.length() <= 0)) {
				return;
			}
			
			// Update the video's suspend/resume state if the screen's current state has changed.
			if (actionName.equals(android.content.Intent.ACTION_SCREEN_OFF) ||
			    actionName.equals(android.content.Intent.ACTION_SCREEN_ON) ||
			    actionName.equals(android.content.Intent.ACTION_USER_PRESENT))
			{
				fVideoActivity.updateSuspendResumeState();
			}
		}
	}
}
