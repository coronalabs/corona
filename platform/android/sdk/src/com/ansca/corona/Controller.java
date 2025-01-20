//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.util.Iterator;
import java.util.HashMap;
import java.util.Locale;
import java.util.Objects;

import android.app.AlertDialog;
import android.app.Activity;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.MailTo;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.provider.MediaStore;
import android.provider.MediaStore.Images;
import android.provider.Settings.Secure;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.webkit.MimeTypeMap;
import android.view.WindowManager;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;

import com.ansca.corona.events.EventManager;
import com.ansca.corona.listeners.CoronaShowApiListener;
import com.ansca.corona.listeners.CoronaSplashScreenApiListener;
import com.ansca.corona.listeners.CoronaStatusBarApiListener;
import com.ansca.corona.listeners.CoronaStoreApiListener;
import com.ansca.corona.listeners.CoronaSystemApiListener;

import com.ansca.corona.permissions.PermissionsSettings;
import com.ansca.corona.permissions.PermissionsServices;
import com.ansca.corona.permissions.PermissionState;
import com.ansca.corona.permissions.RequestPermissionsResultData;

import com.ansca.corona.storage.ResourceServices;

import com.ansca.corona.version.AndroidVersionSpecificFactory;
import com.ansca.corona.version.IAndroidVersionSpecific;

/**
 * Manages ongoing issues
 * 
 * @author eherrman
 */
public class Controller {

	private boolean					myInitialResume;
	private boolean					myHasRenderedFirstFrame;
	private boolean					myIsNaturalOrientationPortrait = true;

	private Handler					myTimerHandler;
	private Runnable				myTimerTask;
	private int						myTimerMilliseconds;
	
	private Context                 myContext;
	private CoronaRuntime           myRuntime;
	private NativeToJavaBridge      myBridge;
	private AlertDialog             myAlertDialog;
	private ActivityIndicatorDialog myActivityIndicatorDialog = null;

	private boolean 				myIdleEnabled;

	private MediaManager			myMediaManager;
	private CoronaSensorManager		mySensorManager;

	private IAndroidVersionSpecific	myAndroidVersion;
	private float myDefaultFontSize;
	private int myDefaultTextFieldPaddingInPixels;

	private RuntimeState myRuntimeState;

	// These are the listeners for actions that should be handled by the activity and not the view
	private CoronaApiListener myCoronaApiListener;
	// All the show* functions eg. showSmsWindow or showEmailWindow
	private CoronaShowApiListener myCoronaShowApiListener;
	// WHen to hide/show the splash screen
	private CoronaSplashScreenApiListener myCoronaSplashScreenApiListener;
	// Current status bar status, height, etc.
	private CoronaStatusBarApiListener myCoronaStatusBarApiListener;
	// store.init() store.purchase() etc.
	private CoronaStoreApiListener myCoronaStoreApiListener;
	// native.requestExit(), pushing in the launch intents
	private CoronaSystemApiListener myCoronaSystemApiListener;

	private SystemMonitor mySystemMonitor;

	private EventManager myEventManager;

	private com.ansca.corona.graphics.opengl.CoronaGLSurfaceView myGLView;

	private Handler myHandler;

	private ResourceServices myResourceServices;

	private String myGoogleMapsAPIKey;

	enum RuntimeState {
		Starting,
		Running,
		Stopping,
		Stopped
	}

	Controller( Context context, CoronaRuntime runtime ) {
		// Keep a reference to the Corona activity.
		myContext = context;

		// Initialize sub-systems.
		myRuntime = runtime;
		myHasRenderedFirstFrame = false;
		myBridge = new NativeToJavaBridge( myContext );
        myMediaManager = new MediaManager( myRuntime, myContext );
        mySensorManager = new CoronaSensorManager( myRuntime );
        mySystemMonitor = new SystemMonitor( myRuntime, myContext );
        // This will create a handler that will be attached to the main thread
        myHandler = new Handler(Looper.getMainLooper());
        myTimerHandler = new Handler(Looper.getMainLooper());
        myEventManager = new EventManager(this);
		myResourceServices = new ResourceServices(CoronaEnvironment.getApplicationContext());
        
		// Determine the natural orientation of the device.
		// This is typically portrait for phones and landscape for tablets.
		String windowServiceName = android.content.Context.WINDOW_SERVICE;
		android.view.Display display = ((android.view.WindowManager)myContext.getSystemService(windowServiceName)).getDefaultDisplay();
		switch (display.getRotation()) {
			case android.view.Surface.ROTATION_0:
			case android.view.Surface.ROTATION_180:
				myIsNaturalOrientationPortrait = (display.getWidth() < display.getHeight());
				break;
			case android.view.Surface.ROTATION_90:
			case android.view.Surface.ROTATION_270:
				myIsNaturalOrientationPortrait = (display.getWidth() > display.getHeight());
				break;
			default:
				// Device has an unknown rotation value. Assume portrait.
				myIsNaturalOrientationPortrait = true;
				break;
		}

		// Fetch a text field's default font size and padding in pixels.
		CoronaEditText editText = new CoronaEditText(context, runtime);
		myDefaultFontSize = editText.getTextSize();
		int verticalPadding = editText.getPaddingTop() + editText.getPaddingBottom();
		verticalPadding -= editText.getBorderPaddingTop() + editText.getBorderPaddingBottom();
		myDefaultTextFieldPaddingInPixels = (int)(((double)verticalPadding / 2.0) + 0.5);
	}
	
	synchronized void init() {
		myMediaManager.init();
		mySensorManager.init();
		mySystemMonitor.start();
		myTimerMilliseconds = 0;
		myInitialResume = true;
		myHasRenderedFirstFrame = false;
		myRuntimeState = RuntimeState.Stopped;
		myAndroidVersion = AndroidVersionSpecificFactory.create();

		// We disable the idle timer for Android 5.1.X to work-around several OS issues with OpenGL.
		myIdleEnabled = android.os.Build.VERSION.SDK_INT != 22;
	}

	void setGLView(com.ansca.corona.graphics.opengl.CoronaGLSurfaceView glView) {
		myGLView = glView;
	}
	
	void setCoronaApiListener(CoronaApiListener listener) {
		myCoronaApiListener = listener;
		mySystemMonitor.setCoronaApiListener(listener);
	}

	CoronaApiListener getCoronaApiListener() {
		return myCoronaApiListener;
	}

	void setCoronaShowApiListener(CoronaShowApiListener listener) {
		myCoronaShowApiListener = listener;
	}

	CoronaShowApiListener getCoronaShowApiListener() {
		return myCoronaShowApiListener;
	}

	void setCoronaSplashScreenApiListener(CoronaSplashScreenApiListener listener) {
		myCoronaSplashScreenApiListener = listener;
	}

	CoronaSplashScreenApiListener getCoronaSplashScreenApiListener() {
		return myCoronaSplashScreenApiListener;
	}

	void setCoronaStatusBarApiListener(CoronaStatusBarApiListener listener) {
		myCoronaStatusBarApiListener = listener;
	}

	CoronaStatusBarApiListener getCoronaStatusBarApiListener() {
		return myCoronaStatusBarApiListener;
	}

	void setCoronaStoreApiListener(CoronaStoreApiListener listener) {
		myCoronaStoreApiListener = listener;
	}

	CoronaStoreApiListener getCoronaStoreApiListener() {
		return myCoronaStoreApiListener;
	}

	void setCoronaSystemApiListener(CoronaSystemApiListener listener) {
		myCoronaSystemApiListener = listener;
	}

	CoronaSystemApiListener getCoronaSystemApiListener() {
		return myCoronaSystemApiListener;
	}

	public void SetGoogleMapsAPIKey(String key) {
		myGoogleMapsAPIKey = key;
	}

	public String GetGoogleMapsAPIKey() {
		return myGoogleMapsAPIKey;
	}

	public IAndroidVersionSpecific getAndroidVersionSpecific() {
		return myAndroidVersion;
	}

	public boolean isMultitouchEnabled() {
		return getAndroidVersionSpecific().apiVersion() >= 5 
		  && mySensorManager.isActive(JavaToNativeShim.EventTypeMultitouch);
	}
	
	public synchronized void start() {
		if ( !myInitialResume ) {
			if(myRuntimeState == RuntimeState.Stopping || myRuntimeState == RuntimeState.Running) {
				myRuntimeState = RuntimeState.Running;
			} else {
				myRuntimeState = RuntimeState.Starting;	
			}
		}
		else {
			myInitialResume = false;
			myRuntimeState = RuntimeState.Running;
		}
		// Create the OpenGL surface and initialize the Corona renderer by "resuming" the OpenGL view.
		myGLView.onResume();
		requestEventRender();
		myMediaManager.resumeAll();
		mySensorManager.resume();
		startTimer();
		internalSetIdleTimer(myIdleEnabled);
	}
	
	public synchronized void stop() {

		stopTimer();
		mySensorManager.pause();
		if (myRuntimeState == RuntimeState.Starting || myRuntimeState == RuntimeState.Stopped) {
			myRuntimeState = RuntimeState.Stopped;
		} else {
			myRuntimeState = RuntimeState.Stopping;
		}

		myGLView.queueEvent(new Runnable() {
			@Override
			public void run() {
				if (myRuntimeState != RuntimeState.Stopping) {
					// pause event already picked up by updateRuntimeState
					return;
				}

				// If we don't do this then there won't be one last onDrawFrame call which means the runtime won't be stopped!
				// it's ok to call ApplicationListener's events
				// from onDrawFrame because it's executing in GL thread
				requestEventRender();
			}
		});
		myMediaManager.pauseAll();
		internalSetIdleTimer(true);

		while (myRuntimeState == RuntimeState.Stopping) {
			try {
				// Android ANR time is 5 seconds, so wait up to 4 seconds before assuming
				// deadlock and killing process.
				this.wait(4000);
				if (myRuntimeState == RuntimeState.Stopping) {
					// pause will never go false if onDrawFrame is never called by the GLThread
					// when entering this method, we MUST enforce continuous rendering
					android.os.Process.killProcess(android.os.Process.myPid());
				}
			} catch (InterruptedException ignored) {
				// Nothing to do here, intent can't be handled.
			}
		}
	}
	
	public synchronized void destroy() {
		cancelNativeAlert(-1);
		closeNativeActivityIndicator();
		stopTimer();
		mySensorManager.stop();
		myMediaManager.release();
		mySystemMonitor.stop();
		myEventManager.removeAllEvents();
		JavaToNativeShim.destroy(myRuntime);
	}
	
