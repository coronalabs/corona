/*
 * $Id: AbstractLuaTest.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua.test;

import org.junit.After;
import org.junit.Before;

import com.naef.jnlua.LuaState;

/**
 * Abstract base class for JNLua unit tests.
 */
public abstract class AbstractLuaTest {
	// ---- State
	protected LuaState luaState;

	// ---- Setup
	/**
	 * Performs setup.
	 */
	@Before
	public void setup() throws Exception {
		luaState = new LuaState();
	}

	/**
	 * Performs teardown.
	 */
	@After
	public void teardown() throws Throwable {
		if (luaState != null) {
			try {
				luaState.close();
			} catch (Throwable e) {
				e.printStackTrace();
				throw e;
			}
		}
	}
}
