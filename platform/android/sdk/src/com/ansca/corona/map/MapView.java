//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.maps;

import android.webkit.WebSettings;

/**
 * View for displaying a map.
 * <p>
 * Map rendering is done via a WebView which displays the Google Maps website. This allows Corona to
 * support maps on all Android devices, including Kindle Fire and Nook which do not include the
 * official Android maps library needed for native rendering. We're also doing this via a WebView
 * because an Android MapView object can only be displayed within a MapActivity derived class.
 * Thus, it is not designed to be embedded in any other activity.
 */
public class MapView extends android.widget.FrameLayout {
	/** The web view used to display the Google Maps web site. */
	private android.webkit.WebView fWebView;

	/** Animated spinner indicating that something is loading. */
	private android.widget.ProgressBar fLoadingIndicatorView;

	/** Timer used to reload the web view in case the map fails to load in time on startup. */
	private com.ansca.corona.MessageBasedTimer fWatchdogTimer;

	/**
	 * Queue of operations to be performed on this map and its web view.
	 * Used to queue operations while the map is loading and then executed once the map has finished loading.
	 */
	private java.util.LinkedList<Runnable> fOperationQueue;

	/** Set true once the web view has successfully loaded Google Maps and is ready for user input. */
	private boolean fIsMapLoaded;

	/** The type of map currently being displayed such as Standard, Satellite, or Hybrid. */
	private MapType fMapType;

	/** Set to true to have the map track the device's current position via GPS. */
	private boolean fIsCurrentLocationTrackingEnabled;

	/** Set to true if zooming is enabled. Set false if disabled. */
	private boolean fIsZoomEnabled;

	/** Set to true to allows users to scroll/pan the map by hand. Set to false to disable scrolling/panning. */
	private boolean fIsScrollEnabled;

	/** Stores this device's current location obtained via the GPS, WiFi, or cellular service. */
	private android.location.Location fCurrentLocation;

	/**
	 * The location bounds displayed by the map, specified in latitude and longitude coordinates.
	 * The bounds are to be updated everytime the map has been panned and zoomed.
	 */
	private LocationBounds fMapLocationBounds;

	/** Used as an id generator for markers. */
	private java.util.concurrent.atomic.AtomicInteger fIdCounter;

	/** Used to store all the markers on a map */
	private java.util.Hashtable<Integer, MapMarker> fMarkerTable;

	private com.ansca.corona.CoronaRuntime fCoronaRuntime;

	private com.ansca.corona.Controller fController;

