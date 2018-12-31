package com.coronalabs.corona.sample.extendingui;


/**
 * Extends the Application class to receive Corona runtime events and to extend the Lua API.
 * <p>
 * Only one instance of this class will be created by the Android OS. It will be created before this application's
 * activity is displayed and will persist after the activity is destroyed. The name of this class must be set in the
 * AndroidManifest.xml file's "application" tag or else an instance of this class will not be created on startup.
 */
public class CoronaApplication extends android.app.Application {
	/** Called when your application has started. */
	@Override
	public void onCreate() {
		// Set up a Corona runtime listener used to identify when the Corona runtime has started up.
		com.ansca.corona.CoronaEnvironment.addRuntimeListener(new CoronaApplication.CoronaRuntimeEventHandler());
	}
	
	
	/** Receives and handles Corona runtime events. */
	private class CoronaRuntimeEventHandler implements com.ansca.corona.CoronaRuntimeListener {
		/**
		 * Called after the Corona runtime has been created and just before executing the "main.lua" file.
		 * This is the application's opportunity to register custom APIs into Lua.
		 * <p>
		 * Warning! This method is not called on the main thread.
		 * @param runtime Reference to the CoronaRuntime object that has just been loaded/initialized.
		 *                Provides a LuaState object that allows the application to extend the Lua API.
		 */
		@Override
		public void onLoaded(com.ansca.corona.CoronaRuntime runtime) {
			// Display custom UI on top of the Corona activity.
			// Note that this method is called on the Corona runtime thread and not the main UI thread.
			// In order to manipulate the UI, we must post a Runnable object to the main thread's message queue.
			com.ansca.corona.CoronaEnvironment.getCoronaActivity().runOnUiThread(new Runnable() {
				@Override
				public void run() {
					// *** We're now running on the main UI thread. ***
					
					// Fetch the currently running CoronaActivity.
					// Warning: Will return null if the activity has just been exited.
					com.ansca.corona.CoronaActivity activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();
					if (activity == null) {
						return;
					}
					
					// Display a title bar at the top of the screen.
					TitleBar titleBar = new TitleBar(activity);
					titleBar.setLayoutParams(new android.widget.FrameLayout.LayoutParams(
								android.widget.FrameLayout.LayoutParams.MATCH_PARENT,
								android.widget.FrameLayout.LayoutParams.WRAP_CONTENT,
								android.view.Gravity.TOP));
					activity.getOverlayView().addView(titleBar);
					
					// Display a toolbar at the bottom of the screen.
					ToolBar toolBar = new ToolBar(activity);
					toolBar.setLayoutParams(new android.widget.FrameLayout.LayoutParams(
								android.widget.FrameLayout.LayoutParams.MATCH_PARENT,
								android.widget.FrameLayout.LayoutParams.WRAP_CONTENT,
								android.view.Gravity.BOTTOM));
					if (android.os.Build.MODEL.toLowerCase().contains("kindle")) {
						// The Kindle Fire displays a navigation bar overlaid on top of the Corona activity.
						// Work-around this by padding our custom toolbar above the Kindle Fire's navigation bar.
						int fullScreenFlag = android.view.WindowManager.LayoutParams.FLAG_FULLSCREEN;
						if ((activity.getWindow().getAttributes().flags & fullScreenFlag) != 0) {
							((android.widget.FrameLayout.LayoutParams)toolBar.getLayoutParams()).bottomMargin = 20;
						}
						else {
							((android.widget.FrameLayout.LayoutParams)toolBar.getLayoutParams()).bottomMargin = 60;
						}
					}
					activity.getOverlayView().addView(toolBar);
				}
			});
		}
		
		/**
		 * Called just after the Corona runtime has executed the "main.lua" file.
		 * <p>
		 * Warning! This method is not called on the main thread.
		 * @param runtime Reference to the CoronaRuntime object that has just been started.
		 */
		@Override
		public void onStarted(com.ansca.corona.CoronaRuntime runtime) {
		}
		
		/**
		 * Called just after the Corona runtime has been suspended which pauses all rendering, audio, timers,
		 * and other Corona related operations. This can happen when another Android activity (ie: window) has
		 * been displayed, when the screen has been powered off, or when the screen lock is shown.
		 * <p>
		 * Warning! This method is not called on the main thread.
		 * @param runtime Reference to the CoronaRuntime object that has just been suspended.
		 */
		@Override
		public void onSuspended(com.ansca.corona.CoronaRuntime runtime) {
		}
		
		/**
		 * Called just after the Corona runtime has been resumed after a suspend.
		 * <p>
		 * Warning! This method is not called on the main thread.
		 * @param runtime Reference to the CoronaRuntime object that has just been resumed.
		 */
		@Override
		public void onResumed(com.ansca.corona.CoronaRuntime runtime) {
		}
		
		/**
		 * Called just before the Corona runtime terminates.
		 * <p>
		 * This happens when the Corona activity is being destroyed which happens when the user presses the Back button
		 * on the activity, when the native.requestExit() method is called in Lua, or when the activity's finish()
		 * method is called. This does not mean that the application is exiting.
		 * <p>
		 * Warning! This method is not called on the main thread.
		 * @param runtime Reference to the CoronaRuntime object that is being terminated.
		 */
		@Override
		public void onExiting(com.ansca.corona.CoronaRuntime runtime) {
		}
	}
}
