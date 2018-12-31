/*
 * $Id: LuaScriptEngine.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua.script;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.Reader;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetEncoder;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.script.AbstractScriptEngine;
import javax.script.Bindings;
import javax.script.Compilable;
import javax.script.CompiledScript;
import javax.script.Invocable;
import javax.script.ScriptContext;
import javax.script.ScriptEngineFactory;
import javax.script.ScriptException;

import com.naef.jnlua.LuaException;
import com.naef.jnlua.LuaState;

/**
 * Lua script engine implementation conforming to JSR 223: Scripting for the
 * Java Platform.
 */
class LuaScriptEngine extends AbstractScriptEngine implements Compilable,
		Invocable {
	// -- Static
	private static final String READER = "reader";
	private static final String WRITER = "writer";
	private static final String ERROR_WRITER = "errorWriter";
	private static final Pattern LUA_ERROR_MESSAGE = Pattern
			.compile("^(.+):(\\d+):");

	// -- State
	private LuaScriptEngineFactory factory;
	private LuaState luaState;

	// -- Construction
	/**
	 * Creates a new instance.
	 */
	LuaScriptEngine(LuaScriptEngineFactory factory) {
		super();
		this.factory = factory;
		luaState = new LuaState();

		// Configuration
		context.setBindings(createBindings(), ScriptContext.ENGINE_SCOPE);
		luaState.openLibs();
		luaState.load("io.stdout:setvbuf(\"no\")", "setvbuf");
		luaState.call(0, 0);
		luaState.load("io.stderr:setvbuf(\"no\")", "setvbuf");
		luaState.call(0, 0);
	}

	// -- ScriptEngine methods
	@Override
	public Bindings createBindings() {
		return new LuaBindings(this);
	}

	@Override
	public Object eval(String script, ScriptContext context)
			throws ScriptException {
		synchronized (luaState) {
			loadChunk(script, context);
			return callChunk(context);
		}
	}

	@Override
	public Object eval(Reader reader, ScriptContext context)
			throws ScriptException {
		synchronized (luaState) {
			loadChunk(reader, context);
			return callChunk(context);
		}
	}

	@Override
	public ScriptEngineFactory getFactory() {
		return factory;
	}

	// -- Compilable method
	@Override
	public CompiledScript compile(String script) throws ScriptException {
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		synchronized (luaState) {
			loadChunk(script, null);
			try {
				dumpChunk(out);
			} finally {
				luaState.pop(1);
			}
		}
		return new CompiledLuaScript(this, out.toByteArray());
	}

	@Override
	public CompiledScript compile(Reader script) throws ScriptException {
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		synchronized (luaState) {
			loadChunk(script, null);
			try {
				dumpChunk(out);
			} finally {
				luaState.pop(1);
			}
		}
		return new CompiledLuaScript(this, out.toByteArray());
	}

	// -- Invocable methods
	@Override
	public <T> T getInterface(Class<T> clasz) {
		synchronized (luaState) {
			luaState.pushValue(LuaState.GLOBALSINDEX);
			try {
				return luaState.getProxy(-1, clasz);
			} finally {
				luaState.pop(1);
			}
		}
	}

	@Override
	public <T> T getInterface(Object thiz, Class<T> clasz) {
		synchronized (luaState) {
			luaState.pushJavaObject(thiz);
			try {
				if (!luaState.isTable(-1)) {
					throw new IllegalArgumentException("object is not a table");
				}
				return luaState.getProxy(-1, clasz);
			} finally {
				luaState.pop(1);
			}
		}
	}

	@Override
	public Object invokeFunction(String name, Object... args)
			throws ScriptException, NoSuchMethodException {
		synchronized (luaState) {
			luaState.getGlobal(name);
			if (!luaState.isFunction(-1)) {
				luaState.pop(1);
				throw new NoSuchMethodException(String.format(
						"function '%s' is undefined", name));
			}
			for (int i = 0; i < args.length; i++) {
				luaState.pushJavaObject(args[i]);
			}
			luaState.call(args.length, 1);
			try {
				return luaState.toJavaObject(-1, Object.class);
			} finally {
				luaState.pop(1);
			}
		}
	}

	@Override
	public Object invokeMethod(Object thiz, String name, Object... args)
			throws ScriptException, NoSuchMethodException {
		synchronized (luaState) {
			luaState.pushJavaObject(thiz);
			try {
				if (!luaState.isTable(-1)) {
					throw new IllegalArgumentException("object is not a table");
				}
				luaState.getField(-1, name);
				if (!luaState.isFunction(-1)) {
					luaState.pop(1);
					throw new NoSuchMethodException(String.format(
							"method '%s' is undefined", name));
				}
				luaState.pushValue(-2);
				for (int i = 0; i < args.length; i++) {
					luaState.pushJavaObject(args[i]);
				}
				luaState.call(args.length + 1, 1);
				try {
					return luaState.toJavaObject(-1, Object.class);
				} finally {
					luaState.pop(1);
				}
			} finally {
				luaState.pop(1);
			}
		}
	}

	// -- Package private methods
	/**
	 * Returns the Lua state.
	 */
	LuaState getLuaState() {
		return luaState;
	}

	/**
	 * Loads a chunk from a string.
	 */
	void loadChunk(String string, ScriptContext scriptContext)
			throws ScriptException {
		try {
			luaState.load(string, getChunkName(scriptContext));
		} catch (LuaException e) {
			throw getScriptException(e);
		}
	}

	/**
	 * Loads a chunk from a reader.
	 */
	void loadChunk(Reader reader, ScriptContext scriptContext)
			throws ScriptException {
		loadChunk(new ReaderInputStream(reader), scriptContext);
	}

	/**
	 * Loads a chunk from an input stream.
	 */
	void loadChunk(InputStream inputStream, ScriptContext scriptContext)
			throws ScriptException {
		try {
			luaState.load(inputStream, getChunkName(scriptContext));
		} catch (LuaException e) {
			throw getScriptException(e);
		} catch (IOException e) {
			throw new ScriptException(e);
		}
	}

	/**
	 * Calls a loaded chunk.
	 */
	Object callChunk(ScriptContext context) throws ScriptException {
		try {
			// Apply context
			Object[] argv;
			if (context != null) {
				// Global bindings
				Bindings bindings;
				bindings = context.getBindings(ScriptContext.GLOBAL_SCOPE);
				if (bindings != null) {
					applyBindings(bindings);
				}

				// Engine bindings
				bindings = context.getBindings(ScriptContext.ENGINE_SCOPE);
				if (bindings != null) {
					if (bindings instanceof LuaBindings
							&& ((LuaBindings) bindings).getScriptEngine() == this) {
						// No need to apply our own live bindings
					} else {
						applyBindings(bindings);
					}
				}

				// Readers and writers
				put(READER, context.getReader());
				put(WRITER, context.getWriter());
				put(ERROR_WRITER, context.getErrorWriter());

				// Arguments
				argv = (Object[]) context.getAttribute(ARGV);
			} else {
				argv = null;
			}

			// Push arguments
			int argCount = argv != null ? argv.length : 0;
			for (int i = 0; i < argCount; i++) {
				luaState.pushJavaObject(argv[i]);
			}

			// Call
			luaState.call(argCount, 1);

			// Return
			try {
				return luaState.toJavaObject(1, Object.class);
			} finally {
				luaState.pop(1);
			}
		} catch (LuaException e) {
			throw getScriptException(e);
		}
	}

	/**
	 * Dumps a loaded chunk into an output stream. The chunk is left on the
	 * stack.
	 */
	void dumpChunk(OutputStream out) throws ScriptException {
		try {
			luaState.dump(out);
		} catch (LuaException e) {
			throw new ScriptException(e);
		} catch (IOException e) {
			throw new ScriptException(e);
		}

	}

	// -- Private methods
	/**
	 * Sets a single binding in a Lua state.
	 */
	private void applyBindings(Bindings bindings) {
		for (Map.Entry<String, Object> binding : bindings.entrySet()) {
			luaState.pushJavaObject(binding.getValue());
			String variableName = binding.getKey();
			int lastDotIndex = variableName.lastIndexOf('.');
			if (lastDotIndex >= 0) {
				variableName = variableName.substring(lastDotIndex + 1);
			}
			luaState.setGlobal(variableName);
		}
	}

	/**
	 * Returns the Lua chunk name from a script context.
	 */
	private String getChunkName(ScriptContext context) {
		if (context != null) {
			Object fileName = context.getAttribute(FILENAME);
			if (fileName != null) {
				return fileName.toString();
			}
		}
		return "null";
	}

	/**
	 * Returns a script exception for a Lua exception.
	 */
	private ScriptException getScriptException(LuaException e) {
		Matcher matcher = LUA_ERROR_MESSAGE.matcher(e.getMessage());
		if (matcher.find()) {
			String fileName = matcher.group(1);
			int lineNumber = Integer.parseInt(matcher.group(2));
			return new ScriptException(e.getMessage(), fileName, lineNumber);
		} else {
			return new ScriptException(e);
		}
	}

	// -- Private classes
	/**
	 * Provides an UTF-8 input stream based on a reader.
	 */
	private static class ReaderInputStream extends InputStream {
		// -- Static
		private static final Charset UTF8 = Charset.forName("UTF-8");

		// -- State
		private Reader reader;
		private CharsetEncoder encoder;
		private boolean flushed;
		private CharBuffer charBuffer = CharBuffer.allocate(1024);
		private ByteBuffer byteBuffer = ByteBuffer.allocate(1024);

		/**
		 * Creates a new instance.
		 */
		public ReaderInputStream(Reader reader) {
			this.reader = reader;
			encoder = UTF8.newEncoder();
			charBuffer.limit(0);
			byteBuffer.limit(0);
		}

		@Override
		public int read() throws IOException {
			if (!byteBuffer.hasRemaining()) {
				if (!charBuffer.hasRemaining()) {
					charBuffer.clear();
					reader.read(charBuffer);
					charBuffer.flip();
				}
				byteBuffer.clear();
				if (charBuffer.hasRemaining()) {
					if (encoder.encode(charBuffer, byteBuffer, false).isError()) {
						throw new IOException("Encoding error");
					}
				} else {
					if (!flushed) {
						if (encoder.encode(charBuffer, byteBuffer, true)
								.isError()) {
							throw new IOException("Encoding error");
						}
						if (encoder.flush(byteBuffer).isError()) {
							throw new IOException("Encoding error");
						}
						flushed = true;
					}
				}
				byteBuffer.flip();
				if (!byteBuffer.hasRemaining()) {
					return -1;
				}
			}
			return byteBuffer.get();
		}
	}
}
