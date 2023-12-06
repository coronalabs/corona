//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.graphics.opengl;


/** OpenGL surface which uses the Corona runtime to render its content. */
public class CoronaGLSurfaceView extends GLSurfaceView {
	/** Reference to the Corona activity window that owns this view. */
	private android.app.Activity fActivity;

	/** Renders content to the OpenGL surface via the Corona runtime. */
	private CoronaRenderer fRenderer;

	/** Timer used to make sure that the OpenGL surface is working. */
	private com.ansca.corona.MessageBasedTimer fWatchdogTimer;
	
	/**
	 * Receives the device's current display orientation/rotation in degrees.
	 * Used by Corona to send orientation events when the application has a fixed orientation.
	 */
	private android.view.OrientationEventListener fOrientationListener;

	/** The current orientation of the device. */
	private com.ansca.corona.WindowOrientation fCurrentDeviceOrientation;

	/** The previous device orientation. */
	private com.ansca.corona.WindowOrientation fPreviousDeviceOrientation;

	/** The current orientation the application's window is being displayed in. */
	private com.ansca.corona.WindowOrientation fCurrentWindowOrientation;

	/** The previous orientation the application's window was displayed in. */
	private com.ansca.corona.WindowOrientation fPreviousWindowOrientation;

	/** Stores the activity's information */
	private com.ansca.corona.CoronaActivityInfo fActivityInfo;

	private com.ansca.corona.CoronaRuntime fCoronaRuntime;

	/**
	 * Creates a new OpenGL surface used to render Corona's content.
	 * @param context Reference to the context. Cannot be null.
	 */
	public CoronaGLSurfaceView(android.content.Context context, com.ansca.corona.CoronaRuntime runtime, boolean isCoronaKit) {
		super(context);

		// Validate.
		if (context == null) {
			throw new NullPointerException();
		}

		fCoronaRuntime = runtime;

		// Fetch the current orientation of the activity's window.
		// This will not match the device's orientation if the activity is configured with a fixed orientation.
		fCurrentWindowOrientation = com.ansca.corona.WindowOrientation.fromCurrentWindowUsing(getContext());
		fPreviousWindowOrientation = fCurrentWindowOrientation;
		
		// Set up a watchdog that monitors the OpenGL surface to make sure it has been created and working.
		// This is needed when the window transitions from an orientation the app does not support
		// (which destroys the OpenGL surface) to an orientation it does support (re-creating the surface).
		fWatchdogTimer = new com.ansca.corona.MessageBasedTimer();
		fWatchdogTimer.setHandler(new android.os.Handler());
		fWatchdogTimer.setInterval(com.ansca.corona.TimeSpan.fromSeconds(1));
		fWatchdogTimer.setListener(new com.ansca.corona.MessageBasedTimer.Listener() {
			@Override
			public void onTimerElapsed() {
				// Do not continue if the holder's surface has been destroyed.
				android.view.SurfaceHolder holder = getHolder();
				if ((holder == null) || (holder.getSurface() == null)) {
					return;
				}

				// Do not continue if the OpenGL surface exists and is working.
				if (hasGLSurface() && canRender()) {
					return;
				}

				// The OpenGL surface no longer exists when it should. Attempt to re-create it.
				surfaceChanged(holder, android.graphics.PixelFormat.RGBA_8888, getWidth(), getHeight());
			}
		});

		// Set up the orientation listener for detecting the device's current orientation.
		// This listener provides current rotation even if the activity is set up with a fixed orientation,
		// which is needed since the onConfigurationChanged() method does not get invoked for orientation
		// changes on fixed orientation apps.
		fCurrentDeviceOrientation = fCurrentWindowOrientation;
		fPreviousDeviceOrientation = com.ansca.corona.WindowOrientation.UNKNOWN;
		fOrientationListener = new android.view.OrientationEventListener(getContext()) {
			@Override
			public void onOrientationChanged(int orientationInDegrees) {
				if (fActivityInfo == null) {
					return;
				}

				// Do not continue if the device is incapable of determining its current orientation.
				if (orientationInDegrees == android.view.OrientationEventListener.ORIENTATION_UNKNOWN) {
					return;
				}
				
				// Do not continue if application is not fully initialized or is currently exiting.
				if ((fCoronaRuntime.isRunning() == false) || (canRender() == false)) {
					return;
				}
				
				// Given orientation angle is clockwise. Convert to counter-clockwise.
				orientationInDegrees = (360 - orientationInDegrees) % 360;
				
				// Notify the system if the device's orientation has changed.
				com.ansca.corona.WindowOrientation currentOrientation =
						com.ansca.corona.WindowOrientation.fromDegrees(getContext(), orientationInDegrees);
				if ((currentOrientation != fCurrentDeviceOrientation) ||
				    (fPreviousDeviceOrientation == com.ansca.corona.WindowOrientation.UNKNOWN)) {
					fPreviousDeviceOrientation = fCurrentDeviceOrientation;
					fCurrentDeviceOrientation = currentOrientation;
					if (fActivityInfo.hasFixedOrientation() && 
						android.provider.Settings.System.getInt(fActivity.getContentResolver(), 
 							android.provider.Settings.System.ACCELEROMETER_ROTATION, 0) != 0) {
						sendOrientationChangedEvent();
					}
				}
			}
		};

		// Set up rendering system to use OpenGL ES 2.0.
		setEGLContextClientVersion(2);

		// Set up OpenGL to render with a 32-bit color depth. (The default is 16-bit.)
		// This must be done before setting the renderer below.
		// Note: The alpha channel must be disabled for the "surface" to work-around an OpenGL driver bug on Kindle Fire
		//       and Nook Tablet where all alpha blended polygons, including the GL clear color, will be blended with black.
		//       Alpha blending of content will still work because of the 32-bit color PixelFormat set down below.
		setEGLConfigChooser(8, 8, 8, 8, 0, 0);

		// Attach the renderer to the surface.
		fRenderer = new CoronaRenderer(this, runtime, isCoronaKit);
		setRenderer(fRenderer);
		setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

		// Set up the surface container to render with 32-bit color depth too. Must be done after setting the renderer.
		getHolder().setFormat(android.graphics.PixelFormat.RGBA_8888);

		// Enable logging.
		setDebugFlags(GLSurfaceView.DEBUG_CHECK_GL_ERROR | GLSurfaceView.DEBUG_LOG_GL_CALLS);
	}

