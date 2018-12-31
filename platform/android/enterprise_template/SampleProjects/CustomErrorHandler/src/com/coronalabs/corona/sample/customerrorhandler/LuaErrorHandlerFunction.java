package com.coronalabs.corona.sample.customerrorhandler;


/**
 * Receives unhandled Lua errors that occurred during execution by the Corona runtime.
 * Errors such as Lua syntax or runtime errors.
 * <p>
 * This handler is implemented like a Lua function, but implemented in Java.
 * A Lua error handling function is typically expected to return a string that describes the error.
 * <p>
 * Instances of this class are to be given to the CoronaEnvironment.setLuaErrorHandler() method.
 * Doing so will replace Corona's default error handler with this class' implementation.
 * <p>
 * Warning! This class's invoke() method will not be called on the main UI thread.
 * The Corona runtime and its Lua scripts run on a separate thread.
 * This means that you should never manipulate an activity's UI elements from this class' method.
 */
public class LuaErrorHandlerFunction implements com.naef.jnlua.JavaFunction {
	/**
	 * This method is called when a Lua error occurs int he Corona runtime.
	 * <p>
	 * Warning! This method is not called on the main UI thread.
	 * @param luaState Reference to the Lua state that the error occurred in.
	 *                 Typically provides an error string at the top of the stack to be returned by
	 *                 this function. The Lua state can also provide access to a stack trace, if available.
	 * @return Returns the number of values to be returned by this error function.
	 *         An error function would typically return one string value describing the error.
	 */
	@Override
	public int invoke(com.naef.jnlua.LuaState luaState) {
		// The first argument in the error function is expected to be a string describing the error.
		// If it is not a string, then we do not know how to handle the error and return it as is.
		if (luaState.isString(1) == false) {
			return 1;
		}
		
		// Fetch the global "debug" table from Lua.
		luaState.getField(com.naef.jnlua.LuaState.GLOBALSINDEX, "debug");
		if (luaState.isTable(-1) == false) {
			luaState.pop(2);
			return 1;
		}
		
		// Fetch the above debug table's traceback() Lua function.
		luaState.getField(-1, "traceback");
		if (luaState.isFunction(-1) == false) {
			luaState.pop(2);
			return 1;
		}
		
		// Call the traceback() Lua function to fetch a stack trace.
		luaState.pushValue(1);		// Push this Lua function's 1st argument to the traceback() 1st argument.
		luaState.pushInteger(1);	// Push integer value 1 to the traceback() function's 2nd argument. 
		luaState.call(2, 1);		// Call the Lua traceback() function with 2 arguments and accept 1 return value.
		
		// Print the Lua stack trace returned by the above Lua traceback() function.
		System.out.println("[LUA ERROR]: " + luaState.toString(-1));
		
		// Display an alert dialog stating the Lua error.
		showErrorMessage(luaState.toString(-1));
		
		// Return the error message that was originally given to this function.
		return 1;
	}
	
	/**
	 * Displays the given error message in an alert dialog and then exits the app.
	 * @param errorMessage The error message to be displayed.
	 */
	private void showErrorMessage(final String errorMessage) {
		// Validate argument.
		if ((errorMessage == null) || (errorMessage.length() <= 0)) {
			return;
		}
		
		// Fetch the activity to display the error message in.
		android.app.Activity activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();
		if (activity == null) {
			return;
		}
		
		// Display the error in an alert dialog on the UI thread.
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				// Do not continue if the activity has already been destroyed.
				// This can happen if the Lua error occurred during an application exit.
				android.app.Activity activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();
				if (activity == null) {
					return;
				}
				
				// Set up a listener for the dialog to exit the activity since the app may be in a bad state.
				android.content.DialogInterface.OnCancelListener cancelListener;
				cancelListener = new android.content.DialogInterface.OnCancelListener() {
					public void onCancel(android.content.DialogInterface dialog) {
						System.exit(1);
					}
				};
				
				// Display an alert dialog.
				android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(activity);
				builder.setTitle("Lua Error");
				builder.setMessage(errorMessage);
				builder.setOnCancelListener(cancelListener);
				android.app.AlertDialog dialog = builder.create();
				dialog.setCanceledOnTouchOutside(false);
				dialog.show();
			}
		});
	}
}
