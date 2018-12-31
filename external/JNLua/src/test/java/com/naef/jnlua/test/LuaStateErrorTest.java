/*
 * $Id: LuaStateErrorTest.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua.test;

import static org.junit.Assert.assertEquals;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.junit.Test;

import com.naef.jnlua.JavaFunction;
import com.naef.jnlua.LuaRuntimeException;
import com.naef.jnlua.LuaState;
import com.naef.jnlua.NamedJavaFunction;

/**
 * Throws illegal arguments at the Lua state for error testing.
 */
public class LuaStateErrorTest extends AbstractLuaTest {
	// -- Static
	private static final int HIGH = 10;
	private static final int LOW = -10;
	private static final int EXTREMELY_HIGH = Integer.MAX_VALUE / 8;
	private static final int EXTREMELY_LOW = Integer.MIN_VALUE / 8;

	// ---- Test cases
	/**
	 * Tests invoking a method after the Lua state has been closed.
	 */
	@Test(expected = IllegalStateException.class)
	public void testClosed() {
		luaState.close();
		luaState.pushInteger(1);
	}

	/**
	 * Tests closing the Lua state while running.
	 */
	@Test(expected = LuaRuntimeException.class)
	public void testIllegalClose() {
		luaState.pushJavaFunction(new JavaFunction() {
			@Override
			public int invoke(LuaState luaState) {
				luaState.close();
				return 0;
			}
		});
		luaState.call(0, 0);
	}

	/**
	 * Off-index (low)
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testLowIndex() {
		luaState.toNumber(LOW);
	}

	/**
	 * Off-index (extremely low)
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testExtremelyLowIndex() {
		luaState.toNumber(EXTREMELY_LOW);
	}

	/**
	 * Off-index (high)
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testHighIndex() {
		luaState.toNumber(HIGH);
	}

	/**
	 * Off-index (extremely high)
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testExtremelyHighIndex() {
		luaState.toNumber(EXTREMELY_HIGH);
	}

	/**
	 * gc(GcAction, int) null action
	 */
	@Test(expected = NullPointerException.class)
	public void testNullGc() {
		luaState.gc(null, 0);
	}

	/**
	 * register(JavaFunction[]) with null function.
	 */
	@Test(expected = NullPointerException.class)
	public void testNullFunctionRegister() {
		luaState.register(null);
	}

	/**
	 * register(String, JavaFunction[]) with null string.
	 */
	@Test(expected = NullPointerException.class)
	public void testNullNameRegister() {
		luaState.register(null, new NamedJavaFunction[0]);
	}

	/**
	 * register(String, JavaFunction[]) with null functions.
	 */
	@Test(expected = NullPointerException.class)
	public void testNullFunctionsRegister() {
		luaState.register("", null);
	}

	/**
	 * load(InputStream, String) with null input stream.
	 */
	@Test(expected = NullPointerException.class)
	public void testNullStreamLoad() throws Exception {
		luaState.load((InputStream) null, "");
	}

	/**
	 * load(InputStream, String) with null string.
	 */
	@Test(expected = NullPointerException.class)
	public void testNullChunkLoad1() throws Exception {
		luaState.load(new ByteArrayInputStream(new byte[0]), null);
	}

	/**
	 * load(String, String) with null string 1.
	 */
	@Test(expected = NullPointerException.class)
	public void testNullStringLoad() throws Exception {
		luaState.load((String) null, "");
	}

	/**
	 * load(String, String) with null string 2.
	 */
	@Test(expected = NullPointerException.class)
	public void testNullChunkLoad2() throws Exception {
		luaState.load("", null);
	}

	/**
	 * load(InputStream, String) with input stream throwing IO exception.
	 */
	@Test(expected = IOException.class)
	public void testIoExceptionLoad() throws Exception {
		luaState.load(new InputStream() {
			@Override
			public int read() throws IOException {
				throw new IOException();
			}
		}, "ioExceptionLoad");
	}

	/**
	 * dump(OutputStream) with null output stream.
	 */
	@Test(expected = NullPointerException.class)
	public void testNullDump() throws Exception {
		luaState.load("return 0", "nullDump");
		luaState.dump(null);
	}

	/**
	 * dump(OutputStream) with an output stream throwing a IO exception.
	 */
	@Test(expected = IOException.class)
	public void testIoExceptionDump() throws Exception {
		luaState.load("return 0", "ioExceptionDump");
		luaState.dump(new OutputStream() {
			@Override
			public void write(int b) throws IOException {
				throw new IOException();
			}
		});
	}

	/**
	 * dump(OutputStream) with insufficient arguments.
	 */
	@Test(expected = IllegalStateException.class)
	public void testUnderflowDump() throws Exception {
		luaState.dump(new ByteArrayOutputStream());
	}

	/**
	 * Call(int, int) with insufficient arguments.
	 */
	@Test(expected = IllegalStateException.class)
	public void testUnderflowCall() {
		luaState.openLibs();
		luaState.getGlobal("print");
		luaState.call(1, 1);
	}

