//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;


import java.util.LinkedList;
import java.util.List;

/**
 * View used to play videos.
 * <p>
 * This class derives from Google's VideoView class to provide additional features such as the ability
 * to change the volume, mute/unmute, and to optionally tap the view to pause/resume playback.
 * <p>
 * This class also resolves issues/bugs in Google's implementation where it'll fail to stream videos from
 * servers that do not support HTTP range requests missing an upper bound. This issue is worked-around
 * by this class by calling its setVideoURIUsingCoronaProxy() method.
 * <p>
 * This class is used by Corona's native.newVideo() and media.playVideo() Lua APIs.
 */
public class CoronaVideoView extends android.widget.VideoView {
	/**
	 * View containing a "CoronaVideoView" centered within it.
	 * <p>
	 * Instances of this class are created by Corona's native.newVideo() Lua function.
	 */
	public static class CenteredLayout extends android.widget.FrameLayout implements com.ansca.corona.NativePropertyResponder {
		/** The video view that is centered within this layout. */
		private CoronaVideoView fVideoView;

		/**
		 * Creates a new view for displaying videos.
		 * @param context The context this view will be associated with. Typically the activity context. Cannot be null.
		 * @param runtime Corona runtime that this video belongs to. Can be null.
		 *                When set, this view will dispatch Lua "video" events to this runtime.
		 *                Only expected to be set for videos created by the Lua native.newVideo() function.
		 */
		public CenteredLayout(android.content.Context context, CoronaRuntime runtime) {
			super(context);

			fVideoView = new CoronaVideoView(context, runtime);
			fVideoView.setLayoutParams(new android.widget.FrameLayout.LayoutParams(
					android.widget.FrameLayout.LayoutParams.FILL_PARENT,
					android.widget.FrameLayout.LayoutParams.FILL_PARENT,
					android.view.Gravity.CENTER));
			addView(fVideoView);
		}

		/**
		 * Gets the CoronaVideoView instance contained in this view.
		 * @return Returns a reference to the video view.
		 */
		public CoronaVideoView getVideoView() {
			return fVideoView;
		}

		/**
		 * Sets the next animation to play for this view.
		 * @param animation The next animation to play. Set to null to clear the last applied animation.
		 */
		@Override
		public void setAnimation(android.view.animation.Animation animation) {
			super.setAnimation(animation);
			fVideoView.setAnimation(animation);
		}

		/**
		 * Starts the given animation immediately.
		 * @param animation The animation to play now.
		 */
		@Override
		public void startAnimation(android.view.animation.Animation animation) {
			super.startAnimation(animation);
			fVideoView.startAnimation(animation);
		}

		/**
		 * Assigns an integer identifier to this view.
		 * The identifier does not have to be unique in this view's hierarchy.
		 * The identifier should be a positive number.
		 */
		public void setId(int id) {
			super.setId(id);

			// The video view needs this ID to dispatch Corona Lua events. Pass it through.
			fVideoView.setId(id);
		}

		@Override
		public List<Object> getNativePropertyResponder() {
			List<Object> out = new LinkedList<Object>();
			out.add(fVideoView);
			out.add(this);
			return out;
		}

		@Override
		public Runnable getCustomPropertyAction(String key, boolean booleanValue, String stringValue, int integerValue, double doubleValue) {
			return null;
		}
	}


	/** Reference to the media player currently playing the video. Set null if not playing a video. */
	private android.media.MediaPlayer fMediaPlayer;

	/** Normalized volume value ranging between 0 and 1.0 to be applied to "fMediaPlayer". */
	private float fVolume;

	/** Previous normalized volume value (ranging 0 and 1.0) that was applied before mute() was called. */
	private float fPrevVolume;

	/** If set to true, error would be handled internally */
	private boolean fIgnoreErrors;

	public void IgnoreErrors(boolean ignore) {
		fIgnoreErrors = ignore;
	}

	/** Set true to enable tapping the view to pause/resume the video. */
	private boolean fTouchTogglesPlay;

	/** Task dispatcher used to dispatch Corona "video" events to Lua. Can be null. */
	private CoronaRuntimeTaskDispatcher fTaskDispatcher;

	/**
	 * Stores the listener give to this video view externally via its setOnPreparedListener() method.
	 * This is needed since this CoronaVideoView class sets its own "onPrepared" listener.
	 */
	private android.media.MediaPlayer.OnPreparedListener fExternalOnPreparedListener;

	/**
	 * Stores the listener give to this video view externally via its setOnCompletionListener() method.
	 * This is needed since this CoronaVideoView class sets its own "onCompletion" listener.
	 */
	private android.media.MediaPlayer.OnCompletionListener fExternalOnCompletionListener;

	/**
	 * Internal HTTP proxy server this video view uses when the setVideoURIUsingCoronaProxy() method is called.
	 * Used to manipulate Google's media player HTTP requests to something that more web servers can support.
	 * <p>
	 * Set to null if a proxy is not currently being used.
	 */
	private ProxyServer fProxyServer;


	/**
	 * Creates a new view for displaying videos.
	 * @param context The context this view will be associated with. Typically the activity context. Cannot be null.
	 */
	public CoronaVideoView(android.content.Context context) {
		this(context, null);
	}
	