	/**
	 * Creates a new view for displaying maps.
	 * @param context The context this view will be associated with. Typically the activity context. Cannot be null.
	 * @throws SecurityException If the INTERNET permission is not provided.
	 */
	public MapView(android.content.Context context, com.ansca.corona.CoronaRuntime runtime, com.ansca.corona.Controller controller) {
		super(context);

		// Throw an exception if this application does not have the following permission.
		context.enforceCallingOrSelfPermission(android.Manifest.permission.INTERNET, null);

		fCoronaRuntime = runtime;

		fController = controller;

		fIdCounter = new java.util.concurrent.atomic.AtomicInteger(1);

		fMarkerTable = new java.util.Hashtable<Integer, MapMarker>();

		// Initialize member variables.
		fOperationQueue = new java.util.LinkedList<Runnable>();
		fIsMapLoaded = false;
		fMapType = MapType.STANDARD;
		fIsCurrentLocationTrackingEnabled = false;
		fIsZoomEnabled = true;
		fIsScrollEnabled = true;
		fCurrentLocation = null;
		fMapLocationBounds = null;

		// Set the background of this view to dark grey.
		// This way the activity indicator (which is typically light colored) will be easy to see.
		setBackgroundColor(android.graphics.Color.DKGRAY);

		// Create an activity indicator and place it at the bottom of the z-order.
		// To be displayed while the map web view is loading.
		fLoadingIndicatorView = new android.widget.ProgressBar(context);
		fLoadingIndicatorView.setLayoutParams(new android.widget.FrameLayout.LayoutParams(
					android.widget.FrameLayout.LayoutParams.WRAP_CONTENT,
					android.widget.FrameLayout.LayoutParams.WRAP_CONTENT,
					android.view.Gravity.CENTER));
		addView(fLoadingIndicatorView);

		// Create the web view and add it to this view.
		fWebView = new android.webkit.WebView(context) {
			@Override
			public void invalidate(int l, int t, int r, int b) {
				if (((r - l) <= 0) || ((b - t) <= 0)) {
					// Redraw the entire view if given an invalid region.
					// Note: Nook HD always calls this method with all zeros, making this work-around necessary.
					invalidate();
				}
				else {
					// Redraw the given region.
					super.invalidate(l, t, r, b);
				}
			}
		};
		fWebView.setLayoutParams(new android.widget.FrameLayout.LayoutParams(
					android.widget.FrameLayout.LayoutParams.MATCH_PARENT,
					android.widget.FrameLayout.LayoutParams.MATCH_PARENT));
		fWebView.setVerticalScrollBarEnabled(false);
		fWebView.setHorizontalScrollBarEnabled(false);
		fWebView.setVisibility(android.view.View.INVISIBLE);
		addView(fWebView);

		// Enable support for JavaScript, HTML5 DOM storage, Flash plug-in, touch zoom controls, etc.
		android.webkit.WebSettings settings = fWebView.getSettings();
		settings.setSupportZoom(false);
		settings.setBuiltInZoomControls(false);
		settings.setLoadWithOverviewMode(true);
		settings.setUseWideViewPort(true);
		settings.setJavaScriptEnabled(true);
		settings.setGeolocationEnabled(true);
		settings.setPluginState(android.webkit.WebSettings.PluginState.ON);
		settings.setDomStorageEnabled(true);

		// Although CoronaEnviornment.getInternalWebCachesDirectory() is our class, we don't want to expose that which is why

		settings.setCacheMode(WebSettings.LOAD_CACHE_ELSE_NETWORK);
		// we have to use reflection to access it
		settings.setAllowFileAccess(true);
		if (android.os.Build.VERSION.SDK_INT >= 11) {
			try {
				java.lang.reflect.Method setEnableSmoothTransitionMethod;
				setEnableSmoothTransitionMethod = android.webkit.WebSettings.class.getMethod(
						"setEnableSmoothTransition", new Class[] { Boolean.TYPE });
				setEnableSmoothTransitionMethod.invoke(settings, new Object[] { true });
			}
			catch (Exception ex) { ex.printStackTrace(); }
		}

		// Set up a web view event listener.
		fWebView.setWebViewClient(new WebViewEventHandler());

		// Set up a UI listener for handling current location tracking request.
		fWebView.setWebChromeClient(new android.webkit.WebChromeClient() {
			@Override
			public void onGeolocationPermissionsShowPrompt(
				String origin, android.webkit.GeolocationPermissions.Callback callback)
			{
				if (callback != null) {
					callback.invoke(origin, true, false);
				}
			}
		});

		// Set up an interface that allows JavaScript to make calls to this map view object.
		fWebView.addJavascriptInterface(new JavaScriptInterface(), "corona");

		// Set up a touch event listner for the web view.
		fWebView.setOnTouchListener(new android.view.View.OnTouchListener() {
			@Override
			public boolean onTouch(android.view.View view, android.view.MotionEvent event) {
				// Override the pinch zoom gesture if zooming is disabled.
				if ((isZoomEnabled() == false) && (event.getPointerCount() > 1)) {
					return true;
				}

				// Make sure the web view has the focus when the user taps it.
				// This allows the virtual keyboard to appear when the user taps on a text field.
				switch (event.getAction()) {
					case android.view.MotionEvent.ACTION_DOWN:
					case android.view.MotionEvent.ACTION_UP:
						if (view.hasFocus() == false) {
							view.requestFocus();
						}
						break;
				}

				// Let this view perform the default handling for this event.
				return false;
			}
		});

		// Disable hardware this view's hardware acceleration if running on an Android OS older than 4.1.
		// This works-around an Android bug where the web view's right and bottom sides will not render the
		// web view's contents (ie: canvas is clipped) if the map view has been shifted to the right and down.
		if (android.os.Build.VERSION.SDK_INT < 16) {
			disableHardwareAcceleration();
		}

		// Extract the map HTML file from app's resources to an external directory, if not done already.
		// This is needed so that the web view can access the HTML file via a URL.
		String resourceName = "corona_map_view2";

		// Although CoronaEnviornment.getInternalWebCachesDirectory() is our class, we don't want to expose that which is why
		// we have to use reflection to access it
		java.io.File file = new java.io.File(getInternalCacheDirectory(context, "getInternalResourceCachesDirectory"), resourceName + ".html");
		if (file.exists() == false) {
			java.io.InputStream inputStream = null;
			java.io.FileOutputStream outputStream = null;
			try {
				com.ansca.corona.storage.ResourceServices resourceServices;
				resourceServices = new com.ansca.corona.storage.ResourceServices(context);
				int resourceId = resourceServices.getRawResourceId(resourceName);
				inputStream = context.getResources().openRawResource(resourceId);
				outputStream = new java.io.FileOutputStream(file);
				byte[] byteBuffer = new byte[1024];
				while (true) {
					int bytesRead = inputStream.read(byteBuffer);
					if (bytesRead <= 0) {
						break;
					}
					outputStream.write(byteBuffer, 0, bytesRead);
				}
				outputStream.flush();
			}
			catch (Exception ex) {
				ex.printStackTrace();
				return;
			}
			finally {
				if (inputStream != null) {
					try { inputStream.close(); }
					catch (Exception ex) { }
				}
				if (outputStream != null) {
					try { outputStream.close(); }
					catch (Exception ex) { }
				}
			}
			if (file.exists() == false) {
				return;
			}
		}

		// Load the Google Maps web page.
		fWebView.loadUrl(file.toURI().toString());

		// Set up a timer to reload the Google Maps web page in case it fails to load.
		// This typically happens if the device does not have Internet access when loading the map.
		// Google Maps cannot auto-recover from this issue, so we have to reload the page.
		fWatchdogTimer = new com.ansca.corona.MessageBasedTimer();
		fWatchdogTimer.setHandler(new android.os.Handler());
		fWatchdogTimer.setInterval(com.ansca.corona.TimeSpan.fromSeconds(30));
		fWatchdogTimer.setListener(new com.ansca.corona.MessageBasedTimer.Listener() {
			@Override
			public void onTimerElapsed() {
				// Disable the timer if the following has occurred:
				// 1) The map web page was loaded successfully.
				// 2) The Java MapView and its WebView have been destroyed.
				if ((fWebView == null) || (fWebView.getVisibility() == android.view.View.VISIBLE)) {
					fWatchdogTimer.stop();
					return;
				}

				// The Google Maps web page has failed to load in time. Reload the web page.
				fWebView.reload();
			}
		});
		fWatchdogTimer.start();
	}

