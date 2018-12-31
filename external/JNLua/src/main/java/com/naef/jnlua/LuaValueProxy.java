/*
 * $Id: LuaValueProxy.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua;

/**
 * Provides proxy access to a Lua value from Java. Lua value proxies are
 * acquired by invoking one of the <code>getProxy()</code> methods on the Lua
 * state.
 * 
 * @see LuaState#getProxy(int)
 * @see LuaState#getProxy(int, Class)
 * @see LuaState#getProxy(int, Class[])
 */
public interface LuaValueProxy {
	/**
	 * Returns the Lua state of this proxy.
	 * 
	 * @return the Lua state
	 */
	public LuaState getLuaState();

	/**
	 * Pushes the proxied Lua value on the stack of the Lua state.
	 */
	public void pushValue();
}
