/*
 * $Id: LuaStateTest.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua.test;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.junit.Test;

import com.naef.jnlua.JavaFunction;
import com.naef.jnlua.LuaRuntimeException;
import com.naef.jnlua.LuaState;
import com.naef.jnlua.LuaType;
import com.naef.jnlua.LuaValueProxy;
import com.naef.jnlua.NamedJavaFunction;

/**
 * Contains unit tests for the Lua state.
 */
public class LuaStateTest extends AbstractLuaTest {
	// -- State
	private JavaFunction javaFunction;
	private Object object;

	// ---- Test cases
	/**
	 * Tests the life-cycle methods.
	 */
	@Test
	public void testLifeCycle() throws Exception {
		assertTrue(luaState.isOpen());
		int kilobytes = luaState.gc(LuaState.GcAction.COUNT, 0);
		assertTrue(kilobytes > 0);
		luaState.close();
		assertFalse(luaState.isOpen());
	}

	/**
	 * Tests the registration methods.
	 */
	@Test
	public void testRegistration() throws Exception {
		// openLib()
		testOpenLib(LuaState.Library.BASE, "coroutine");
		testOpenLib(LuaState.Library.TABLE, "table");
		testOpenLib(LuaState.Library.IO, "io");
		testOpenLib(LuaState.Library.OS, "os");
		testOpenLib(LuaState.Library.STRING, "string");
		testOpenLib(LuaState.Library.MATH, "math");
		testOpenLib(LuaState.Library.DEBUG, "debug");
		testOpenLib(LuaState.Library.PACKAGE, "package");
		testOpenLib(LuaState.Library.JAVA, "java");

		// openLibs()
		LuaState newLuaState = new LuaState();
		newLuaState.getGlobal("table");
		assertEquals(LuaType.NIL, newLuaState.type(-1));
		newLuaState.pop(1);
		newLuaState.openLibs();
		newLuaState.getGlobal("table");
		assertEquals(LuaType.TABLE, newLuaState.type(-1));
		newLuaState.pop(1);
		newLuaState.close();

		// register(NamedJavaFunction)
		NamedJavaFunction javaFunction = new SimpleJavaFunction();
		luaState.register(javaFunction);
		luaState.getGlobal("test");
		assertEquals(LuaType.FUNCTION, luaState.type(-1));
		luaState.pop(1);

		// register(String, NamedJavaFunction[])
		luaState.register("testlib", new NamedJavaFunction[] { javaFunction });
		assertEquals(LuaType.TABLE, luaState.type(-1));
		luaState.pop(1);
		luaState.getGlobal("testlib");
		assertEquals(LuaType.TABLE, luaState.type(-1));
		luaState.pop(1);

		// Finish
		assertEquals(0, luaState.getTop());
	}

	/**
	 * Tests the load and dump methods.
	 */
	@Test
	public void testLoadAndDump() throws Exception {
		InputStream inputStream = new ByteArrayInputStream("a = {}"
				.getBytes("UTF-8"));
		// load(InputStream)
		luaState.load(inputStream, "test1");
		luaState.call(0, 0);
		luaState.getGlobal("a");
		assertEquals(LuaType.TABLE, luaState.type(-1));
		luaState.pop(1);

		// load(String)
		luaState.load("b = 2", "test2");
		luaState.call(0, 0);
		luaState.getGlobal("b");
		assertEquals(LuaType.NUMBER, luaState.type(-1));
		luaState.pop(1);

		// dump()
		luaState.load("c = 3", "test3");
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		luaState.dump(out);
		byte[] bytes = out.toByteArray();
		assertTrue(bytes.length > 4);
		assertEquals((byte) 27, bytes[0]);
		assertEquals((byte) 'L', bytes[1]);
		assertEquals((byte) 'u', bytes[2]);
		assertEquals((byte) 'a', bytes[3]);
		luaState.pop(1);

		// Finish
		assertEquals(0, luaState.getTop());
	}