	/**
	 * Sets whether or not this view will be use software or hardware rendering.
	 * @param layerType The type of layer to use with this view.
	 *                  Must be either LAYER_TYPE_NONE, LAYER_TYPE_SOFTWARE, or LAYER_TYPE_HARDWARE.
	 * @param paint The object used to cmopose the layer.
	 */
	@Override
	public void setLayerType(int layerType, android.graphics.Paint paint) {
		if (android.os.Build.VERSION.SDK_INT >= 11) {
			try {
				java.lang.reflect.Method setLayerTypeMethod = android.view.View.class.getMethod(
						"setLayerType", new Class[] {Integer.TYPE, android.graphics.Paint.class});
				setLayerTypeMethod.invoke(fWebView, new Object[] { layerType, paint });
			}
			catch (Exception ex) { }
		}
	}

	/** Disables hardware acceleration for this view. */
	public void disableHardwareAcceleration() {
		setLayerType(1, null);
	}

	/**
	 * Destroys the internal state of this map view.
	 * <p>
	 * This method should be called after this map view has been removed from its parent view.
	 * No other method may be called on this object after calling this destroy() method.
	 */
	public void destroy() {
		// This is called so that all the lua references are removed.
		removeAllMarkers();

		if (fWebView != null) {
			fWebView.destroy();
			fWebView = null;
		}
		if (fWatchdogTimer != null) {
			fWatchdogTimer.stop();
		}
	}

	/**
	 * Sets the visibility state of this view.
	 * @param visibility Set to VISIBILE, INVISIBLE, or GONE.
	 */
	@Override
	public void setVisibility(int visibility) {
		// Apply the given visibility state to this view's web view.
		// This works-around an Android bug where a hardware accelerated web view
		// will display a black rectangle if its parent view was made invisible/gone.
		if ((fWebView != null) && fIsMapLoaded) {
			fWebView.setVisibility(visibility);
		}

		// Set the visibility state of this view.
		super.setVisibility(visibility);
	}

	/**
	 * Sets the next animation to play for this view.
	 * @param animation The next animation to play. Set to null to clear the last applied animation.
	 */
	@Override
	public void setAnimation(android.view.animation.Animation animation) {
		// Apply the given animation to this view.
		super.setAnimation(animation);

		// Apply the given animation to this view's child views.
		if (fWebView != null) {
			fWebView.setAnimation(animation);
		}
		fLoadingIndicatorView.setAnimation(animation);
	}

	/**
	 * Starts the given animation immediately.
	 * @param animation The animation to play now.
	 */
	@Override
	public void startAnimation(android.view.animation.Animation animation) {
		// Apply the given animation to this view.
		super.startAnimation(animation);

		// Apply the given animation to this view's child views.
		if (fWebView != null) {
			fWebView.startAnimation(animation);
		}
		fLoadingIndicatorView.startAnimation(animation);
	}

	/**
	 * Determines if the map has been successfully loaded and is ready for user input.
	 * @return Returns true if this view is displaying a map and is now accepting user input.
	 *         <p>
	 *         Returns false if the map failed to load information from the Google Maps website
	 *         or if this view has been destroyed.
	 */
	public boolean isLoaded() {
		return (fWebView != null) && fIsMapLoaded;
	}

	/**
	 * Sets the type of map view to be displayed.
	 * @param mapType The type of map view to display such as standard, satellite, or hybrid.
	 */
	public void setMapType(MapType mapType) {
		// Validate.
		if (mapType == null) {
			return;
		}

		// Display the requested map type.
		if (isLoaded()) {
			if (mapType.equals(MapType.STANDARD)) {
				fWebView.loadUrl("javascript:showRoadmapView()");
			}
			else if (mapType.equals(MapType.SATELLITE)) {
				fWebView.loadUrl("javascript:showSatelliteView()");
			}
			else if (mapType.equals(MapType.HYBRID)) {
				fWebView.loadUrl("javascript:showHybridView()");
			}
		}

		// Store the type of map currently being shown.
		fMapType = mapType;
	}

