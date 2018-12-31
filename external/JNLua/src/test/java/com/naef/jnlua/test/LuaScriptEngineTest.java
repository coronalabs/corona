/*
 * $Id: LuaScriptEngineTest.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua.test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;

import java.io.StringReader;
import java.util.List;

import javax.script.Bindings;
import javax.script.Compilable;
import javax.script.CompiledScript;
import javax.script.Invocable;
import javax.script.ScriptContext;
import javax.script.ScriptEngine;
import javax.script.ScriptEngineFactory;
import javax.script.ScriptEngineManager;
import javax.script.ScriptException;
import javax.script.SimpleBindings;
import javax.script.SimpleScriptContext;

import org.junit.Before;
import org.junit.Test;

public class LuaScriptEngineTest {
	// ---- State
	private ScriptEngineManager scriptEngineManager;
	private ScriptEngine scriptEngine;

	// ---- Setup
	/**
	 * Performs setup.
	 */
	@Before
	public void setup() throws Exception {
		scriptEngineManager = new ScriptEngineManager();
		scriptEngine = scriptEngineManager.getEngineByName("Lua");
	}

	// -- Unit tests
	/**
	 * Tests the acquisition of the Lua script engine from the manager.
	 */
	@Test
	public void testAcquisition() {
		assertNotNull(scriptEngineManager.getEngineByExtension("lua"));
		assertNotNull(scriptEngineManager
				.getEngineByMimeType("application/x-lua"));
		assertNotNull(scriptEngineManager.getEngineByMimeType("text/x-lua"));
		assertNotNull(scriptEngineManager.getEngineByName("lua"));
		assertNotNull(scriptEngineManager.getEngineByName("Lua"));
		assertNotNull(scriptEngineManager.getEngineByName("jnlua"));
		assertNotNull(scriptEngineManager.getEngineByName("JNLua"));
	}

	/**
	 * Tests the Lua script engine factory.
	 */
	@Test
	public void testScriptEngineFactory() {
		// Get factory
		ScriptEngineFactory factory = scriptEngine.getFactory();

		// getEngineName()
		assertEquals("JNLua", factory.getEngineName());

		// getEngineVersion()
		assertEquals("0.9", factory.getEngineVersion());

		// getNames()
		List<String> names = factory.getNames();
		assertTrue(names.contains("lua"));
		assertTrue(names.contains("Lua"));
		assertTrue(names.contains("jnlua"));
		assertTrue(names.contains("JNLua"));

		// getLanguageName()
		assertEquals("Lua", factory.getLanguageName());

		// getLanguageVersion()
		assertEquals("5.1", factory.getLanguageVersion());

		// getExtensions()
		List<String> extensions = factory.getExtensions();
		assertTrue(extensions.contains("lua"));

		// getMimeTypes()
		List<String> mimeTypes = factory.getMimeTypes();
		assertTrue(mimeTypes.contains("application/x-lua"));
		assertTrue(mimeTypes.contains("text/x-lua"));

		// getParameter()
		assertEquals(factory.getEngineName(), factory
				.getParameter(ScriptEngine.ENGINE));
		assertEquals(factory.getEngineVersion(), factory
				.getParameter(ScriptEngine.ENGINE_VERSION));
		assertTrue(factory.getNames().contains(
				factory.getParameter(ScriptEngine.NAME)));
		assertEquals(factory.getLanguageName(), factory
				.getParameter(ScriptEngine.LANGUAGE));
		assertEquals(factory.getLanguageVersion(), factory
				.getParameter(ScriptEngine.LANGUAGE_VERSION));

		// getScriptEngine()
		assertNotNull(factory.getScriptEngine());

		// getMethodCallSyntax()
		assertEquals("process:execute(a, b)", factory.getMethodCallSyntax(
				"process", "execute", "a", "b"));

		// getOutputStatement()
		assertEquals("print(\"test\")", factory.getOutputStatement("test"));
		assertEquals("print(\"\\\"quoted\\\"\")", factory
				.getOutputStatement("\"quoted\""));

		// getProgram()
		assertEquals("a = 1\nreturn b\n", factory.getProgram("a = 1",
				"return b"));
	}

	/**
	 * Tests the Lua script engine.
	 */
	@Test
	public void testScriptEngine() throws Exception {
		// eval(String)
		assertEquals(Double.valueOf(1.0), scriptEngine.eval("return 1"));

		// eval(Reader)
		assertEquals(Double.valueOf(1.0), scriptEngine.eval(new StringReader(
				"return 1")));

		// createBindings()
		Bindings bindings = scriptEngine.createBindings();
		assertNotNull(bindings);

		// eval(String, Bindings)
		bindings.put("t", "test1");
		assertEquals("test1", scriptEngine.eval("return t", bindings));

		// eval(Reader, Bindings)
		bindings.put("t", "test2");
		assertEquals("test2", scriptEngine.eval(new StringReader("return t"),
				bindings));

		// eval(String, ScriptContext)
		ScriptContext context = new SimpleScriptContext();
		context.setBindings(new SimpleBindings(), ScriptContext.GLOBAL_SCOPE);
		context.setBindings(bindings, ScriptContext.ENGINE_SCOPE);
		bindings.put("t", "test3");
		assertEquals("test3", scriptEngine.eval("return t", context));

		// eval(Reader, ScriptContext)
		bindings.put("t", "test4");
		assertEquals("test4", scriptEngine.eval("return t", context));

		// getBindings(), setBindings()
		scriptEngine.setBindings(bindings, ScriptContext.ENGINE_SCOPE);
		assertSame(bindings, scriptEngine
				.getBindings(ScriptContext.ENGINE_SCOPE));
		scriptEngine.getBindings(ScriptContext.ENGINE_SCOPE).put("t", "test5");
		assertEquals("test5", scriptEngine.eval("return t"));

		// get(), put()
		scriptEngine.put("t", "test6");
		assertEquals("test6", scriptEngine.get("t"));
		assertEquals("test6", bindings.get("t"));
		assertEquals("test6", scriptEngine.eval("return t"));

		// getContext(), setContext()
		scriptEngine.setContext(context);
		assertSame(context, scriptEngine.getContext());
		context.setAttribute("t", "test7", ScriptContext.GLOBAL_SCOPE);
		context.removeAttribute("t", ScriptContext.ENGINE_SCOPE);
		assertEquals("test7", scriptEngine.eval("return t"));

		// getFactory()
		assertNotNull(scriptEngine.getFactory());

		// Special variables
		scriptEngine.put(ScriptEngine.ARGV, new Object[] { "test8" });
		assertEquals("test8", scriptEngine.eval("return ..."));
		assertSame(context.getReader(), scriptEngine.eval("return reader"));
		assertSame(context.getWriter(), scriptEngine.eval("return writer"));
		assertSame(context.getErrorWriter(), scriptEngine
				.eval("return errorWriter"));

		// Syntax error diagnostics
		scriptEngine.put(ScriptEngine.FILENAME, "test9");
		ScriptException scriptException = null;
		try {
			scriptEngine.eval("a bad script");
		} catch (ScriptException e) {
			scriptException = e;
		}
		assertNotNull(scriptException);
		assertEquals("test9", scriptException.getFileName());
		assertEquals(1, scriptException.getLineNumber());

		// Runtime error diagnostics
		scriptException = null;
		try {
			scriptEngine.eval("error(\"error\")");
		} catch (ScriptException e) {
			scriptException = e;
		}
		assertNotNull(scriptException);
		assertEquals("test9", scriptException.getFileName());
		assertEquals(1, scriptException.getLineNumber());
	}

	/**
	 * Tests the compilable interface.
	 */
	@Test
	public void testCompilable() throws Exception {
		// Check
		assertTrue(scriptEngine instanceof Compilable);
		Compilable compilable = (Compilable) scriptEngine;

		// Setup
		Bindings bindings = new SimpleBindings();
		bindings.put("t", Double.valueOf(2.0));
		ScriptContext scriptContext = new SimpleScriptContext();
		scriptContext.setBindings(new SimpleBindings(),
				ScriptContext.GLOBAL_SCOPE);
		scriptContext.setAttribute("t", Double.valueOf(3.0),
				ScriptContext.GLOBAL_SCOPE);

		// Compile(String)
		CompiledScript compiledScript = compilable.compile("return t");
		scriptEngine.put("t", Double.valueOf(1.0));
		assertEquals(Double.valueOf(1.0), compiledScript.eval());
		assertEquals(Double.valueOf(2.0), compiledScript.eval(bindings));
		assertEquals(Double.valueOf(3.0), compiledScript.eval(scriptContext));

		// Compile(Reader)
		compiledScript = compilable.compile(new StringReader("return t"));
		scriptEngine.put("t", Double.valueOf(1.0));
		assertEquals(Double.valueOf(1.0), compiledScript.eval());
		assertEquals(Double.valueOf(2.0), compiledScript.eval(bindings));
		assertEquals(Double.valueOf(3.0), compiledScript.eval(scriptContext));
	}

	/**
	 * Tests the invocable interface.
	 */
	@Test
	public void testIncocable() throws Exception {
		// Check
		assertTrue(scriptEngine instanceof Invocable);
		Invocable invocable = (Invocable) scriptEngine;

		// Setup
		scriptEngine.eval("function echo(s) return s end");
		scriptEngine.eval("function run() hasRun = true end");
		scriptEngine.eval("runnable = { run = run }");

		// invokeFunction()
		assertEquals("test", invocable.invokeFunction("echo",
				new Object[] { "test" }));

		// invokeMethod()
		scriptEngine.put("hasRun", Boolean.FALSE);
		assertEquals(Boolean.FALSE, scriptEngine.get("hasRun"));
		Object runnableObj = scriptEngine.get("runnable");
		invocable.invokeMethod(runnableObj, "run", new Object[0]);
		assertEquals(Boolean.TRUE, scriptEngine.get("hasRun"));

		// getInterface()
		scriptEngine.put("hasRun", Boolean.FALSE);
		assertEquals(Boolean.FALSE, scriptEngine.get("hasRun"));
		Runnable runnable = invocable.getInterface(Runnable.class);
		Thread thread = new Thread(runnable);
		thread.start();
		thread.join();
		assertEquals(Boolean.TRUE, scriptEngine.get("hasRun"));

		// getInterface(Object)
		scriptEngine.put("hasRun", Boolean.FALSE);
		assertEquals(Boolean.FALSE, scriptEngine.get("hasRun"));
		runnable = invocable.getInterface(runnableObj, Runnable.class);
		thread = new Thread(runnable);
		thread.start();
		thread.join();
		assertEquals(Boolean.TRUE, scriptEngine.get("hasRun"));
	}

	/**
	 * Tests the bindings.
	 */
	@Test
	public void testBindings() throws Exception {
		// Get bindings
		Bindings bindings = scriptEngine
				.getBindings(ScriptContext.ENGINE_SCOPE);

		// Put
		bindings.put("t", "test");
		assertEquals("test", scriptEngine.eval("return t"));

		// Get
		scriptEngine.eval("t = \"test2\"");
		assertEquals("test2", bindings.get("t"));

		// Remove
		bindings.remove("t");
		assertNull(scriptEngine.eval("return t"));
	}
}