	/**
	 * Creates a new view for displaying videos.
	 * @param context The context this view will be associated with. Typically the activity context. Cannot be null.
	 * @param runtime Corona runtime that this video belongs to. Can be null.
	 *                When set, this view will dispatch Lua "video" events to this runtime.
	 *                Only expected to be set for videos created by the Lua native.newVideo() function.
	 */
	public CoronaVideoView(android.content.Context context, CoronaRuntime runtime) {
		super(context);

		// Initialize member variables.
		fMediaPlayer = null;
		fVolume = 1.0f;
		fPrevVolume = 1.0f;
		fTouchTogglesPlay = false;
		fExternalOnPreparedListener = null;
		fExternalOnCompletionListener = null;
		fIgnoreErrors = false;
		fTaskDispatcher = null;
		fProxyServer = null;
		if (runtime != null) {
			fTaskDispatcher = runtime.getTaskDispatcher();
		}

		// Set up a listener to be invoked when the video has been prepared/loaded.
		super.setOnPreparedListener(new android.media.MediaPlayer.OnPreparedListener() {
			@Override
			public void onPrepared(android.media.MediaPlayer mediaPlayer) {
				// A new media player object was created. Keep a reference to it for easy access.
				// This is needed so that the view can manipulate its volume or mute it.
				fMediaPlayer = mediaPlayer;

				// Apply the view's last given volume setting to the new media player.
				setVolume(fVolume);

				// Dispatch a Corona Lua event, if configured.
				if (fTaskDispatcher != null) {
					fTaskDispatcher.send(new VideoViewPreparedTask(getId()));
				}

				// If this view was given a listener for this event externally, then invoke it now.
				if (fExternalOnPreparedListener != null) {
					fExternalOnPreparedListener.onPrepared(mediaPlayer);
				}
			}
		});

		android.media.MediaPlayer.OnCompletionListener completedListener = new android.media.MediaPlayer.OnCompletionListener() {
			@Override
			public void onCompletion(android.media.MediaPlayer mediaPlayer) {
				// Dispatch a Corona Lua event, if configured.
				if (fTaskDispatcher != null) {
					fTaskDispatcher.send(new VideoViewEndedTask(getId()));
				}

				// If this view was given a listener for this event externally, then invoke it now.
				if (fExternalOnCompletionListener != null) {
					fExternalOnCompletionListener.onCompletion(mediaPlayer);
				}
			}
		};

		// Set up a listener to be invoked when we've reached the end of the video.
		super.setOnCompletionListener(completedListener);

		super.setOnErrorListener(new android.media.MediaPlayer.OnErrorListener() {
			@Override
			public boolean onError(android.media.MediaPlayer mediaPlayer, int what, int extra) {
				if (fTaskDispatcher != null) {
					fTaskDispatcher.send(new VideoViewFailedTask(getId()));
				}
				// Dispatch manual events, since listener would be ignored
				if(fIgnoreErrors) {
					completedListener.onCompletion(mediaPlayer);
				}
				return fIgnoreErrors;
			}
		});
	}

	/**
	 * Sets the volume to applied to the video.
	 * This does not change the system's global volume settings.
	 * @param volume Normalized volume value ranging from 0 to 1, where 1 is loudest and 0 mean no sound.
	 */
	public void setVolume(float volume) {
		// Make sure the given volume value does not exceed its bounds.
		if (volume < 0) {
			volume = 0;
		}
		else if (volume > 1.0f) {
			volume = 1.0f;
		}

		// Apply the given volume to view's media player, if available.
		if (fMediaPlayer != null) {
			fMediaPlayer.setVolume(volume, volume);
		}

		// Store the given volume setting.
		// Note: If "fMediaPlayer" is not currently available, then we need to store this volume setting
		//       so that we can apply them in the onPrepared() event handler.
		fVolume = volume;
		if (fVolume > 0.0f ) {
			fPrevVolume = fVolume;
		}
	}

	/**
	 * Gets the current volume value applied to the video.
	 * @return Returns a normalized value ranging from 0 to 1, where 1 is loudest and 0 means no sound.
	 */
	public float getVolume() {
		return fVolume;
	}

	/**
	 * Determines if the video was muted by this view's mute() method.
	 * <p>
	 * Note that this view's mute setting is separate from the system's mute setting.
	 * This method will not return true if the device was muted at the system level.
	 * @return Returns true if muted via this view's mute() method. Returns false if not muted.
	 */
	public boolean isMuted() {
		return !(fVolume > 0.0f);
	}

	/**
	 * Mutes or unmutes the video's audio.
	 * <p>
	 * Note that this methd does not affect the system level audio. This method only applies to the video.
	 * @param mute Set true to mute the video's audio. Set false to unmute it.
	 */
	public void mute(boolean mute) {
		if (mute) {
			setVolume(0.0f);
		} else {
			setVolume(fPrevVolume);
		}
	}

	/**
	 * Sets a URI to the video to be played by this view.
	 * <p>
	 * Will use an internal HTTP localhost proxy server to work-around playback/seeking issues with Google's implementation.
	 * @param uri The URI/URL path to the video. Setting this to null will stop current playback.
	 */
	public void setVideoURIUsingCoronaProxy(android.net.Uri uri) {
		// Check if a proxy server is already running.
		if (fProxyServer != null) {
			// Do not continue if we're already playing the given URL.
			if ((uri != null) && uri.equals(fProxyServer.getVideoUri())) {
				return;
			}

			// A different URL was given. Close the current proxy.
			fProxyServer.close();
			fProxyServer = null;
		}

		// Stop current video playback.
		stopPlayback();

		// Do not continue if given an invalid URL.
		if (uri == null) {
			setVideoURI(null);
			return;
		}

		// Set up the view to play the video referenced by the given URL.
		String uriString = uri.toString();
		if (android.webkit.URLUtil.isHttpUrl(uriString)) {
			// We were given an "http:" URL.
			// Set up our own internal HTTP proxy to the given URL.
			// We will give the Google VideoView the proxy's localhost URL later, once it's ready.
			// Note: This proxy works-around an issue where Google's implementation will display "Can't play the video" when
			//       seeking a streaming video from a server that does not support HTTP range requests without an upper bound.
			//       The proxy applies an upper bound to range requests, providing the HTTP 206 response Google is looking for.
			fProxyServer = new ProxyServer(this, uri);
		} else {
			// Https urls seems to not need a ProxyServer anymore for HTTPs, looks Google fixed this issue?

			// Do not use a proxy for all other URLs such as "file:" and "content:".
			// Load the video as-is via Google's VideoView APIs.
			setVideoURI(uri);
		}
	}