	/**
	 * Gets the type of map view being displayed such as standard, satellite, or hybrid.
	 * @return Returns the type of map view currently being displayed.
	 */
	public MapType getMapType() {
		return fMapType;
	}

	/**
	 * Enables or disables current position tracking on the map.
	 * @param enabled Set true to enable current position tracking. Set false to disable it.
	 */
	public void setCurrentLocationTrackingEnabled(boolean enabled) {
		if (isLoaded()) {
			fWebView.loadUrl("javascript:setCurrentLocationTrackingEnabled(" + Boolean.toString(enabled) + ")");
		}
		fIsCurrentLocationTrackingEnabled = enabled;
	}

	/**
	 * Determines if the map is set up to track the device's current position via GPS.
	 * @return Returns true if the map is tracking the device's current position. Returns false if not.
	 */
	public boolean isCurrentLocationTrackingEnabled() {
		return fIsCurrentLocationTrackingEnabled;
	}

	/**
	 * Gets the device's current location.
	 * @return Returns the current location of this device.
	 *         <p>
	 *         Returns null if current location has not been retrieved yet.
	 */
	public android.location.Location getCurrentLocation() {
		return fCurrentLocation;
	}

	/**
	 * Determines if the device's current location is visible on the map view.
	 * @return Returns true if the current location is visible on the map.
	 *         <p>
	 *         Returns false if not or if current location could not be retrieved.
	 */
	public boolean isCurrentLocationVisible() {
		// Keep a local reference to these member variables in case they suddenly change from another thread.
		android.location.Location currentLocation = fCurrentLocation;
		LocationBounds mapBounds = fMapLocationBounds;
		if ((currentLocation == null) || (mapBounds == null)) {
			return false;
		}

		// Determine if the current location is within the map view's current bounds.
		return mapBounds.contains(currentLocation);
	}

	/**
	 * Enables or disables map zooming via pinch gestures or double tapping.
	 * @param enabled Set true to enable zooming. Set false to disable.
	 */
	public void setZoomEnabled(boolean enabled) {
		if (isLoaded()) {
			fWebView.loadUrl("javascript:setZoomingEnabled(" + Boolean.toString(enabled) + ")");
		}
		fIsZoomEnabled = enabled;
	}

	/**
	 * Determines if map zooming is enabled.
	 * @return Returns true if zooming is enabled. Returns false if disabled.
	 */
	public boolean isZoomEnabled() {
		return fIsZoomEnabled;
	}

	/**
	 * Enables or disables map scrolling and panning.
	 * @param enabled Set true to enable scrolling/panning. Set false to disable.
	 */
	public void setScrollEnabled(boolean enabled) {
		if (isLoaded()) {
			fWebView.loadUrl("javascript:setPanningEnabled(" + Boolean.toString(enabled) + ")");
		}
		fIsScrollEnabled = enabled;
	}

	/**
	 * Determines if map scrolling/panning is enabled.
	 * @return Returns true if scrolling/panning is enabled. Returns false if disabled.
	 */
	public boolean isScrollEnabled() {
		return fIsScrollEnabled;
	}

	/**
	 * Pans the map to show the given coordinates at the center of the screen.
	 * @param latitude The latitude position to center the map at.
	 * @param longitude The longitude position to center the map at.
	 * @param isAnimated Set true to have the map show a nice transition from its current position to the
	 *                   new position. Set false to jump to the new position without any animation.
	 */
	public void setCenter(final double latitude, final double longitude, final boolean isAnimated) {
		fOperationQueue.add(new Runnable() {
			@Override
			public void run() {
				StringBuilder builder = new StringBuilder(64);
				builder.append("javascript:setCenter(");
				builder.append(Double.toString(latitude));
				builder.append(",");
				builder.append(Double.toString(longitude));
				builder.append(",");
				builder.append(Boolean.toString(isAnimated));
				builder.append(")");
				fWebView.loadUrl(builder.toString());
			}
		});
		requestExecuteQueuedOperations();
	}

