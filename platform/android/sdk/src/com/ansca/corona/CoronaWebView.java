//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

import android.content.Context;
import android.graphics.Color;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.webkit.JavascriptInterface;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.LinkedList;
import java.util.List;

/** View for displaying web pages. */
public class CoronaWebView extends WebView  implements NativePropertyResponder {
	private static final String CORONA_INTERFACE_NAME = "corona";

	// JavaScript injection code
	private static final String NATIVE_BRIDGE_CODE =
		"const NativeBridge = {\n" +
		"    callNative: function(method, args) {\n" +
		"        return new Promise((resolve, reject) => {\n" +
		"            var eventName = 'JS_' + method;\n" +
		"            window.addEventListener(eventName, function(e) {\n" +
		"                resolve(e.detail);\n" +
		"            }, { once: true });\n" +
		"            window.corona.postMessage(JSON.stringify({\n" +
		"                type: eventName,\n" +
		"                data: JSON.stringify(args),\n" +
		"                noResult: false\n" +
		"            }));\n" +
		"        });\n" +
		"    },\n" +
		"    sendToLua: function(event, data) {\n" +
		"       var eventName = 'JS_' + event;\n" +
		"        window.corona.postMessage(JSON.stringify({\n" +
		"            type: eventName,\n" +
		"            data: JSON.stringify(data),\n" +
		"            noResult: true\n" +
		"        }));\n" +
		"    },\n" +
		"    on: function(event, callback, options) {\n" +
		"        var eventName = 'JS_' + event;\n" +
		"        window.addEventListener(eventName, function(e) {\n" +
		"            callback(e.detail)\n" +
		"        }, options);\n" +
		"    }\n" +
		"};\n";

	// JavaScript interface class
	private class WebViewJSInterface {
		@JavascriptInterface
		public void postMessage(String message) {
			try {
				JSONObject json = new JSONObject(message);
				String type = json.getString("type");
				String data = json.getString("data");
				boolean noResult = json.optBoolean("noResult", false);

				fCoronaRuntime.getTaskDispatcher().send(new com.ansca.corona.events.WebJSInterfaceCommonTask(getId(), type, data, noResult));
			} catch (JSONException e) {
				e.printStackTrace();
			}
		}
	}

	/**
	 * Class providing URL request source type IDs matching Corona's C++ UrlRequestEvent::Type enum.
	 * These types indicate where the URL request came from such as via a tapped link, reload, etc.
	 * <p>
	 * You cannot create instances of this class.
	 */
	private static class UrlRequestSourceType {
		/** Constructor made private to prevent objects from being made. */
		private UrlRequestSourceType() { }
		
		
		/** Indicates that the URL request came from a link that the user tapped on. */
		public static final int LINK = 0;
		
		/** Indicates that the URL request came from a submit form event via POST or GET. */
		public static final int FORM = 1;
		
		/** Indicates that the URL request came from going back/forward in the browser's web history. */
		public static final int HISTORY = 2;
		
		/** Indicates that the URL request came from a page reload. */
		public static final int RELOAD = 3;
		
		/** Indicates that the URL request came from re-submitting a form via POST or GET. */
		public static final int RESUBMITTED = 4;
		
		/** Indicates that the URL request came from this web view's methods via Java or Lua. */
		public static final int OTHER = 5;
	}
	
	
	/** Set true to allow the back key to go back to the previous web page. Set false to ignore back key. */
	private boolean fBackKeySupported;
	
	/** Set true to allow the Back key close the web view if there is no more web history. */
	private boolean fAutoCloseEnabled;
	
	/** Indicates where the URL request came from such as a link or reload. */
	private int fUrlRequestSourceType;
	
	// If the webview is currently loading a page
	private boolean fIsLoading;

	/**
	 * Stores a collection of error event information that was received by the WebViewClient.onReceivedError() method.
	 * Uses URLs as keys to indicate which URLs failed to load.
	 */
	private java.util.HashMap<String, com.ansca.corona.events.DidFailLoadUrlTask> fReceivedErrorEvents;
	
	private CoronaRuntime fCoronaRuntime;