	/**
	 * Registers the given listener to be invoked when the end of the video has been reached.
	 * @param listener Listener to be invoked. Can be null.
	 */
	@Override
	public void setOnCompletionListener(android.media.MediaPlayer.OnCompletionListener listener) {
		// This class already has a listener assigned for this event. Don't overwite it.
		// Instead, store the given listener to be invoked by this class' listener. (ie: Chain the callbacks.)
		fExternalOnCompletionListener = listener;
	}

	/**
	 * Registers the given listener to be invoked when the video has been loaded and is ready to be started.
	 * @param listener Listener to be invoked. Can be null.
	 */
	@Override
	public void setOnPreparedListener(android.media.MediaPlayer.OnPreparedListener listener) {
		// This class already has a listener assigned for this event. Don't overwite it.
		// Instead, store the given listener to be invoked by this class' listener. (ie: Chain the callbacks.)
		fExternalOnPreparedListener = listener;
	}

	/**
	 * Enables or disables the view to support pausing/resuming the video by tapping the view.
	 * @param enable Set true to enable pausing/resuming the video by tapping the view. Set false to ignore taps.
	 */
	public void touchTogglesPlay(boolean enable) {
		fTouchTogglesPlay = enable;
	}

	/**
	 * Determines if the tapping the view will pause/resume the video.
	 * @return Returns true if enabled. Returns false if taps are ignored.
	 */
	public boolean isTouchTogglesPlay() {
		return fTouchTogglesPlay;
	}

	/**
	 * Called when the video view has been touched/tapped.
	 * @param event The touch event that was received.
	 * @return Returns true if the touch/tap event was overriden by the view.
	 *         Returns false to let the system do its default handling.
	 */
	@Override
	public boolean onTouchEvent(android.view.MotionEvent event) {
		// Let the VideoView do its default handling if touch toggle is disabled.
		if (!fTouchTogglesPlay) {
			return super.onTouchEvent(event);
		}

		// Pause/resume the video if the view has been tapped on.
		// Note: ACTION_DOWN is used because on some devices there is no ACTION_UP event
		switch (event.getAction()) {
			case android.view.MotionEvent.ACTION_DOWN:
				if (this.isPlaying()) {
					if (this.canPause()) {
						this.pause();
					}
				}
				else {
					this.start();
				}
		}

		// Let the system do its default handling.
		return false;
	}

	/**
	 * Suspends/pauses video playback and clears its video buffer.
	 * <p>
	 * This method is expected to be called when the activity's onPause() method has been called.
	 * <p>
	 * After calling this method, you are expected to call this view's resume() method to restore the video's
	 * last playback state. It is best to call this method when the activity's onResume() method has been called
	 * and preferrably after the display's screen-lock is no longer covering the activity.
	 */
	@Override
	public void suspend() {
		// Fetch our current position in the video in milliseconds.
		// Will be zero if stopped or a video hasn't been loaded yet.
		int lastPosition = getCurrentPosition();

		// Suspend the video.
		super.suspend();

		// Tell the base class to seek to the last playback position when the video has been resumed.
		// Note: The above suspend() method throws away the last playback and seek position. (Which is really a bug.)
		//       So, we must do a seekTo() after a suspend() so that Google can queue the seek operation.
		seekTo(lastPosition);
	}

	/** Called when the video view has been attech to an activity window. */
	@Override
	protected void onAttachedToWindow() {
		super.onAttachedToWindow();

		// If we were running a proxy server before this view was detached, then run a new proxy to resume playback.
		if ((fProxyServer != null) && fProxyServer.wasClosed()) {
			android.net.Uri uri = fProxyServer.getVideoUri();
			if (uri != null) {
				fProxyServer = new ProxyServer(this, uri);
			}
		}
	}

	/**
	 * Called when this video view has been detached from an activity window
	 * or if the activity that owns this view has been destroyed.
	 */
	@Override
	protected void onDetachedFromWindow() {
		super.onDetachedFromWindow();

		// Stop our internal HTTP proxy server if currently running.
		if (fProxyServer != null) {
			fProxyServer.close();
			stopPlayback();
		}
	}

	/** Called by this view's internal HTTP proxy server if it failed to startup. */
	private void onProxyCannotStart() {
		if (fProxyServer != null) {
			setVideoURI(fProxyServer.getVideoUri());
		}
	}

	/**
	 * Called by this view's internal HTTP proxy server if it successfullly started up. This means that it
	 * now has an HTTP localhost URL that can be applied to the video view to start streaming video playback.
	 */
	private void onProxyStarted() {
		if (fProxyServer != null) {
			setVideoURI(fProxyServer.getProxyUri());
		}
	}


