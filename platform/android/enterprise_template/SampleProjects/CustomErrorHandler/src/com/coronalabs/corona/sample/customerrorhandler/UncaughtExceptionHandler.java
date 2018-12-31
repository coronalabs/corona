package com.coronalabs.corona.sample.customerrorhandler;


/**
 * Handles exceptions that have not been caught by the application.
 * <p>
 * Instances of this class are to be given to the Thread.setUncaughtExceptionHandler() method.
 */
public class UncaughtExceptionHandler implements Thread.UncaughtExceptionHandler {
	/**
	 * Called when an uncaught exception has been thrown.
	 * @param thread The thread that the exception was thrown on.
	 * @param ex The exception that occurred.
	 */
	@Override
	public void uncaughtException(Thread thread, Throwable ex) {
		// Print the exception message and stack trace to the Android logging system.
		System.out.println("[Uncaught Exception]:");
		ex.printStackTrace();
		
		// Attempt to display an alert dialog with the given exception message.
		android.app.Activity activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();
		if (activity != null) {
			final String errorMessage = ex.toString();
			activity.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					// Do not continue if the activity has already been destroyed.
					android.app.Activity activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();
					if (activity == null) {
						return;
					}
					
					// Set up a listener for the dialog to exit this application since it is in a bad state.
					android.content.DialogInterface.OnCancelListener cancelListener;
					cancelListener = new android.content.DialogInterface.OnCancelListener() {
						public void onCancel(android.content.DialogInterface dialog) {
							System.exit(1);
						}
					};
					
					// Display an alert dialog.
					android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(activity);
					builder.setTitle("Uncaught Exception");
					builder.setMessage(errorMessage);
					builder.setOnCancelListener(cancelListener);
					android.app.AlertDialog dialog = builder.create();
					dialog.setCanceledOnTouchOutside(false);
					dialog.show();
				}
			});
		}
	}
}
