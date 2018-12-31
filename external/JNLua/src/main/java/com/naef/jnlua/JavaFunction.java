/*
 * $Id: JavaFunction.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua;

/**
 * Provides a Lua function implemented in Java.
 */
public interface JavaFunction {
	/**
	 * Invokes this Java function. The function arguments are on the stack. The
	 * method returns the number of values on the stack which constitute the
	 * return values of this function.
	 * 
	 * <p>
	 * Java functions should indicate application errors by returning
	 * appropriate error codes to the caller. Programming errors should be
	 * indicated by throwing a runtime exception.
	 * </p>
	 * 
	 * @param luaState
	 *            the Lua state this function has been invoked on
	 * @return the number of return values
	 */
	public int invoke(LuaState luaState);
}