	/**
	 * Pans the map to show the given coordinates at the center of the screen with the given zoom level.
	 * @param latitude The latitude position to center the map at.
	 * @param longitude The longitude position to center the map at.
	 * @param latitudeSpan The North-to-South distance in degrees the map will zoom to around the center point.
	 * @param longitudeSpan The East-to-West distance in degrees the map will zoom to around the center point.
	 * @param isAnimated Set true to have the map show a nice transition from its current position to the
	 *                   new position. Set false to jump to the new position without any animation.
	 */
	public void setRegion(
		final double latitude, final double longitude,
		final double latitudeSpan, final double longitudeSpan, final boolean isAnimated)
	{
		fOperationQueue.add(new Runnable() {
			@Override
			public void run() {
				StringBuilder builder = new StringBuilder(64);
				builder.append("javascript:setRegion(");
				builder.append(Double.toString(latitude));
				builder.append(",");
				builder.append(Double.toString(longitude));
				builder.append(",");
				builder.append(Double.toString(latitudeSpan));
				builder.append(",");
				builder.append(Double.toString(longitudeSpan));
				builder.append(",");
				builder.append(Boolean.toString(isAnimated));
				builder.append(")");
				fWebView.loadUrl(builder.toString());
			}
		});
		requestExecuteQueuedOperations();
	}

	/**
	 * Gets the next id that should be used for a marker
	 * Returns the next id(int) that should be used
	 */
	public int getNewMarkerId() {
		return fIdCounter.getAndIncrement();
	}

	/**
	 * Adds a marker to the map.
	 * <p>
	 * Setting both arguments "title" and "subtitle" to null or empty string will prevent an annotation
	 * from being displayed on the marker.
	 * @param mapMarker contains description of the map marker
	 */
	public void addMarker(final MapMarker mapMarker) {
		fOperationQueue.add(new Runnable() {
			@Override
			public void run() {
				StringBuilder builder = new StringBuilder(64);
				builder.append("javascript:addMarker(");

				builder.append(Integer.toString(mapMarker.getMarkerId()));

				builder.append(",");
				builder.append(Double.toString(mapMarker.getLatitude()));

				builder.append(",");
				builder.append(Double.toString(mapMarker.getLongitude()));

				builder.append(",");
				if (mapMarker.getTitle() != null) {
					builder.append("\"");
					builder.append(createHtmlTextFrom(mapMarker.getTitle()));
					builder.append("\"");
				}
				else {
					builder.append("null");
				}

				builder.append(",");
				if (mapMarker.getSubtitle() != null) {
					builder.append("\"");
					builder.append(createHtmlTextFrom(mapMarker.getSubtitle()));
					builder.append("\"");
				}
				else {
					builder.append("null");
				}

				builder.append(", ");
				if (mapMarker.getImageFile() != null) {
					builder.append("\"");
					builder.append(com.ansca.corona.storage.FileContentProvider.createContentUriForFile(getContext(), mapMarker.getImageFile()).toString());
					builder.append("\"");
				}
				else {
					builder.append("null");
				}

				builder.append(")");
				fWebView.loadUrl(builder.toString());
			}
		});
		requestExecuteQueuedOperations();

		synchronized(fMarkerTable) {
			fMarkerTable.put(mapMarker.getMarkerId(), mapMarker);
		}
	}

	/**
	 * Remvoes a marker to the map.
	 * @param markerId The id of the marker to remove
	 */
	synchronized public void removeMarker(final int markerId) {
		if (fOperationQueue != null) {
			fOperationQueue.add(new Runnable() {
				@Override
				public void run() {
					if(fWebView != null) {
						StringBuilder builder = new StringBuilder(30);
						builder.append("javascript:removeMarker(");
						builder.append(markerId);
						builder.append(")");
						fWebView.loadUrl(builder.toString());
					}
				}
			});
		}
		requestExecuteQueuedOperations();
		MapMarker marker = fMarkerTable.get(markerId);

		if (marker != null) {
			marker.deleteRef(fCoronaRuntime);
			fMarkerTable.remove(markerId);
		}
	}

	/**
	 * Removes all markers that were added to the map.
	 */
	public void removeAllMarkers() {
		fOperationQueue.add(new Runnable() {
			@Override
			public void run() {
				fWebView.loadUrl("javascript:removeAllMarkers()");
			}
		});
		requestExecuteQueuedOperations();

		java.util.Collection<MapMarker> collection;
		synchronized(fMarkerTable) {
			 collection = fMarkerTable.values();
			 fMarkerTable.clear();
		}

		for (MapMarker marker : collection) {
			if (marker != null) {
				marker.deleteRef(fCoronaRuntime);
			}
		}
	}

	/**
	 * Executes all operations in the "fOperationQueue" member variable if the map is loaded and ready.
	 */
	private void requestExecuteQueuedOperations() {
		// Do not continue if Google Maps has not been loaded yet.
		if (isLoaded() == false) {
			return;
		}

		// Execute all queued operations.
		Runnable runnable;
		while (fOperationQueue.size() > 0) {
			runnable = fOperationQueue.remove();
			if (runnable != null) {
				runnable.run();
			}
		}
	}