	public void setActivity(android.app.Activity activity) {
		fActivity = activity;
		fActivityInfo = new com.ansca.corona.CoronaActivityInfo(fActivity);
	}

	/**
	 * Called when the device configuration changes, such as orientation changes.
	 * @param newConfig The new device configuration.
	 */
	@Override
	public void onConfigurationChanged(android.content.res.Configuration newConfig) {
		// Let the base class handle this event first.
		super.onConfigurationChanged(newConfig);
		if (fActivityInfo == null) {
			return;
		}

		// If device does not have the sensors to detect orientation changes, or the OS triggered this configuration 
		// change when disabling the auto-rotate seting, then determine the new orientation here.
		// Remember that orientation changes can be triggered by slide-out keyboards and possibly other events.
		if (fOrientationListener.canDetectOrientation() == false) {
			com.ansca.corona.WindowOrientation currentOrientation;
			if (newConfig.orientation == android.content.res.Configuration.ORIENTATION_LANDSCAPE) {
				currentOrientation = com.ansca.corona.WindowOrientation.LANDSCAPE_RIGHT;
			}
			else {
				currentOrientation = com.ansca.corona.WindowOrientation.PORTRAIT_UPRIGHT;
			}
			if ((currentOrientation != fCurrentDeviceOrientation) ||
			    (fPreviousDeviceOrientation == com.ansca.corona.WindowOrientation.UNKNOWN)) {
				fPreviousDeviceOrientation = fCurrentDeviceOrientation;
				fCurrentDeviceOrientation = currentOrientation;
				if (fActivityInfo.hasFixedOrientation()	&& 
					android.provider.Settings.System.getInt(fActivity.getContentResolver(), 
 						android.provider.Settings.System.ACCELEROMETER_ROTATION, 0) != 0) {
					sendOrientationChangedEvent();
				}
			}
		}
	}
	
	public void requestExitAndWait() {
		mGLThread.requestExitAndWait();
	}

	private void sendOrientationChangedEvent() {
		if (fCoronaRuntime != null && fCoronaRuntime.isRunning()) {
			fCoronaRuntime.getTaskDispatcher().send(
				new com.ansca.corona.events.OrientationTask(
					fCurrentDeviceOrientation.toCoronaIntegerId(),
					fPreviousDeviceOrientation.toCoronaIntegerId()));
		}
	}