	/**
	 * Corona runtime task used to raise a "video" event to be received by a Lua listener.
	 * Will be set to the "ready" phase.
	 * <p>
	 * Instances of this class are expected to be passed to the CoronaRuntimeTaskDisplatcher.send() method.
	 * The dispatcher will then have this task executed by the Corona runtime in a thread safe manner.
	 * @see com.ansca.corona.CoronaRuntimeTaskDispatcher
	 */
	private static class VideoViewPreparedTask implements CoronaRuntimeTask {
		private int fId;

		public VideoViewPreparedTask(int id) {
			fId = id;
		}

		@Override
		public void executeUsing(CoronaRuntime runtime) {
			com.ansca.corona.JavaToNativeShim.videoViewPrepared(runtime, fId);
		}
	}


	/**
	 * Corona runtime task used to raise a "video" event to be received by a Lua listener.
	 * Will be set to the "ended" phase.
	 * <p>
	 * Instances of this class are expected to be passed to the CoronaRuntimeTaskDisplatcher.send() method.
	 * The dispatcher will then have this task executed by the Corona runtime in a thread safe manner.
	 * @see com.ansca.corona.CoronaRuntimeTaskDispatcher
	 */
	private static class VideoViewEndedTask implements CoronaRuntimeTask {
		private int fId;

		public VideoViewEndedTask(int id) {
			fId = id;
		}

		@Override
		public void executeUsing(CoronaRuntime runtime) {
			com.ansca.corona.JavaToNativeShim.videoViewEnded(runtime, fId);
		}
	}

	private static class VideoViewFailedTask implements CoronaRuntimeTask {
		private int fId;

		public VideoViewFailedTask(int id) {
			fId = id;
		}

		@Override
		public void executeUsing(CoronaRuntime runtime) {
			com.ansca.corona.JavaToNativeShim.videoViewFailed(runtime, fId);
		}
	}


	/**
	 * Internal HTTP localhost proxy server that is only used by the CoronaVideoView class.
	 * <p>
	 * Used to work-around issues with Google's VideoView/MediaPlayer implementation where it'll display
	 * "Can't play the video" if the web server does not support HTTP range requests without an upper bound.
	 * This proxy will intercept Google's HTTP requests and apply an upper bound to range requests so
	 * that the server can respond with an HTTP status code of 206, which is the response Google requires.
	 */
	private static class ProxyServer {
		/** Handler used to post runnables on the main UI thread. */
		private android.os.Handler fHandler;

		/** The video view that owns this proxy. */
		private CoronaVideoView fVideoView;

		/** The localhost URI that this proxy is using. Will be null until the proxy successfully starts up. */
		private android.net.Uri fProxyUri;

		/** URI to the video file that was originally given to this proxy via its constructor. */
		private android.net.Uri fOriginalVideoUri;

		/**
		 * Caches the redirected URI to the video to improve performance.
		 * Will be set to "fOriginalVideoUri" if the server did not respond with a redirect.
		 */
		private java.net.URL fRedirectedVideoUrl;

		/** The local socket server that Google's VideoView/MediaPlayer will be communicating with. */
		private java.net.ServerSocket fServerSocket;

		/**
		 * Stores the size of the video file in bytes that the URI references.
		 * This is needed by the proxy to apply an upper bound to an HTTP range request.
		 * <p>
		 * Will be zero if the size has not been retrieved yet.
		 */
		private long fVideoFileSizeInBytes;

		/** Set true if the proxy was requested to be closed by the video view. */
		private boolean fWasCloseRequested;