	/*
	 * Warning: This method should always be called on the OpenGL thread
	 */
	public static void updateRuntimeState(CoronaRuntime runtime, boolean render) {
		final Controller controller = runtime.getController();
		EventManager eventManager = controller.getEventManager();
		if ((controller != null) && (eventManager != null)) {

			boolean localShouldStart = false;
			boolean localShouldRender = false;
			boolean localShouldStop = false;

			synchronized (controller) {
				if (RuntimeState.Starting == controller.myRuntimeState && render) {
					localShouldStart = true;
					localShouldRender = true;
					controller.myRuntimeState = RuntimeState.Running;
				}  else if (RuntimeState.Running == controller.myRuntimeState) {
					localShouldRender = render;
				} else if (RuntimeState.Stopping == controller.myRuntimeState) {
					localShouldStop = true;
					controller.myRuntimeState = RuntimeState.Stopped;
					controller.notifyAll();
				} else if (RuntimeState.Stopped == controller.myRuntimeState) {
					// all local boolean are false
				}
			}

			if (localShouldStart) {
				Log.i("Corona", "JavaToNativeShim.resume(runtime)");
				JavaToNativeShim.resume(runtime);
				synchronized (controller) {
					Log.i("Corona", "controller.requestRender()");
					controller.requestRender();
				}
			}

			if (render) {
				// Send queued events to the native side of Corona.
				eventManager.sendEvents();
			}

			if (localShouldStop) {
				Log.i("Corona", "JavaToNativeShim.pause(runtime)");
				JavaToNativeShim.pause(runtime);

				synchronized (controller) {
					// Now we can pause the GLView on the UI thread if needed since we've
					// guaranteed that the Corona Runtime has stopped.
					Handler handler = controller.getHandler();
					handler.post (new Runnable() {
						@Override
						public void run() {
							// Ensure that we still need to pause before doing so.
							// This is to cover the case of onPause() and onResume()
							// happening back to back, which is possible on Android 2.3.x devices.
							if (RuntimeState.Stopped == controller.myRuntimeState) {
								// Pause the OpenGL view's thread. This stops its rendering loop.
								Log.i("Corona", "controller.getGLView().onPause()");
								controller.getGLView().onPause();
							}
						}
					});
				}
			}

			// Render the next frame, but only if the Corona runtime is currently running.
			// We must check the running state "after" sending the above events because a suspend/resume event
			// will change the running state after the event has been dispatched.
			if (localShouldRender) {
				// If this is the 1st rendered frame, then remove the splash screen if shown.
				if (controller.myHasRenderedFirstFrame == false) {
					controller.myHasRenderedFirstFrame = true;
					CoronaSplashScreenApiListener listener = controller.getCoronaSplashScreenApiListener();
					if (listener != null) {
						listener.hideSplashScreen();
					}
				}
				// Render the frame.
				JavaToNativeShim.render(runtime);
			}
		}
	}

	public boolean isRunning() {
		return (RuntimeState.Running == myRuntimeState) || (RuntimeState.Stopping == myRuntimeState);
	}

	public EventManager getEventManager() {
		return myEventManager;
	}

	public NativeToJavaBridge getBridge() {
		return myBridge;
	}

	public MediaManager getMediaManager() {
		return myMediaManager;
	}

	public SystemMonitor getSystemMonitor() {
		return mySystemMonitor;
	}

	public Handler getHandler() {
		return myHandler;
	}

	public com.ansca.corona.graphics.opengl.CoronaGLSurfaceView getGLView() {
		return myGLView;
	}

	public Context getContext() {
		return myContext;
	}
    
    public void requestRender() {
    	if (myContext != null) {
    		mySystemMonitor.update();
			if (myGLView != null) {
				myGLView.requestRender();
			}
    	}
    }

    public void requestEventRender() {
    	if (myTimerTask == null) {
    		mySystemMonitor.update();
			if (myGLView != null) {
				myGLView.requestRender();
			}
		}
    }

	/**
	 * Determines if the natural orientation of the device is portrait or landscape.
	 * This orientation is typically portrait for phones and landscape for tablets.
	 * This indicates the orientation of how sensors are mounted in the device, meaning that accelerometer, gyroscope,
	 * and general orientation sensor data will be relative to the device's natural orientation.
	 * @return Returns true if the device's natural orientation is portrait. Returns false if landscape.
	 */
	public boolean isNaturalOrientationPortrait() {
		return myIsNaturalOrientationPortrait;
	}
	
    public void setTimer( int milliseconds ) {
    	myTimerMilliseconds = milliseconds;
    	startTimer();
	}
    
    public void startTimer() {
    	if ( myTimerMilliseconds == 0 ) {
    		return;
    	}

    	if ( myTimerTask == null ) {
    		myTimerTask = new Runnable() {
				public void run() {
					if ( myTimerMilliseconds != 0 ) {
						myTimerHandler.postDelayed(this, myTimerMilliseconds);
						requestRender();
					}
				}
    		};

        	myTimerHandler.postDelayed( myTimerTask, myTimerMilliseconds);
    	}
	}

    public void cancelTimer() {
    	stopTimer();
    	myTimerMilliseconds = 0;
    }
    
    public void stopTimer() {
    	if ( myTimerTask != null ) {
    		myTimerHandler.removeCallbacks(myTimerTask);
    		myTimerTask = null;
    	}
    }
    
    public void displayUpdate() {
		if (myGLView != null) {
			myGLView.setNeedsSwap();
		}
    }

	/**
	 * Creates a Lua system.openURL() intent equivalent for the given URL string.
	 * @param url The url to be launched such as "http:", "mailto:", "tel:", or a file path.
	 * @return Returns an intent used to open the given URL. This intent is intended to be passed into
	 *         the startActivity() or queryIntentActivities() method.
	 *         <p>
	 *         Returns null if given an invalid URL.
	 */
	private android.content.Intent createOpenUrlIntentFor(String url) {
		// Validate.
		if ((url == null) || (url.length() <= 0)) {
			return null;
		}

		// Fetch the activity context.
		android.content.Context context = myContext;
		if (context == null) {
			return null;
		}

		// Convert the URL string to a URI object.
		android.net.Uri uri = android.net.Uri.parse(url);
		if (uri == null) {
			return null;
		}
		
		// If the given URL references a sandboxed file belonging to this app, then convert the URL to
		// a "content://" URL which allows external apps to access the file via our FileContentProvider class.
		if ((uri.getScheme() == null) || (uri.getScheme().toLowerCase().equals("file"))) {
			String filePath = uri.getSchemeSpecificPart();
			if (filePath != null) {
				boolean isSandboxedFile = false;
				if (filePath.indexOf(context.getApplicationInfo().dataDir) >= 0) {
					// The URL references a file under this app's "/data" folder.
					// Strip off the 2 leading slashes "//" from the path and verify that it exists.
					filePath = uri.getPath();
					java.io.File file = new java.io.File(filePath);
					isSandboxedFile = file.exists();
				}
				else {
					// The URL likely references a file inside of the APK or expansion file.
					// Attempt to fetch a relative path to the file, if in fact it belongs to this app.
					final String androidAssetsPrefix = "file:///android_asset/";
					if (url.toLowerCase().startsWith(androidAssetsPrefix)) {
						// This is a special URL defined by Google to access this app's assets.
						// Note: This should be a rare case.
						if (androidAssetsPrefix.length() < url.length()) {
							filePath = url.substring(androidAssetsPrefix.length());
						}
						else {
							filePath = null;
						}
					}
					else {
						// Strip off any leading slashes from the path, turning it into a relative path.
						int index;
						for (index = 0; index < filePath.length(); index++) {
							if (filePath.charAt(index) != '/') {
								break;
							}
						}
						if (index >= filePath.length()) {
							filePath = null;
						}
						else if (index > 0) {
							filePath = filePath.substring(index);
						}
					}

					// Verify that the file is an asset belonging to this app.
					if (filePath != null) {
						com.ansca.corona.storage.FileServices fileServices =
								new com.ansca.corona.storage.FileServices(context);
						if (fileServices.doesAssetFileExist(filePath)) {
							isSandboxedFile = true;
						}
					}
				}

				// Generate a new URI object if the URL references a sandboxed file.
				if (isSandboxedFile && (filePath != null)) {
					uri = com.ansca.corona.storage.FileContentProvider.createContentUriForFile(context, filePath);
					if (uri == null) {
						return null;
					}
				}
			}
		}

		// Create an intent used to launch the given URL.
		android.content.Intent intent = null;
		if (uri.getScheme() == null) {
			// The URL does not have a scheme, making it invalid.
			return null;
		}
		else if (uri.getScheme().toLowerCase().equals("content")) {
			// A "content://" URL is expected to reference a file.
			// Determine the file's MIME type via its extension.
			String mimeTypeName = null;
			String filePath = uri.toString();
			int index = filePath.lastIndexOf('.');
			if ((index >= 0) && ((index + 1) < filePath.length())) {
				String fileExtension = filePath.substring(index + 1);
				MimeTypeMap mapping = MimeTypeMap.getSingleton();
				if (mapping.hasExtension(fileExtension)) {
					mimeTypeName = mapping.getMimeTypeFromExtension(fileExtension);
				}
				if (mimeTypeName == null) {
					mimeTypeName = "application/" + fileExtension;
				}
			}
			
			// Set up an intent to open the file via an external application that supports its file type.
			intent = new android.content.Intent(android.content.Intent.ACTION_VIEW, uri);
			if (mimeTypeName != null) {
				intent.setDataAndType(uri, mimeTypeName);
			}
		}
		else if (MailTo.isMailTo(url)) {
			// The URL is a "mailto". Fetch mail settings from URL.
			MailSettings mailSettings = MailSettings.fromUrl(url);
			
			// Set up an intent to launch an e-mail window.
			intent = android.content.Intent.createChooser(mailSettings.toIntent(), "Send mail...");
		}
		else {
			// The URL likely references a web address, telephone number, or custom URL scheme.
			String action = android.content.Intent.ACTION_VIEW;
			if (uri.getScheme().toLowerCase().equals("tel")) {
				action = android.content.Intent.ACTION_CALL;
			}
			intent = new android.content.Intent(action, uri);
		}
		return intent;
	}