	/**
	 * Creates a new WebView for displaying web pages.
	 * @param context A Context object used to access application assets.
	 */
	public CoronaWebView(Context context, CoronaRuntime runtime) {
		super(context);
		
		fCoronaRuntime = runtime;

		// Initialize member variables.
		fBackKeySupported = true;
		fAutoCloseEnabled = true;
		fUrlRequestSourceType = UrlRequestSourceType.OTHER;
		fReceivedErrorEvents = new java.util.HashMap<String, com.ansca.corona.events.DidFailLoadUrlTask>();
		
		// Set up a web browser event listener.
		setWebViewClient(new CoronaWebViewClient());
		
		// Set up a web chrome client for enabling JavaScript alerts and location/GPS tracking.
		setWebChromeClient(new android.webkit.WebChromeClient() {
			View mCustomView;

			@Override
			public void onGeolocationPermissionsShowPrompt(
				String origin, android.webkit.GeolocationPermissions.Callback callback)
			{
				if (callback != null) {
					callback.invoke(origin, true, false);
				}
			}

			/*
			 Override only exists on Lollipop(5.0) or above.
			 On pre 5.0 devices, this could be supported through an undocumented API, openFileChooser().
			 However, we won't support this behavior prior to Lollipop because of the undocumented nature.
			 Furthermore, this undocumented method didn't work for 4.4.0 - 4.4.2.
			*/
			@Override
			public boolean onShowFileChooser(WebView webView, 
				android.webkit.ValueCallback<android.net.Uri[]> filePathCallback, 
				android.webkit.WebChromeClient.FileChooserParams fileChooserParams) {

				ApiLevel21.openFileUpload(filePathCallback);
				return true;
			}

			@Override
			public void onShowCustomView(View view, android.webkit.WebChromeClient.CustomViewCallback callback) {
				// Kitkat(4.4) or above
				if (android.os.Build.VERSION.SDK_INT >= 19) {
					CoronaActivity activity = CoronaEnvironment.getCoronaActivity();
					if (activity != null) {
						android.widget.FrameLayout layout = activity.getOverlayView();
						ViewGroup.LayoutParams params = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
						layout.addView(view, params);
						mCustomView = view;
					}
				} else {
					/*
					 On pre 4.4 devices we disable full screen videos because:
					 1) On 4.0 > x devices the controls don't show up.
					 2) On 4.4 > x > 2.3 devices the touch goes through the video and goes to the webview.
					 3) On 4.4 > x devices the view would result in a black rectangle.  This can be solved by setting setZOrderMediaOverlay for both the CoronaGLView and the view that we get.

					 Attempts at displaying the custom view in another activity also failed.  The issues were:
					 1) On 4.4 > x devices the controls wouldn't show up.
					*/
					onHideCustomView();
					callback.onCustomViewHidden();
				}
			}

			@Override
			public void onHideCustomView() {
				if (mCustomView != null) {
					CoronaActivity activity = CoronaEnvironment.getCoronaActivity();
					if (activity != null) {
						ViewManager viewManager = fCoronaRuntime.getViewManager();
						android.widget.FrameLayout layout = activity.getOverlayView();
						layout.removeView(mCustomView);
					}
				}
			}
		});
		
		// Enable support for JavaScript, HTML5 DOM storage, Flash plug-in, touch zoom controls, etc.
		android.webkit.WebSettings settings = getSettings();
		settings.setJavaScriptEnabled(true);
		settings.setSupportZoom(true);
		settings.setBuiltInZoomControls(true);
		settings.setLoadWithOverviewMode(true);
		settings.setUseWideViewPort(true);
		settings.setPluginState(android.webkit.WebSettings.PluginState.ON);
		settings.setDomStorageEnabled(true);
		if (android.os.Build.VERSION.SDK_INT >= 17) {
			settings.setMediaPlaybackRequiresUserGesture(false);
		}
		if (android.os.Build.VERSION.SDK_INT >= 11) {
			try {
				java.lang.reflect.Method setEnableSmoothTransitionMethod;
				setEnableSmoothTransitionMethod = android.webkit.WebSettings.class.getMethod(
						"setEnableSmoothTransition", new Class[] { Boolean.TYPE });
				setEnableSmoothTransitionMethod.invoke(settings, new Object[] { true });
			}
			catch (Exception ex) { ex.printStackTrace(); }
		}
		if (android.os.Build.VERSION.SDK_INT >= 21) {
			ApiLevel21.setAcceptThirdPartyCookies(this, true);
			ApiLevel21.setMixedContentModeToAlwaysAllowFor(settings);
		}

		addJavascriptInterface(new WebViewJSInterface(), CORONA_INTERFACE_NAME);

		// Set up web view to have the focus when touched.
		// This allows the virtual keyboard to appear when the user taps on a text field.
		setOnTouchListener(new android.view.View.OnTouchListener() {
			@Override
			public boolean onTouch(android.view.View view, android.view.MotionEvent event) {
				switch (event.getAction()) {
					case android.view.MotionEvent.ACTION_DOWN:
					case android.view.MotionEvent.ACTION_UP:
						if (view.hasFocus() == false) {
							view.requestFocus();
						}
						break;
				}
				return false;
			}
		});
	}

