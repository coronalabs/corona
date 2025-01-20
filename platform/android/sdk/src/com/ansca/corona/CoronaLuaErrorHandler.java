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
 * Corona's default Lua error handler used to display an alert dialog of a Lua error that occurs
 * at runtime and then exits out of the application.
 * <p>
 * Instances of this class are to be given to the CoronaEnvironment.setLuaErrorHandler() method.
 * Doing so will replace Corona's default error handler with this class' implementation.
 */
public class CoronaLuaErrorHandler implements com.naef.jnlua.JavaFunction {
	/** Set true if an error dialog is currently being shown. */
	private boolean fIsShowingError = false;

	private Controller fController;

	void setController(Controller controller) {
		fController = controller;
	}

	/**
	 * This method is called when a Lua error occurs in the Corona runtime.
	 * <p>
	 * Warning! This method is not called on the main UI thread.
	 * @param luaState Reference to the Lua state that the error occurred in.
	 *                 Typically provides an error string at the top of the stack to be returned by
	 *                 this function. The Lua state can also provide access to a stack trace, if available.
	 * @return Returns 1 indicating this function has pushed one error message to the Lua stack.
	 */
	@Override
	public int invoke(com.naef.jnlua.LuaState luaState) {
		String errorMessage = null;
		String javaStackDump = null;
		String luaStackDump = null;

		// The first argument in the Lua function describes the error. Extract its error information.
		if (luaState.isString(1)) {
			// The error was provided as a string, which is typically the case.
			errorMessage = luaState.toString(1);
		}
		else if (luaState.isJavaObjectRaw(1)) {
			// JNLua normally provides errors as a LuaError object.
			Object value = luaState.toJavaObjectRaw(1);
			if (value instanceof com.naef.jnlua.LuaError) {
				com.naef.jnlua.LuaError luaError = (com.naef.jnlua.LuaError)value;
				errorMessage = luaError.toString();
				luaStackDump = getStackTraceFrom(luaError.getLuaStackTrace());
				javaStackDump = getStackTraceFrom(luaError.getCause());
			}
		}

		// If we were unable to extract an error message up above, then use a generic error message.
		if ((errorMessage == null) || (errorMessage.length() <= 0)) {
			errorMessage = "Lua runtime error occurred.";
		}

		// Extract the Java exception stack trace from the error message, if it exists.
		int index = errorMessage.indexOf(com.naef.jnlua.LuaError.JAVA_STACK_TRACE_HEADER_MESSAGE);
		if (index > 0) {
			if (javaStackDump == null) {
				javaStackDump = errorMessage.substring(index + 1);
			}
			errorMessage = errorMessage.substring(0, index);
		}
		
		// Fetch a Lua stack dump by calling the Lua debug.traceback() function.
		if (luaStackDump == null) {
			index = luaState.getTop();
			luaState.getField(com.naef.jnlua.LuaState.GLOBALSINDEX, "debug");
			if (luaState.isTable(-1)) {
				luaState.getField(-1, "traceback");
				if (luaState.isFunction(-1)) {
					luaState.call(0, 1);
					if (luaState.isString(-1)) {
						luaStackDump = luaState.toString(-1);
					}
				}
			}
			if (luaStackDump.equals("stack traceback:")) {
				// didn't actually get a stack trace
				luaStackDump = "";
			}

			luaState.setTop(index);
		}

		// Create an exception to be thrown after the error message has been displayed.
		// This way, the Android OS can report the error to the developer via the app store.
		StringBuilder builder = new StringBuilder();
		builder.append(errorMessage);
		if ((javaStackDump != null) && (javaStackDump.length() > 0)) {
			builder.append("\n");
			builder.append(javaStackDump);
		}
		if ((luaStackDump != null) && (luaStackDump.length() > 0)) {
			builder.append("\n");
			builder.append(luaStackDump);
		}

		// Emit the error to the log tagged with "Corona"
		android.util.Log.i("Corona", "ERROR: Runtime error");
		android.util.Log.i("Corona", builder.toString());

		// Implement the "unhandledError" listener
		int top = luaState.getTop();
		boolean bail = true;

		CoronaLua.newEvent( luaState, "unhandledError" );

		// Put the error message and stacktrace in the table that's handed to the handler
		if ((errorMessage.length() > 0) && (luaStackDump.length() == 0))
		{
			// This handles a common case where the error message and the stack trace are combined
			String[] lines = errorMessage.split("\n", 2);
			if (lines.length > 1)
			{
				errorMessage = lines[0];
				luaStackDump = "\nstack traceback:\n" + lines[1];
			}
		}
		luaState.pushString( errorMessage );
		luaState.setField( -2, "errorMessage" );
		luaState.pushString( luaStackDump );
		luaState.setField( -2, "stackTrace" );

		CoronaLua.dispatchRuntimeEvent( luaState, 1 );

		if (luaState.isBoolean( -1 ))
		{
			boolean result = luaState.toBoolean( -1 );

			if (result)
			{
				// They returned "true" ... carry on
				bail = false;
			}
		}

		luaState.setTop(top);

		// By default or if the "unhandledError" listener returns false, we shutdown the app
		if (bail)
		{
			RuntimeException exception = new RuntimeException(builder.toString());
			exception.setStackTrace(new StackTraceElement[] {});

			// Suspend the Corona runtime.
			CoronaActivity activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();
			if (activity != null) {
				android.os.Handler handler = activity.getHandler();
				if (handler != null) {
					handler.postDelayed(new Runnable() {
						@Override
						public void run() {
							if (fController != null) {
								fController.stop();
							}
						}
					}, 10);
				}
			}

			// Display an alert dialog stating the Lua error.
			reportError(errorMessage, exception);
		}

		// Return the error message that was originally given to this function.
		return 1;
	}
	