	/**
	 * Tests the call method.
	 */
	@Test
	public void testCall() throws Exception {
		// call()
		luaState.load("function add(a, b) return a + b end", "test");
		luaState.call(0, 0);
		luaState.getGlobal("add");
		luaState.pushInteger(1);
		luaState.pushInteger(1);
		luaState.call(2, 1);
		assertEquals(2, luaState.toInteger(-1));
		luaState.pop(1);

		// Finish
		assertEquals(0, luaState.getTop());
	}

	/**
	 * Tests the global methods.
	 */
	@Test
	public void testGlobal() throws Exception {
		// setGlobal()
		luaState.pushNumber(1);
		luaState.setGlobal("a");

		// getGlobal()
		luaState.getGlobal("a");
		assertEquals(LuaType.NUMBER, luaState.type(-1));
		luaState.pop(1);

		// Finish
		assertEquals(0, luaState.getTop());
	}

	/**
	 * Tests the stack push methods.
	 */
	@Test
	public void testStackPush() throws Exception {
		// pushBoolean()
		luaState.pushBoolean(true);
		assertEquals(LuaType.BOOLEAN, luaState.type(1));
		luaState.pop(1);

		// pushInteger()
		luaState.pushInteger(1);
		assertEquals(LuaType.NUMBER, luaState.type(1));
		luaState.pop(1);

		// pushJavaFunction()
		JavaFunction javaFunction = new SimpleJavaFunction();
		luaState.pushJavaFunction(javaFunction);
		assertEquals(LuaType.FUNCTION, luaState.type(1));
		assertSame(javaFunction, luaState.toJavaFunction(1));
		luaState.pop(1);

		// pushJavaObject()
		luaState.pushJavaObject(null);
		assertEquals(LuaType.NIL, luaState.type(1));
		assertNull(luaState.toJavaObject(1, Object.class));
		luaState.pop(1);

		luaState.pushJavaObject(Boolean.FALSE);
		assertEquals(LuaType.BOOLEAN, luaState.type(1));
		assertEquals(Boolean.FALSE, luaState.toJavaObject(1, Boolean.class));
		luaState.pop(1);

		luaState.pushJavaObject(Double.valueOf(1.0));
		assertEquals(LuaType.NUMBER, luaState.type(1));
		assertEquals(Double.valueOf(1.0), luaState
				.toJavaObject(1, Double.class));
		luaState.pop(1);

		luaState.pushJavaObject("test");
		assertEquals(LuaType.STRING, luaState.type(1));
		assertEquals("test", luaState.toJavaObject(1, String.class));
		luaState.pop(1);

		Object obj = new Object();
		luaState.pushJavaObjectRaw(obj);
		assertEquals(LuaType.USERDATA, luaState.type(1));
		assertSame(obj, luaState.toJavaObjectRaw(1));
		luaState.pop(1);

		// pushNil()
		luaState.pushNil();
		assertEquals(LuaType.NIL, luaState.type(1));
		luaState.pop(1);

		// pushNumber()
		luaState.pushNumber(1);
		assertEquals(LuaType.NUMBER, luaState.type(1));
		luaState.pop(1);

		// Finish
		assertEquals(0, luaState.getTop());
	}

