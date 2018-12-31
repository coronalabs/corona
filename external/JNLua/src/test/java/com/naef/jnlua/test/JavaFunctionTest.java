/*
 * $Id: JavaFunctionTest.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua.test;

import static org.junit.Assert.assertEquals;

import org.junit.Test;

import com.naef.jnlua.JavaFunction;
import com.naef.jnlua.LuaState;

/**
 * Contains unit tests for Java functions.
 */
public class JavaFunctionTest extends AbstractLuaTest {
	// ---- Test cases
	/**
	 * Tests the call of a Lua function implemented in Java.
	 */
	@Test
	public void testLuaFunction() throws Exception {
		// Push function
		luaState.pushJavaFunction(new Add());

		// Push arguments
		luaState.pushNumber(1);
		luaState.pushNumber(1);
		luaState.call(2, 1);

		// Test result
		assertEquals(2.0, luaState.toNumber(1), 0.0);
		luaState.pop(1);

		// Finish
		assertEquals(0, luaState.getTop());
	}

	// -- Private classes
	/**
	 * A simple Lua function.
	 */
	private static class Add implements JavaFunction {
		public int invoke(LuaState luaState) {
			double a = luaState.toNumber(1);
			double b = luaState.toNumber(2);
			luaState.setTop(0);
			luaState.pushNumber(a + b);
			return 1;
		}
	}
}