	/**
	 * Converts the given string to HTML text.
	 * <p>
	 * Properly escapes characters to appear correctly in HTML such as '<' and '>'.
	 * Replaces newline '\n' characters with "<br>" line break tags.
	 * @param text The string to be converted to HTML text.
	 * @return Returns an HTML encoded string matching the given string.
	 */
	private String createHtmlTextFrom(String text) {
		// Validate argument.
		if ((text == null) || (text.length() <= 0)) {
			return text;
		}

		// Convert the given string to HTML text.
		text = text.replace("&", "&amp;");
		text = text.replace("\"", "&quot;");
		text = text.replace("<", "&lt;");
		text = text.replace(">", "&gt;");
		text = text.replace("\n", "<br>");
		return text;
	}

	/**
	 * Uses reflection to gain access to CoronaEnvironment methods which are default.  We don't want to expose
	 * those methods which is why we are using reflection.
	 */
	private java.io.File getInternalCacheDirectory(android.content.Context context, String directoryName) {
		java.io.File internalCacheDirectory = com.ansca.corona.CoronaEnvironment.getCachesDirectory(context);
		java.lang.reflect.Method[] methods = com.ansca.corona.CoronaEnvironment.class.getDeclaredMethods();
		for(java.lang.reflect.Method method : methods) {
			if (method.getName().equals(directoryName)) {
				try {
					method.setAccessible(true);
					internalCacheDirectory = (java.io.File)method.invoke(null, context);
				} catch (Exception ex) {
				}
			}
		}
		return internalCacheDirectory;
	}

	/**
	 * Provides an interface for JavaScript to make calls to the MapView object.
	 * <p>
	 * Instances of this class are to be passed into the WebView.addJavaScriptInterface() method.
	 */
	private class JavaScriptInterface {
		/** Array of drawable images used to animate a "current location" marker. */
		private final String[] fCurrentLocationMarkerResourceNames = new String[] {
			"ic_maps_indicator_current_position",
			"ic_maps_indicator_current_position_anim1",
			"ic_maps_indicator_current_position_anim2",
			"ic_maps_indicator_current_position_anim3",
			"ic_maps_indicator_current_position_anim2",
			"ic_maps_indicator_current_position_anim1"
		};

		/**
		 * Gets googles api key fetched earlier
		 * @return Returns the Maps API Key.
		 */
		@android.webkit.JavascriptInterface
		public String getGoogleMapAPIKey() {
			return fController.GetGoogleMapsAPIKey();
		}

		/**
		 * Gets the number of URLs to image files used by a "current location" marker animation.
		 * @return Returns the number of URLs to image files.
		 */
		@android.webkit.JavascriptInterface
		public int getCurrentLocationMarkerUrlCount() {
			return fCurrentLocationMarkerResourceNames.length;
		}

		/**
		 * Fetches the URL to the specified marker image file.
		 * @param index Zero based index to the marker's image file URL.
		 * @return Returns a URL to the animated "current location" marker's image file.
		 *         <p>
		 *         Returns NULL if given an invalid index.
		 */
		@android.webkit.JavascriptInterface
		public String getCurrentLocationMarkerUrlByIndex(int index) {
			// Validate.
			if ((index < 0) || (index >= fCurrentLocationMarkerResourceNames.length)) {
				return null;
			}

			// Copy the drawable from the resource file to an external directory, if not done already.
			// This is needed so that the web page can access this image file via a URL.
			java.io.File file = new java.io.File(
					getInternalCacheDirectory(getContext(), "getInternalResourceCachesDirectory"),
					fCurrentLocationMarkerResourceNames[index] + ".png");
			if (file.exists() == false) {
				android.graphics.Bitmap bitmap = null;
				java.io.FileOutputStream stream = null;
				try {
					com.ansca.corona.storage.ResourceServices resourceServices;
					resourceServices = new com.ansca.corona.storage.ResourceServices(
										com.ansca.corona.CoronaEnvironment.getApplicationContext());
					int resourceId = resourceServices.getDrawableResourceId(
										fCurrentLocationMarkerResourceNames[index]);
					bitmap = android.graphics.BitmapFactory.decodeResource(
										resourceServices.getResources(), resourceId);
					stream = new java.io.FileOutputStream(file);
					bitmap.compress(android.graphics.Bitmap.CompressFormat.PNG, 100, stream);
					stream.flush();
				}
				catch (Exception ex) {
					ex.printStackTrace();
					return null;
				}
				finally {
					if (stream != null) {
						try { stream.close(); }
						catch (Exception ex) { }
					}
					if (bitmap != null) {
						// bitmap.recycle();
					}
				}
			}

			// Return the URL to the image file.
			return file.toURI().toString();
		}

		/**
		 * Fetches the map type currently assigned to this view.
		 * @return Returns the string returned by MapType.toInvariantString().
		 */
		@android.webkit.JavascriptInterface
		public String getMapTypeName() {
			MapType mapType = MapView.this.getMapType();
			if (mapType == null) {
				mapType = MapType.STANDARD;
			}
			return mapType.toInvariantString();
		}

		/**
		 * Determines if map zooming is enabled.
		 * @return Returns true if zooming is enabled. Returns false if disabled.
		 */
		@android.webkit.JavascriptInterface
		public boolean isZoomEnabled() {
			return MapView.this.isZoomEnabled();
		}