	/**
	 * Destroys the internal state of this web view.
	 * <p>
	 * This method should be called after this web view has been removed from the view system.
	 * No other methods may be called on this web after it has been destroyed.
	 */
	@Override
	public void destroy() {
		// Remove our onTouch listener to avoid possibly doing something with a destroyed WebView instance later on.
		setOnTouchListener(null);
		
		if (fCoronaRuntime.wasNotDisposed()) {
			// Notify the native side of Corona that this web view has been closed/destroyed.
			com.ansca.corona.events.EventManager eventManager = fCoronaRuntime.getController().getEventManager();
			if (eventManager != null) {
				final int id = getId();
				eventManager.addEvent(new com.ansca.corona.events.RunnableEvent(new Runnable() {
					@Override
					public void run() {
						JavaToNativeShim.webViewClosed(fCoronaRuntime, id);
					}
				}));
			}
		}
		
		// Destroy this web view.
		super.destroy();
	}
	
	/**
	 * Gets the background color that was set via the setBackgroundColor() method.
	 * @return Returns an ARGB color value in integer form.
	 */
	public int getBackgroundColor() {
		return CoronaWebView.getBackgroundColorFrom(this);
	}
	
	/**
	 * Gets the background color that was assigned to the given web view via the setBackgroundColor() method.
	 * @return Returns an ARGB color value integer that is compatible with the Android "Color" class.
	 */
	public static int getBackgroundColorFrom(android.webkit.WebView view) {
		// Validate.
		if (view == null) {
			throw new NullPointerException();
		}
		
		// Fetch the web view's background color from its private member variable via reflection.
		int color = android.graphics.Color.WHITE;
		try {
			java.lang.reflect.Field backgroundColorField = android.webkit.WebView.class.getDeclaredField("mBackgroundColor");
			backgroundColorField.setAccessible(true);
			color = backgroundColorField.getInt(view);
		}
		catch (Exception ex) { }
		return color;
	}
	
	/**
	 * Displays the last loaded page in the browser.
	 */
    @Override
	public void goBack() {
		// Check if the browser has no more history to go back to.
		if (canGoBack() == false) {
			if (fAutoCloseEnabled) {
				// Automatically close this view since there is no more web history.
				ViewManager viewManager = fCoronaRuntime.getViewManager();
				if (viewManager != null) {
					viewManager.destroyDisplayObject(getId());
				}
			}
			else {
				// Do not close this view.
				// Instead, notify the owner that there is no more history by setting the URL to an empty string.
				if (fCoronaRuntime != null && fCoronaRuntime.isRunning()) {
					fCoronaRuntime.getTaskDispatcher().send(new com.ansca.corona.events.ShouldLoadUrlTask(getId(), "", UrlRequestSourceType.HISTORY));
				}
			}
			return;
		}
		
		// Go back to the previous page.
		stopLoading();
		fUrlRequestSourceType = UrlRequestSourceType.HISTORY;
		super.goBack();
	}
	
