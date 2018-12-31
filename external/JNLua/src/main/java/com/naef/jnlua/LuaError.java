/*
 * $Id: LuaError.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua;

/**
 * Contains information about a Lua error condition. This object is created in
 * the native library.
 */
public class LuaError {
	/** Header used by the toString() method to denote a Java exception stack trace. */
	public static final String JAVA_STACK_TRACE_HEADER_MESSAGE = "\nJava Stack Trace:";


	// -- State
	private String message;
	private LuaStackTraceElement[] luaStackTrace;
	private Throwable cause;


	// -- Construction
	/**
	 * Creates a new instance.
	 */
	public LuaError(String message, Throwable cause) {
		this.message = message;
		this.cause = cause;
	}

	// -- Properties
	/**
	 * Returns the message.
	 */
	public String getMessage() {
		return message;
	}

	/**
	 * Returns the Lua stack trace.
	 */
	public LuaStackTraceElement[] getLuaStackTrace() {
		return luaStackTrace;
	}

	/**
	 * Returns the cause.
	 */
	public Throwable getCause() {
		return cause;
	}

	// -- Object methods
	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();

		// Add the Lua error message first.
		if (message != null) {
			builder.append(message);
		}

		// Next, append the exception's message and stack trace.
		if (cause != null) {
			if (builder.length() > 0) {
				builder.append("\n");
			}
			builder.append(cause.toString());
			if (cause.getStackTrace().length > 0) {
				builder.append(JAVA_STACK_TRACE_HEADER_MESSAGE);
				for (StackTraceElement element : cause.getStackTrace()) {
					builder.append("\n\t");
					builder.append(element.toString());
				}
			}
		}

		// Return the error string.
		return builder.toString();
	}

	// -- Package private methods
	/**
	 * Sets the Lua stack trace.
	 */
	void setLuaStackTrace(LuaStackTraceElement[] luaStackTrace) {
		this.luaStackTrace = luaStackTrace;
	}
}