	/**
	 * Determines if an activity can open the given URL.
	 * @param url The url to be launched such as "http:", "mailto:", "tel:", or a file path.
	 * @return Return true if the URL can be opened by an activity.
	 *         <p>
	 *         Returns false if no installed app/activity can open the URL of if the given URL is invalid.
	 */
	public boolean canOpenUrl(String url) {
		// Fetch the activity context.
		android.content.Context context = myContext;
		if (context == null) {
			return false;
		}

		// Attempt to create an intent for the given URL.
		android.content.Intent intent = createOpenUrlIntentFor(url);
		if (intent == null) {
			return false;
		}

		// If the above generated intent still contains the given URL/URI, then validate it.
		android.net.Uri uri = intent.getData();
		if (uri != null) {
			// The intent's URL must have a scheme such as "http:", "content:", etc.
			if (uri.getScheme() == null) {
				return false;
			}

			// Validate based on the URL's scheme.
			String lowerCaseUriScheme = uri.getScheme().toLowerCase();
			if (lowerCaseUriScheme.equals("content")) {
				// The URL references a sandboxed file belonging to this app or another app.
				// Verify that the file exists via a content provider.
				android.content.ContentResolver contentResolver = context.getContentResolver();
				if (contentResolver != null) {
					boolean wasFound = false;
					try {
						android.content.res.AssetFileDescriptor fileDescriptor;
						fileDescriptor = contentResolver.openAssetFileDescriptor(uri, "r");
						if (fileDescriptor != null) {
							wasFound = true;
							fileDescriptor.close();
						}
					}
					catch (Exception ex) {}
					if (wasFound == false) {
						try {
							android.os.ParcelFileDescriptor fileDescriptor;
							fileDescriptor = contentResolver.openFileDescriptor(uri, "r");
							if (fileDescriptor != null) {
								wasFound = true;
								fileDescriptor.close();
							}
						}
						catch (Exception ex) {}
					}
					if (wasFound == false) {
						return false;
					}
				}
			}
			else if (lowerCaseUriScheme.equals("file")) {
				// The URL references a non-sandboxed file. Verify that the file exists.
				java.io.File file = new java.io.File(uri.getPath());
				if (file.exists() == false) {
					return false;
				}
			}
		}

		// If the intent is for making a phone call, then make sure the call permission is defined in the manifest.
		// Note: It's okay to return true if the permission is defined, but not granted yet.
		//       This is because permission can be granted by the user later.
		if (android.content.Intent.ACTION_CALL.equals(intent.getAction())) {
			com.ansca.corona.permissions.PermissionsServices permissionsServices;
			permissionsServices = new com.ansca.corona.permissions.PermissionsServices(context);
			String permissionName = com.ansca.corona.permissions.PermissionsServices.Permission.CALL_PHONE;
			if (permissionsServices.isPermissionInManifest(permissionName) == false) {
				return false;
			}
		}

		// Finally, determine if the Android OS can launch the intent or not.
		return canShowActivityFor(intent);
	}

	/**
	 * Attempts to launch an activity window that supports the given URL.
	 * @param url The url to be launched such as "http:", "mailto:", "tel:", or a file path.
	 * @return Returns true if this method was able to launch an activity that supports the given URL.
	 *         <p>
	 *         Returns false if unable to execute the given URL.
	 */
	public boolean openUrl(String url) {
		// Validate.
		if ((url == null) || (url.length() <= 0)) {
			return false;
		}

		// Fetch the activity context.
		android.content.Context context = myContext;
		if (context == null) {
			return false;
		}

		// Create an intent used to launch the given URL.
		android.content.Intent intent = createOpenUrlIntentFor(url);
		if (intent == null) {
			return false;
		}

		// If the intent is for making a phone call, then check if we have permission to do so.
		if (android.content.Intent.ACTION_CALL.equals(intent.getAction())) {
			com.ansca.corona.permissions.PermissionsServices permissionsServices;
			permissionsServices = new com.ansca.corona.permissions.PermissionsServices(context);
			switch(permissionsServices.getPermissionStateFor(
				com.ansca.corona.permissions.PermissionsServices.Permission.CALL_PHONE))
			{
				case MISSING:
					this.showPermissionMissingFromManifestAlert(
							com.ansca.corona.permissions.PermissionsServices.Permission.CALL_PHONE,
							"system.openURL() needs access to telephone features when given a telephone number!");
					return false;
				case DENIED:
					// Request the missing permission and exit out of this method.
					// The handler below will call this openUrl() method again once permission has been granted.
					com.ansca.corona.permissions.PermissionsSettings settings;
					settings = new com.ansca.corona.permissions.PermissionsSettings(
							com.ansca.corona.permissions.PermissionsServices.Permission.CALL_PHONE);
					permissionsServices.requestPermissions(settings, new OpenUrlRequestPermissionsResultHandler(url));
					return true;
				default:
					// Permission is granted! Carry on!
					break;
			}
		}

		// Attempt to execute the given URL.
		boolean hasSucceeded = false;
		try {
			context.startActivity(intent);
			hasSucceeded = true;
		}
		catch (Exception ignore) {
			// Nothing to do here, intent can't be handled.
		}
		return hasSucceeded;
	}

	private static class OpenUrlRequestPermissionsResultHandler 
		implements CoronaActivity.OnRequestPermissionsResultHandler {

		// The URL we want to open.
		private String fUrl;

		public OpenUrlRequestPermissionsResultHandler(String url) {
			fUrl = url;
		}

		@Override
		public void onHandleRequestPermissionsResult(
				CoronaActivity activity, int requestCode, String[] permissions, int[] grantResults) {
			com.ansca.corona.permissions.PermissionsSettings permissionsSettings = activity.unregisterRequestPermissionsResultHandler(this);

			if (permissionsSettings != null) {
				permissionsSettings.markAsServiced();

				com.ansca.corona.permissions.PermissionsServices permissionsServices 
					= new com.ansca.corona.permissions.PermissionsServices(activity);
				if (permissionsServices.getPermissionStateFor(
						com.ansca.corona.permissions.PermissionsServices.Permission.CALL_PHONE) 
						== com.ansca.corona.permissions.PermissionState.GRANTED) {
					
					// Now open the URL if the user hasn't killed off the Corona app between making their original request and now.
					CoronaRuntime runtime = activity.getRuntime();
					if (runtime != null) {
						Controller controller = runtime.getController();
						if (controller != null) {
							controller.openUrl(fUrl);
						}
					}
				}
				// Otherwise, do nothing like iOS.
			} else {
				// Don't have any permissions settings corresponding with this request! SAY SOMETHING!
				Log.i("Corona", "Controller.OpenUrlRequestPermissionsResultHandler.onHandleRequestPermissionsResult(): " +
					"Can't open this URL as there's no PermissionsSettings corresponding to the permission request related to opening this URL!");
			}
		}
	}

