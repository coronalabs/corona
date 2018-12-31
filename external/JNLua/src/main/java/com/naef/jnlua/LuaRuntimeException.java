/*
 * $Id: LuaRuntimeException.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua;

import java.io.PrintStream;
import java.io.PrintWriter;

/**
 * Indicates a Lua runtime error.
 * 
 * <p>
 * This exception is thrown if a Lua runtime error occurs, such as indexing a
 * <code>nil</code> value. The class provides access to the Lua stack trace by
 * means of the {@link #getLuaStackTrace()} method.
 * </p>
 */
public class LuaRuntimeException extends LuaException {
	// -- Static
	private static final long serialVersionUID = 1L;
	private static final LuaStackTraceElement[] EMPTY_LUA_STACK_TRACE = new LuaStackTraceElement[0];

	// -- State
	private LuaStackTraceElement[] luaStackTrace;

	// -- Construction
	/**
	 * Creates a new instance. The instance is created with an empty Lua stack
	 * trace.
	 * 
	 * @param msg
	 *            the message
	 */
	public LuaRuntimeException(String msg) {
		super(msg);
		luaStackTrace = EMPTY_LUA_STACK_TRACE;
	}

	/**
	 * Creates a new instance. The instance is created with an empty Lua stack
	 * trace.
	 * 
	 * @param msg
	 *            the message
	 * @param cause
	 *            the cause of this exception
	 */
	public LuaRuntimeException(String msg, Throwable cause) {
		super(msg, cause);
		luaStackTrace = EMPTY_LUA_STACK_TRACE;
	}

	/**
	 * Creates a new instance. The instance is created with an empty Lua stack
	 * trace.
	 * 
	 * @param cause
	 *            the cause of this exception
	 */
	public LuaRuntimeException(Throwable cause) {
		super(cause);
		luaStackTrace = EMPTY_LUA_STACK_TRACE;
	}

	// -- Properties
	/**
	 * Returns the Lua stack trace of this runtime exception.
	 */
	public LuaStackTraceElement[] getLuaStackTrace() {
		return luaStackTrace.clone();
	}

	// -- Operations
	/**
	 * Prints this exception and its Lua stack trace to the standard error
	 * stream.
	 */
	public void printLuaStackTrace() {
		printLuaStackTrace(System.err);
	}

	/**
	 * Prints this exception and its Lua stack trace to the specified print
	 * stream.
	 * 
	 * @param s
	 *            the print stream
	 */
	public void printLuaStackTrace(PrintStream s) {
		synchronized (s) {
			s.println(this);
			for (int i = 0; i < luaStackTrace.length; i++) {
				s.println("\tat " + luaStackTrace[i]);
			}
		}
	}

	/**
	 * Prints this exception and its Lua stack trace to the specified print
	 * writer.
	 * 
	 * @param s
	 *            the print writer
	 */
	public void printLuaStackTrace(PrintWriter s) {
		synchronized (s) {
			s.println(this);
			for (int i = 0; i < luaStackTrace.length; i++) {
				s.println("\tat " + luaStackTrace[i]);
			}
		}
	}

	// -- Package private methods
	/**
	 * Sets the Lua error in this exception. The method in invoked from the
	 * native library.
	 */
	void setLuaError(LuaError luaError) {
		initCause(luaError.getCause());
		luaStackTrace = luaError.getLuaStackTrace();
	}
}