		/**
		 * Creates an HTTP localhost proxy server that sits between the given video view and the video server.
		 * <p>
		 * The proxy server will attempt to create a local socket server immediately upon creation and will
		 * invoke the given video view's onProxyStarted() method if started successfully.
		 * <p>
		 * This server will invoke the given video view's onProxyCannotStart() method if the proxy server
		 * failed to start up.
		 * @param videoView The video view that owns this proxy server. Cannot be null.
		 * @param videoUri URI/URL to the video file that the view will be playing. Cannot be null.
		 */
		public ProxyServer(CoronaVideoView videoView, android.net.Uri videoUri) {
			// Validate.
			if ((videoView == null) || (videoUri == null)) {
				throw new NullPointerException();
			}

			// Initialize member variables.
			fHandler = new android.os.Handler();
			fVideoView = videoView;
			fProxyUri = null;
			fOriginalVideoUri = videoUri;
			fVideoFileSizeInBytes = 0;
			fWasCloseRequested = false;
			try {
				fRedirectedVideoUrl = null;
				fRedirectedVideoUrl = new java.net.URL(videoUri.toString());
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}

			// Invoke the following on another thread.
			Thread asyncOperation = new Thread(new Runnable() {
				@Override
				public void run() {
					// Attempt to open a local socket server.
					// The below also requests the system to give us an unused TCP port number.
					try {
						fServerSocket = new java.net.ServerSocket(0, 8);
						if (fServerSocket !=  null) {
							String proxyUriString = "http://127.0.0.1:" + Integer.toString(fServerSocket.getLocalPort());
							String uriFilePath = fOriginalVideoUri.getPath();
							if (uriFilePath != null) {
								proxyUriString += uriFilePath;
							}
							android.net.Uri proxyUri = android.net.Uri.parse(proxyUriString);
							if (proxyUri != null) {
								fProxyUri = proxyUri;
							}
						}
					}
					catch (Exception ex) {
						ex.printStackTrace();
						if (fServerSocket != null) {
							try { fServerSocket.close(); }
							catch (Exception ex2) {}
							fServerSocket = null;
						}
					}

					// Handle the result of the above operation.
					{
						// Determine if the socket server was successfully open.
						final boolean wasStarted = (fServerSocket != null);

						// Notify the video view on the main UI thread on whether or not the proxy has started.
						fHandler.post(new Runnable() {
							@Override
							public void run() {
								CoronaVideoView view = fVideoView;
								if (view != null) {
									if (wasStarted) {
										// Proxy connection is open.
										// Video view now has a localhost URI that it can use to start streaming with.
										view.onProxyStarted();
									}
									else {
										// Failed to open the proxy connection.
										// Let the video view use its fallback behavior.
										view.onProxyCannotStart();
									}
								}
							}
						});

						// Do not continue if we've failed open the proxy server's local socket server.
						if (wasStarted == false) {
							fWasCloseRequested = true;
							return;
						}
					}

					// The proxy's socket server was successfully opened.
					// Start listening for client socket conections from the video view.
					try {
						while (fWasCloseRequested == false) {
							// This blocks until a client socket connection has been established
							// or will throw an exception if the proxy's socket server was closed.
							java.net.Socket clientSocket = fServerSocket.accept();
							if (clientSocket == null) {
								continue;
							}

							// A client socket connection has been opened.
							// Handle its HTTP communications on another thread.
							try {
								Thread asyncOperation = new Thread(new ClientSocketRunnable(ProxyServer.this, clientSocket));
								asyncOperation.start();
							}
							catch (Exception ex) {
								ex.printStackTrace();
							}
						}
					}
					catch (java.net.SocketException ex) {
						// The proxy's local socket server has been closed.
						// This exception is normal and can be safely ignored.
					}
					catch (Exception ex) {
						ex.printStackTrace();
					}

					// Close the proxy's socket server, if not done already. (Odds are it's already closed.)
					ProxyServer.this.close();
				}
			});
			asyncOperation.start();
		}

		/**
		 * Gets a URI to this proxy's HTTP localhost socket server.
		 * This URI is expected to be given to Google's VideoView/MediaPlayer once available.
		 * @return Returns a URI to the proxy server.
		 *         Returns null if the proxy server is not currently running.
		 */
		public android.net.Uri getProxyUri() {
			if (fWasCloseRequested) {
				return null;
			}
			return fProxyUri;
		}

		/**
		 * Gets a URI to the video file that was given to this proxy server via its constructor.
		 * @return Returns a URI to the video file.
		 */
		public android.net.Uri getVideoUri() {
			return fOriginalVideoUri;
		}

		/**
		 * Determines if the proxy server has been shutdown and is no longer available.
		 * @return Returns true if the proxy server is not running.
		 *         Returns false if it is starting up or is currently running.
		 */
		public boolean wasClosed() {
			return fWasCloseRequested;
		}

		/** Requests the proxy server to shutdown/disconnect, if not done already. */
		public synchronized void close() {
			// Do not continue if already closed.
			if (fWasCloseRequested) {
				return;
			}

			// Flag that the server is about to be closed.
			fWasCloseRequested = true;

			// Close the proxy's socket server.
			// This will raise a SocketException in its blocking socket API, signaling that it is closed.
			if (fServerSocket != null) {
				try { fServerSocket.close(); }
				catch (Exception ex) {}
			}
		}

		/**
		 * Executes a blocking HTTP "HEAD" request with the server hosting the video file in an attempt to
		 * fetch the video's file size. The received size will be stored to member variable "fVideoFileSizeInBytes".
		 */
		private void requestVideoFileSize() {
			// Attempt to send the HTTP request.
			// Note: This blocks until an HTTP response has been received or until the connection times-out.
			java.net.HttpURLConnection urlConnection;
			urlConnection = HttpServices.sendRequest(fRedirectedVideoUrl, new HttpServices.RequestHandler() {
				@Override
				public void onSendRequestUsing(java.net.HttpURLConnection urlConnection)
					throws java.net.ProtocolException, java.io.IOException
				{
					urlConnection.setRequestMethod("HEAD");
					urlConnection.setRequestProperty("Accept-Encoding", "identity");
					urlConnection.connect();
				}
			});

			// Handel the HTTP response, if received.
			try {
				if (urlConnection != null) {
					int responseCode = urlConnection.getResponseCode();
					if ((responseCode >= 200) && (responseCode < 300)) {
						// A valid response was received. Fetch the file size.
						String stringValue = urlConnection.getHeaderField("Content-Length");
						if (stringValue != null) {
							long longValue = Long.parseLong(stringValue);
							if (longValue > 0) {
								fVideoFileSizeInBytes = longValue;
							}
						}

						// Store the connection's URL in case it's been redirected.
						// Will be used by all future HTTP requests to improve performance.
						java.net.URL currentUrl = urlConnection.getURL();
						if (currentUrl.equals(fRedirectedVideoUrl) == false) {
							fRedirectedVideoUrl = currentUrl;
						}
					}
				}
			}
			catch (Exception ex) {}

			// Close the connection.
			if (urlConnection != null) {
				try { urlConnection.disconnect(); }
				catch (Exception ex) {}
			}
		}


		/** Runnable intended to be used by a ProxyServer to handle a client socket connection on another thread. */
		private static class ClientSocketRunnable implements Runnable {
			/** The proxy server that created this runnable and received the client socket connection. */
			private ProxyServer fProxyServer;

