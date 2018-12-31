/*
 * $Id: LuaException.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua;

/**
 * Abstract base class for Lua error conditions. Lua exceptions are unchecked
 * runtime exceptions.
 */
public abstract class LuaException extends RuntimeException {
	// -- Static
	private static final long serialVersionUID = 1L;

	// -- Construction
	/**
	 * Creates a new instance.
	 * 
	 * @param msg
	 *            the message
	 */
	public LuaException(String msg) {
		super(msg);
	}

	/**
	 * Creates a new instance.
	 * 
	 * @param msg
	 *            the message
	 * @param cause
	 *            the cause of this exception
	 */
	public LuaException(String msg, Throwable cause) {
		super(msg, cause);
	}

	/**
	 * Creates a new instance.
	 * 
	 * @param cause
	 *            the cause of this exception
	 */
	public LuaException(Throwable cause) {
		super(cause);
	}
}