	/**
	 * Called when the view's surface has changed size or color quality.
	 * @param holder Reference to the surface that contains this OpenGL surface.
	 * @param format The new PixelForamt of the surface.
	 * @param width The new width of the surface.
	 * @param height The new height of the surface.
	 */
	@Override
	public void surfaceChanged(android.view.SurfaceHolder holder, int format, int width, int height) {
		// Validate.
		if ((holder == null) || (holder.getSurface() == null) || (holder.getSurface().isValid() == false)) {
			return;
		}

		// Fetch the application's current orientation.
		com.ansca.corona.WindowOrientation currentOrientation;
		currentOrientation = com.ansca.corona.WindowOrientation.fromCurrentWindowUsing(getContext());

		// Determine if the current orientation is supported by this application.
		boolean isCurrentOrientationSupported = fActivity == null ? true : currentOrientation.isSupportedBy(fActivity);

		// Determine if the window's orientation has changed.
		// This won't match device orientation if the activity was assigned a fixed orientation.
		// Note: Only store the current orientation if the application supports it.
		//       This way it'll never be reported to Lua via an orientation event.
		if (isCurrentOrientationSupported && (fCurrentWindowOrientation != currentOrientation)) {
			fPreviousWindowOrientation = fCurrentWindowOrientation;
			fCurrentWindowOrientation = currentOrientation;
		}

		// Resize the OpenGL viewport with the given surface's width and height.
		super.surfaceChanged(holder, format, width, height);
	}

	/**
	 * Called when this view's OpenGL surface is being requested to be destroyed.
	 * @param holder Reference to the surface that contains this OpenGL surface.
	 */
	@Override
	public void surfaceDestroyed(android.view.SurfaceHolder holder) {
		// Stop the watchdog, if running.
		fWatchdogTimer.stop();

		// Destroy the OpenGL surface.
		super.surfaceDestroyed(holder);
	}

	/** Called when the Corona runtime has been started or resumed. */
	public void onResumeCoronaRuntime() {
		// Start the OpenGL watchdog.
		fWatchdogTimer.start();

		// Enable the orientation listener if the device has the sensor.
		if (fOrientationListener.canDetectOrientation()) {
			fOrientationListener.enable();
		}
	}

	/** Called when the Corona runtime has been suspended or exited. */
	public void onSuspendCoronaRuntime() {
		// Stop the OpenGL watchdog.
		fWatchdogTimer.stop();

		// Disable the orientation listener.
		if (fOrientationListener.canDetectOrientation()) {
			fOrientationListener.disable();
		}
	}

	/** Informs the OpenGL rendering system to clear the first rendered frame. */
	public void clearFirstSurface() {
		fRenderer.clearFirstSurface();
	}
    
    /**
     * Determines if this surface can currently render to OpenGL.
     * @return Returns true if OpenGL content can be rendered. Returns false if not.
     */
	@Override
	public boolean canRender() {
		return ((fRenderer != null) && fRenderer.canRender() && super.canRender());
	}

	/** Internal class used to render OpenGL content via the Corona runtime. */
	private static class CoronaRenderer implements GLSurfaceView.Renderer {
		/** Reference to the OpenGL view that owns this renderer. */
		private CoronaGLSurfaceView fView;

		/** Set true to indicate that this renderer is ready to draw to the OpenGL view. */
		private boolean fCanRender;

		/** The last orienation that was sent to Corona's orientation event listener. */
		private com.ansca.corona.WindowOrientation fLastReceivedWindowOrientation;

		/** Stores the last received view width in pixels. */
		private int fLastViewWidth;

		/** Stores the last received view height in pixels. */
		private int fLastViewHeight;

		/** Indicates if this renderer is drawing to the first surface it has received. */
		private static boolean sFirstSurface = true;
		
		private com.ansca.corona.CoronaRuntime fCoronaRuntime;

		private boolean fIsCoronaKit;

		/**
		 * Creates a new renderer.
		 * @param view Reference to the OpenGL surface that owns this renderer. Cannot be null.
		 */
		public CoronaRenderer(CoronaGLSurfaceView view, com.ansca.corona.CoronaRuntime runtime, boolean isCoronaKit) {
			if (view == null) {
				throw new NullPointerException();
			}
			fView = view;
			fCanRender = false;
			fLastReceivedWindowOrientation = com.ansca.corona.WindowOrientation.UNKNOWN;
			fLastViewWidth = -1;
			fLastViewHeight = -1;
			fIsCoronaKit = isCoronaKit;
			fCoronaRuntime = runtime;
		}

		/**
		 * Called when a new OpenGL surface has been created.
		 * <p>
		 * Warning: This method is always called on the OpenGL thread.
		 * @param gl The OpenGL interface.
		 * @param config The OpenGL ES configuration the new surface is using.
		 */
		@Override
		public void onSurfaceCreated(
			javax.microedition.khronos.opengles.GL10 gl, javax.microedition.khronos.egl.EGLConfig config)
		{
			// Re-draw what was last rendered if the last surface has been replaced by a new surface.
			// This can happen when the end user leaves and returns back to the activity window.
			if (!sFirstSurface) {
				fView.setNeedsSwap();
			}
			sFirstSurface = false;
			
			// Unload textures that belonged to the previous OpenGL context which are no longer valid.
			// This forces textures to be re-loaded for the new OpenGL context.
			com.ansca.corona.JavaToNativeShim.unloadResources(fCoronaRuntime);
		}