			/** The client socket connection to be communicated with. */
			private java.net.Socket fClientSocket;


			/**
			 * Creates a new runnable to handle the given client socket connection accepted by the given proxy server.
			 * @param proxyServer The Proxy server that accepted the given client socket connection. Cannot be null.
			 * @param clientSocket The client socket connection to be communicated with. Cannot be null.
			 */
			public ClientSocketRunnable(ProxyServer proxyServer, java.net.Socket clientSocket) {
				fProxyServer = proxyServer;
				fClientSocket = clientSocket;
			}

			/**
			 * To be called to start handling the communications between the client socket and the video server.
			 * <p>
			 * This method is expected to be called on another thread.
			 */
			@Override
			public void run() {
				// Validate.
				if ((fProxyServer == null) || (fClientSocket == null)) {
					return;
				}

				// Fetch the video file's size in bytes from the server, if not done already.
				if (fProxyServer.fVideoFileSizeInBytes <= 0) {
					fProxyServer.requestVideoFileSize();
				}

				// Fetch the received HTTP request from the client socket.
				// Note: This request is expected to come from Google's VideoView/MediaPlayer.
				String requestMethod = "GET";
				java.util.HashMap<String, String> requestHeaders = new java.util.HashMap<String, String>();
				java.util.ArrayList<byte[]> requestBodyByteArrays = null;
				try {
					// Acquire an input stream to the socket.
					java.io.InputStream inputStream = fClientSocket.getInputStream();
					java.io.BufferedReader reader = new java.io.BufferedReader(
							new java.io.InputStreamReader(inputStream));

					// Fetch the received HTTP request "line" and "method".
					String requestLine = reader.readLine();
					requestMethod = requestLine.split("\\s+")[0];

					// Fetch the received HTTP headers and copy them to the "requestHeaders" dictionary.
					while (reader.ready()) {
						// Fetch the next HTTP header.
						String nextLine = reader.readLine();
						if (nextLine == null) {
							// We've reached the end of the HTTP request.
							break;
						}
						if (nextLine.isEmpty()) {
							// An empty line means, that there are no more HTTP headers.
							break;
						}

						// Split the header string to a key/value pair and add it to the dictionary.
						String headerName = nextLine;
						String headerValue = "";
						int index = nextLine.indexOf(':');
						if (index >= 0) {
							headerName = nextLine.substring(0, index);
							if ((index + 1) < nextLine.length()) {
								headerValue = nextLine.substring(index + 1);
							}
						}
						requestHeaders.put(headerName.trim(), headerValue.trim());
					}

					// Fetch the HTTP request's body, if it has one. (Odds are it doesn't.)
					int byteCount = 0;
					try { byteCount = inputStream.available(); }
					catch (Exception ex) {}
					if (byteCount > 0) {
						requestBodyByteArrays = new java.util.ArrayList<byte[]>();
						do {
							byte[] byteArray = new byte[byteCount];
							inputStream.read(byteArray);
							requestBodyByteArrays.add(byteArray);
							byteCount = inputStream.available();
						} while (byteCount > 0);
					}
				}
				catch (Exception ex) {
					// An error has occurred. Print it to the Android log.
					// Note: Do not log an error if a "SocketException" has occurred,
					//       which means the client has closed the connection. (This is okay.)
					if ((ex instanceof java.net.SocketException) == false) {
						ex.printStackTrace();
					}

					// Close the client connection, if not done already, and exit out.
					try { fClientSocket.close(); }
					catch (Exception ex2) {}
					return;
				}

				// Send the client's HTTP request to the server hosting the video file.
				// Note: The below re-assembles the original HTTP request with some modifications.
				java.net.HttpURLConnection urlConnection;
				{
					final String finalRequestMethod = requestMethod;
					final java.util.HashMap<String, String> finalRequestHeaders = requestHeaders;
					final java.util.ArrayList<byte[]> finalRequestBodyByteArrays = requestBodyByteArrays;
					urlConnection = HttpServices.sendRequest(fProxyServer.fRedirectedVideoUrl, new HttpServices.RequestHandler() {
						@Override
						public void onSendRequestUsing(java.net.HttpURLConnection urlConnection) throws
							java.net.ProtocolException, java.io.IOException
						{
							// Set the HTTP request method/verb.
							urlConnection.setRequestMethod(finalRequestMethod);

							// Copy the client socket's HTTP headers to the URL connection.
							for (java.util.Map.Entry<String, String> headerEntry : finalRequestHeaders.entrySet()) {
								// Fetch the next HTTP header key/value pair.
								String headerName = headerEntry.getKey();
								String headerValue = headerEntry.getValue();

								// Do the following special header handling.
								String lowercaseHeaderName = headerName.toLowerCase();
								if (lowercaseHeaderName.equals("host")) {
									// Change the "Host" header to match the server URL's domain.
									// Note: The one we received from the client will be for localhost and won't work.
									headerValue = urlConnection.getURL().getHost();
									if (urlConnection.getURL().getPort() >= 0) {
										headerValue += ":" + Integer.toString(urlConnection.getURL().getPort());
									}
								}
								else if (lowercaseHeaderName.equals("range") && headerValue.endsWith("-")) {
									// A "Range" request header without an upper bound has been received.
									// Change this header to use an upper bound.
									// Note: This is needed to get an HTTP 206 response code from servers that don't support
									//       a range request without an upper bound. Google requires a 206 response for videos.
									if (fProxyServer.fVideoFileSizeInBytes > 0) {
										headerValue += Long.toString(fProxyServer.fVideoFileSizeInBytes);
									}
								}
								else if (lowercaseHeaderName.equals("content-length")) {
									// Do not copy the client's "Content-Length" header.
									// The Java HttpUrlConnection object will generate one for us.
									continue;
								}

								// Copy the HTTP header to the URL connection.
								urlConnection.setRequestProperty(headerName, headerValue);
							}

							// Always request the video to be received as an uncompressed HTTP response from the server.
							// Video files such as MP4s are already naturally compressed. (Improves performance.)
							urlConnection.setRequestProperty("Accept-Encoding", "identity");

							// Attempt to connect to the server and send the HTTP request.
							if ((finalRequestBodyByteArrays != null) && (finalRequestBodyByteArrays.isEmpty() == false)) {
								// The HTTP request has a body.
								// The below connects to the server and streams the body's bytes to it.
								urlConnection.setDoOutput(true);
								for (byte[] byteArray : finalRequestBodyByteArrays) {
									urlConnection.getOutputStream().write(byteArray);
									urlConnection.getOutputStream().flush();
								}
							}
							else {
								// The HTTP request does not have a body. Simply connect to the server.
								urlConnection.connect();
							}
						}
					});
				}
				if (urlConnection == null) {
					// We've failed to connect to the server hosting the video file.
					// Close the client socket connection and exit out.
					try { fClientSocket.close(); }
					catch (Exception ex2) {}
					return;
				}

				// The above has successfully sent the HTTP request to the server and response headers has been received.
				// Send the server's HTTP response to the client socket connection.
				try {
					// Store the connection's URL in case it's been redirected.
					// Will be used by all future HTTP requests to improve performance.
					java.net.URL currentUrl = urlConnection.getURL();
					if (currentUrl.equals(fProxyServer.fRedirectedVideoUrl) == false) {
						fProxyServer.fRedirectedVideoUrl = currentUrl;
					}

					// Generate an HTTP response string containing all of the headers received from the server.
					StringBuilder stringBuilder = new StringBuilder();
					stringBuilder.append("HTTP/1.1 ");
					stringBuilder.append(Integer.toString(urlConnection.getResponseCode()));
					stringBuilder.append(" " + urlConnection.getResponseMessage() + "\r\n");
					for (java.util.Map.Entry<String, java.util.List<String>> headerEntry : urlConnection.getHeaderFields().entrySet()) {
						if (headerEntry.getKey() == null) {
							continue;
						}
						stringBuilder.append(headerEntry.getKey() + ": ");
						boolean isFirstEntry = true;
						for (String headerValue : headerEntry.getValue()) {
							if (headerValue == null) {
								continue;
							}
							if (isFirstEntry) {
								isFirstEntry = false;
							}
							else {
								stringBuilder.append(", ");
							}
							stringBuilder.append(headerValue);
						}
						stringBuilder.append("\r\n");
					}
					stringBuilder.append("\r\n");		// This empty line indicates that there are no more headers.

					// Send the above HTTP headers string to the client socket in UTF-8 form.
					java.io.OutputStream outputStream = fClientSocket.getOutputStream();
					{
						byte[] byteArray = stringBuilder.toString().getBytes("UTF-8");
						outputStream.write(byteArray);
					}
					outputStream.flush();

					// Stream the HTTP body bytes received from the server to the client socket.
					java.io.InputStream inputStream = urlConnection.getInputStream();
					if (inputStream != null) {
						byte[] byteBuffer = new byte[2048];
						while (true) {
							int bytesRead = inputStream.read(byteBuffer, 0, byteBuffer.length);
							if (bytesRead < 0) {
								break;
							}
							else if (bytesRead == 0) {
								continue;
							}
							outputStream.write(byteBuffer, 0, bytesRead);
							outputStream.flush();
						}
					}
				}
				catch (java.net.SocketException ex) {
					// The connection to the server or the client has been closed.
					// This exception is normal and can be safely ignored.
				}
				catch (Exception ex) {
					ex.printStackTrace();
				}
				finally {
					// Close the client connection, if not done already.
					try { fClientSocket.close(); }
					catch (Exception ex) {}

					// Close server input stream.
					try {
						java.io.InputStream inputStream = urlConnection.getInputStream();
						if (inputStream != null) {
							inputStream.close();
						}
					}
					catch (Exception ex) {}

					// Close the server connection.
					try { urlConnection.disconnect(); }
					catch (Exception ex) {}
				}
			}
		}
	}