	/**
	 * Call(int, int) with an extremely high number of returns.
	 */
	@Test(expected = IllegalStateException.class)
	public void testOverflowCall() {
		luaState.openLibs();
		luaState.getGlobal("print");
		luaState.pushString("");
		luaState.call(1, EXTREMELY_HIGH);
	}

	/**
	 * Call(int, int) with an illegal number of arguments.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalCall1() {
		luaState.openLibs();
		luaState.getGlobal("print");
		luaState.call(-1, 1);
	}

	/**
	 * Call(int, int) with an illegal number of returns.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalCall2() {
		luaState.openLibs();
		luaState.getGlobal("print");
		luaState.pushString("");
		luaState.call(1, -2);
		assertEquals(0, luaState.getTop());
	}

	/**
	 * getGlobal(String) with null.
	 */
	@Test(expected = NullPointerException.class)
	public void testNullGetGlobal() {
		luaState.getGlobal(null);
	}

	/**
	 * setGlobal(String) with null.
	 */
	@Test(expected = NullPointerException.class)
	public void testNullSetGlobal() {
		luaState.pushNumber(0.0);
		luaState.setGlobal(null);
	}

	/**
	 * setGlobal(String) with insufficient arguments.
	 */
	@Test(expected = IllegalStateException.class)
	public void testUnderflowSetGlobal() {
		luaState.setGlobal("global");
	}

	/**
	 * setGlobal(String) with insufficient arguments.
	 */
	@Test(expected = IllegalStateException.class)
	public void testIllegalSetGlobal() {
		luaState.setGlobal("illegal");
	}

	/**
	 * pushJavaFunction(JavaFunction) with null argument.
	 */
	@Test(expected = NullPointerException.class)
	public void testNullPushJavaFunction() {
		luaState.pushJavaFunction(null);
	}

	/**
	 * pushJavaObject(Object) with null argument.
	 */
	@Test(expected = NullPointerException.class)
	public void testNullPushJavaObject() {
		luaState.pushJavaObjectRaw(null);
	}

	/**
	 * pushString(String) with null argument.
	 */
	@Test(expected = NullPointerException.class)
	public void testNullPushString() {
		luaState.pushString(null);
	}

	/**
	 * pushNumber(Double) until stack overflow.
	 */
	@Test(expected = IllegalStateException.class)
	public void testStackOverflow() {
		for (int i = 0; i < EXTREMELY_HIGH; i++) {
			luaState.pushNumber(0.0);
		}
	}

	/**
	 * lessThan(int, int) with illegal types.
	 */
	@Test(expected = LuaRuntimeException.class)
	public void testIllegalLessThan() {
		luaState.pushNil();
		luaState.pushNumber(0.0);
		luaState.lessThan(1, 2);
	}

	/**
	 * length(int) with illegal index.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalLength() {
		luaState.length(getIllegalIndex());
	}

	/**
	 * rawEqual(int, int) with illegal indexes.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalRawEqual() {
		luaState.rawEqual(getIllegalIndex(), getIllegalIndex());
	}

	/**
	 * toBoolean(int) with illegal index.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalToBoolean() {
		luaState.toBoolean(getIllegalIndex());
	}

	/**
	 * concat(int) with insufficient arguments.
	 */
	@Test(expected = IllegalStateException.class)
	public void testUnderflowConcat1() {
		luaState.concat(1);
	}

	/**
	 * concat(int) with insufficient arguments.
	 */
	@Test(expected = IllegalStateException.class)
	public void testUnderflowConcat2() {
		luaState.pushString("");
		luaState.pushString("");
		luaState.concat(3);
	}

	/**
	 * concat(int) with an illegal number of arguments.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalConcat() {
		luaState.concat(-1);
	}

	/**
	 * remove(int) with illegal index.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalRemove() {
		luaState.remove(getIllegalIndex());
	}

	/**
	 * replace(int) with illegal index.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalReplace() {
		luaState.replace(getIllegalIndex());
	}

	/**
	 * insert(int) with illegal index.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalInsert() {
		luaState.insert(getIllegalIndex());
	}

	/**
	 * pop(int) with insufficient arguments.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testUnderflowPop() {
		luaState.pop(1);
	}

	/**
	 * pop(int) with an illegal number of arguments.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalPop() {
		luaState.pop(-1);
	}

	/**
	 * pushValue(int) with illegal index.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalPushValue() {
		luaState.pushValue(getIllegalIndex());
	}

	/**
	 * setTop(int) with an illegal argument.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalSetTop() {
		luaState.setTop(-1);
	}

	/**
	 * getTable(int) with invalid table.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalGetTable() {
		luaState.pushNumber(0.0);
		luaState.pushString("");
		luaState.getTable(1);
	}

	/**
	 * getField(int, String) with invalid table.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalGetField() {
		luaState.pushNumber(0.0);
		luaState.getField(1, "");
	}

	/**
	 * rawGet(int) with invalid table.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalRawGet1() {
		luaState.pushNumber(0.0);
		luaState.pushString("");
		luaState.rawGet(1);
	}

	/**
	 * rawGet(int, int) with invalid table.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalRawGet2() {
		luaState.pushNumber(0.0);
		luaState.rawGet(1, 1);
	}

	/**
	 * setTable(int) with invalid table.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalSetTable() {
		luaState.pushNil();
		luaState.pushString("");
		luaState.pushString("");
		luaState.setTable(1);
	}

	/**
	 * setTable(int) with nil index.
	 */
	@Test(expected = LuaRuntimeException.class)
	public void testNilSetTable() {
		luaState.newTable();
		luaState.pushNil();
		luaState.pushString("");
		luaState.setTable(1);
	}