		/**
		 * Called when the OpenGL surface has changed size.
		 * <p>
		 * Warning: This method is always called on the OpenGL thread.
		 * @param gl The OpenGL interface.
		 * @param width The new width in pixels.
		 * @param height The new height in pixels.
		 */
		@Override
		public void onSurfaceChanged(javax.microedition.khronos.opengles.GL10 gl, int width, int height) {
			// Fetch the view's current orientation.
			// Note: We need to keep a local copy of the view's orientation settings in case they suddenly
			//       change on the main UI thread while executing the below resize() operation, which can
			//       take a long time on the first call since it executes the "main.lua" on startup.
			com.ansca.corona.WindowOrientation currentWindowOrientation = fView.fCurrentWindowOrientation;
			com.ansca.corona.WindowOrientation previousWindowOrientation = fView.fPreviousWindowOrientation;

			// Do not render anything if given a width/height for an orientation that the we do not support.
			// This can happen while the activity is transitioning from one orientation to another.
			// Note: This check assumes that this view is being displayed full screen.
			// CoronaKit does not have this restriction because the activity can be in portrait but the view
			// can just be a different wide and vice versa.
			if (!fIsCoronaKit && 
				((currentWindowOrientation.isPortrait() && (width > height)) ||
				 (currentWindowOrientation.isLandscape() && (width < height)))) {
				fCanRender = false;
				return;
			}
			
			if (fIsCoronaKit) {
				currentWindowOrientation = com.ansca.corona.WindowOrientation.PORTRAIT_UPRIGHT;
			}

			// Update the OpenGL view port on the C++ side of Corona. This also happens to initialize Corona too.
			// It is okay to call the C++ side directly since we're on the OpenGL thread here.
			com.ansca.corona.JavaToNativeShim.resize(fCoronaRuntime, fView.getContext(), width, height, currentWindowOrientation, fIsCoronaKit);
			
			// Flag that the OpenGL render has been set up and is ready to draw stuff.
			// This flag also indicates that the C++ side of Corona has finished initializing at this point.
			fCanRender = true;
			
			// Raise an orientation event if it has changed.
			// This must be done after giving Corona the new width and height up above so that
			// that Corona's orientation event listeners will can correctly relayout the screen.
			if (fLastReceivedWindowOrientation == com.ansca.corona.WindowOrientation.UNKNOWN) {
				fLastReceivedWindowOrientation = currentWindowOrientation;
			}
			else if (fLastReceivedWindowOrientation != currentWindowOrientation) {
				fLastReceivedWindowOrientation = currentWindowOrientation;
				if (fCoronaRuntime != null) {
					fCoronaRuntime.getTaskDispatcher().send(
						new com.ansca.corona.events.OrientationTask(
							currentWindowOrientation.toCoronaIntegerId(),
							previousWindowOrientation.toCoronaIntegerId()));
				}
			}

			// Raise a resize event if the OpenGL view width and height has changed.
			// This is expected to be raised after the orientation event.
			if ((fLastViewWidth >= 0) && (fLastViewHeight >= 0) &&
			    ((fLastViewWidth != width) || (fLastViewHeight != height)))
			{
				if (fCoronaRuntime != null) {
					fCoronaRuntime.getTaskDispatcher().send(new com.ansca.corona.events.ResizeTask());
				}
			}
			fLastViewWidth = width;
			fLastViewHeight = height;
		}

		/**
		 * Called when the OpenGL surface is requesting this renderer to draw its content.
		 * <p>
		 * Warning: This method is always called on the OpenGL thread.
		 * @param gl The OpenGL interface.
		 */
	    @Override
	    public void onDrawFrame(javax.microedition.khronos.opengles.GL10 gl) {
			com.ansca.corona.Controller.updateRuntimeState(fCoronaRuntime, fCanRender);
		}
	    
	    /** Informs this renderer to clear the first rendered frame. */
	    public void clearFirstSurface() {
	    	sFirstSurface = true;
	    }
		
		/**
		 * Determines if this object is ready to render its content to OpenGL.
		 * @return Returns true if this renderer is ready to draw to OpenGL. Returns false if not.
		 */
		public boolean canRender() {
			return fCanRender;
		}
	}
}