	// TODO: Remove this if it's not being used?
	public boolean saveBitmap(Bitmap bitmap, Uri uri) {
		boolean result = false;
		
		// Validate.
		if ((myContext == null) || (bitmap == null) || (uri == null)) {
			return false;
		}
		
		// Determine the format to save as by the MIME type.
		Bitmap.CompressFormat format;
		int quality;
		String mimeTypeName = myContext.getContentResolver().getType(uri);
		if ((mimeTypeName != null) && mimeTypeName.toLowerCase().endsWith("png")) {
			format = Bitmap.CompressFormat.PNG;
			quality = 100;
		}
		else {
			format = Bitmap.CompressFormat.JPEG;
			quality = 90;
		}
		
		// Save the given bitmap to file.
		try {
			OutputStream outStream = myContext.getContentResolver().openOutputStream( uri );
			result = bitmap.compress( format, quality, outStream );
			outStream.flush();
			outStream.close();
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return result;
	}
	
	public boolean saveBitmap(Bitmap bitmap, int quality, String filePathName) {
		boolean result = false;
		
		// Validate.
		if ((bitmap == null) || (filePathName == null) || (filePathName.length() <= 0)) {
			return false;
		}
		
		// Determine the format to save as by the file name's extension.
		Bitmap.CompressFormat format;
		if (filePathName.toLowerCase().endsWith(".png")) {
			format = Bitmap.CompressFormat.PNG;
		}
		else {
			format = Bitmap.CompressFormat.JPEG;
		}
		
		// Save the given image to file.
		try {
			//Work around for Android 10 to save to Photo Folder
			java.io.FileOutputStream stream = null;
			if(filePathName.contains("/storage/emulated/0/Pictures/") && Build.VERSION.SDK_INT == Build.VERSION_CODES.Q){
				final String name = filePathName.replace("/storage/emulated/0/Pictures/", "");
				ContentResolver resolver = myContext.getContentResolver();
				ContentValues contentValues = new ContentValues();
				contentValues.put(MediaStore.MediaColumns.DISPLAY_NAME, name);
				if(name.toLowerCase().endsWith(".png")){
					contentValues.put(MediaStore.MediaColumns.MIME_TYPE, "image/png");
				}else {
					contentValues.put(MediaStore.MediaColumns.MIME_TYPE, "image/jpeg");
				}
				contentValues.put(Images.Media.DATE_ADDED, System.currentTimeMillis());
				contentValues.put(Images.Media.DATE_TAKEN, System.currentTimeMillis());
				contentValues.put(MediaStore.MediaColumns.RELATIVE_PATH, Environment.DIRECTORY_DCIM);
				Uri imageUri = resolver.insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, contentValues);
				stream = (FileOutputStream) resolver.openOutputStream(Objects.requireNonNull(imageUri));
			}else{
				stream = new java.io.FileOutputStream(filePathName);

			}

			result = bitmap.compress(format, quality, stream);
			stream.flush();
			stream.close();
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		
		// Returns true if the save was successful.
		return result;
	}
	
	public void addImageFileToPhotoGallery(String imageFilePathName) {
		// Validate.
		if ((imageFilePathName == null) || (imageFilePathName.length() <= 0)) {
			return;
		}
		
		// Add given file name to the photo gallery app's list.
        android.media.MediaScannerConnection.scanFile(myContext,
													  new String[] { imageFilePathName }, null,
													  new android.media.MediaScannerConnection.OnScanCompletedListener() {
            public void onScanCompleted(String path, Uri uri) { }
        });
	}
	
	private void internalSetIdleTimer( boolean enabled )
	{
		if (myCoronaApiListener == null) {
			Log.i("Corona", "Controller.internalSetIdleTimer(): Can't set internal idle timer because our ApiListener is gone!");
			return;
		}

		// We disable the idle timer for Android 5.1.X to work-around several OS issues with OpenGL.
		if (enabled && android.os.Build.VERSION.SDK_INT != 22)
		{
			myCoronaApiListener.removeKeepScreenOnFlag();
		}
		else
		{
			myCoronaApiListener.addKeepScreenOnFlag();
		}
	}

	public void setIdleTimer( boolean enabled )
	{
		internalSetIdleTimer(enabled);
		if (android.os.Build.VERSION.SDK_INT == 22) {
			// We disable the idle timer for Android 5.1.X to work-around several OS issues with OpenGL.
			myIdleEnabled = false;
		} else {
			myIdleEnabled = enabled;
		}
	}

	public boolean getIdleTimer()
	{
		return myIdleEnabled;
	}

	/**
	 * Creates an AlertDialog.Builder with the proper theme for the given device.
	 * TODO: Somehow choose light or dark themed alert based on what theme choice someone makes wither in their manifest or through Corona widgets.
	 */
	android.app.AlertDialog.Builder createAlertDialogBuilder(android.content.Context context) {

		// We create alert dialog builders differently depending on manufacturer and API level.
		// First, go through the manufacturer's with their own rules
		String deviceManufacturer = getManufacturerName();
		if (deviceManufacturer.equals("Amazon")) {
			// Follow Amazon's theming rules.
			return createAmazonAlertDialogBuilder(context);
		} else if (
			deviceManufacturer.toLowerCase().contains("huawei") ||
			deviceManufacturer.toLowerCase().contains("alps")  || // POSH Mobile
			deviceManufacturer.toLowerCase().contains("sony")
			) {
			// These guys use all light themes.
			return createLightAlertDialogBuilder(context);
		}

		// Otherwise, there's no manufacturer-specific theming rules.
		return createDefaultAlertDialogBuilder(context);
	}

	/**
	 * Creates an AlertDialog.Builder following the standard theming rules set forth by Google's APIs.
	 */
	private android.app.AlertDialog.Builder createDefaultAlertDialogBuilder(android.content.Context context) {
		int deviceAPILevel = android.os.Build.VERSION.SDK_INT;
		// Go through the standard API-specific theming rules.
		if (deviceAPILevel >= 22) {
			return ApiLevel22.createDefaultAlertDialogBuilder(context);
		} else if (deviceAPILevel >= 21) {
			return ApiLevel21.createDefaultAlertDialogBuilder(context);
		} else if (deviceAPILevel >= 14) {
			return ApiLevel14.createDefaultAlertDialogBuilder(context);
		} else if (deviceAPILevel >= 11) {
			return ApiLevel11.createDefaultAlertDialogBuilder(context);
		} else { // API Level 10
			// Apply the desired theme to the context.
			android.view.ContextThemeWrapper contextThemeWrapper = 
				new android.view.ContextThemeWrapper(context, android.R.style.Theme_Dialog);

			// Create the Alert Dialog Builder.
			return new AlertDialog.Builder(contextThemeWrapper);
		}
	}

	/**
	 * Creates an AlertDialog.Builder using all dark themes.
	 */
	android.app.AlertDialog.Builder createDarkAlertDialogBuilder(android.content.Context context) {
		int deviceAPILevel = android.os.Build.VERSION.SDK_INT;
		// Go through the standard API-specific theming rules.
		if (deviceAPILevel >= 22) {
			return ApiLevel22.createDarkAlertDialogBuilder(context);
		} else if (deviceAPILevel >= 21) {
			return ApiLevel21.createDarkAlertDialogBuilder(context);
		} else if (deviceAPILevel >= 14) {
			return ApiLevel14.createDarkAlertDialogBuilder(context);
		} else if (deviceAPILevel >= 11) {
			return ApiLevel11.createDarkAlertDialogBuilder(context);
		} else { // API Level 10
			// Apply the desired theme to the context.
			android.view.ContextThemeWrapper contextThemeWrapper = 
				new android.view.ContextThemeWrapper(context, android.R.style.Theme_Dialog);

			// Create the Alert Dialog Builder.
			return new AlertDialog.Builder(contextThemeWrapper);
		}
	}

	/**
	 * Creates an AlertDialog.Builder using all light themes.
	 */
	android.app.AlertDialog.Builder createLightAlertDialogBuilder(android.content.Context context) {
		int deviceAPILevel = android.os.Build.VERSION.SDK_INT;
		// Go through the standard API-specific theming rules.
		if (deviceAPILevel >= 22) {
			return ApiLevel22.createLightAlertDialogBuilder(context);
		} else if (deviceAPILevel >= 21) {
			return ApiLevel21.createLightAlertDialogBuilder(context);
		} else if (deviceAPILevel >= 14) {
			return ApiLevel14.createLightAlertDialogBuilder(context);
		} else if (deviceAPILevel >= 11) {
			return ApiLevel11.createLightAlertDialogBuilder(context);
		} else { // API Level 10
			// Apply the desired theme to the context.
			android.view.ContextThemeWrapper contextThemeWrapper = 
				new android.view.ContextThemeWrapper(context, android.R.style.Theme_Dialog);

			// Create the Alert Dialog Builder.
			return new AlertDialog.Builder(contextThemeWrapper);
		}
	}

	/**
	 * Creates an AlertDialog.Builder following the theming rules for Amazon Kindle Fire devices.
	 * TODO: Use Theme.Amazon where appropriate.
	 */
	private android.app.AlertDialog.Builder createAmazonAlertDialogBuilder(android.content.Context context) {
		int deviceAPILevel = android.os.Build.VERSION.SDK_INT;
		// Go through the standard API-specific theming rules.
		if (deviceAPILevel >= 22) { // Fire OS 5
			// Use the dark themes for Amazon Fire OS 5-based devices.
			return ApiLevel22.createDarkAlertDialogBuilder(context);
		} else if (deviceAPILevel >= 14) { // Android 4.0.3 fork and Fire OS 3 - 4
			return ApiLevel14.createDefaultAlertDialogBuilder(context);
		} else { // API Level 13 and lower. 
			// No known Amazon devices are based on Android 3, so this is effectively just the first gen Kindle Fires.
			// Apply the desired theme to the context.
			android.view.ContextThemeWrapper contextThemeWrapper = 
				new android.view.ContextThemeWrapper(context, android.R.style.Theme_Dialog);

			// Create the Alert Dialog Builder.
			return new AlertDialog.Builder(contextThemeWrapper);
		}
	}

	public void showNativeAlert(final String title, final String msg, final String[] buttonLabels) {
		// Do not continue if an alert dialog is already shown.
		synchronized (this) {
			if (myAlertDialog != null) {
				return;
			}
		}

		// Display the alert dialog on the main UI thread.
		myHandler.post(new Runnable() {
			@Override
			public void run() {				
				// Configure the alert dialog.
				final AlertDialog.Builder builder = createAlertDialogBuilder(myContext);
				DialogInterface.OnClickListener clickListener = new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface arg0, int which) {
						int buttonIndex = which;
						if ( which < 0 ) {
							switch( which ) {
								case DialogInterface.BUTTON_NEUTRAL:
									buttonIndex = 1;
									break;
								case DialogInterface.BUTTON_NEGATIVE:
									buttonIndex = 2;
									break;
								case DialogInterface.BUTTON_POSITIVE:
									buttonIndex = 0;
									break;
							}
						}

						if (myRuntime != null && myRuntime.isRunning()) {
							myRuntime.getTaskDispatcher().send(new com.ansca.corona.events.AlertTask(buttonIndex, false));
						}
						synchronized (Controller.this) {
							myAlertDialog = null;
						}
					}
				};
				android.content.DialogInterface.OnCancelListener cancelListener = new android.content.DialogInterface.OnCancelListener() {
					public void onCancel(android.content.DialogInterface dialog) {
						if (myRuntime != null && myRuntime.isRunning()) {
							myRuntime.getTaskDispatcher().send(new com.ansca.corona.events.AlertTask(-1, true));
						}
						synchronized (Controller.this) {
							myAlertDialog = null;
						}
					}
				};
				final int buttonCount = (buttonLabels != null) ? buttonLabels.length : 0;
				if (buttonCount <= 0) {
					builder.setTitle( title );
					builder.setMessage( msg );
				}
				else if (buttonCount <= 3) {
					builder.setTitle( title );
					builder.setMessage( msg );
					builder.setPositiveButton( buttonLabels[0], clickListener );
					if (buttonCount > 1) {
						builder.setNeutralButton( buttonLabels[1], clickListener );
					}
					if (buttonCount > 2) {
						builder.setNegativeButton( buttonLabels[2], clickListener );
					}
				}
				else {
					builder.setTitle( title + ": " + msg );
					builder.setItems( buttonLabels, clickListener );
				}
				builder.setOnCancelListener(cancelListener);

				// Display the alert dialog.
				synchronized (Controller.this) {
					myAlertDialog = builder.create();
					myAlertDialog.setCanceledOnTouchOutside(false);
					myAlertDialog.show();
				}
			}
		} );
	}
	
	public void cancelNativeAlert( final int buttonIndex ) {
		// Fetch the currently displayed alert dialog.
		final AlertDialog theDialog;
		synchronized(this) {
			theDialog = myAlertDialog;
			myAlertDialog = null;
		}
		if (theDialog == null) {
			return;
		}

		// Close the alert dialog via the main UI thread.
		myHandler.post(new Runnable() {
			@Override
			public void run() {
				theDialog.cancel();
				if (myRuntime != null && myRuntime.isRunning()) {
					myRuntime.getTaskDispatcher().send(new com.ansca.corona.events.AlertTask(buttonIndex, true));
				}
			}
		});
	}
	
	/**
	 * Displays a native alert message indicating that this is a trial version.
	 * Also provides a "Learn More" button which takes the user to Ansca's website for buying a non-trial version.
	 */
	public void showTrialAlert() {
		// Display the alert dialog on the UI thread.
		myHandler.post(new Runnable() {
			@Override
			public void run() {
				Context context = myContext;
				if (context == null) {
					return;
				}

				AlertDialog.Builder builder = createAlertDialogBuilder(context);
				DialogInterface.OnClickListener clickListener = new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface arg0, int which) {
						openUrl("http://www.coronalabs.com/products/corona-sdk/?utm_source=corona-sdk&utm_medium=corona-sdk&utm_campaign=trial-popup");
					}
				};
				builder.setTitle("Corona Trial");
				builder.setMessage("This message only appears in the trial version");
				builder.setPositiveButton(myResourceServices.getResources().getString(android.R.string.ok), null);
				builder.setNeutralButton("Learn More", clickListener);
				builder.show();
			}
		});
	}

	/**
	 * Displays a native alert message indicating that the store.* library is no longer supported on Android.
	 * Also provides a "Learn More" button which takes the user to our IAP Guide.
	 */
	public void showStoreDeprecatedAlert() {
		// Display the alert dialog on the UI thread.
		myHandler.post(new Runnable() {
			@Override
			public void run() {
				Context context = myContext;
				if (context == null) {
					return;
				}

				AlertDialog.Builder builder = createAlertDialogBuilder(context);
				DialogInterface.OnClickListener clickListener = new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface arg0, int which) {
						openUrl("https://docs.coronalabs.com/guide/monetization/IAP/index.html#google-play-setup");
					}
				};
				builder.setTitle("store.* library removed on Android");
				builder.setMessage("Due to Google removing In-App Billing Version 2 in January 2015, "
					+ "the Corona store.* library on Android is no longer active.\n\n"
					+ "Please migrate to the Google IAP V3 plugin.\n\nSee our IAP Guide for more info.");
				builder.setPositiveButton(myResourceServices.getResources().getString(android.R.string.ok), null);
				builder.setNeutralButton("Learn More", clickListener);
				builder.show();
			}
		});
	}

	/** TODO: ACTUALLY USE THE PERMISSION ARGUMENT FOR SOMETHING!
	 * Displays a native alert explaining why this permission is needed.
	 * Gives option to re-request the permission or still deny it.
	 * @param permission The name of the permission being checked.
	 * @param requestPermissionsResultData The RequestPermissionsResultData related to this permission that rationale should be shown for.
	 */
	public void showPermissionRationaleAlert(final String permission, final RequestPermissionsResultData requestPermissionsResultData) {
		// Display the alert dialog on the UI thread.
		myHandler.post(new Runnable() {
			@Override
			public void run() {
				final CoronaActivity coronaActivity = CoronaEnvironment.getCoronaActivity();
				if (coronaActivity == null) {
					return;
				}

				final PermissionsSettings settings = requestPermissionsResultData.getPermissionsSettings();

				AlertDialog.Builder builder = createAlertDialogBuilder(coronaActivity);
				DialogInterface.OnClickListener okClickListener = new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface arg0, int which) {
						coronaActivity.showRequestPermissionsWindowUsing(settings);
					}
				};
				DialogInterface.OnClickListener cancelClickListener = new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface arg0, int which) {
						// Forward the RequestPermissionsResultData to Lua!
						CoronaActivity.DefaultRequestPermissionsResultHandler requestPermissionsResultHandler =
							((CoronaActivity.DefaultRequestPermissionsResultHandler)requestPermissionsResultData
								.getRequestPermissionsResultHandler());
						if (requestPermissionsResultHandler != null) {
							requestPermissionsResultHandler.forwardRequestPermissionsResultToLua(requestPermissionsResultData);
						}
					}
				};

				String rationaleTitle = settings.getRationaleTitle();
				String rationaleDescription = settings.getRationaleDescription();
				if (rationaleTitle == null || rationaleTitle.equals("") || 
					rationaleDescription == null || rationaleDescription.equals("")) {

					// Put in some default text if none is provided in these PermissionsSettings
					PermissionsServices permissionsServices = new PermissionsServices(coronaActivity);
					
					// Formulate the default messaging based on the permission in question.
					String permissionForAlert = permission;
					if (permissionsServices.isPartOfPAAppPermission(permission)) {
						permissionForAlert = permissionsServices.getPAAppPermissionNameFromAndroidPermission(permission);
					}
					
					// Grab the app name for the default messaging.
					String applicationName = CoronaEnvironment.getApplicationName();

					// Create the needed messages.
					if (rationaleTitle == null || rationaleTitle.equals("")) {
						settings.setRationaleTitle("Need Access To " + permissionForAlert + "!");
					}
					if (rationaleDescription == null || rationaleDescription.equals("")) {
						settings.setRationaleDescription(applicationName + " needs access to " + permissionForAlert + " to continue."
							+ " Please re-request the permission.");
					}
				}

				builder.setTitle(settings.getRationaleTitle());
				builder.setMessage(settings.getRationaleDescription());
				builder.setPositiveButton(myResourceServices.getResources().getString(android.R.string.ok), okClickListener);
				builder.setNegativeButton(myResourceServices.getResources().getString(android.R.string.cancel), cancelClickListener);
				AlertDialog permissionRationaleDialog = builder.create();
				permissionRationaleDialog.setCanceledOnTouchOutside(false);
				permissionRationaleDialog.show();
			}
		});
	}

	/**
	 * Displays a native alert asking the user to grant this permission in the Settings app.
	 * Gives option to open the Settings window for this app's permissions.
	 * @param permission The name of the permission being checked.
	 * @param requestPermissionsResultData The RequestPermissionsResultData related to this permission that the settings redirect should be shown for.
	 */
	public void showSettingsRedirectForPermissionAlert(final String permission, final RequestPermissionsResultData requestPermissionsResultData) {
		// Display the alert dialog on the UI thread.
		myHandler.post(new Runnable() {
			@Override
			public void run() {
				final CoronaActivity coronaActivity = CoronaEnvironment.getCoronaActivity();
				if (coronaActivity == null) {
					return;
				}

				final PermissionsSettings settings = requestPermissionsResultData.getPermissionsSettings();

				AlertDialog.Builder builder = createAlertDialogBuilder(coronaActivity);
				DialogInterface.OnClickListener settingsClickListener = new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface arg0, int which) {
						// Go to the appropriate place in Settings.
						Intent intent = new Intent(android.provider.Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
						Uri uri = Uri.fromParts("package", coronaActivity.getPackageName(), null);
						intent.setData(uri);
						coronaActivity.startActivity(intent);

						// TODO: Add an activity result listener for the Settings activity and when said listener is invoked, 
						// it should re-query the state of the requested permission and forward that result to Lua.
					}
				};
				DialogInterface.OnClickListener cancelClickListener = new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface arg0, int which) {
						// Forward the RequestPermissionsResultData to Lua!
						CoronaActivity.DefaultRequestPermissionsResultHandler requestPermissionsResultHandler =
							((CoronaActivity.DefaultRequestPermissionsResultHandler)requestPermissionsResultData
								.getRequestPermissionsResultHandler());
						if (requestPermissionsResultHandler != null) {
							requestPermissionsResultHandler.forwardRequestPermissionsResultToLua(requestPermissionsResultData);
						}
					}
				};
				// Put in some default text if none is provided in these PermissionsSettings
				String settingsRedirectTitle = settings.getSettingsRedirectTitle();
				String settingsRedirectDescription = settings.getSettingsRedirectDescription();
				if (settingsRedirectTitle == null || settingsRedirectTitle.equals("") || 
					settingsRedirectDescription == null || settingsRedirectDescription.equals("")) {

					// Put in some default text if none is provided in these PermissionsSettings
					PermissionsServices permissionsServices = new PermissionsServices(coronaActivity);
					
					// Formulate the default messaging based on the permission in question.
					String permissionForAlert = permission;
					if (permissionsServices.isPartOfPAAppPermission(permission)) {
						permissionForAlert = permissionsServices.getPAAppPermissionNameFromAndroidPermission(permission);
					}
					
					// Grab the app name for the default messaging.
					String applicationName = CoronaEnvironment.getApplicationName();

					// Create the needed messages.
					if (settingsRedirectTitle == null || settingsRedirectTitle.equals("")) {
						settings.setSettingsRedirectTitle("Access To " + permissionForAlert + " is Critical!");
					}
					if (settingsRedirectDescription == null || settingsRedirectDescription.equals("")) {
						settings.setSettingsRedirectDescription(applicationName + " cannot continue!"
							+ " Please enable access to " + permissionForAlert + " in App Settings.");
					}
				}

				builder.setTitle(settings.getSettingsRedirectTitle());
				builder.setMessage(settings.getSettingsRedirectDescription());
				builder.setPositiveButton("Settings", settingsClickListener);
				builder.setNegativeButton(myResourceServices.getResources().getString(android.R.string.cancel), cancelClickListener);
				AlertDialog settingsRedirectDialog = builder.create();
				settingsRedirectDialog.setCanceledOnTouchOutside(false);
				settingsRedirectDialog.show();
			}
		});
	}

	/**
	 * Displays a native alert stating that this permission is missing from AndroidManifest.xml.
	 * @param permission The name of the permission that's missing.
	 * @param message A message to explain what action can't be performed.
	 */
	void showPermissionMissingFromManifestAlert(final String permission, final String message) {
		// Display the alert dialog on the UI thread.
		myHandler.post(new Runnable() {
			@Override
			public void run() {
				Context context = myContext;
				if (context == null) {
					return;
				}

				String permissionMissingMessage = message + "\n\n" + 
					"Ensure that your app is using the \"" + permission + "\" permission.";
				Log.i("Corona", "ERROR: " + permissionMissingMessage);

				AlertDialog.Builder builder = createAlertDialogBuilder(context);
				builder.setTitle("Corona: Developer Error");
				builder.setMessage(permissionMissingMessage);
				builder.setPositiveButton(myResourceServices.getResources().getString(android.R.string.ok), null);
				builder.show();
			}
		});
	}

	/**
	 * Displays a native alert stating that any permissions in this permission group are missing from AndroidManifest.xml.
	 * @param permissionGroup The name of the permission group that's there's no permissions for.
	 */
	void showPermissionGroupMissingFromManifestAlert(final String permissionGroup) {

		// Display the alert dialog on the UI thread.
		myHandler.post(new Runnable() {
			@Override
			public void run() {
				final CoronaActivity coronaActivity = CoronaEnvironment.getCoronaActivity();
				if (coronaActivity == null) {
					return;
				}

				AlertDialog.Builder builder = createAlertDialogBuilder(coronaActivity);
				DialogInterface.OnClickListener learnMoreClickListener = new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface arg0, int which) {
						openUrl("http://developer.android.com/guide/topics/security/permissions.html#permission-groups");
					}
				};
					
				// Compose the message for this alert.
				String applicationName = CoronaEnvironment.getApplicationName();

				final String permissionGroupMissingMessage = applicationName + " tried to request access to a permission group"
					 + " without having any permissions from that group in build.settings/AndroidManifest.xml!\n\n" + 
					 "Before requesting access, please add a permission from the " + permissionGroup + " permission group!";

				Log.i("Corona", "ERROR: " + permissionGroupMissingMessage);

				builder.setTitle("Corona: Developer Error");
				builder.setMessage(permissionGroupMissingMessage);
				builder.setPositiveButton(myResourceServices.getResources().getString(android.R.string.ok), null);
				builder.setNegativeButton("Learn More", learnMoreClickListener);
				AlertDialog permissionGroupMissingDialog = builder.create();
				permissionGroupMissingDialog.setCanceledOnTouchOutside(false);
				permissionGroupMissingDialog.show();
			}
		});
	}

	/**
	 * Displays a native alert stating that location permissions are missing from AndroidManifest.xml.
	 */
	void showLocationPermissionsMissingFromManifestAlert() {
		// Display the alert dialog on the UI thread.
		myHandler.post(new Runnable() {
			@Override
			public void run() {
				Context context = myContext;
				if (context == null) {
					return;
				}

				String message = "This application does not have permission to read your current location.\n\n" + 
					"Ensure that your app is using at least one of the following permissions:\n " + 
						"- ACCESS_COARSE_LOCATION\n - ACCESS_FINE_LOCATION";
				Log.i("Corona", "ERROR: " + message);

				AlertDialog.Builder builder = createAlertDialogBuilder(context);
				builder.setTitle("Corona: Developer Error");
				builder.setMessage(message);
				builder.setPositiveButton(myResourceServices.getResources().getString(android.R.string.ok), null);
				builder.show();
			}
		});
	}
	
	/**
	 * Displays an activity indicator dialog onscreen. It is displayed modally, meaning you cannot tap behind it.
	 */
	public void showNativeActivityIndicator() {
		// Display the dialog via the UI thread.
		myHandler.post(new Runnable() {
			@Override
			public void run() {
				synchronized (Controller.this) {
					Context context = myContext;
					if (context == null) {
						return;
					}

					// Create the dialog if not done already.
					if (myActivityIndicatorDialog == null) {
						int themeId;
						if (android.os.Build.VERSION.SDK_INT >= 21) {
							themeId = 16974393;		// android.R.style.Theme_Material_Light_Dialog
						}
						else if (android.os.Build.VERSION.SDK_INT >= 11) {
							themeId = 16973935;		// android.R.style.Theme_Holo_Dialog
						}
						else {
							themeId = android.R.style.Theme_Dialog;
						}
						android.view.ContextThemeWrapper contextWrapper;
						contextWrapper = new android.view.ContextThemeWrapper(context, themeId);
						myActivityIndicatorDialog = new ActivityIndicatorDialog(contextWrapper);
						myActivityIndicatorDialog.setCancelable(false);
					}
					
					// Display the dialog.
					if (myActivityIndicatorDialog.isShowing() == false) {
						myActivityIndicatorDialog.show();
					}
				}
			}
		});
	}
	
	/**
	 * Closes the activity indicator window if currently shown.
	 */
	public void closeNativeActivityIndicator() {
		// Close the dialog and dereference it.
		// Note: You do not have to call the dismiss method on the UI thread, 
		// but we do so that order is maintained between set and close calls.
		myHandler.post(new Runnable() {
			@Override
			public void run() {
				synchronized (Controller.this) {
					if (myActivityIndicatorDialog != null) {
						myActivityIndicatorDialog.dismiss();
						myActivityIndicatorDialog = null;
					}
				}
			}
		});
	}
	
	// Image source type IDs matching PlatformImageProvider::Source enum on the C++ side.
	private static final int IMAGE_SOURCE_PHOTO_LIBRARY = 0;
	private static final int IMAGE_SOURCE_CAMERA = 1;
	private static final int IMAGE_SOURCE_SAVED_PHOTOS_ALBUM = 2;

	private static final int MEDIA_CAPTURE_QUALITY_LOW = 0;
	private static final int MEDIA_CAPTURE_QUALITY_MEDIUM = 1;
	private static final int MEDIA_CAPTURE_QUALITY_HIGH = 2;
	
	/**
	 * Determines if the device has the given media source type.
	 * @param mediaSourceType Unique integer ID specifying which media source to check for such as the Camera or Photo Library.
	 *                        This integer ID comes from C++ enum PlatformImageProvider::Source.
	 */
	public boolean hasMediaSource(final int mediaSourceType) {
		boolean hasSource = false;
		switch (mediaSourceType) {
			case IMAGE_SOURCE_PHOTO_LIBRARY:
			case IMAGE_SOURCE_SAVED_PHOTOS_ALBUM:
				hasSource = true;
				break;
			case IMAGE_SOURCE_CAMERA:
				hasSource = CameraServices.hasCamera();
				break;
		}
		return hasSource;
	}
	
	/**
	 * Determines if the app has access to the given media source type.
	 * @param mediaSourceType Unique integer ID specifying which media source to check for such as the Camera or Photo Library.
	 *                        This integer ID comes from C++ enum PlatformImageProvider::Source.
	 */
	public boolean hasAccessToMediaSource(final int mediaSourceType) {
		boolean hasAccessToSource = false;
		switch (mediaSourceType) {
			case IMAGE_SOURCE_PHOTO_LIBRARY:
			case IMAGE_SOURCE_SAVED_PHOTOS_ALBUM:
				hasAccessToSource = true;
				break;
			case IMAGE_SOURCE_CAMERA:
				hasAccessToSource = (CameraServices.hasCamera() && CameraServices.hasPermission());
				break;
		}
		return hasAccessToSource;
	}

	/**
	 * Displays an image picker activity for selecting an image to be displayed in Corona.
	 * @param imageSourceType Unique integer ID specifying what kind of window to display such as the camera or photo library.
	 *                        This integer ID comes from C++ enum PlatformImageProvider::Source.
	 * @param destinationFilePath Set to a path\file name to save the selected photo to.
	 *                            It is okay to set this to null or empty string. For camera shots, a file name will be automatically
	 *                            generated and saved to the cache directory.
	 */
	public void showImagePickerWindow(final int imageSourceType, final String destinationFilePath) {
		// Display the requested window for image selection via the UI thread.
		myHandler.post( new Runnable() {
			public void run() {
				synchronized (this) {
					// Do not continue if the app is about to exit.
					if (myCoronaShowApiListener == null) {
						return;
					}

					// Display the requested window for image selection.
					switch (imageSourceType) {
						case IMAGE_SOURCE_PHOTO_LIBRARY:
						case IMAGE_SOURCE_SAVED_PHOTOS_ALBUM:
							myCoronaShowApiListener.showSelectImageWindowUsing(destinationFilePath);
							break;
							
						case IMAGE_SOURCE_CAMERA:
							myCoronaShowApiListener.showCameraWindowForImage(destinationFilePath);
							break;
							
						default:
							Log.v("Corona", "The given image source is not supported.");
							return;
					}
				}
			}
		} );
	}

	/**
	 * Displays video picker activity for selecting a video file to be returned to corona
	 * @param videoSourceType Unique integer ID specifying what kind of window to display such as the camera or photo library.
	 *                        This integer ID comes from C++ enum PlatformMediaProviderBase::Source.
	 * @param maxTime The maximum time lime of a captured video.
	 * @param quality Unique inter ID specifying what the quality of a captured video should be.
	 */
	public void showVideoPickerWindow(final int videoSourceType, final int maxTime, final int quality) {
		// Display the requested window for video selection via the UI thread.
		myHandler.post( new Runnable() {
			public void run() {
				synchronized (this) {
					// Do not continue if the app is about to exit.
					if (myCoronaShowApiListener == null) {
						return;
					}

					// Display the requested window for video selection.
					switch (videoSourceType) {
						case IMAGE_SOURCE_PHOTO_LIBRARY:
						case IMAGE_SOURCE_SAVED_PHOTOS_ALBUM:
							myCoronaShowApiListener.showSelectVideoWindow();
							break;
							
						case IMAGE_SOURCE_CAMERA:
							int maxVideoTime = maxTime;
							int videoQuality = quality;

							if (maxVideoTime < 1) {
								maxVideoTime = java.lang.Integer.MAX_VALUE;
							}

							// You can only pass one of 2 possible values, 0 for low quality and 1 for high quality.  There is no medium quality.
							if (videoQuality == MEDIA_CAPTURE_QUALITY_LOW ||
								videoQuality == MEDIA_CAPTURE_QUALITY_MEDIUM) {
								
								videoQuality = 0;
							} else {
								videoQuality = 1;
							}

							myCoronaShowApiListener.showCameraWindowForVideo(maxVideoTime, videoQuality);
							break;
							
						default:
							Log.v("Corona", "The given video source is not supported.");
							return;
					}
				}
			}
		} );
	}

	/**
	 * Determines if the given intent will launch an activity.
	 * @param intent The intent used to launch an activity. Cannot be null.
	 * @return Returns true if the given intent will launch an activity.
	 *         <p>
	 *         Returns false if the intent will not launch an activity or if given a null argument.
	 */
	public boolean canShowActivityFor(android.content.Intent intent) {
		// Validate.
		if (intent == null) {
			return false;
		}

		// Fetch the application context.
		android.content.Context context = myContext;
		if (context == null) {
			return false;
		}

		// Determine if the given intent will launch at least one activity on the system.
		java.util.List<ResolveInfo> activitiesToResolveIntent;
		activitiesToResolveIntent = context.getPackageManager().queryIntentActivities(
					intent, android.content.pm.PackageManager.MATCH_DEFAULT_ONLY);

		int numActivitiesToResolveIntent = activitiesToResolveIntent.size();
		if (numActivitiesToResolveIntent > 0) {
			// Verify that we've found a legitimate activity to handle this intent, not a vendor-specific stub.
			if (numActivitiesToResolveIntent == 1) {
				// This activity may just be a stub.
				ResolveInfo resolveInfo = activitiesToResolveIntent.get(0);
				if (resolveInfo.activityInfo.packageName.startsWith("com.google.android.tv.frameworkpackagestubs")) {
					// This is an Android TV stub activity. We can't actually resolve this intent. 
					// Trying to do so will just provide a "You don't have an app that can do this!" toast.
					return false;
				}
			} 

			// There must be at least one legitimate activity found.
			return true;
		}

		// No activities will handle this intent.
		return false;
	}

	public boolean canShowPopup(String name) {
		boolean result = false;
		Intent intent = null;

		Context context = myContext; // Create a temp ref b/c UI thread can GC myActivity
		if ( null != context ) {
			String nameLowerCase = name.toLowerCase();
			if ( nameLowerCase.equals( "mail" ) ) {
				MailSettings settings = new MailSettings();
				intent = settings.toIntent();
			} else if ( nameLowerCase.equals( "sms" ) ) {
				SmsSettings settings = new SmsSettings();
				intent = settings.toIntent();
			} else if ( nameLowerCase.equals( "appstore" ) || nameLowerCase.equals( "rateapp" ) ) {
				// This just indicates that Android version understand the concept of "appStore" and "rateApp"
				result = true;
			} else if ( nameLowerCase.equals( "requestapppermission" ) || nameLowerCase.equals( "requestapppermissions" ) ) {
				// The request permission dialog only exists on Android 6.0 and above.
				result = android.os.Build.VERSION.SDK_INT >= 23;
			}

			if ( ! result && null != intent ) {
				result = canShowActivityFor(intent);
			}
		}

		return result;
	}

	/**
	 * Displays a "Send Mail" window initialized with the given settings.
	 * @param settings A hashtable of e-mail settings matching the table settings in Lua. Can be null.
	 */
	public void showSendMailWindow(java.util.HashMap<String, Object> settings) {
		// Fetch mail settings from hashtable.
		final MailSettings mailSettings = MailSettings.from(myContext, settings);
		
		// Display the requested window via the UI thread.
		myHandler.post( new Runnable() {
			public void run() {
				if (myCoronaShowApiListener == null) {
					return;
				}

				synchronized (this) {
					myCoronaShowApiListener.showSendMailWindowUsing(mailSettings);
				}
			}
		} );
	}
	
	/**
	 * Displays a "Send SMS" window initialized with the given settings.
	 * @param settings A hashtable of SMS settings matching the table settings in Lua. Can be null.
	 */
	public void showSendSmsWindow(java.util.HashMap<String, Object> settings) {		
		 // Fetch SMS settings from hashtable.
		 final SmsSettings smsSettings = SmsSettings.from(settings);
		 
		 // Display the requested window via the UI thread.
		 myHandler.post( new Runnable() {
			 public void run() {
			 	if (myCoronaShowApiListener == null) {
					return;
				}

				 synchronized (this) {
					myCoronaShowApiListener.showSendSmsWindowUsing(smsSettings);
				 }
			 }
		 } );
	}
	
	/**
	 * Displays the app store window for displaying an app's details and writing a review.
	 * @param settings A hashtable of app IDs and supported stores.
	 * @return Returns true if the window is about to be displayed.
	 *         <p>
	 *         Returns false if the App Store could not be found and is unable to display a window.
	 */
	public boolean showAppStoreWindow(java.util.HashMap<String, Object> settings) {
		if (myCoronaShowApiListener == null) {
			return false;
		}

		return myCoronaShowApiListener.showAppStoreWindow(settings);
	}

	/**
	 * Displays a "Request Permissions" window initialized with the given settings.
	 * @param settings A hashtable of permission settings matching the table settings in Lua. Cannot be null.
	 */
	public void showRequestPermissionsWindow(java.util.HashMap<String, Object> settings) {		
		 // Fetch Permission settings from hashtable.
		 final PermissionsSettings permissionsSettings = PermissionsSettings.from(settings);
		 
		 // Display the requested window via the UI thread.
		 myHandler.post( new Runnable() {
			 public void run() {
			 	if (myCoronaShowApiListener == null) {
			 		Log.v("Corona", "Cannot request permissions. No show API listener!");
					return;
				}

				 synchronized (this) {
					myCoronaShowApiListener.showRequestPermissionsWindowUsing(permissionsSettings);
				 }
			 }
		 } );
	}
	
	public void setAccelerometerInterval( int frequency ) {
		mySensorManager.setAccelerometerInterval(frequency);
	}

	public void setGyroscopeInterval( int frequency ) {
		mySensorManager.setGyroscopeInterval(frequency);
	}
	
	public void setLocationThreshold( double meters ) {
		mySensorManager.setLocationThreshold(meters);
	}
	
	/**
	 * Determines if this device has an accelerometer sensor.
	 * @return Returns true if this device has an accelerometer. Returns false if not.
	 */
	public boolean hasAccelerometer()
	{
		return mySensorManager.hasAccelerometer();
	}
	
	/**
	 * Determines if this device has a gyroscope sensor.
	 * @return Returns true if this device has a gyroscope. Returns false if not.
	 */
	public boolean hasGyroscope()
	{
		return mySensorManager.hasGyroscope();
	}

	/**
	 * Determines if this device has the ability to provide heading events.
	 * @return Returns true if this device has heading hardware. Returns false if not.
	 */
	public boolean hasHeadingHardware()
	{
		return mySensorManager.hasHeadingHardware();
	}
	
	public void setEventNotification( int eventType, boolean enable ) {
		mySensorManager.setEventNotification(eventType, enable);
	}
	
	public void vibrate(String hapticType, String hapticStyle) {
		Context context = myContext;
		if (context == null) {
			return;
		}

		Vibrator v = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
		String type = "";
		if(hapticType != null){
			type = hapticType;
		}
		String style = "";
		if(hapticStyle != null){
			style = hapticStyle;
		}
		long[] timings = new long[]{};
		int[] amplitudes= new int[]{};
		long[] oldPattern= new long[]{};
		if(type.equals("impact")){
			if(style.equals("light")){
				timings = new long[]{0, 50};
				amplitudes = new int[]{0, 110};
				oldPattern = new long[]{0, 20};
			}
			else if(style.equals("heavy")){
				timings = new long[]{0, 60};
				amplitudes = new int[]{0, 255};
				oldPattern = new long[]{0, 61};
			}
			else{//medium
				timings = new long[]{0, 43};
				amplitudes = new int[]{0, 180};
				oldPattern = new long[]{0, 43};
			}
		}else if(type.equals("selection")){
			timings = new long[]{0, 100};
			amplitudes = new int[]{0, 100};
			oldPattern = new long[]{0, 70};
		}else if(type.equals("notification")){
			if(style.equals("warning")) {
				timings = new long[]{0, 30, 40, 30, 50, 60};
				amplitudes = new int[]{255, 255, 255, 255, 255, 255};
				oldPattern = new long[]{0, 30, 40, 30, 50, 60};
			}else if(style.equals("error")){
				timings = new long[]{0, 27, 45, 50};
				amplitudes = new int[]{0, 120, 0, 250};
				oldPattern = new long[]{0, 27, 45, 50};
			}else{//success
				timings = new long[]{0, 35, 65, 21};
				amplitudes = new int[]{0, 250, 0, 180};
				oldPattern = new long[]{0, 35, 65, 21};
			}
		}else{
			if(!type.isEmpty()){ Log.i("Corona", "WARNING: invalid hapticType");} //just in case user misspells or puts a wrong type
			v.vibrate( 100 );
		}
		if(timings.length != 0 && amplitudes.length != 0 && oldPattern.length != 0) {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
				v.vibrate(VibrationEffect.createWaveform(timings, amplitudes, -1));
			}else{
				v.vibrate(oldPattern, -1);
			}
		}
	}

	public String getManufacturerName() {
		return android.os.Build.MANUFACTURER;
	}
	
	public String getModel() {
		return android.os.Build.MODEL;
	}

	public String getName() {
		// TODO: Attempt to get the user-customizable name on devices that support it!
		return "unknown";
	}
	
	protected String GetHardwareIdentifier()
	{
		String stringId = null;

		try {
			String permissionName = android.Manifest.permission.READ_PHONE_STATE;
			if (myContext.checkCallingOrSelfPermission(permissionName) == android.content.pm.PackageManager.PERMISSION_GRANTED) {
				TelephonyManager telephonyManager = (TelephonyManager)myContext.getSystemService(Context.TELEPHONY_SERVICE);
				if (telephonyManager != null) {
					stringId = telephonyManager.getDeviceId();
				}
			}
		}
		catch (Exception ex) { }

		return stringId;
	}

	protected String GetOSIdentifier()
	{
		Context context = myContext;
		if (context == null) {
			return null;
		}

		// Unable to fetch the phone ID from this device.
		// Fallback to the OS installation's generated ID. (This ID changes when re-installing the OS.)
		return Secure.getString(context.getContentResolver(), Secure.ANDROID_ID);
	}

	public String getUniqueIdentifier( int identifierType ) {
		String stringId = null;
		
		switch ( identifierType ) {
			case 0: // kDeviceIdentifier
				// First attempt to fetch unique phone ID of this device. This ID will never change on the device.
				// This will only work provided that the "READ_PHONE_STATE" permission has been set and this device is a phone.
				String hardwareId = GetHardwareIdentifier();
				if ((hardwareId != null) && (hardwareId.length() > 0)) {
					stringId = hardwareId;
				}

				// Unable to fetch the phone ID from this device.
				// Fallback to the OS installation's generated ID. (This ID changes when re-installing the OS.)
				if ( null == stringId ) {
					stringId = GetOSIdentifier();
				}
				break;

			case 1: // kHardwareIdentifier
				stringId = GetHardwareIdentifier();
				break;

			case 2: // kOSIdentifier
				stringId = GetOSIdentifier();
				break;

			default:
				break;
		}
		
		// Failed to retreive a unique ID for this device.
		// Return an empty string, because odds are the Lua code won't check for null/nil case.
		if ( null == stringId ) {
			stringId = "";
		}

		return stringId;
	}

	public String getPlatformVersion() {
		return android.os.Build.VERSION.RELEASE;
	}
    
    public String getProductName() {
        return android.os.Build.PRODUCT;
    }

    public float getDefaultFontSize() {
    	return myDefaultFontSize;
    }

    public int getDefaultTextFieldPaddingInPixels() {
    	return myDefaultTextFieldPaddingInPixels;
    }
	
	void setSystemUiVisibility(final String visibility) {
		final com.ansca.corona.graphics.opengl.CoronaGLSurfaceView glView = myGLView;
		if (glView == null || android.os.Build.VERSION.SDK_INT < 11 ||
			android.os.Build.MANUFACTURER.equals("BN LLC")) {
			return;
		}

		myHandler.post(new Runnable() {
			public void run() {
				int vis = -1;
				if (visibility.equals("immersiveSticky") && (
					(android.os.Build.VERSION.SDK_INT >= 19) ||
					(android.os.Build.MANUFACTURER.equals("Amazon") && android.os.Build.VERSION.SDK_INT >= 14))) {
						// For Amazon devices, we can't do all of immersiveSticky mode, but we can at least go into super fullscreen mode.
						// See: https://developer.amazon.com/public/solutions/devices/fire-tablets/app-development/01--screen-layout-and-resolution#Understand How Fullscreen Modes Affect Layout
						// 0x00001000 View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY lets touch events pass to the corona app
						// 0x00000002 View.SYSTEM_UI_FLAG_HIDE_NAVIGATION hides any on screen navigation buttons
						// 0x00000004 View.SYSTEM_UI_FLAG_FULLSCREEN hides the status bar
						// 0x00000200 View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION avoids resize event
						vis = 0x00001000 | 0x00000002 | 0x00000004 | 0x00000200;
				} else if (visibility.equals("immersive") && (
					(android.os.Build.VERSION.SDK_INT >= 19) ||
					(android.os.Build.MANUFACTURER.equals("Amazon") && android.os.Build.VERSION.SDK_INT >= 14))) {
						// For Amazon devices, we can't do all of immersive mode, but we can at least go into super fullscreen mode.
						// See: https://developer.amazon.com/public/solutions/devices/fire-tablets/app-development/01--screen-layout-and-resolution#Understand How Fullscreen Modes Affect Layout
						// 0x00000800 View.SYSTEM_UI_FLAG_IMMERSIVE lets touch events pass to the corona app
						// 0x00000002 View.SYSTEM_UI_FLAG_HIDE_NAVIGATION hides any on screen navigation buttons
						// 0x00000004 View.SYSTEM_UI_FLAG_FULLSCREEN hides the status bar
						// 0x00000200 View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION avoids resize event
						vis = 0x00000800 | 0x00000002 | 0x00000004 | 0x00000200;
				} else if (visibility.equals("lowProfile")) {
					// On API Level 14 and above: View.SYSTEM_UI_FLAG_LOW_PROFILE dims any on screen buttons if they exists
					// For API Level 11 - 13: View.STATUS_BAR_HIDDEN has the same effect
					// Will have no visibile effect on Amazon devices running Fire OS 4 or lower.
					vis = 0x00000001;
				} else if (visibility.equals("default")) {
					// Clear all flags
					vis = 0x00000000;
				}

				if (vis  > -1) {
					if(android.os.Build.VERSION.SDK_INT >= 23) {
						vis |= ApiLevel11.getSystemUiVisibility(glView) & android.view.View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
					}
					ApiLevel11.setSystemUiVisibility(glView, vis);

					final int finalVis = vis;
					glView.setOnSystemUiVisibilityChangeListener(new android.view.View.OnSystemUiVisibilityChangeListener() {

						@Override
						public void onSystemUiVisibilityChange(int visibilityInt)
						{
							if((finalVis & android.view.View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY) != 0)
							{
								ApiLevel11.setSystemUiVisibility(glView, finalVis);
							}
						}
					});
				}
			}
		});
	}
	
	String getSystemUiVisibility() { 
		final com.ansca.corona.graphics.opengl.CoronaGLSurfaceView glView = myGLView;

		if (glView == null) {
			return "unknown";
		} else if (android.os.Build.VERSION.SDK_INT < 11 || 
			android.os.Build.MANUFACTURER.equals("BN LLC")) {
			return "default";
		}

		int visibility = ApiLevel11.getSystemUiVisibility(glView);

		if ((visibility & 0x00001006) == 0x00001006) {
			return "immersiveSticky";
		} else if ((visibility & 0x00000806) == 0x00000806) {
			return "immersive";
		} else if ((visibility & 0x1) == 0x1) {
			return "lowProfile";
		} else if (visibility == 0 || 
			(visibility & 0x00001000) == 0x00001000 ||
			(visibility & 0x00000800) == 0x00000800) {
			// When the user swipes from immerisve mode or brings the Soft Key Bar on Amazon devices back up,
			// the immersive flag stays but its not immersive.
			return "default";
		}

		return "unknown";
	}

	/** Provides easy access to Android 3.x APIs. */
	// TODO: Less duplication of all the AlertDialig.Builder code.
	private static class ApiLevel11 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel11() { }

		public static void setSystemUiVisibility(android.view.View v, int visibility) {
			v.setSystemUiVisibility(visibility);
		}

		public static int getSystemUiVisibility(android.view.View v) {
			return v.getSystemUiVisibility();
		}

		/**
		 * Creates an "AlertDialog.Builder" object using the Holo theme by default.
		 * @param context The parent that will host the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createDefaultAlertDialogBuilder(android.content.Context context) {
			return createDarkAlertDialogBuilder(context);
		}

		/**
		 * Creates an "AlertDialog.Builder" object using the THEME_HOLO_DARK theme.
		 * @param context The parent that will host the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createDarkAlertDialogBuilder(android.content.Context context) {
			return new AlertDialog.Builder(context, android.app.AlertDialog.THEME_HOLO_DARK);
		}

		/**
		 * Creates an "AlertDialog.Builder" object using the THEME_HOLO_LIGHT theme.
		 * @param context The parent that will host the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createLightAlertDialogBuilder(android.content.Context context) {
			return new AlertDialog.Builder(context, android.app.AlertDialog.THEME_HOLO_LIGHT);
		}

		/**
		 * Creates an "AlertDialog.Builder" object.
		 * @param context The parent that will host the dialog.
		 * @param theme A "style" resource ID used to theme the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createAlertDialogBuilder(
			android.content.Context context, int theme)
		{
			return new AlertDialog.Builder(context, theme);
		}
	}

	/**
	 * Provides access to API Level 14 (Android 4.0 Ice Cream Sandwich) features.
	 * Should only be accessed if running on an operating system matching this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * Instead, you are expected to call its static methods instead.
	 */
	private static class ApiLevel14 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel14() {}

		/**
		 * Creates an "AlertDialog.Builder" object using the device default theme if possible.
		 * <p>
		 * If no device default theme is provided, the Holo theme is used.
		 * @param context The parent that will host the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createDefaultAlertDialogBuilder(android.content.Context context) {
			return createDarkAlertDialogBuilder(context);
		}

		/**
		 * Creates an "AlertDialog.Builder" object using the device default dark theme if possible.
		 * <p>
		 * If no device default theme is provided, the Holo Dark theme is used.
		 * @param context The parent that will host the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createDarkAlertDialogBuilder(android.content.Context context) {
			if (isDeviceDefaultThemeAvailable(context)) {
				return new AlertDialog.Builder(context, android.app.AlertDialog.THEME_DEVICE_DEFAULT_DARK);
			}
			return new AlertDialog.Builder(context, android.app.AlertDialog.THEME_HOLO_DARK);
		}

		/**
		 * Creates an "AlertDialog.Builder" object using the device default light theme if possible.
		 * <p>
		 * If no device default theme is provided, the Holo Light theme is used.
		 * @param context The parent that will host the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createLightAlertDialogBuilder(android.content.Context context) {
			if (isDeviceDefaultThemeAvailable(context)) {
				return new AlertDialog.Builder(context, android.app.AlertDialog.THEME_DEVICE_DEFAULT_LIGHT);
			}
			return new AlertDialog.Builder(context, android.app.AlertDialog.THEME_HOLO_LIGHT);
		}

		/**
		 * Creates an "AlertDialog.Builder" object using the given theme.
		 * @param context The parent that will host the dialog.
		 * @param theme A "style" resource ID used to theme the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createAlertDialogBuilder(
			android.content.Context context, int theme)
		{
			return new AlertDialog.Builder(context, theme);
		}

		/**
		 * Determines if this Android device has a default theme specified by the OEM available to it.
		 * @param context The context used to resolve the existance of a device default theme.
		 * @return Returns true if a DeviceDefaultTheme is available, false otherwise.
		 */
		public static boolean isDeviceDefaultThemeAvailable(android.content.Context context) {
			android.util.TypedValue outValue = new android.util.TypedValue();
			context.getTheme().resolveAttribute(android.R.attr.alertDialogTheme, outValue, true);
			String themeString = context.getResources().getResourceEntryName(outValue.resourceId);
			return "Theme.Dialog.Alert".equals(themeString);
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

		/**
		 * Creates an "AlertDialog.Builder" object using the device default theme if possible.
		 * <p>
		 * If no device default theme is provided, the Material theme is used.
		 * @param context The parent that will host the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createDefaultAlertDialogBuilder(android.content.Context context) {
			return createLightAlertDialogBuilder(context);
		}

		/**
		 * Creates an "AlertDialog.Builder" object using the device default dark theme if possible.
		 * <p>
		 * If no device default theme is provided, the Material dark theme is used.
		 * @param context The parent that will host the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createDarkAlertDialogBuilder(android.content.Context context) {
			if (ApiLevel14.isDeviceDefaultThemeAvailable(context)) {
				return new AlertDialog.Builder(context, android.app.AlertDialog.THEME_DEVICE_DEFAULT_DARK);
			}
			return new AlertDialog.Builder(context, android.R.style.Theme_Material_Dialog_Alert);
		}

		/**
		 * Creates an "AlertDialog.Builder" object using the device default light theme if possible.
		 * <p>
		 * If no device default theme is provided, the Material light theme is used.
		 * @param context The parent that will host the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createLightAlertDialogBuilder(android.content.Context context) {
			if (ApiLevel14.isDeviceDefaultThemeAvailable(context)) {
				return new AlertDialog.Builder(context, android.app.AlertDialog.THEME_DEVICE_DEFAULT_LIGHT);
			}
			return new AlertDialog.Builder(context, android.R.style.Theme_Material_Light_Dialog_Alert);
		}

		/**
		 * Creates an "AlertDialog.Builder" object using the given theme.
		 * @param context The parent that will host the dialog.
		 * @param theme A "style" resource ID used to theme the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createAlertDialogBuilder(
			android.content.Context context, int theme)
		{
			return new AlertDialog.Builder(context, theme);
		}
	}

	/**
	 * Provides access to API Level 22 (Android 5.1 Lollipop MR1) features.
	 * Should only be accessed if running on an operating system matching this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * Instead, you are expected to call its static methods instead.
	 */
	private static class ApiLevel22 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel22() {}

		/**
		 * Creates an "AlertDialog.Builder" object using the device default theme if possible.
		 * <p>
		 * If no device default theme is provided, the Material theme is used.
		 * @param context The parent that will host the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createDefaultAlertDialogBuilder(android.content.Context context) {
			return createLightAlertDialogBuilder(context);
		}

		/**
		 * Creates an "AlertDialog.Builder" object using the device default dark theme if possible.
		 * <p>
		 * If no device default theme is provided, the Material dark theme is used.
		 * @param context The parent that will host the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createDarkAlertDialogBuilder(android.content.Context context) {
			if (ApiLevel14.isDeviceDefaultThemeAvailable(context)) {
				return new AlertDialog.Builder(context, android.R.style.Theme_DeviceDefault_Dialog_Alert);
			}
			return new AlertDialog.Builder(context, android.R.style.Theme_Material_Dialog_Alert);
		}

		/**
		 * Creates an "AlertDialog.Builder" object using the device default light theme if possible.
		 * <p>
		 * If no device default theme is provided, the Material light theme is used.
		 * @param context The parent that will host the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createLightAlertDialogBuilder(android.content.Context context) {
			if (ApiLevel14.isDeviceDefaultThemeAvailable(context)) {
				return new AlertDialog.Builder(context, android.R.style.Theme_DeviceDefault_Light_Dialog_Alert);
			}
			return new AlertDialog.Builder(context, android.R.style.Theme_Material_Light_Dialog_Alert);
		}

		/**
		 * Creates an "AlertDialog.Builder" object using the given theme.
		 * @param context The parent that will host the dialog.
		 * @param theme A "style" resource ID used to theme the dialog.
		 * @return Returns a new "AlertDialog.Builder" object.
		 */
		public static android.app.AlertDialog.Builder createAlertDialogBuilder(
			android.content.Context context, int theme)
		{
			return new AlertDialog.Builder(context, theme);
		}
	}
}