	/** Navigates forward in this view's web history. */
	@Override
	public void goForward() {
		if (canGoForward()) {
			stopLoading();
			fUrlRequestSourceType = UrlRequestSourceType.HISTORY;
			super.goForward();
		}
	}
	
	/** Reloads the current URL. */
	@Override
	public void reload() {
		stopLoading();
		fUrlRequestSourceType = UrlRequestSourceType.RELOAD;
		super.reload();
	}
	
	/**
	 * Requests this WebView to go to the given URL.
	 * @param url The URL to be loaded.
	 */
	@Override
	public void loadUrl(String url) {
		// Make sure that the given URL is set to a valid string.
		if (url == null) {
			url = "";
		}
		
		// Load the given URL.
		stopLoading();
		fIsLoading = true;
		fUrlRequestSourceType = UrlRequestSourceType.OTHER;
		super.loadUrl(url);
	}

	@Override
	public void stopLoading() {
		// If the webview isn't loading anything then calling stopLoading will cause it to freeze
		if (fIsLoading) {
			super.stopLoading();
			fIsLoading = false;
		}
	}

	/**
	 * Requests this web view to post data to the following URL.
	 * @param url The URL to post data to.
	 * @param postData The binary data to be sent.
	 */
	@Override
	public void postUrl(String url, byte[] postData) {
		stopLoading();
		fUrlRequestSourceType = UrlRequestSourceType.FORM;
		super.postUrl(url, postData);
	}
	
	/**
	 * Enables or disables back key support for this web view.
	 * @param isSupported Set true to allow the back key go back to the previous web page in this web view.
	 *                    Set false to have this web view ignore the back key.
	 */
	public void setBackKeySupported(boolean isSupported) {
		fBackKeySupported = isSupported;
	}
	
	/**
	 * Determines if this web view supports the back key.
	 * @return Returns true if the back key is supported. Returns false if ignored.
	 */
	public boolean isBackKeySupported() {
		return fBackKeySupported;
	}
	
	/**
	 * Sets up this web view to auto-close itself.
	 * @param enabled Set true to auto-close this view when the user taps back when there is no more web history.
	 *                Set false to never auto-close this view.
	 */
	public void setAutoCloseEnabled( boolean enabled ) {
		fAutoCloseEnabled = enabled;
	}
	
	/**
	 * Determines if this view is set up to auto-close itself.
	 * @return Returns true if auto-close is enabled. Returns false if not.
	 */
	public boolean isAutoCloseEnabled() {
		return fAutoCloseEnabled;
	}
	
	/**
	 * Called when a key press has been detected.
	 * Overriden to close this view when the back key has been pressed when there is no more web pages to go back to.
	 * @param keyCode The unique integer ID of the key that was pressed down.
	 * @param event Provides information about the key press.
	 * @return Returns true if the key was handled by this view.
	 *         Returns false if not and allows the next view or activity to handle it.
	 */
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// Check if the back button was pressed.
		if (keyCode == KeyEvent.KEYCODE_BACK) {

			// Ignore the back key if not supported.
			if (fBackKeySupported == false) {
				return false;
			}
			
			// Request the previous page.
			goBack();
			
			// Override the back key only if there is a page to go back to or if we're auto-closing the view.
			boolean shouldOverrideKey = canGoBack() ? true : fAutoCloseEnabled;
			return shouldOverrideKey;
		}
		
