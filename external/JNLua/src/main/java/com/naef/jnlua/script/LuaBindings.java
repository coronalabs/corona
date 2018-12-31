/*
 * $Id: LuaBindings.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua.script;

import javax.script.Bindings;

import com.naef.jnlua.LuaState;
import com.naef.jnlua.util.AbstractTableMap;

/**
 * Lua bindings implementation conforming to JSR 223: Scripting for the Java
 * Platform.
 */
class LuaBindings extends AbstractTableMap<String> implements Bindings {
	// -- State
	private LuaScriptEngine scriptEngine;

	// -- Construction
	public LuaBindings(LuaScriptEngine scriptEngine) {
		this.scriptEngine = scriptEngine;
	}

	// -- AbstractTableMap methods
	@Override
	protected void checkKey(Object key) {
		super.checkKey(key);
		if (!(key instanceof String)) {
			throw new IllegalArgumentException("key must be a string");
		}
		if (((String) key).length() == 0) {
			throw new IllegalArgumentException("key must not be empty");
		}
	}

	@Override
	protected boolean filterKeys() {
		return true;
	}

	@Override
	protected boolean acceptKey(int index) {
		return getLuaState().isString(index)
				&& getLuaState().toString(index).length() > 0;
	}

	@Override
	protected String convertKey(int index) {
		return getLuaState().toString(index);
	}

	// -- LuaProxy methods
	@Override
	public LuaState getLuaState() {
		return scriptEngine.getLuaState();
	}

	@Override
	public void pushValue() {
		getLuaState().pushValue(LuaState.GLOBALSINDEX);
	}

	// -- Package-private methods
	/**
	 * Returns the script engine.
	 */
	LuaScriptEngine getScriptEngine() {
		return scriptEngine;
	}
}