	/**
	 * Displays the given error message in an alert dialog and then terminates this application
	 * by throwing the given runtime exception.
	 * @param errorMessage The error message to be displayed. Cannot be null or empty.
	 * @param exception The exception to be thrown to terminate this application after the user
	 *                  has cleared the alert dialog. Cannot be null.
	 */
	private void reportError(final String errorMessage, final RuntimeException exception) {
		// Validate argument.
		if (exception == null) {
			throw new NullPointerException();
		}
		if ((errorMessage == null) || (errorMessage.length() <= 0)) {
			throw exception;
		}

		// Do not continue if this error handler is already showing an alert dialog for a previous error.
		// This prevents multiple error dialogs from being stacked on top of each other.
		// It's better to show the first error instead since it will likely be the cause of subsequent errors.
		if (fIsShowingError) {
			return;
		}
		
		// Fetch the activity to display the error message in.
		android.app.Activity activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();

		// Terminate this application now if we cannot access the activity.
		if (activity == null) {
			throw exception;
		}
		
		// Display the error in an alert dialog on the UI thread.
		fIsShowingError = true;
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				// Do not continue if the activity has already been destroyed.
				// This can happen if the Lua error occurred during an application exit.
				android.app.Activity activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();
				if (activity == null) {
					throw exception;
				}
				
				// Set up a listener for the dialog to exit the activity since the app may be in a bad state.
				android.content.DialogInterface.OnCancelListener cancelListener;
				cancelListener = new android.content.DialogInterface.OnCancelListener() {
					public void onCancel(android.content.DialogInterface dialog) {
						throw exception;
					}
				};
				
				// Display an alert dialog.
				android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(activity);
				builder.setTitle("Runtime Error");
				builder.setMessage(errorMessage);
				builder.setOnCancelListener(cancelListener);
				android.app.AlertDialog dialog = builder.create();
				dialog.setCanceledOnTouchOutside(false);
				dialog.show();
			}
		});
	}

	/**
	 * Converts the given Lua stack trace array to a single string.
	 * @param elements Lua stack trace array received from a LuaError.getLuaStackTrace() method call.
	 * @return Returns a string containing the entire Lua stack trace given.
	 *         <p>
	 *         Returns null if the given argument is null or is an empty array.
	 */
	private String getStackTraceFrom(com.naef.jnlua.LuaStackTraceElement[] elements) {
		// Validate.
		if ((elements == null) || (elements.length <= 0)) {
			return null;
		}

		// Return the given array of Lua stack elements as a single string.
		StringBuilder builder = new StringBuilder();
		builder.append("Lua Stack Trace:");
		for (com.naef.jnlua.LuaStackTraceElement element : elements) {
			builder.append("\n\t");
			builder.append(element.toString());
		}
		return builder.toString();
	}

	/**
	 * Gets an exception stack trace from the given Java exception object.
	 * @param ex Reference to the Java exception. Can be null.
	 * @return Returns a string containing the entire Java exception stack trace.
	 *         <p>
	 *         Returns null if given a null argument or if the exception does not contain a stack trace.
	 */
	private String getStackTraceFrom(Throwable ex) {
		// Validate.
		if ((ex == null) || (ex.getStackTrace() == null) || (ex.getStackTrace().length <= 0)) {
			return null;
		}

		// Return the given exception's stack trace as a single string.
		StringBuilder builder = new StringBuilder();
		builder.append(com.naef.jnlua.LuaError.JAVA_STACK_TRACE_HEADER_MESSAGE);
		for (StackTraceElement element : ex.getStackTrace()) {
			builder.append("\n\t");
			builder.append(element.toString());
		}
		return builder.toString();
	}
}