	/**
	 * setTable(int) with insufficient arguments.
	 */
	@Test(expected = IllegalStateException.class)
	public void testUnderflowSetTable() {
		luaState.newTable();
		luaState.setTable(1);
	}

	/**
	 * setField(int, String) with invalid table.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalSetField() {
		luaState.pushNumber(0.0);
		luaState.pushString("");
		luaState.setField(1, "key");
	}

	/**
	 * setField(int, String) with null key.
	 */
	@Test(expected = NullPointerException.class)
	public void testNullSetField() {
		luaState.newTable();
		luaState.pushString("value");
		luaState.setField(1, null);
	}

	/**
	 * rawSet(int) with invalid table.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalRawSet1() {
		luaState.pushNumber(0.0);
		luaState.pushString("key");
		luaState.pushString("value");
		luaState.rawSet(1);
	}

	/**
	 * rawSet(int) with nil index.
	 */
	@Test(expected = LuaRuntimeException.class)
	public void testNilRawSet() {
		luaState.newTable();
		luaState.pushNil();
		luaState.pushString("value");
		luaState.rawSet(1);
	}

	/**
	 * rawSet(int) with nil index.
	 */
	@Test(expected = IllegalStateException.class)
	public void testUnderflowRawSet() {
		luaState.newTable();
		luaState.rawSet(1);
	}

	/**
	 * rawSet(int, int) with invalid table.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalRawSet2() {
		luaState.pushNumber(0.0);
		luaState.pushString("value");
		luaState.rawSet(1, 1);
	}

	/**
	 * newTable(int, int) with negative array count.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalNewTable1() {
		luaState.newTable(-1, 0);
	}

	/**
	 * newTable(int, int) with negative record count.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalNewTable2() {
		luaState.newTable(0, -1);
	}

	/**
	 * next(int) with invalid table.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalNext() {
		luaState.pushNumber(0.0);
		luaState.pushNil();
		luaState.next(1);
	}

	/**
	 * setMetaTable(int) with invalid table.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalSetMetaTable() {
		luaState.newTable();
		luaState.pushNumber(0.0);
		luaState.setMetatable(1);
	}

	/**
	 * setFEnv(int) with invalid table.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalSetFEnv() {
		luaState.openLibs();
		luaState.getGlobal("print");
		luaState.pushNumber(0.0);
		luaState.setFEnv(1);
	}

	/**
	 * resume(int, int) with invalid thread.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalResume1() {
		luaState.pushNumber(0.0);
		luaState.resume(1, 0);
	}

	/**
	 * resume(int, int) with an illegal number of returns.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalResume2() {
		luaState.openLibs();
		luaState.getGlobal("print");
		luaState.newThread();
		luaState.resume(1, -1);
	}

	/**
	 * resume(int, int) with insufficient arguments.
	 */
	@Test(expected = IllegalStateException.class)
	public void testUnderflowResume() {
		luaState.openLibs();
		luaState.getGlobal("print");
		luaState.newThread();
		luaState.resume(1, 1);
	}

	/**
	 * yield(int) with no running thread.
	 */
	@Test(expected = IllegalStateException.class)
	public void testIllegalYield() {
		luaState.pushNumber(0.0);
		luaState.yield(0);
	}

	/**
	 * status(int) with illegal thread.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalStatus() {
		luaState.pushNumber(0.0);
		luaState.status(1);
	}

	/**
	 * ref(int) with illegal table.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalRef() {
		luaState.pushNumber(0.0);
		luaState.pushNumber(0.0);
		luaState.ref(1);
	}

	/**
	 * unref(int, int) with illegal table.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalUnref() {
		luaState.pushNumber(0.0);
		luaState.pushNumber(0.0);
		luaState.unref(1, 1);
	}

	/**
	 * getProxy(int, Class<?>) with illegal table.
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testIllegalProxy() {
		luaState.pushNumber(0.0);
		luaState.getProxy(1, Runnable.class);
	}

	// -- Private methods
	/**
	 * Returns an illegal index.
	 */
	private int getIllegalIndex() {
		int multiplier = Math.random() >= 0.5 ? Integer.MAX_VALUE : 1000;
		int index;
		do {
			index = Math.round((float) ((Math.random() - 0.5) * multiplier));
		} while (index >= -15 && index <= 15);
		return index;
	}
}