		/**
		 * Determines if map scrolling/panning is enabled.
		 * @return Returns true if scrolling/panning is enabled. Returns false if disabled.
		 */
		@android.webkit.JavascriptInterface
		public boolean isScrollEnabled() {
			return MapView.this.isScrollEnabled();
		}

		/**
		 * Called when new "current location" data has been received.
		 * @param latitude The device's current latitude location.
		 * @param longitude The device's current longitude location.
		 * @param positionAccuracyInMeters The latitude and longitude accuracy.
		 * @param altitudeInMeters The device's current altitude. Set to zero if unknown.
		 * @param altitudeAccuracyInMeters The altitude value's accuracy. Set to zero if altitude is unknown.
		 * @param headingInDegrees The direction that the device is currently facing.
		 * @param speedInMetersPerSecond The speed the device is estimated to be moving in.
		 * @param timestamp The time this data was recorded on in milliseconds since 1970, the Unix time epoch.
		 */
		@android.webkit.JavascriptInterface
		public void onCurrentLocationReceived(
			double latitude, double longitude, double positionAccuracyInMeters,
			double altitudeInMeters, double altitudeAccuracyInMeters, double headingInDegrees,
			double speedInMetersPerSecond, long timestamp)
		{
			android.location.Location currentLocation = new android.location.Location("");
			currentLocation.setLatitude(latitude);
			currentLocation.setLongitude(longitude);
			currentLocation.setAccuracy((float)positionAccuracyInMeters);
			currentLocation.setAltitude(altitudeInMeters);
			currentLocation.setBearing((float)headingInDegrees);
			currentLocation.setSpeed((float)speedInMetersPerSecond);
			currentLocation.setTime(timestamp);
			fCurrentLocation = currentLocation;
		}

		/** Called when current location is unknown. */
		@android.webkit.JavascriptInterface
		public void onCurrentLocationLost() {
			fCurrentLocation = null;
		}

		/**
		 * Called when the map has been successfully loaded on the JavaScript side.
		 * Hides the activity indicator and display the web view that is hosting the map.
		 */
		@android.webkit.JavascriptInterface
		public void onMapLoadFinished() {
			getHandler().post(new Runnable() {
				@Override
				public void run() {
					// Flag that Google Maps has been successfully loaded.
					fIsMapLoaded = true;

					// Disable the web view's watchdog timer.
					fWatchdogTimer.stop();

					// Show the web view, provided that this MapView object is visible too.
					fWebView.setVisibility(getVisibility());
					fWebView.setAnimation(getAnimation());
					fWebView.invalidate();

					// Remove the map view's "Loading Indicator" view and its background.
					// This way if the map is displayed semi-transparent, then the views behind it won't show through.
					// Note: There is a hardware acceleration bug on Android 4.0 and higher where removing this
					//       view causes the screen to turn black. The only known work-around is to disable
					//       hardware acceleration on this view and then hide it by setting the alpha to zero.
					setBackgroundDrawable(null);
					if (android.os.Build.VERSION.SDK_INT >= 11) {
						try {
							java.lang.reflect.Method setLayerTypeMethod = android.view.View.class.getMethod(
									"setLayerType", new Class[] {Integer.TYPE, android.graphics.Paint.class});
							setLayerTypeMethod.invoke(fLoadingIndicatorView, new Object[] { 1, null });
							java.lang.reflect.Method setAlphaMethod = android.view.View.class.getMethod(
									"setAlpha", new Class[] {Float.TYPE});
							setAlphaMethod.invoke(fLoadingIndicatorView, new Object[] { 0.0f });
						}
						catch (Exception ex) { }
					}
					else {
 						removeView(fLoadingIndicatorView);
					}

					// Execute all pending map operations now that Google Maps has been loaded.
					requestExecuteQueuedOperations();
				}
			});
		}

		/**
		 * Called when the map bounds have been changed after it has been zoomed or panned.
		 * @param southWestLatitude The bottom-left latitude coordinate currently being shown on the map.
		 * @param southWestLongitude The bottom-left longitude coordinate currently being shown on the map.
		 * @param northEastLatitude The top-right latitude coordinate currently being shown on the map.
		 * @param northEastLongitude The top-right longitude coordinate currently being shown on the map.
		 */
		@android.webkit.JavascriptInterface
		public void onMapBoundsChanged(
			double southWestLatitude, double southWestLongitude, double northEastLatitude, double northEastLongitude)
		{
			fMapLocationBounds = new LocationBounds(
					southWestLatitude, southWestLongitude, northEastLatitude, northEastLongitude);
		}

		@android.webkit.JavascriptInterface
		public void onMarkerTouch(final int markerId) {
			if (fCoronaRuntime != null && fCoronaRuntime.isRunning()) {
				MapMarker marker = fMarkerTable.get(markerId);
				if (marker != null) {
					fCoronaRuntime.getTaskDispatcher().send(new MapMarkerTask(marker));
				}
			}
		}