	/**
	 * Tests the stack type test methods.
	 */
	@Test
	public void testStackTypeTest() throws Exception {
		// Setup stack
		luaState.openLibs();
		makeStack();

		// isBoolean()
		assertFalse(luaState.isBoolean(1));
		assertTrue(luaState.isBoolean(2));
		assertFalse(luaState.isBoolean(3));
		assertFalse(luaState.isBoolean(4));
		assertFalse(luaState.isBoolean(5));
		assertFalse(luaState.isBoolean(6));
		assertFalse(luaState.isBoolean(7));
		assertFalse(luaState.isBoolean(8));
		assertFalse(luaState.isBoolean(9));
		assertFalse(luaState.isBoolean(10));
		assertFalse(luaState.isBoolean(11));

		// isCFunction()
		assertFalse(luaState.isCFunction(1));
		assertFalse(luaState.isCFunction(2));
		assertFalse(luaState.isCFunction(3));
		assertFalse(luaState.isCFunction(4));
		assertFalse(luaState.isCFunction(5));
		assertFalse(luaState.isCFunction(6));
		assertFalse(luaState.isCFunction(7));
		assertFalse(luaState.isCFunction(8));
		assertTrue(luaState.isCFunction(9));
		assertFalse(luaState.isCFunction(10));
		assertFalse(luaState.isCFunction(11));

		// isFunction()
		assertFalse(luaState.isFunction(1));
		assertFalse(luaState.isFunction(2));
		assertFalse(luaState.isFunction(3));
		assertFalse(luaState.isFunction(4));
		assertFalse(luaState.isFunction(5));
		assertFalse(luaState.isFunction(6));
		assertTrue(luaState.isFunction(7));
		assertFalse(luaState.isFunction(8));
		assertTrue(luaState.isFunction(9));
		assertTrue(luaState.isFunction(10));
		assertFalse(luaState.isFunction(11));

		// isJavaFunction()
		assertFalse(luaState.isJavaFunction(1));
		assertFalse(luaState.isJavaFunction(2));
		assertFalse(luaState.isJavaFunction(3));
		assertFalse(luaState.isJavaFunction(4));
		assertFalse(luaState.isJavaFunction(5));
		assertFalse(luaState.isJavaFunction(6));
		assertTrue(luaState.isJavaFunction(7));
		assertFalse(luaState.isJavaFunction(8));
		assertFalse(luaState.isJavaFunction(9));
		assertFalse(luaState.isJavaFunction(10));
		assertFalse(luaState.isJavaFunction(11));

		// isJavaObject(int)
		assertFalse(luaState.isJavaObjectRaw(1));
		assertFalse(luaState.isJavaObjectRaw(2));
		assertFalse(luaState.isJavaObjectRaw(3));
		assertFalse(luaState.isJavaObjectRaw(4));
		assertFalse(luaState.isJavaObjectRaw(5));
		assertFalse(luaState.isJavaObjectRaw(6));
		assertFalse(luaState.isJavaObjectRaw(7));
		assertTrue(luaState.isJavaObjectRaw(8));
		assertFalse(luaState.isJavaObjectRaw(9));
		assertFalse(luaState.isJavaObjectRaw(10));
		assertFalse(luaState.isJavaObjectRaw(11));

		// isJavaObject(int, Class)
		assertTrue(luaState.isJavaObject(1, Object.class));
		assertTrue(luaState.isJavaObject(2, Boolean.class));
		assertTrue(luaState.isJavaObject(3, Double.class));
		assertTrue(luaState.isJavaObject(4, String.class));
		assertTrue(luaState.isJavaObject(5, String.class));
		assertTrue(luaState.isJavaObject(6, Map.class));
		assertTrue(luaState.isJavaObject(7, JavaFunction.class));
		assertTrue(luaState.isJavaObject(8, Object.class));
		assertTrue(luaState.isJavaObject(9, LuaValueProxy.class));
		assertTrue(luaState.isJavaObject(10, LuaValueProxy.class));

		// isNil()
		assertTrue(luaState.isNil(1));
		assertFalse(luaState.isNil(2));
		assertFalse(luaState.isNil(3));
		assertFalse(luaState.isNil(4));
		assertFalse(luaState.isNil(5));
		assertFalse(luaState.isNil(6));
		assertFalse(luaState.isNil(7));
		assertFalse(luaState.isNil(8));
		assertFalse(luaState.isNil(9));
		assertFalse(luaState.isNil(10));
		assertFalse(luaState.isNil(11));

		// isNone()
		assertFalse(luaState.isNone(1));
		assertFalse(luaState.isNone(2));
		assertFalse(luaState.isNone(3));
		assertFalse(luaState.isNone(4));
		assertFalse(luaState.isNone(5));
		assertFalse(luaState.isNone(6));
		assertFalse(luaState.isNone(7));
		assertFalse(luaState.isNone(8));
		assertFalse(luaState.isNone(9));
		assertFalse(luaState.isNone(10));
		assertTrue(luaState.isNone(11));

		// isNoneOrNil()
		assertTrue(luaState.isNoneOrNil(1));
		assertFalse(luaState.isNoneOrNil(2));
		assertFalse(luaState.isNoneOrNil(3));
		assertFalse(luaState.isNoneOrNil(4));
		assertFalse(luaState.isNoneOrNil(5));
		assertFalse(luaState.isNoneOrNil(6));
		assertFalse(luaState.isNoneOrNil(7));
		assertFalse(luaState.isNoneOrNil(8));
		assertFalse(luaState.isNoneOrNil(9));
		assertFalse(luaState.isNoneOrNil(10));
		assertTrue(luaState.isNoneOrNil(11));

		// isNumber()
		assertFalse(luaState.isNumber(1));
		assertFalse(luaState.isNumber(2));
		assertTrue(luaState.isNumber(3));
		assertFalse(luaState.isNumber(4));
		assertTrue(luaState.isNumber(5));
		assertFalse(luaState.isNumber(6));
		assertFalse(luaState.isNumber(7));
		assertFalse(luaState.isNumber(8));
		assertFalse(luaState.isNumber(9));
		assertFalse(luaState.isNumber(10));
		assertFalse(luaState.isNumber(11));

		// isString()
		assertFalse(luaState.isString(1));
		assertFalse(luaState.isString(2));
		assertTrue(luaState.isString(3));
		assertTrue(luaState.isString(4));
		assertTrue(luaState.isString(5));
		assertFalse(luaState.isString(6));
		assertFalse(luaState.isString(7));
		assertFalse(luaState.isString(8));
		assertFalse(luaState.isString(9));
		assertFalse(luaState.isString(10));
		assertFalse(luaState.isString(10));

		// isTable()
		assertFalse(luaState.isTable(1));
		assertFalse(luaState.isTable(2));
		assertFalse(luaState.isTable(3));
		assertFalse(luaState.isTable(4));
		assertFalse(luaState.isTable(5));
		assertTrue(luaState.isTable(6));
		assertFalse(luaState.isTable(7));
		assertFalse(luaState.isTable(8));
		assertFalse(luaState.isTable(9));
		assertFalse(luaState.isTable(10));
		assertFalse(luaState.isTable(11));

		// Finish
		luaState.pop(10);
		assertEquals(0, luaState.getTop());
	}