	/**
	 * Provides helper methods to simply HTTP communications.
	 * <p>
	 * You cannot create instances of this class. You must call its static methods instead.
	 */
	private static class HttpServices {
		/**
		 * Interface used to send an HTTP request.
		 * Instances of this interface are expected to be passed to the static HttpServices.sendRequest() method.
		 */
		public interface RequestHandler {
			/**
			 * Called by the HttpServices.sendRequest() method when a new HttpURLConnection instance has been made.
			 * This method is expected to configure and send an HTTP request with the given connection.
			 * <p>
			 * It is okay for the implementation of this method to throw an exception.
			 * They are expected to be caught by the caller.
			 * @param urlConnection The new HTTP URL connection object that was created by the caller.
			 */
			public void onSendRequestUsing(java.net.HttpURLConnection urlConnection) throws
					java.net.ProtocolException, java.io.IOException;
		}


		/** Constructor made private to prevent instances from being made. */
		private HttpServices() {
		}

		/**
		 * Opens a connection the server referenced by the given URI/URL and sends an HTTP request using the given handler.
		 * This method will automatically handle HTTP redirect responses and resend the HTTP request to the redirected URL.
		 * @param uri URI/URL to the web server to send the HTTP request to.
		 * @param requestHandler Handler used to configure and send a HTTP request using an HttpURLConnection created
		 *                       by this method.
		 * @return Returns a reference to an HttpURLConnection object if the HTTP request was successfully sent to
		 *         the server referenced by "uri" and an HTTP response code was received.
		 *         <p>
		 *         Returns null if failed to connect with the server or if given invalid arguments.
		 */
		public static java.net.HttpURLConnection sendRequest(android.net.Uri uri, RequestHandler requestHandler) {
			// Validate.
			if ((uri == null) || (requestHandler == null)) {
				return null;
			}

			// Attempt to create a connection  and send an HTTP request to the given URI.
			java.net.HttpURLConnection urlConnection = null;
			try {
				urlConnection = sendRequest(new java.net.URL(uri.toString()), requestHandler);
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}
			return urlConnection;
		}