		@android.webkit.JavascriptInterface
		public void onMapTapped(double latitude, double longitude) {
			if (fCoronaRuntime != null && fCoronaRuntime.isRunning()) {
				fCoronaRuntime.getTaskDispatcher().send(new MapTappedTask(MapView.this.getId(), latitude, longitude));
			}
		}
	}


	/** Handles web view events. */
    private class WebViewEventHandler extends android.webkit.WebViewClient {
		/**
		 * Called when a link has been tapped/clicked on and before it has been started to load.
		 * This is the application's opportunity to block the URL request, if necessary.
		 * <p>
		 * Note that this method does not get called for JavaScript URL requests or requests done via Java/Lua code.
		 * @param view Reference to the WebView that is about to load the given URL.
		 * @param url The URL that is being requested to load.
		 * @return Returns true if this web view is denying/overriding the URL request. Returns false to load the URL.
		 */
		@Override
		public boolean shouldOverrideUrlLoading(android.webkit.WebView view, String url) {
			// Display the requested web page in Android's default web browser.
			// Ignore URL requests to Google Maps since it is already being displayed by this MapView.
			if ((url != null) && (url.toLowerCase().contains("maps.google.com/maps?") == false)) {
				fController.openUrl(url);
			}

			// Return true to inform the MapView's web view to not load the given URL.
			return true;
		}

		/**
		 * Called when a web page is about to be loaded.
		 * @param view Reference to the WebView that the page is loading in.
		 * @param url The URL that is being loaded.
		 * @param favicon The web page's favorite icon.
		 */
		@Override
		public void onPageStarted(android.webkit.WebView view, String url, android.graphics.Bitmap favicon) {
			super.onPageStarted(view, url, favicon);
		}

		/**
		 * Called when the web page has finished loading.
		 * @param view Reference to the WebView that has finished loading the page.
		 * @param url The URL that the web page has been loaded from.
		 */
		@Override
		public void onPageFinished(android.webkit.WebView view, String url) {
			super.onPageFinished(view, url);
		}

		/**
		 * Called when the WebView has failed to load the given URL.
		 * @param view Reference to the WebView that has failed to load the web page.
		 * @param errorCode Unique integer ID indicating the type of error that occurred.
		 * @param description Message indicating the error that has occurred.
		 */
		@Override
		public void onReceivedError(android.webkit.WebView view, int errorCode, String description, String failingUrl) {
			super.onReceivedError(view, errorCode, description, failingUrl);

			fWebView.setVisibility(android.view.View.GONE);
		}
	}

	/**
	 * Represents a region on a globe specified in latitude and longitude coordinates.
	 * <p>
	 * Instances of this class are immutable.
	 */
	private class LocationBounds {
		/** The south-west point of the region. */
		private android.location.Location fSouthWestLocation;

		/** The north-east point of the region. */
		private android.location.Location fNorthEastLocation;

		/**
		 * Creates a new location bounds with the given coordinates.
		 * @param southWestLatitude The latitude of the south-west point of the location bounds.
		 * @param southWestLongitude The longitude of the south-west point of the location bounds.
		 * @param northEastLatitude The latitude of the north-east point of the location bounds.
		 * @param northEastLongitude The longitude of the north-east point of the location bounds.
		 */
		public LocationBounds(
			double southWestLatitude, double southWestLongitude,
			double northEastLatitude, double northEastLongitude)
		{
			fSouthWestLocation = new android.location.Location("");
			fSouthWestLocation.setLatitude(southWestLatitude);
			fSouthWestLocation.setLongitude(southWestLongitude);

			fNorthEastLocation = new android.location.Location("");
			fNorthEastLocation.setLatitude(northEastLatitude);
			fNorthEastLocation.setLongitude(northEastLongitude);
		}

		/**
		 * Determines if the given location lies with the bounds.
		 * @param location The location to check if it is within this object's bounds.
		 * @return Returns true if the location is within the bounds.
		 *         <p>
		 *         Returns false if not or if given null.
		 */
		public boolean contains(android.location.Location location) {
			// Validate.
			if (location == null) {
				return false;
			}

			// Determine if the given location lies within the bounds.
			if ((location.getLatitude() >= fSouthWestLocation.getLatitude()) &&
			    (location.getLatitude() <= fNorthEastLocation.getLatitude())) {
				if (fSouthWestLocation.getLongitude() <= fNorthEastLocation.getLongitude()) {
					if ((location.getLongitude() >= fSouthWestLocation.getLongitude()) &&
					    (location.getLongitude() <= fNorthEastLocation.getLongitude())) {
						return true;
					}
				}
				else {
					if ((location.getLongitude() <= fSouthWestLocation.getLongitude()) &&
					    (location.getLongitude() >= fNorthEastLocation.getLongitude())) {
						return true;
					}
				}
			}
			return false;
		}
	}
}
