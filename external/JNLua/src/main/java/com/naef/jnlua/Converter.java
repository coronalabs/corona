/*
 * $Id: Converter.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua;

/**
 * Converts between Lua values and Java objects.
 */
public interface Converter {
	/**
	 * Returns the type distance between a Lua value and a formal Java type.
	 * Distances are comparable for the same Lua value only. If a Lua value
	 * cannot be converted to the specified formal type, the method returns
	 * <code>Integer.MAX_VALUE</code>.
	 * 
	 * @param luaState
	 *            the Lua state
	 * @param index
	 *            the stack index containing the value
	 * @param formalType
	 *            the formal Java type
	 * @return the type distance, or <code>Integer.MAX_VALUE</code> if the
	 *         conversion is not supported
	 */
	public int getTypeDistance(LuaState luaState, int index, Class<?> formalType);

	/**
	 * Converts a Lua value to a Java object of the specified formal type.
	 * 
	 * <p>
	 * If the Lua value is <code>nil</code>, the method returns
	 * <code>null</code>.
	 * </p>
	 * 
	 * @param luaState
	 *            the Lua state
	 * @param index
	 *            the stack index containing the value
	 * @return the Java object, or <code>null</code>
	 * @param formalType
	 *            the formal Java type
	 * @throws ClassCastException
	 *             if the conversion is not possible
	 */
	public <T> T convertLuaValue(LuaState luaState, int index,
			Class<T> formalType);

	/**
	 * Converts a Java object to a Lua value and pushes that value on the stack.
	 * 
	 * <p>
	 * If the object is <code>null</code>, the method pushes <code>nil</code>.
	 * </p>
	 * 
	 * @param luaState
	 *            the Lua state
	 * @param object
	 *            the Java object, or <code>null</code>
	 */
	public void convertJavaObject(LuaState luaState, Object object);
}