		/**
		 * Opens a connection the server referenced by the given URI/URL and sends an HTTP request using the given handler.
		 * This method will automatically handle HTTP redirect responses and resend the HTTP request to the redirected URL.
		 * @param uri URI/URL to the web server to send the HTTP request to.
		 * @param requestHandler Handler used to configure and send a HTTP request using an HttpURLConnection created
		 *                       by this method.
		 * @return Returns a reference to an HttpURLConnection object if the HTTP request was successfully sent to
		 *         the server referenced by "uri" and an HTTP response code was received.
		 *         <p>
		 *         Returns null if failed to connect with the server or if given invalid arguments.
		 */
		public static java.net.HttpURLConnection sendRequest(java.net.URL url, RequestHandler requestHandler) {
			// Validate.
			if ((url == null) || (requestHandler == null)) {
				return null;
			}

			// Attempt to create a connection and send an HTTP request to the given URL.
			java.net.HttpURLConnection urlConnection = null;
			try {
				java.util.List<String> redirectCookies = null;
				int redirectCount = 0;
				int retryCount = 0;
				while (true) {
					// Open a socket connection to the given URL.
					urlConnection = (java.net.HttpURLConnection)url.openConnection();
					if (urlConnection == null) {
						break;
					}

					// Disable automatic redirects. We'll handle them manually down below.
					// Note: Java only supports automatic redirects on the same protocol.
					//       Java won't redirect when switching switching between http to https and vice-versa.
					urlConnection.setInstanceFollowRedirects(false);

					// If this was an HTTP redirect, then apply cookies from the redirect response, if any.
					if ((redirectCookies != null) && (redirectCookies.isEmpty() == false)) {
						String cookieHeaderValue = android.text.TextUtils.join("; ", redirectCookies);
						urlConnection.setRequestProperty("Cookie", cookieHeaderValue);
						redirectCookies = null;
					}

					// Set up a connection timeout. Increase timeout when attempting a retry.
					// Note: For the first try, use a short a short timeout to work-around a DNS handling issue in the
					//       Java HttpURLConnection class where it blocks while waiting for a socket connection response
					//       for all IP addresses associated with a single URL. Happens with "http://www.whitehouse.gov" site.
					int timeoutInMilliseconds = 2000;
					if (retryCount > 0) {
						timeoutInMilliseconds = 10000;
					}
					urlConnection.setConnectTimeout(timeoutInMilliseconds);
					urlConnection.setReadTimeout(10000);

					// Let the given handler set up and send the HTTP request.
					int responseCode = -1;
					try {
						requestHandler.onSendRequestUsing(urlConnection);
						responseCode = urlConnection.getResponseCode();
					}
					catch (java.net.SocketTimeoutException ex) {
						// A connection timeout occurred.
						// Do a retry until we've reached the max retries allowed.
						retryCount++;
						if (retryCount >= 2) {
							throw ex;
						}
						continue;
					}
					retryCount = 0;

					// Check the HTTP response code.
					if (responseCode == -1) {
						// Failed to connect to the given URL.
						// Disconnect and bail out.
						try { urlConnection.disconnect(); }
						catch (Exception ex) {}
						urlConnection = null;
					}
					else if ((responseCode == 301) || (responseCode == 302) || (responseCode == 303) || (responseCode == 307)) {
						// *** The server has responded with a redirect. ***

						// Do not continue if we've exceeded the max number of redirects allowed.
						// We do this in case the server has a recursive redirect issue.
						redirectCount++;
						if (redirectCount > 10) {
							break;
						}

						// Fetch the new URL and any cookies from the HTTP response.
						url = new java.net.URL(urlConnection.getHeaderField("Location"));
						redirectCookies = urlConnection.getHeaderFields().get("Set-Cookie");

						// Close the server connection and attempt to connect to the received/redirected URL.
						urlConnection.disconnect();
						continue;
					}

					// We're done.
					break;
				}
			}
			catch (Exception ex) {
				// Close the socket connection and bail out.
				if (urlConnection != null) {
					try { urlConnection.disconnect(); }
					catch (Exception ex2) {}
					urlConnection = null;
				}
				if (((ex instanceof java.net.UnknownHostException) == false) &&
				    ((ex instanceof java.net.SocketTimeoutException) == false))
				{
					ex.printStackTrace();
				}
			}

			// Returns a reference to a connection if the HTTP request was received by the server.
			// Will return null if the request was not received or if an invalid operation occurred.
			return urlConnection;
		}
	}
}