	/**
	 * Tests the stack query methods.
	 */
	@Test
	public void testStackQuery() throws Exception {
		// Setup stack
		luaState.openLibs();
		makeStack();
		luaState.pushInteger(-1);
		luaState.pushInteger(0);
		luaState.pushInteger(1);

		// equal()
		for (int i = 1; i <= 10; i++) {
			for (int j = 1; j <= 10; j++) {
				if (i == j) {
					assertTrue(String.format("%d, %d", i, j), luaState.equal(i,
							j));
				} else {
					assertFalse(String.format("%d, %d", i, j), luaState.equal(
							i, j));
				}
			}
		}
		assertFalse(luaState.equal(20, 21));
		

		// lessThan()
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				assertEquals(i < j,
						luaState.lessThan(i - 3, j - 3));
			}
		}
		assertFalse(luaState.lessThan(20, 21));

		// length()
		assertEquals(0, luaState.length(1));
		assertEquals(0, luaState.length(2));
		// assertEquals(0, luaState.length(3)); -> would change type
		assertEquals(4, luaState.length(4));
		assertEquals(1, luaState.length(5));
		assertEquals(1, luaState.length(6));
		assertEquals(0, luaState.length(7));
		assertTrue(luaState.length(8) == 4 || luaState.length(8) == 8);
		assertEquals(0, luaState.length(9));
		assertEquals(0, luaState.length(10));

		// rawEqual()
		for (int i = 1; i <= 10; i++) {
			for (int j = 1; j <= 10; j++) {
				if (i == j) {
					assertTrue(String.format("%d, %d", i, j), luaState
							.rawEqual(i, j));
				} else {
					assertFalse(String.format("%d, %d", i, j), luaState
							.rawEqual(i, j));
				}
			}
		}

		// toBoolean()
		assertFalse(luaState.toBoolean(1));
		assertFalse(luaState.toBoolean(2));
		assertTrue(luaState.toBoolean(3));
		assertTrue(luaState.toBoolean(4));
		assertTrue(luaState.toBoolean(5));
		assertTrue(luaState.toBoolean(6));
		assertTrue(luaState.toBoolean(7));
		assertTrue(luaState.toBoolean(8));
		assertTrue(luaState.toBoolean(9));
		assertTrue(luaState.toBoolean(10));

		// toInteger()
		assertEquals(0, luaState.toInteger(1));
		assertEquals(0, luaState.toInteger(2));
		assertEquals(1, luaState.toInteger(3));
		assertEquals(0, luaState.toInteger(4));
		assertEquals(1, luaState.toInteger(5));
		assertEquals(0, luaState.toInteger(6));
		assertEquals(0, luaState.toInteger(7));
		assertEquals(0, luaState.toInteger(8));
		assertEquals(0, luaState.toInteger(9));
		assertEquals(0, luaState.toInteger(10));

		// toJavaFunction()
		assertNull(luaState.toJavaFunction(1));
		assertNull(luaState.toJavaFunction(2));
		assertNull(luaState.toJavaFunction(3));
		assertNull(luaState.toJavaFunction(4));
		assertNull(luaState.toJavaFunction(5));
		assertNull(luaState.toJavaFunction(6));
		//assertSame(javaFunction, luaState.toJavaFunction(7));
		assertNull(luaState.toJavaFunction(8));
		assertNull(luaState.toJavaFunction(9));
		assertNull(luaState.toJavaFunction(10));

		// toJavaObject(int)
		assertNull(luaState.toJavaObjectRaw(1));
		assertNull(luaState.toJavaObjectRaw(2));
		assertNull(luaState.toJavaObjectRaw(3));
		assertNull(luaState.toJavaObjectRaw(4));
		assertNull(luaState.toJavaObjectRaw(5));
		assertNull(luaState.toJavaObjectRaw(6));
		assertNull(luaState.toJavaObjectRaw(7));
		//assertSame(object, luaState.toJavaObjectRaw(8));
		assertNull(luaState.toJavaObjectRaw(9));
		assertNull(luaState.toJavaObjectRaw(10));

		// toJavaObject(int, Class)
		assertNull(luaState.toJavaObject(1, Object.class));
		assertEquals(Boolean.FALSE, luaState.toJavaObject(2, Boolean.class));
		assertEquals(Double.valueOf(1.0), luaState
				.toJavaObject(3, Double.class));
		assertEquals("test", luaState.toJavaObject(4, String.class));
		assertEquals("1", luaState.toJavaObject(5, String.class));
		assertArrayEquals(new Double[] { 1.0 }, luaState.toJavaObject(6,
				Double[].class));
		//assertSame(javaFunction, luaState.toJavaObject(7, JavaFunction.class));
		//assertSame(object, luaState.toJavaObject(8, Object.class));
		assertTrue(luaState.toJavaObject(9, Object.class) instanceof LuaValueProxy);
		assertTrue(luaState.toJavaObject(10, Object.class) instanceof LuaValueProxy);

		// toNumber()
		assertEquals(0.0, luaState.toNumber(1), 0.0);
		assertEquals(0.0, luaState.toNumber(2), 0.0);
		assertEquals(1.0, luaState.toNumber(3), 0.0);
		assertEquals(0.0, luaState.toNumber(4), 0.0);
		assertEquals(1.0, luaState.toNumber(5), 0.0);
		assertEquals(0.0, luaState.toNumber(6), 0.0);
		assertEquals(0.0, luaState.toNumber(7), 0.0);
		assertEquals(0.0, luaState.toNumber(8), 0.0);
		assertEquals(0.0, luaState.toNumber(9), 0.0);
		assertEquals(0.0, luaState.toNumber(10), 0.0);

		// toPointer()
		assertEquals(0L, luaState.toPointer(1));
		assertEquals(0L, luaState.toPointer(2));
		assertEquals(0L, luaState.toPointer(3));
		assertEquals(0L, luaState.toPointer(4));
		assertEquals(0L, luaState.toPointer(5));
		assertTrue(luaState.toPointer(6) != 0L);
		assertTrue(luaState.toPointer(7) != 0L);
		assertTrue(luaState.toPointer(8) != 0L);
		assertTrue(luaState.toPointer(9) != 0L);
		assertTrue(luaState.toPointer(10) != 0L);

		// toString()
		assertNull(luaState.toString(1));
		assertNull(luaState.toString(2));
		assertEquals("1", luaState.toString(3));
		assertEquals("test", luaState.toString(4));
		assertEquals("1", luaState.toString(5));
		assertNull(luaState.toString(6));
		assertNull(luaState.toString(7));
		assertNull(luaState.toString(8));
		assertNull(luaState.toString(9));
		assertNull(luaState.toString(10));

		// Finish
		luaState.pop(13);
		assertEquals(0, luaState.getTop());
	}

	/**
	 * Tests the stack operation methods.
	 */
	@Test
	public void testStackOperation() throws Exception {
		// concat()
		luaState.pushString("a");
		luaState.pushString("b");
		luaState.concat(2);
		assertEquals("ab", luaState.toString(1));
		luaState.pop(1);

		// getTop()
		assertEquals(0, luaState.getTop());
		luaState.pushNumber(1);
		assertEquals(1, luaState.getTop());

		// insert()
		luaState.pushNumber(2);
		luaState.insert(1);
		assertArrayEquals(new Object[] { 2.0, 1.0 }, getStack());

		// pop()
		luaState.pop(1);
		assertArrayEquals(new Object[] { 2.0 }, getStack());

		// pushValue()
		luaState.pushValue(1);
		assertArrayEquals(new Object[] { 2.0, 2.0 }, getStack());

		// remove()
		luaState.remove(1);
		assertArrayEquals(new Object[] { 2.0 }, getStack());

		// replace()
		luaState.pushNumber(1);
		luaState.replace(1);
		assertArrayEquals(new Object[] { 1.0 }, getStack());

		// setTop()
		luaState.setTop(0);
		assertEquals(0, luaState.getTop());

		// type()
		luaState.pushBoolean(true);
		luaState.pushJavaObject(new Object());
		assertEquals(LuaType.BOOLEAN, luaState.type(1));
		assertEquals(LuaType.USERDATA, luaState.type(2));

		// Finish
		luaState.pop(2);
		assertEquals(0, luaState.getTop());
	}

	/**
	 * Tests the table methods.
	 */
	@Test
	public void testTable() throws Exception {
		// newTable()
		luaState.newTable();
		assertEquals(LuaType.TABLE, luaState.type(1));
		luaState.pop(1);

		// newTable(int, int)
		luaState.newTable(1, 1);
		assertEquals(LuaType.TABLE, luaState.type(1));

		// next()
		luaState.pushString("value");
		luaState.setField(1, "key");
		luaState.pushNil();
		assertTrue(luaState.next(1));
		assertEquals("key", luaState.toString(-2));
		assertEquals("value", luaState.toString(-1));
		luaState.pop(1);
		assertFalse(luaState.next(1));

		// rawSet|Get(int)
		luaState.pushInteger(1);
		luaState.pushNumber(10);
		luaState.rawSet(1);
		luaState.pushInteger(1);
		luaState.rawGet(1);
		assertEquals(10.0, luaState.toNumber(-1), 0.0);
		luaState.pop(1);

		// rawSet|Get(int, int)
		luaState.pushNumber(20);
		luaState.rawSet(1, 2);
		luaState.rawGet(1, 2);
		assertEquals(20.0, luaState.toNumber(-1), 0.0);
		luaState.pop(1);

		// set|getTable()
		luaState.pushString("key1");
		luaState.pushString("value1");
		luaState.setTable(1);
		luaState.pushString("key1");
		luaState.getTable(1);
		assertEquals("value1", luaState.toString(-1));
		luaState.pop(1);

		// set|getField()
		luaState.pushString("value2");
		luaState.setField(1, "key2");
		luaState.getField(1, "key2");
		assertEquals("value2", luaState.toString(-1));
		luaState.pop(1);

		// Finish
		luaState.pop(1);
		assertEquals(0, luaState.getTop());
	}

	/**
	 * Tests the meta table methods.
	 */
	@Test
	public void testMetaTable() throws Exception {
		// setMetaTable()
		luaState.newTable();
		luaState.newTable();
		assertTrue(luaState.setMetatable(1));

		// getMetaTable()
		assertTrue(luaState.getMetatable(1));
		assertEquals(LuaType.TABLE, luaState.type(-1));
		luaState.pop(1);

		// Finish
		luaState.pop(1);
		assertEquals(0, luaState.getTop());
	}

	/**
	 * Tests the environment table methods.
	 */
	@Test
	public void testEnvironmentTable() throws Exception {
		// getMetaTable()
		luaState.load("function a() end", "test");
		luaState.call(0, 0);

		// getFEnv()
		luaState.getGlobal("a");
		luaState.getFEnv(1);
		assertEquals(LuaType.TABLE, luaState.type(-1));
		luaState.pop(1);

		// setFEnv()
		luaState.newTable();
		assertTrue(luaState.setFEnv(1));

		// Finish
		luaState.pop(1);
		assertEquals(0, luaState.getTop());
	}

	/**
	 * Tests the thread methods.
	 */
	@Test
	public void testThread() throws Exception {
		// Create thread
		luaState.openLibs();
		luaState.register(new NamedJavaFunction() {
			public int invoke(LuaState luaState) {
				luaState.pushInteger(luaState.toInteger(1));
				return luaState.yield(1);
			}

			public String getName() {
				return "yieldfunc";
			}
		});
		luaState.load("function run(n)\n" + "yieldfunc(n + 1)\n"
				+ "coroutine.yield(n + 2)\n" + "return n + 3\n" + "end",
				"threadtest");
		luaState.call(0, 0);
		luaState.getGlobal("run");
		luaState.newThread();

		// Start
		luaState.pushInteger(1);
		assertEquals(1, luaState.resume(1, 1));
		assertEquals(LuaState.YIELD, luaState.status(1));
		assertEquals(2, luaState.getTop());
		assertEquals(2, luaState.toInteger(-1));
		luaState.pop(1);

		// Resume
		assertEquals(1, luaState.resume(1, 0));
		assertEquals(LuaState.YIELD, luaState.status(1));
		assertEquals(2, luaState.getTop());
		assertEquals(3, luaState.toInteger(-1));
		luaState.pop(1);

		// Resume
		assertEquals(1, luaState.resume(1, 0));
		assertEquals(0, luaState.status(1));
		assertEquals(2, luaState.getTop());
		assertEquals(4, luaState.toInteger(-1));
		luaState.pop(1);

		// Cleanup
		luaState.pop(1);
		assertEquals(0, luaState.getTop());
	}

	/**
	 * Tests the reference functions.
	 */
	@Test
	public void testReference() {
		// Create object
		luaState.newTable();
		luaState.pushString("value");
		luaState.setField(1, "key");

		// Get reference
		int ref = luaState.ref(LuaState.REGISTRYINDEX);

		// Get table back via reference
		luaState.rawGet(LuaState.REGISTRYINDEX, ref);
		luaState.getField(1, "key");
		assertEquals("value", luaState.toString(2));
		luaState.pop(2);

		// Release reference
		luaState.unref(LuaState.REGISTRYINDEX, ref);

		// Finish
		assertEquals(0, luaState.getTop());
	}

	/**
	 * Tests the argument check methods.
	 */
	@Test
	public void testArgumentChecks() {
		// Simple checks
		luaState.openLibs();
		makeStack();
		assertFalse(luaState.checkBoolean(2));
		assertTrue(luaState.checkBoolean(1, true));
		assertEquals(1, luaState.checkInteger(3));
		assertEquals(1, luaState.checkInteger(1, 1));
		assertEquals(1.0, luaState.checkNumber(3), 0.0);
		assertEquals(1.0, luaState.checkNumber(1, 1.0), 0.0);
		assertEquals("test", luaState.checkString(4));
		assertEquals("test", luaState.checkString(1, "test"));
		assertEquals("test", luaState.checkOption(4, new String[] { "a", "b",
				"test" }));
		assertEquals("test", luaState.checkOption(1, new String[] { "a", "b",
				"test" }, "test"));
		luaState.checkType(3, LuaType.NUMBER);
		luaState.checkArg(3, true, "");

		// Exception check 1
		LuaRuntimeException luaRuntimeException = null;
		try {
			luaState.checkArg(3, false, "msg");
		} catch (LuaRuntimeException e) {
			luaRuntimeException = e;
		}
		assertNotNull(luaRuntimeException);
		assertEquals("bad argument #3 (msg)", luaRuntimeException.getMessage());

		// Exception check 2
		luaRuntimeException = null;
		try {
			luaState.checkNumber(4, 1.0);
		} catch (LuaRuntimeException e) {
			luaRuntimeException = e;
		}
		assertNotNull(luaRuntimeException);
		assertEquals("bad argument #4 (expected number, got string)",
				luaRuntimeException.getMessage());

		// Cleanup
		luaState.pop(10);
		assertEquals(0, luaState.getTop());
	}

	/**
	 * Tests the proxy methods.
	 */
	@Test
	public void testProxy() throws Exception {
		// Simple value proxy
		luaState.pushNumber(1.0);
		LuaValueProxy luaProxy = luaState.getProxy(-1);
		luaState.pop(1);
		luaProxy.pushValue();
		assertEquals(1.0, luaState.toNumber(-1), 0.0);
		luaState.pop(1);

		// Implement the runnable interface in Lua
		luaState
				.load("return { run = function () hasRun = true end }", "proxy");
		luaState.call(0, 1);

		// Get the proxy
		Runnable runnable = luaState.getProxy(-1, Runnable.class);
		luaState.pop(1);

		// Let a thread run it
		Thread thread = new Thread(runnable);
		thread.start();
		thread.join();

		// Check execution
		luaState.getGlobal("hasRun");
		assertTrue(luaState.toBoolean(-1));
		luaState.pop(1);

		// Proxy garbage collection
		for (int i = 0; i < 20000; i++) {
			luaState.pushInteger(i);
			luaState.getProxy(-1);
			luaState.pop(1);
		}
		System.gc();

		// Finish
		assertEquals(0, luaState.getTop());
	}

	// -- Private methods
	/**
	 * Tests the opening of a library.
	 */
	private void testOpenLib(LuaState.Library library, String tableName) {
		luaState.getGlobal(tableName);
		assertEquals(LuaType.NIL, luaState.type(-1));
		luaState.pop(1);
		luaState.openLib(library);
		luaState.getGlobal(tableName);
		assertEquals(LuaType.TABLE, luaState.type(-1));
		luaState.pop(1);

	}

	/**
	 * Returns the current stack as Java objects.
	 */
	private Object[] getStack() {
		List<Object> objects = new ArrayList<Object>();
		for (int i = 1; i <= luaState.getTop(); i++) {
			switch (luaState.type(i)) {
			case BOOLEAN:
				objects.add(Boolean.valueOf(luaState.toBoolean(i)));
				break;

			case NUMBER:
				objects.add(Double.valueOf(luaState.toNumber(i)));
				break;

			case STRING:
				objects.add(luaState.toString(i));
			}
		}
		return objects.toArray(new Object[objects.size()]);
	}

	/**
	 * Creates a stack with all types.
	 */
	private void makeStack() {
		luaState.pushNil(); // 1
		luaState.pushBoolean(false); // 2
		luaState.pushNumber(1.0); // 3
		luaState.pushString("test"); // 4
		luaState.pushString("1"); // 5
		luaState.newTable(); // 6
		luaState.pushNumber(1.0);
		luaState.rawSet(6, 1);
		javaFunction = new SimpleJavaFunction();
		luaState.pushJavaFunction(javaFunction); // 7
		object = new Object();
		luaState.pushJavaObject(object); // 8
		luaState.getGlobal("print"); // 9
		luaState.load("function a() end", "test");
		luaState.call(0, 0);
		luaState.getGlobal("a"); // 10
	}

	// -- Private classes
	/**
	 * A simple Lua function.
	 */
	private static class SimpleJavaFunction implements NamedJavaFunction {
		@Override
		public int invoke(LuaState luaState) {
			return 0;
		}

		@Override
		public String getName() {
			return "test";
		}
	}
}