		// Let the web view handle the key event.
		return super.onKeyDown(keyCode, event);
	}
	
	/** Handles web redirect and error events. */
    private class CoronaWebViewClient extends WebViewClient {
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
		public boolean shouldOverrideUrlLoading(WebView view, String url) {
			fUrlRequestSourceType = UrlRequestSourceType.LINK;
			return false;
		}
		
		/**
		 * Called when a form POST/GET has been submitted more than once on the same web page.
		 * @param view Reference to the WebView that the resubmission has occurred on.
		 * @param dontResend The message to send if the browser should not resend.
		 * @param resend The message to send if the browser should resend.
		 */
		@Override
		public void onFormResubmission(WebView view, android.os.Message dontResend, android.os.Message resend) {
			// Re-send form data to the server. We have to do this because Android will not do this by default
			// and will instead display the last cached web page.
			fUrlRequestSourceType = UrlRequestSourceType.RESUBMITTED;
			if (resend != null) {
				resend.sendToTarget();
			}
		}
		
		/**
		 * Called when a web page is about to be loaded.
		 * Raises an event allowing a listener a chance to cancel the web page from being loaded.
		 * <p>
		 * Note: The shouldOverrideUrlLoading() method override does not get invoked by JavaScript redirects.
		 *       So, overriding this method is the only way to handle all page loads, including bad URLs.
		 * @param view Reference to the WebView that the page is loading in.
		 * @param url The URL that is being loaded.
		 * @param favicon The web page's favorite icon.
		 */
		@Override
		public void onPageStarted(WebView view, String url, android.graphics.Bitmap favicon) {
			super.onPageStarted(view, url, favicon);
			
			// Reset the URL request source type. This must be done before raising the URL request event
			// below since an event listener could call loadUrl() and change the source type.
			int sourceType = fUrlRequestSourceType;
			fUrlRequestSourceType = UrlRequestSourceType.OTHER;
			
			// Raise an event to notify the rest of the system about this URL request.
			// Note: Do not raise an event if the web view is loading an error page for this URL.
			//       This is because the failed URL's request event has already been raised and should not be repeated.
			if (fReceivedErrorEvents.containsKey(url) == false) {
				if (fCoronaRuntime != null && fCoronaRuntime.isRunning()) {
					fCoronaRuntime.getTaskDispatcher().send(new com.ansca.corona.events.ShouldLoadUrlTask(getId(), url, sourceType));
				}
			}
		}
		
		/**
		 * Called when the web page has finished loading.
		 * Raises an event to inform the native side of Corona.
		 * @param view Reference to the WebView that has finished loading the page.
		 * @param url The URL that the web page has been loaded from.
		 */
		@Override
		public void onPageFinished(WebView view, String url) {
			fIsLoading = false;

			super.onPageFinished(view, url);

			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT) {
				view.evaluateJavascript(NATIVE_BRIDGE_CODE, null);
			}

			if (fCoronaRuntime == null || !fCoronaRuntime.isRunning()) {
				return;
			}

			//Fixes loading issue when alpha is used on long site 
			view.setLayerType(View.LAYER_TYPE_HARDWARE, null);

			// Check if an error occurred for the given URL.
			com.ansca.corona.events.DidFailLoadUrlTask errorEvent = fReceivedErrorEvents.get(url);
			
			// Dump all other errors in the collection in case some iframes failed to load within the main page.
			// We should not raise error events for iframes since we never raise URL request events for them.
			fReceivedErrorEvents.clear();
			
			// First, post an event indicating that this web view's back/forward history has been updated.
			fCoronaRuntime.getTaskDispatcher().send(new com.ansca.corona.events.WebHistoryUpdatedTask(getId(), canGoBack(), canGoForward()));
			
			// Post an event indicating that the page has either finished loading or an error occurred.
			// We raise the error event here because we must wait for the error page to finish loading.
			if (errorEvent != null) {
				fCoronaRuntime.getTaskDispatcher().send(errorEvent);
			}
			else {
				fCoronaRuntime.getTaskDispatcher().send(new com.ansca.corona.events.FinishedLoadUrlTask(getId(), url));
			}
		}
		
		/**
		 * Called when the WebView has failed to load the given URL.
		 * Raises an event to inform the native side of Corona to handle this error.
		 * @param view Reference to the WebView that has failed to load the web page.
		 * @param errorCode Unique integer ID indicating the type of error that occurred.
		 * @param description Message indicating the error that has occurred.
		 */
		@Override
		public void onReceivedError(WebView view, int errorCode, String description, String failingUrl) {
			super.onReceivedError(view, errorCode, description, failingUrl);
			
			// Reset the URL request source type. This must be done before raising the event
			// below since an event listener could call loadUrl() and change the source type.
			fUrlRequestSourceType = UrlRequestSourceType.OTHER;
			
			// Store the received error information to be sent via an error event after Android finishes
			// loading its default error page in the web view.
			com.ansca.corona.events.DidFailLoadUrlTask errorEvent;
			errorEvent = new com.ansca.corona.events.DidFailLoadUrlTask(getId(), failingUrl, description, errorCode);
			fReceivedErrorEvents.put(failingUrl, errorEvent);
		}
	}

	/**
	 * Provides access to API Level 21 (Android 5.0 Lollipop) features.
	 * Should only be accessed if running on an operating system matching this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * Instead, you are expected to call its static methods instead.
	 */
	private static class ApiLevel21 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel21() {}

		private static class FileUploadActivityResultHandler implements CoronaActivity.OnActivityResultHandler {
			
			/** Callback invoked after file upload dialog closes. */
			private android.webkit.ValueCallback<android.net.Uri[]> fFilePathCallback;

			public FileUploadActivityResultHandler(android.webkit.ValueCallback<android.net.Uri[]> filePathCallback) {
				fFilePathCallback = filePathCallback;
			}

			@Override
			public void onHandleActivityResult(CoronaActivity activity, int requestCode, int resultCode, android.content.Intent data) {
				// Based on: https://github.com/GoogleChrome/chromium-webview-samples/blob/master/input-file-example/app/src/main/java/inputfilesample/android/chrome/google/com/inputfilesample/MainFragment.java
				activity.unregisterActivityResultHandler(this);
				android.net.Uri[] results = null;

				// Check that the response is a good one
				if (resultCode == android.app.Activity.RESULT_OK && data != null) {
					try {
						results = new android.net.Uri[]{android.net.Uri.parse(data.getDataString())};
					} catch (Exception e) {}
				}

				// Send our results over
				if (fFilePathCallback != null) {
					fFilePathCallback.onReceiveValue(results);
					fFilePathCallback = null;
				}
			}
		}

		public static void openFileUpload(android.webkit.ValueCallback<android.net.Uri[]> filePathCallback) {
			// Register ActivityResultHandler for file upload.
			CoronaActivity activity = CoronaEnvironment.getCoronaActivity();
			if (activity != null) {
				int requestCode = activity.registerActivityResultHandler(new FileUploadActivityResultHandler(filePathCallback));

				// Display the selection window for file upload.
				android.content.Intent intent = new android.content.Intent(android.content.Intent.ACTION_GET_CONTENT);
				intent.setType("*/*");
				intent = android.content.Intent.createChooser(intent, "");
				activity.startActivityForResult(intent, requestCode);
			}
		}

		/**
		 * Enables or disables 3rd party cookie support for the given WebView.
		 * This is disabled by default on API Level 21 and higher operating systems.
		 * @param webView Reference to the WebView to change this setting for.
		 * @param accept Set true to support/accept 3rd party cookies. Set false to deny them.
		 */
		public static void setAcceptThirdPartyCookies(android.webkit.WebView webView, boolean accept) {
			android.webkit.CookieManager cookieManager = android.webkit.CookieManager.getInstance();
			if ((cookieManager != null) && (webView != null)) {
				cookieManager.setAcceptThirdPartyCookies(webView, accept);
			}
		}

		/**
		 * Enables support for HTTP iframes within an HTTPS for the given WebView's settings object.
		 * @param settings The settings object to be updated for mixed content mode.
		 */
		public static void setMixedContentModeToAlwaysAllowFor(android.webkit.WebSettings settings) {
			if (settings != null) {
				settings.setMixedContentMode(android.webkit.WebSettings.MIXED_CONTENT_ALWAYS_ALLOW);
			}
		}
	}

	@Override
	public List<Object> getNativePropertyResponder() {
		List<Object> out = new LinkedList<Object>();
		out.add(this);
		out.add(getSettings());
		return out;
	}

	@Override
	public Runnable getCustomPropertyAction(String key, boolean booleanValue, String stringValue, int integerValue, double doubleValue) {
		if(key.equalsIgnoreCase("http.agent") && stringValue.equalsIgnoreCase("system")) {
			return new Runnable() {
				@Override
				public void run() {
					getSettings().setUserAgentString(System.getProperty("http.agent"));
				}
			};
		}
		return null;
	}
}
