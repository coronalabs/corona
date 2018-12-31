/*
 * $Id: LuaState.java 156 2012-10-05 22:57:25Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.ref.PhantomReference;
import java.lang.ref.ReferenceQueue;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

import com.naef.jnlua.JavaReflector.Metamethod;

/**
 * JNLua core class representing a Lua instance.
 * 
 * <p>
 * The class performs extensive checking on all arguments and its state.
 * Specifically, the following exceptions are thrown under the indicated
 * conditions:
 * </p>
 * 
 * <table class="doc">
 * <tr>
 * <th>Exception</th>
 * <th>When</th>
 * </tr>
 * <tr>
 * <td>{@link java.lang.NullPointerException}</td>
 * <td>if an argument is <code>null</code> and the API does not explicitly
 * specify that the argument may be <code>null</code></td>
 * </tr>
 * <tr>
 * <td>{@link java.lang.IllegalStateException}</td>
 * <td>if the Lua state is closed and the API does not explicitly specify that
 * the method may be invoked on a closed Lua state</td>
 * </tr>
 * <tr>
 * <td>{@link java.lang.IllegalArgumentException}</td>
 * <td>if a stack index refers to an undefined stack location and the API does
 * not explicitly specify that the stack index may be undefined</td>
 * </tr>
 * <tr>
 * <td>{@link java.lang.IllegalArgumentException}</td>
 * <td>if a stack index refers to an stack location with value type that is
 * different from the value type explicitly specified in the API</td>
 * </tr>
 * <tr>
 * <td>{@link java.lang.IllegalArgumentException}</td>
 * <td>if a count is negative or out of range and the API does not explicitly
 * specify that the count may be negative or out of range</td>
 * </tr>
 * <tr>
 * <td>{@link com.naef.jnlua.LuaRuntimeException}</td>
 * <td>if a Lua runtime error occurs</td>
 * </tr>
 * <tr>
 * <td>{@link com.naef.jnlua.LuaSyntaxException}</td>
 * <td>if a the syntax of a Lua chunk is incorrect</td>
 * </tr>
 * <tr>
 * <td>{@link com.naef.jnlua.LuaMemoryAllocationException}</td>
 * <td>if the Lua memory allocator runs out of memory or if a JNI allocation
 * fails</td>
 * </tr>
 * </table>
 */
public class LuaState {
	// -- Static
	/**
	 * Registry pseudo-index.
	 */
	public static final int REGISTRYINDEX = -10000;

	/**
	 * Environment pseudo-index.
	 */
	public static final int ENVIRONINDEX = -10001;

	/**
	 * Globals pseudo-index.
	 */
	public static final int GLOBALSINDEX = -10002;

	/**
	 * Multiple returns pseudo return value count.
	 */
	public static final int MULTRET = -1;

	/**
	 * Status indicating that a thread is suspended.
	 */
	public static final int YIELD = 1;

	/**
	 * The JNLua version. The format is &lt;major&gt;.&lt;minor&gt;.
	 */
	public static final String VERSION = "0.9";

	/**
	 * The Lua version. The format is &lt;major&gt;.&lt;minor&gt;.
	 */
	public static final String LUA_VERSION;

	static {
		NativeSupport.getInstance().getLoader().load();
		LUA_VERSION = lua_version();
	}

	/**
	 * The API version.
	 */
	private static final int APIVERSION = 2;

	// -- State
	/**
	 * Whether the <code>lua_State</code> on the JNI side is owned by the Java
	 * state and must be closed when the Java state closes.
	 */
	private boolean ownState;

	/**
	 * The <code>lua_State</code> pointer on the JNI side. <code>0</code>
	 * implies that this Lua state is closed. The field is modified exclusively
	 * on the JNI side and must not be touched on the Java side.
	 */
	private long luaState;

	/**
	 * The <code>lua_State</code> pointer on the JNI side for the running
	 * coroutine. This field is modified exclusively on the JNI side and must
	 * not be touched on the Java side.
	 */
	private long luaThread;

	/**
	 * Ensures proper finalization of this Lua state.
	 */
	private Object finalizeGuardian;

	/**
	 * The class loader for dynamically loading classes.
	 */
	private ClassLoader classLoader;

	/**
	 * Reflects Java objects.
	 */
	private JavaReflector javaReflector;

	/**
	 * Converts between Lua types and Java types.
	 */
	private Converter converter;

	/**
	 * Set of Lua proxy phantom references for pre-mortem cleanup.
	 */
	private Set<LuaValueProxyRef> proxySet = new HashSet<LuaValueProxyRef>();

	/**
	 * Reference queue for pre-mortem cleanup.
	 */
	private ReferenceQueue<LuaValueProxyImpl> proxyQueue = new ReferenceQueue<LuaValueProxyImpl>();

	// -- Construction
	/**
	 * Creates a new instance. The class loader of this Lua state is set to the
	 * context class loader of the calling thread. The Java reflector and the
	 * converter are initialized with the default implementations.
	 * 
	 * @see #getClassLoader()
	 * @see #setClassLoader(ClassLoader)
	 * @see #getJavaReflector()
	 * @see #setJavaReflector(JavaReflector)
	 * @see #getConverter()
	 * @see #setConverter(Converter)
	 */
	public LuaState() {
		this(0L);
	}
	
	/**
	 * Creates a new instance.
	 */
	public LuaState(long luaState) {
		ownState = luaState == 0L;
		lua_newstate(APIVERSION, luaState);
		check();

		// Create a finalizer to close the Lua state, but only if we own it.
		// Note: We should never close a Lua state that we do not own from the garbage collector's
		//       thread or else it may cause a race condition if it is still being used.
		if (ownState) {
			finalizeGuardian = new Object() {
				@Override
				public void finalize() {
					synchronized (LuaState.this) {
						closeInternal();
					}
				}
			};
		}
		else {
			finalizeGuardian = null;
		}

		// Add metamethods
		for (int i = 0; i < JavaReflector.Metamethod.values().length; i++) {
			final JavaReflector.Metamethod metamethod = JavaReflector.Metamethod
					.values()[i];
			lua_pushjavafunction(new JavaFunction() {
				@Override
				public int invoke(LuaState luaState) {
					JavaFunction javaFunction = getMetamethod(
							luaState.toJavaObjectRaw(1), metamethod);
					if (javaFunction != null) {
						return javaFunction.invoke(LuaState.this);
					} else {
						throw new UnsupportedOperationException(
								metamethod.getMetamethodName());
					}
				}
			});
			lua_setfield(-2, metamethod.getMetamethodName());
		}
		lua_pop(1);

		// Set fields
		classLoader = Thread.currentThread().getContextClassLoader();
		javaReflector = DefaultJavaReflector.getInstance();
		converter = DefaultConverter.getInstance();
	}

	// -- Properties
	/**
	 * Returns the class loader of this Lua state. The class loader is used for
	 * dynamically loading classes.
	 * 
	 * <p>
	 * The method may be invoked on a closed Lua state.
	 * </p>
	 * 
	 * @return the class loader
	 */
	public synchronized ClassLoader getClassLoader() {
		return classLoader;
	}

	/**
	 * Sets the class loader of this Lua state. The class loader is used for
	 * dynamically loading classes.
	 * 
	 * <p>
	 * The method may be invoked on a closed Lua state.
	 * </p>
	 * 
	 * @param classLoader
	 *            the class loader to set
	 */
	public synchronized void setClassLoader(ClassLoader classLoader) {
		if (classLoader == null) {
			throw new NullPointerException();
		}
		this.classLoader = classLoader;
	}

	/**
	 * Returns the Java reflector of this Lua state.
	 * 
	 * <p>
	 * The method may be invoked on a closed Lua state.
	 * </p>
	 * 
	 * @return the Java reflector converter
	 */
	public synchronized JavaReflector getJavaReflector() {
		return javaReflector;
	}

	/**
	 * Sets the Java reflector of this Lua state.
	 * 
	 * <p>
	 * The method may be invoked on a closed Lua state.
	 * </p>
	 * 
	 * @param javaReflector
	 *            the Java reflector
	 */
	public synchronized void setJavaReflector(JavaReflector javaReflector) {
		if (javaReflector == null) {
			throw new NullPointerException();
		}
		this.javaReflector = javaReflector;
	}

	/**
	 * Returns a metamethod for a specified object. If the object implements the
	 * {@link com.naef.jnlua.JavaReflector} interface, the metamethod is first
	 * queried from the object. If the object provides the requested metamethod,
	 * that metamethod is returned. Otherwise, the method returns the metamethod
	 * provided by the Java reflector configured in this Lua state.
	 * 
	 * <p>
	 * Clients requiring access to metamethods should go by this method to
	 * ensure consistent class-by-class overriding of the Java reflector.
	 * </p>
	 * 
	 * @param obj
	 *            the object, or <code>null</code>
	 * @return the Java reflector
	 */
	public synchronized JavaFunction getMetamethod(Object obj,
			Metamethod metamethod) {
		if (obj != null && obj instanceof JavaReflector) {
			JavaFunction javaFunction = ((JavaReflector) obj)
					.getMetamethod(metamethod);
			if (javaFunction != null) {
				return javaFunction;
			}
		}
		return javaReflector.getMetamethod(metamethod);
	}

	/**
	 * Returns the converter of this Lua state.
	 * 
	 * <p>
	 * The method may be invoked on a closed Lua state.
	 * </p>
	 * 
	 * @return the converter
	 */
	public synchronized Converter getConverter() {
		return converter;
	}

	/**
	 * Sets the converter of this Lua state.
	 * 
	 * <p>
	 * The method may be invoked on a closed Lua state.
	 * </p>
	 * 
	 * @param converter
	 *            the converter
	 */
	public synchronized void setConverter(Converter converter) {
		if (converter == null) {
			throw new NullPointerException();
		}
		this.converter = converter;
	}

	/**
	 * Returns whether this Lua state is open.
	 * 
	 * <p>
	 * The method may be invoked on a closed Lua state.
	 * </p>
	 * 
	 * @return whether this Lua state is open
	 */
	public final synchronized boolean isOpen() {
		return isOpenInternal();
	}

	// -- Life cycle
	/**
	 * Closes this Lua state and releases all resources.
	 * 
	 * <p>
	 * The method may be invoked on a closed Lua state and has no effect in that
	 * case.
	 * </p>
	 */
	public synchronized void close() {
		closeInternal();
	}

	/**
	 * Performs a garbage collection operation.
	 * 
	 * @param what
	 *            the operation to perform
	 * @param data
	 *            the argument required by some operations (see Lua Reference
	 *            Manual)
	 * @return a return value depending on the GC operation performed (see Lua
	 *         Reference Manual)
	 */
	public synchronized int gc(GcAction what, int data) {
		check();
		return lua_gc(what.ordinal(), data);
	}

	// -- Registration
	/**
	 * Opens the specified library in this Lua state.
	 * 
	 * @param library
	 *            the library
	 */
	public synchronized void openLib(Library library) {
		check();
		library.open(this);
	}

	/**
	 * Opens the Lua standard libraries and the JNLua Java module in this Lua
	 * state.
	 * 
	 * <p>
	 * The method opens all libraries defined by the {@link Library}
	 * enumeration.
	 * </p>
	 */
	public synchronized void openLibs() {
		check();
		for (Library library : Library.values()) {
			library.open(this);
		}
	}

	/**
	 * Registers a named Java function as a global variable.
	 * 
	 * @param namedJavaFunction
	 *            the Java function to register
	 */
	public synchronized void register(NamedJavaFunction namedJavaFunction) {
		check();
		String name = namedJavaFunction.getName();
		if (name == null) {
			throw new IllegalArgumentException("Anonymous function");
		}
		pushJavaFunction(namedJavaFunction);
		setGlobal(name);
	}

	/**
	 * Registers a module and pushes the module on the stack. The module name is
	 * allowed to contain dots to define module hierarchies.
	 * 
	 * @param moduleName
	 *            the module name
	 * @param namedJavaFunctions
	 *            the Java functions of the module
	 */
	public synchronized void register(String moduleName,
			NamedJavaFunction[] namedJavaFunctions) {
		check();
		/*
		 * The following code corresponds to luaL_openlib() and must be kept in
		 * sync. The original code cannot be called due to the necessity of
		 * pushing each C function with an individual closure.
		 */
		lua_findtable(REGISTRYINDEX, "_LOADED", 1);
		getField(-1, moduleName);
		if (!isTable(-1)) {
			pop(1);
			String conflict = lua_findtable(GLOBALSINDEX, moduleName,
					namedJavaFunctions.length);
			if (conflict != null) {
				throw new IllegalArgumentException(String.format(
						"naming conflict for module name '%s' at '%s'",
						moduleName, conflict));
			}
			pushValue(-1);
			setField(-3, moduleName);
		}
		remove(-2);
		for (int i = 0; i < namedJavaFunctions.length; i++) {
			String name = namedJavaFunctions[i].getName();
			if (name == null) {
				throw new IllegalArgumentException(String.format(
						"anonymous function at index %d", i));
			}
			pushJavaFunction(namedJavaFunctions[i]);
			setField(-2, name);
		}
	}

	// -- Load and dump
	/**
	 * Loads a Lua chunk from an input stream and pushes it on the stack as a
	 * function. The Lua chunk must be either a UTF-8 encoded source chunk or a
	 * pre-compiled binary chunk.
	 * 
	 * @param inputStream
	 *            the input stream
	 * @param chunkName
	 *            the name of the chunk for use in error messages
	 * @throws IOException
	 *             if an IO error occurs
	 */
	public synchronized void load(InputStream inputStream, String chunkName)
			throws IOException {
		if (chunkName == null) {
			throw new NullPointerException();
		}
		check();
		lua_load(inputStream, "=" + chunkName);
	}

	/**
	 * Loads a Lua chunk from a string and pushes it on the stack as a function.
	 * The string must contain a source chunk.
	 * 
	 * @param chunk
	 *            the Lua source chunk
	 * @param chunkName
	 *            the name of the chunk for use in error messages
	 */
	public synchronized void load(String chunk, String chunkName) {
		try {
			load(new ByteArrayInputStream(chunk.getBytes("UTF-8")), chunkName);
		} catch (IOException e) {
			throw new LuaMemoryAllocationException(e.getMessage());
		}
	}

	/**
	 * Dumps the function on top of the stack as a pre-compiled binary chunk
	 * into an output stream.
	 * 
	 * @param outputStream
	 *            the output stream
	 * @throws IOException
	 *             if an IO error occurs
	 */
	public synchronized void dump(OutputStream outputStream) throws IOException {
		check();
		lua_dump(outputStream);
	}

	// -- Call
	/**
	 * Calls a Lua function. The function to call and the specified number of
	 * arguments are on the stack. After the call, the specified number of
	 * returns values are on stack. If the number of return values has been
	 * specified as {@link #MULTRET}, the number of values on the stack
	 * corresponds the to number of values actually returned by the called
	 * function.
	 * 
	 * @param argCount
	 *            the number of arguments
	 * @param returnCount
	 *            the number of return values, or {@link #MULTRET} to accept all
	 *            values returned by the function
	 */
	public synchronized void call(int argCount, int returnCount) {
		check();
		lua_pcall(argCount, returnCount);
	}

	// -- Global
	/**
	 * Pushes the value of a global variable on the stack.
	 * 
	 * @param name
	 *            the global variable name
	 */
	public synchronized void getGlobal(String name) {
		check();
		lua_getglobal(name);
	}

	/**
	 * Sets the value on top of the stack as a global variable and pops the
	 * value from the stack.
	 * 
	 * @param name
	 *            the global variable name
	 */
	public synchronized void setGlobal(String name)
			throws LuaMemoryAllocationException, LuaRuntimeException {
		check();
		lua_setglobal(name);
	}

	// -- Stack push
	/**
	 * Pushes a boolean value on the stack.
	 * 
	 * @param b
	 *            the boolean value to push
	 */
	public synchronized void pushBoolean(boolean b) {
		check();
		lua_pushboolean(b ? 1 : 0);
	}

	/**
	 * Pushes an integer value as a number value on the stack.
	 * 
	 * @param n
	 *            the integer value to push
	 */
	public synchronized void pushInteger(int n) {
		check();
		lua_pushinteger(n);
	}

	/**
	 * Pushes a Java function on the stack.
	 * 
	 * @param javaFunction
	 *            the function to push
	 */
	public synchronized void pushJavaFunction(JavaFunction javaFunction) {
		check();
		lua_pushjavafunction(javaFunction);
	}

	/**
	 * Pushes a Java object on the stack. The object is pushed "as is", i.e.
	 * without conversion.
	 * 
	 * <p>
	 * If you require to push a Lua value that represents the Java object, then
	 * invoke <code>pushJavaObject(object)</code>.
	 * </p>
	 * 
	 * <p>
	 * You cannot push <code>null</code> without conversion since
	 * <code>null</code> is not a Java object. The converter converts
	 * <code>null</code> to <code>nil</code>.
	 * </p>
	 * 
	 * @param object
	 *            the Java object
	 * @see #pushJavaObject(Object)
	 */
	public synchronized void pushJavaObjectRaw(Object object) {
		check();
		lua_pushjavaobject(object);
	}

	/**
	 * Pushes a Java object on the stack with conversion. The object is
	 * processed the by the configured converter.
	 * 
	 * @param object
	 *            the Java object
	 * @see #getConverter()
	 * @see #setConverter(Converter)
	 */
	public synchronized void pushJavaObject(Object object) {
		check();
		getConverter().convertJavaObject(this, object);
	}

	/**
	 * Pushes a nil value on the stack.
	 */
	public synchronized void pushNil() {
		check();
		lua_pushnil();
	}

	/**
	 * Pushes a number value on the stack.
	 * 
	 * @param n
	 *            the number to push
	 */
	public synchronized void pushNumber(double n) {
		check();
		lua_pushnumber(n);
	}

	/**
	 * Pushes a string value on the stack.
	 * 
	 * @param s
	 *            the string value to push
	 */
	public synchronized void pushString(String s) {
		check();
		lua_pushstring(s);
	}

	/**
	 * Pushes a byte array as a string value on the stack.
	 * 
	 * @param b
	 *            the byte array to push
	 */
	public synchronized void pushString(byte[] b) {
		check();
		lua_pushbytearray(b, b.length);
	}

	/**
	 * Pushes a byte array as a string value on the stack.
	 * 
	 * @param b
	 *            the byte array to push
	 * @param count
	 *            the number of bytes in the array to push, starting from index zero
	 */
	public synchronized void pushString(byte[] b, int count) {
		check();
		lua_pushbytearray(b, count);
	}

	// -- Stack type test
	/**
	 * Returns whether the value at the specified stack index is a boolean.
	 * 
	 * <p>
	 * The stack index may be undefined.
	 * </p>
	 * 
	 * @param index
	 *            the stack index
	 * @return whether the value is a boolean
	 */
	public synchronized boolean isBoolean(int index) {
		check();
		return lua_isboolean(index) != 0;
	}

	/**
	 * Returns whether the value at the specified stack index is a C function.
	 * 
	 * <p>
	 * The stack index may be undefined.
	 * </p>
	 * 
	 * @param index
	 *            the stack index
	 * @return whether the value is a function
	 */
	public synchronized boolean isCFunction(int index) {
		check();
		return lua_iscfunction(index) != 0;
	}

	/**
	 * Returns whether the value at the specified stack index is a function
	 * (either a C function, a Java function or a Lua function.)
	 * 
	 * <p>
	 * The stack index may be undefined.
	 * </p>
	 * 
	 * @param index
	 *            the stack index
	 * @return whether the value is a function
	 */
	public synchronized boolean isFunction(int index) {
		check();
		return lua_isfunction(index) != 0;
	}

	/**
	 * Returns whether the value at the specified stack index is a Java
	 * function.
	 * 
	 * <p>
	 * The stack index may be undefined.
	 * </p>
	 * 
	 * @param index
	 *            the stack index
	 * @return whether the value is a function
	 */
	public synchronized boolean isJavaFunction(int index) {
		check();
		return lua_isjavafunction(index) != 0;
	}

	/**
	 * Returns whether the value at the specified stack index is a Java object.
	 * 
	 * <p>
	 * Note that the method does not perform conversion. If you want to check if
	 * a value <i>is convertible to</i> a Java object, then invoke <code>
	 * isJavaObject(index, Object.class)</code>.
	 * </p>
	 * 
	 * <p>
	 * The stack index may be undefined.
	 * </p>
	 * 
	 * @param index
	 *            the stack index
	 * @return whether the value is a Java object
	 * @see #isJavaObject(int, Class)
	 */
	public synchronized boolean isJavaObjectRaw(int index) {
		check();
		return lua_isjavaobject(index) != 0;
	}

	/**
	 * Returns whether the value at the specified stack index is convertible to
	 * a Java object of the specified type. The conversion is checked by the
	 * configured converter.
	 * 
	 * <p>
	 * The stack index may be undefined.
	 * </p>
	 * 
	 * @param index
	 *            the stack index
	 * @return whether the value is convertible to a Java object of the
	 *         specified type
	 * @see #setConverter(Converter)
	 * @see #getConverter()
	 */
	public synchronized boolean isJavaObject(int index, Class<?> type) {
		check();
		return converter.getTypeDistance(this, index, type) != Integer.MAX_VALUE;
	}

	/**
	 * Returns whether the value at the specified stack index is
	 * <code>nil</code>.
	 * 
	 * <p>
	 * The stack index may be undefined.
	 * </p>
	 * 
	 * @param index
	 *            the stack index
	 * @return whether the value is <code>nil</code>
	 */
	public synchronized boolean isNil(int index) {
		check();
		return lua_isnil(index) != 0;
	}

	/**
	 * Returns whether the value at the specified stack index is undefined.
	 * 
	 * <p>
	 * The stack index may be undefined.
	 * </p>
	 * 
	 * @param index
	 *            the stack index
	 * @return whether the value is undefined
	 */
	public synchronized boolean isNone(int index) {
		check();
		return lua_isnone(index) != 0;
	}

	/**
	 * Returns whether the value at the specified stack index is undefined or
	 * <code>nil</code>.
	 * 
	 * <p>
	 * The stack index may be undefined.
	 * </p>
	 * 
	 * @param index
	 *            the stack index
	 * @return whether the value is undefined
	 */
	public synchronized boolean isNoneOrNil(int index) {
		check();
		return lua_isnoneornil(index) != 0;
	}

	/**
	 * Returns whether the value at the specified stack index is a number or a
	 * string convertible to a number.
	 * 
	 * <p>
	 * The stack index may be undefined.
	 * </p>
	 * 
	 * @param index
	 *            the stack index
	 * @return whether the value is a number or a string convertible to a number
	 */
	public synchronized boolean isNumber(int index) {
		check();
		return lua_isnumber(index) != 0;
	}

	/**
	 * Returns whether the value at the specified stack index is a string or a
	 * number (which is always convertible to a string.)
	 * 
	 * <p>
	 * The stack index may be undefined.
	 * </p>
	 * 
	 * @param index
	 *            the stack index
	 * @return whether the value is a string or a number
	 */
	public synchronized boolean isString(int index) {
		check();
		return lua_isstring(index) != 0;
	}

	/**
	 * Returns whether the value at the specified stack index is a table.
	 * 
	 * <p>
	 * The stack index may be undefined.
	 * </p>
	 * 
	 * @param index
	 *            the stack index
	 * @return whether the value is a table
	 */
	public synchronized boolean isTable(int index) {
		check();
		return lua_istable(index) != 0;
	}

	/**
	 * Returns whether the value at the specified stack index is a thread.
	 * 
	 * <p>
	 * The stack index may be undefined.
	 * </p>
	 * 
	 * @param index
	 *            the stack index
	 * @return whether the value is a thread
	 */
	public synchronized boolean isThread(int index) {
		check();
		return lua_isthread(index) != 0;
	}

	// -- Stack query
	/**
	 * Returns whether the values at two specified stack indexes are equal
	 * according to Lua semantics.
	 * 
	 * @param index1
	 *            the first stack index
	 * @param index2
	 *            the second stack index
	 * @return whether the values are equal
	 */
	public synchronized boolean equal(int index1, int index2) {
		check();
		return lua_equal(index1, index2) != 0;
	}

	/**
	 * Returns whether a value at a first stack index is less than the value at
	 * a second stack index according to Lua semantics.
	 * 
	 * @param index1
	 *            the first stack index
	 * @param index2
	 *            the second stack index
	 * @return whether the value at the first index is less than the value at
	 *         the second index
	 */
	public synchronized boolean lessThan(int index1, int index2)
			throws LuaMemoryAllocationException, LuaRuntimeException {
		check();
		return lua_lessthan(index1, index2) != 0;
	}

	/**
	 * Returns the length of the value at the specified stack index. The
	 * definition of the length depends on the type of the value. For strings,
	 * it is the length of the string, for tables it is the result of the length
	 * operator. For other types, the return value is undefined.
	 * 
	 * @param index
	 *            the stack index
	 * @return the length
	 */
	public synchronized int length(int index) {
		check();
		return lua_objlen(index);
	}

	/**
	 * Bypassing metatable logic, returns whether the values at two specified
	 * stack indexes are equal according to Lua semantics.
	 * 
	 * @param index1
	 *            the first stack index
	 * @param index2
	 *            the second stack index
	 * @return whether the values are equal
	 */
	public synchronized boolean rawEqual(int index1, int index2) {
		check();
		return lua_rawequal(index1, index2) != 0;
	}

	/**
	 * Returns the boolean representation of the value at the specified stack
	 * index. The boolean representation is <code>true</code> for all values
	 * except <code>false</code> and <code>nil</code>.
	 * 
	 * @param index
	 *            the stack index
	 * @return the boolean representation of the value
	 */
	public synchronized boolean toBoolean(int index) {
		check();
		return lua_toboolean(index) != 0;
	}

	/**
	 * Returns the byte array representation of the value at the specified stack
	 * index. The value must be a string or a number. If the value is a number,
	 * it is in place converted to a string. Otherwise, the method returns
	 * <code>null</code>.
	 * 
	 * @param index
	 *            the stack index
	 * @return the byte array representation of the value
	 */
	public synchronized byte[] toByteArray(int index) {
		check();
		return lua_tobytearray(index);
	}

	/**
	 * Returns the integer representation of the value at the specified stack
	 * index. The value must be a number or a string convertible to a number.
	 * Otherwise, the method returns <code>0</code>.
	 * 
	 * @param index
	 *            the stack index
	 * @return the integer representation, or <code>0</code>
	 */
	public synchronized int toInteger(int index) {
		check();
		return lua_tointeger(index);
	}

	/**
	 * Returns the Java function of the value at the specified stack index. If
	 * the value is not a Java function, the method returns <code>null</code>.
	 * 
	 * @param index
	 *            the stack index
	 * @return the Java function, or <code>null</code>
	 */
	public synchronized JavaFunction toJavaFunction(int index) {
		check();
		return lua_tojavafunction(index);
	}

	/**
	 * Returns the Java object of the value at the specified stack index. If the
	 * value is not a Java object, the method returns <code>null</code>.
	 * 
	 * <p>
	 * Note that the method does not convert values to Java objects. If you
	 * require <i>any</i> Java object that represents the value at the specified
	 * index, then invoke <code>toJavaObject(index, Object.class)</code>.
	 * </p>
	 * 
	 * @param index
	 *            the stack index
	 * @return the Java object, or <code>null</code>
	 * @see #toJavaObject(int, Class)
	 */
	public synchronized Object toJavaObjectRaw(int index) {
		check();
		return lua_tojavaobject(index);
	}

	/**
	 * Returns a Java object of the specified type representing the value at the
	 * specified stack index. The value must be convertible to a Java object of
	 * the specified type. The conversion is executed by the configured
	 * converter.
	 * 
	 * @param index
	 *            the stack index
	 * @param type
	 *            the Java type to convert to
	 * @return the object
	 * @throws ClassCastException
	 *             if the conversion is not supported by the converter
	 * @see #getConverter()
	 * @see #setConverter(Converter)
	 */
	public synchronized <T> T toJavaObject(int index, Class<T> type) {
		check();
		return converter.convertLuaValue(this, index, type);
	}

	/**
	 * Returns the number representation of the value at the specified stack
	 * index. The value must be a number or a string convertible to a number.
	 * Otherwise, the method returns <code>0.0</code>.
	 * 
	 * @param index
	 *            the stack index
	 * @return the number representation, or <code>0.0</code>
	 */
	public synchronized double toNumber(int index) {
		check();
		return lua_tonumber(index);
	}

	/**
	 * Returns the pointer representation of the value at the specified stack
	 * index. The value must be a table, thread, function or userdata (such as a
	 * Java object.) Otherwise, the method returns <code>0L</code>. Different
	 * values return different pointers. Other than that, the returned value has
	 * no portable significance.
	 * 
	 * @param index
	 *            the stack index
	 * @return the pointer representation, or <code>0L</code> if none
	 */
	public synchronized long toPointer(int index) {
		check();
		return lua_topointer(index);
	}

	/**
	 * Returns the string representation of the value at the specified stack
	 * index. The value must be a string or a number. If the value is a number,
	 * it is in place converted to a string. Otherwise, the method returns
	 * <code>null</code>.
	 * 
	 * @param index
	 *            the stack index
	 * @return the string representation, or <code>null</code>
	 */
	public synchronized String toString(int index) {
		check();
		return lua_tostring(index);
	}

	/**
	 * Returns the type of the value at the specified stack index.
	 * 
	 * <p>
	 * The stack index may be undefined.
	 * </p>
	 * 
	 * @param index
	 *            the stack index
	 * @return the type, or <code>null</code> if the stack index is undefined
	 */
	public synchronized LuaType type(int index) {
		check();
		int type = lua_type(index);
		return type >= 0 ? LuaType.values()[type] : null;
	}

	/**
	 * Returns the name of the type at the specified stack index. The type name
	 * is the display text for the Lua type except for Java objects where the
	 * type name is the canonical class name.
	 * 
	 * <p>
	 * The stack index may be undefined.
	 * </p>
	 * 
	 * @param index
	 *            the index
	 * @return the type name
	 * @see LuaType#displayText()
	 * @see Class#getCanonicalName()
	 */
	public synchronized String typeName(int index) {
		check();
		LuaType type = type(index);
		if (type == null) {
			return "no value";
		}
		switch (type) {
		case USERDATA:
			if (isJavaObjectRaw(index)) {
				Object object = toJavaObjectRaw(index);
				Class<?> clazz;
				if (object instanceof Class<?>) {
					clazz = (Class<?>) object;
				} else {
					clazz = object.getClass();
				}
				return clazz.getCanonicalName();
			}
			break;
		}
		return type.displayText();
	}

	// -- Stack operation
	/**
	 * Concatenates the specified number values on top of the stack and replaces
	 * them with the concatenated value.
	 * 
	 * @param n
	 *            the number of values to concatenate
	 */
	public synchronized void concat(int n) {
		check();
		lua_concat(n);
	}

	/**
	 * Returns the number of values on the stack.
	 * 
	 * @return the number of values on the tack
	 */
	public synchronized int getTop() {
		check();
		return lua_gettop();
	}

	/**
	 * Pops the value on top of the stack inserting it at the specified index
	 * and moving up elements above that index.
	 * 
	 * @param index
	 *            the stack index
	 */
	public synchronized void insert(int index) {
		check();
		lua_insert(index);
	}

	/**
	 * Pops values from the stack.
	 * 
	 * @param count
	 *            the number of values to pop
	 */
	public synchronized void pop(int count) {
		check();
		lua_pop(count);
	}

	/**
	 * Pushes the value at the specified index on top of the stack.
	 * 
	 * @param index
	 *            the stack index
	 */
	public synchronized void pushValue(int index) {
		check();
		lua_pushvalue(index);
	}

	/**
	 * Removes the value at the specified stack index moving down elements above
	 * that index.
	 * 
	 * @param index
	 *            the stack index
	 */
	public synchronized void remove(int index) {
		check();
		lua_remove(index);
	}

	/**
	 * Replaces the value at the specified index with the value popped from the
	 * top of the stack.
	 * 
	 * @param index
	 *            the stack index
	 */
	public synchronized void replace(int index) {
		check();
		lua_replace(index);
	}

	/**
	 * Sets the specified index as the new top of the stack.
	 * 
	 * <p>
	 * The new top of the stack may be above the current top of the stack. In
	 * this case, new values are set to <code>nil</code>.
	 * </p>
	 * 
	 * @param index
	 *            the index of the new top of the stack
	 */
	public synchronized void setTop(int index) {
		check();
		lua_settop(index);
	}

	// -- Table
	/**
	 * Pushes on the stack the value indexed by the key on top of the stack in
	 * the table at the specified index. The key is replaced by the value from
	 * the table.
	 * 
	 * @param index
	 *            the stack index containing the table
	 */
	public synchronized void getTable(int index) {
		check();
		lua_gettable(index);
	}

	/**
	 * Pushes on the stack the value indexed by the specified string key in the
	 * table at the specified index.
	 * 
	 * @param index
	 *            the stack index containing the table
	 * @param key
	 *            the string key
	 */
	public synchronized void getField(int index, String key) {
		check();
		lua_getfield(index, key);
	}

	/**
	 * Creates a new table and pushes it on the stack.
	 */
	public synchronized void newTable() {
		check();
		lua_newtable();
	}

	/**
	 * Creates a new table with pre-allocated space for a number of array
	 * elements and record elements and pushes it on the stack.
	 * 
	 * @param arrayCount
	 *            the number of array elements
	 * @param recordCount
	 *            the number of record elements
	 */
	public synchronized void newTable(int arrayCount, int recordCount) {
		check();
		lua_createtable(arrayCount, recordCount);
	}

	/**
	 * Pops a key from the stack and pushes on the stack the next key and its
	 * value in the table at the specified index. If there is no next key, the
	 * key is popped but nothing is pushed. The method returns whether there is
	 * a next key.
	 * 
	 * @param index
	 *            the stack index containing the table
	 * @return whether there is a next key
	 */
	public synchronized boolean next(int index) {
		check();
		return lua_next(index) != 0;
	}

	/**
	 * Bypassing metatable logic, pushes on the stack the value indexed by the
	 * key on top of the stack in the table at the specified index. The key is
	 * replaced by the value from the table.
	 * 
	 * @param index
	 *            the stack index containing the table
	 */
	public synchronized void rawGet(int index) {
		check();
		lua_rawget(index);
	}

	/**
	 * Bypassing metatable logic, pushes on the stack the value indexed by the
	 * specified integer key in the table at the specified index.
	 * 
	 * @param index
	 *            the stack index containing the table
	 * @param key
	 *            the integer key
	 */
	public synchronized void rawGet(int index, int key) {
		check();
		lua_rawgeti(index, key);
	}

	/**
	 * Bypassing metatable logic, sets the value on top of the stack in the
	 * table at the specified index using the value on the second highest stack
	 * position as the key. Both the value and the key are popped from the
	 * stack.
	 * 
	 * @param index
	 *            the stack index containing the table
	 */
	public synchronized void rawSet(int index) {
		check();
		lua_rawset(index);
	}

	/**
	 * Bypassing metatable logic, sets the value on top of the stack in the
	 * table at the specified index using the specified integer key. The value
	 * is popped from the stack.
	 * 
	 * @param index
	 *            the stack index containing the table
	 * @param key
	 *            the integer key
	 */
	public synchronized void rawSet(int index, int key) {
		check();
		lua_rawseti(index, key);
	}

	/**
	 * Sets the value on top of the stack in the table at the specified index
	 * using the value on the second highest stack position as the key. Both the
	 * value and the key are popped from the stack.
	 * 
	 * @param index
	 *            the stack index containing the table
	 */
	public synchronized void setTable(int index) {
		check();
		lua_settable(index);
	}

	/**
	 * Sets the value on top of the stack in the table at the specified index
	 * using the specified string key. The value is popped from the stack.
	 * 
	 * @param index
	 *            the stack index containing the table
	 * @param key
	 *            the string key
	 */
	public synchronized void setField(int index, String key) {
		check();
		lua_setfield(index, key);
	}

	// -- Metatable
	/**
	 * Pushes on the stack the value of the named field in the metatable of the
	 * value at the specified index and returns <code>true</code>. If the value
	 * does not have a metatable or if the metatable does not contain the named
	 * field, nothing is pushed and the method returns <code>false</code>.
	 * 
	 * @param index
	 *            the stack index containing the value to get the metafield from
	 * @param key
	 *            the string key
	 * @return whether the metafield was pushed on the stack
	 */
	public synchronized boolean getMetafield(int index, String key) {
		check();
		return lua_getmetafield(index, key) != 0;
	}

	/**
	 * Pushes on the stack the metatable of the value at the specified index. If
	 * the value does not have a metatable, the method returns
	 * <code>false</code> and nothing is pushed.
	 * 
	 * @param index
	 *            the stack index containing the value to get the metatable from
	 * @return whether the metatable was pushed on the stack
	 */
	public synchronized boolean getMetatable(int index) {
		check();
		return lua_getmetatable(index) != 0;
	}

	/**
	 * Sets the value on top of the stack as the metatable of the value at the
	 * specified index. The metatable to be set is popped from the stack
	 * regardless whether it can be set or not.
	 * 
	 * @param index
	 *            the stack index containing the value to set the metatable for
	 * @return whether the metatable was set
	 */
	public synchronized boolean setMetatable(int index) {
		check();
		return lua_setmetatable(index) != 0;
	}

	// -- Environment table
	/**
	 * Pushes on the stack the environment table of the value at the specified
	 * index. If the value does not have an environment table, <code>nil</code>
	 * is pushed on the stack.
	 * 
	 * @param index
	 *            the stack index containing the value to get the environment
	 *            table from
	 */
	public synchronized void getFEnv(int index) {
		check();
		lua_getfenv(index);
	}

	/**
	 * Sets the value on top of the stack as the environment table of the value
	 * at the specified index. The environment table to be set is popped from
	 * the stack regardless whether it can be set or not.
	 * 
	 * @param index
	 *            the stack index containing the value to set the environment
	 *            table for
	 * @return whether the environment table was set
	 */
	public synchronized boolean setFEnv(int index) {
		check();
		return lua_setfenv(index) != 0;
	}

	// -- Thread
	/**
	 * Pops the start function of a new Lua thread from the stack and creates
	 * the new thread with that start function. The new thread is pushed on the
	 * stack.
	 */
	public synchronized void newThread() {
		check();
		lua_newthread();
	}

	/**
	 * Resumes the thread at the specified stack index, popping the specified
	 * number of arguments from the top of the stack and passing them to the
	 * resumed thread. The method returns the number of values pushed on the
	 * stack as the return values of the resumed thread.
	 * 
	 * @param index
	 *            the stack index containing the thread
	 * @param argCount
	 *            the number of arguments to pass
	 * @return the number of values returned by the thread
	 */
	public synchronized int resume(int index, int argCount) {
		check();
		return lua_resume(index, argCount);
	}

	/**
	 * Returns the status of the thread at the specified stack index. If the
	 * thread is in initial state of has finished its execution, the method
	 * returns <code>0</code>. If the thread has yielded, the method returns
	 * {@link #YIELD}. Other return values indicate errors for which an
	 * exception has been thrown.
	 * 
	 * @param index
	 *            the index
	 * @return the status
	 */
	public synchronized int status(int index) {
		check();
		return lua_status(index);
	}

	/**
	 * Yields the running thread, popping the specified number of values from
	 * the top of the stack and passing them as return values to the thread
	 * which has resumed the running thread. The method must be used exclusively
	 * at the exit point of Java functions, i.e.
	 * <code>return luaState.yield(n)</code>.
	 * 
	 * @param returnCount
	 *            the number of results to pass
	 * @return the return value of the Java function
	 */
	public synchronized int yield(int returnCount) {
		check();
		return lua_yield(returnCount);
	}

	// -- Reference
	/**
	 * Stores the value on top of the stack in the table at the specified index
	 * and returns the integer key of the value in that table as a reference.
	 * The value is popped from the stack.
	 * 
	 * @param index
	 *            the stack index containing the table where to store the value
	 * @return the reference integer key
	 * @see #unref(int, int)
	 */
	public synchronized int ref(int index) {
		check();
		return lua_ref(index);

	}

	/**
	 * Removes a previously created reference from the table at the specified
	 * index. The value is removed from the table and its integer key of the
	 * reference is freed for reuse.
	 * 
	 * @param index
	 *            the stack index containing the table where the value was
	 *            stored
	 * @param reference
	 *            the reference integer key
	 * @see #ref(int)
	 */
	public synchronized void unref(int index, int reference) {
		check();
		lua_unref(index, reference);
	}

	// -- Optimization
	/**
	 * Counts the number of entries in a table.
	 * 
	 * <p>
	 * The method provides optimized performance over a Java implementation of
	 * the same functionality due to the reduced number of JNI transitions.
	 * </p>
	 * 
	 * @param index
	 *            the stack index containing the table
	 * @return the number of entries in the table
	 */
	public synchronized int tableSize(int index) {
		check();
		return lua_tablesize(index);
	}

	/**
	 * Moves the specified number of sequential elements in a table used as an
	 * array from one index to another.
	 * 
	 * <p>
	 * The method provides optimized performance over a Java implementation of
	 * the same functionality due to the reduced number of JNI transitions.
	 * </p>
	 * 
	 * @param index
	 *            the stack index containing the table
	 * @param from
	 *            the index to move from
	 * @param to
	 *            the index to move to
	 * @param count
	 *            the number of elements to move
	 */
	public synchronized void tableMove(int index, int from, int to, int count) {
		check();
		lua_tablemove(index, from, to, count);
	}

	// -- Argument checking
	/**
	 * Checks if a condition is true for the specified function argument. If
	 * not, the method throws a Lua runtime exception with the specified error
	 * message.
	 * 
	 * @param index
	 *            the argument index
	 * @param condition
	 *            the condition
	 * @param msg
	 *            the error message
	 */
	public synchronized void checkArg(int index, boolean condition, String msg) {
		check();
		if (condition) {
			return;
		}
		throw getArgException(index, msg);
	}

	/**
	 * Checks if the value of the specified function argument is a boolean. If
	 * so, the argument value is returned as a boolean. Otherwise, the method
	 * throws a Lua runtime exception with a descriptive error message.
	 * 
	 * @param index
	 *            the argument index
	 * @return the boolean value, or the default value
	 */
	public synchronized boolean checkBoolean(int index) {
		check();
		if (!isBoolean(index)) {
			throw getArgTypeException(index, LuaType.BOOLEAN);
		}
		return toBoolean(index);
	}

	/**
	 * Checks if the value of the specified function argument is a boolean. If
	 * so, the argument value is returned as a boolean. If the value of the
	 * specified argument is undefined or <code>nil</code>, the method returns
	 * the specified default value. Otherwise, the method throws a Lua runtime
	 * exception with a descriptive error message.
	 * 
	 * @param index
	 *            the argument index
	 * @param d
	 *            the default value
	 * @return the boolean value
	 */
	public synchronized boolean checkBoolean(int index, boolean d) {
		check();
		if (isNoneOrNil(index)) {
			return d;
		}
		return checkBoolean(index);
	}

	/**
	 * Checks if the value of the specified function argument is a string or a
	 * number. If so, the argument value is returned as a byte array. Otherwise,
	 * the method throws a Lua runtime exception with a descriptive error
	 * message.
	 * 
	 * @param index
	 *            the argument index
	 * @return the byte array value
	 */
	public synchronized byte[] checkByteArray(int index) {
		check();
		if (!isString(index)) {
			throw getArgTypeException(index, LuaType.STRING);
		}
		return toByteArray(index);
	}

	/**
	 * Checks if the value of the specified function argument is a string or a
	 * number. If so, the argument value is returned as a byte array. If the
	 * value of the specified argument is undefined or <code>nil</code>, the
	 * method returns the specified default value. Otherwise, the method throws
	 * a Lua runtime exception with a descriptive error message.
	 * 
	 * @param index
	 *            the argument index
	 * @param d
	 *            the default value
	 * @return the string value, or the default value
	 */
	public synchronized byte[] checkByteArray(int index, byte[] d) {
		check();
		if (isNoneOrNil(index)) {
			return d;
		}
		return checkByteArray(index);
	}

	/**
	 * Checks if the value of the specified function argument is a number or a
	 * string convertible to a number. If so, the argument value is returned as
	 * an integer. Otherwise, the method throws a Lua runtime exception with a
	 * descriptive error message.
	 * 
	 * @param index
	 *            the argument index
	 * @return the integer value
	 */
	public synchronized int checkInteger(int index) {
		check();
		if (!isNumber(index)) {
			throw getArgTypeException(index, LuaType.NUMBER);
		}
		return toInteger(index);
	}

	/**
	 * Checks if the value of the specified function argument is a number or a
	 * string convertible to a number. If so, the argument value is returned as
	 * an integer. If the value of the specified argument is undefined or
	 * <code>nil</code>, the method returns the specified default value.
	 * Otherwise, the method throws a Lua runtime exception with a descriptive
	 * error message.
	 * 
	 * @param index
	 *            the argument index
	 * @param d
	 *            the default value
	 * @return the integer value, or the default value
	 */
	public synchronized int checkInteger(int index, int d) {
		check();
		if (isNoneOrNil(index)) {
			return d;
		}
		return checkInteger(index);
	}

	/**
	 * Checks if the value of the specified function argument is a number or a
	 * string convertible to a number. If so, the argument value is returned as
	 * a number. Otherwise, the method throws a Lua runtime exception with a
	 * descriptive error message.
	 * 
	 * @param index
	 *            the argument index
	 * @return the number value
	 */
	public synchronized double checkNumber(int index) {
		check();
		if (!isNumber(index)) {
			throw getArgTypeException(index, LuaType.NUMBER);
		}
		return toNumber(index);
	}

	/**
	 * Checks if the value of the specified function argument is a number or a
	 * string convertible to a number. If so, the argument value is returned as
	 * a number. If the value of the specified argument is undefined or
	 * <code>nil</code>, the method returns the specified default value.
	 * Otherwise, the method throws a Lua runtime exception with a descriptive
	 * error message.
	 * 
	 * @param index
	 *            the argument index
	 * @param d
	 *            the default value
	 * @return the number value, or the default value
	 */
	public synchronized double checkNumber(int index, double d) {
		check();
		if (isNoneOrNil(index)) {
			return d;
		}
		return checkNumber(index);
	}

	/**
	 * Checks if the value of the specified function argument is convertible to
	 * a Java object of the specified type. If so, the argument value is
	 * returned as a Java object of the specified type. Otherwise, the method
	 * throws a Lua runtime exception with a descriptive error message.
	 * 
	 * <p>
	 * Note that the converter converts <code>nil</code> to <code>null</code>.
	 * Therefore, the method may return <code>null</code> if the value is
	 * <code>nil</code>.
	 * </p>
	 * 
	 * @param index
	 *            the argument index
	 * @param clazz
	 *            the expected type
	 * @return the Java object, or <code>null</code>
	 */
	public synchronized <T> T checkJavaObject(int index, Class<T> clazz) {
		check();
		if (!isJavaObject(index, clazz)) {
			throw getArgException(
					index,
					String.format("exptected %s, got %s",
							clazz.getCanonicalName(), typeName(index)));
		}
		return toJavaObject(index, clazz);
	}

	/**
	 * Checks if the value of the specified function argument is convertible to
	 * a Java object of the specified type. If so, the argument value is
	 * returned as a Java object of the specified type. If the value of the
	 * specified argument is undefined or <code>nil</code>, the method returns
	 * the specified default value. Otherwise, the method throws a Lua runtime
	 * exception with a descriptive error message.
	 * 
	 * @param index
	 *            the argument index
	 * @param clazz
	 *            the expected class
	 * @param d
	 *            the default value
	 * @return the Java object, or the default value
	 */
	public synchronized <T> T checkJavaObject(int index, Class<T> clazz, T d) {
		check();
		if (isNoneOrNil(index)) {
			return d;
		}
		return checkJavaObject(index, clazz);
	}

	/**
	 * Checks if the value of the specified function argument is a string or a
	 * number matching one of the specified options. If so, the argument value
	 * is returned as a string. Otherwise, the method throws a Lua runtime
	 * exception with a descriptive error message.
	 * 
	 * @param index
	 *            the argument index
	 * @param options
	 *            the options
	 * @return the string value
	 */
	public synchronized String checkOption(int index, String[] options) {
		check();
		String s = checkString(index);
		for (int i = 0; i < options.length; i++) {
			if (s.equals(options[i])) {
				return s;
			}
		}
		throw getArgException(
				index,
				String.format("expected one of %s, got %s",
						Arrays.asList(options), s));
	}

	/**
	 * Checks if the value of the specified function argument is a string or a
	 * number matching one of the specified options. If so, argument value is
	 * returned as a string. If the value of the specified argument is undefined
	 * or <code>nil</code>, the method returns the specified default value.
	 * Otherwise, the method throws a Lua runtime exception with a descriptive
	 * error message.
	 * 
	 * @param index
	 *            the argument index
	 * @param options
	 *            the options
	 * @param d
	 *            the default value
	 * @return the string value, or the default value
	 */
	public synchronized String checkOption(int index, String[] options, String d) {
		check();
		if (isNoneOrNil(index)) {
			return d;
		}
		return checkOption(index, options);
	}

	/**
	 * Checks if the value of the specified function argument is a string or a
	 * number. If so, the argument value is returned as a string. Otherwise, the
	 * method throws a Lua runtime exception with a descriptive error message.
	 * 
	 * @param index
	 *            the argument index
	 * @return the string value
	 */
	public synchronized String checkString(int index) {
		check();
		if (!isString(index)) {
			throw getArgTypeException(index, LuaType.STRING);
		}
		return toString(index);
	}

	/**
	 * Checks if the value of the specified function argument is a string or a
	 * number. If so, the argument value is returned as a string. If the value
	 * of the specified argument is undefined or <code>nil</code>, the method
	 * returns the specified default value. Otherwise, the method throws a Lua
	 * runtime exception with a descriptive error message.
	 * 
	 * @param index
	 *            the argument index
	 * @param d
	 *            the default value
	 * @return the string value, or the default value
	 */
	public synchronized String checkString(int index, String d) {
		check();
		if (isNoneOrNil(index)) {
			return d;
		}
		return checkString(index);
	}

	/**
	 * Checks if the value of the specified function argument is of the
	 * specified type. If not, the method throws a Lua runtime exception with a
	 * descriptive error message.
	 * 
	 * @param index
	 *            the argument index
	 * @param type
	 *            the type
	 */
	public synchronized void checkType(int index, LuaType type) {
		check();
		if (type(index) != type) {
			throw getArgTypeException(index, type);
		}
	}

	// -- Proxy
	/**
	 * Returns a proxy object for the Lua value at the specified index.
	 * 
	 * @param index
	 *            the stack index containing the Lua value
	 * @return the Lua value proxy
	 */
	public synchronized LuaValueProxy getProxy(int index) {
		pushValue(index);
		return new LuaValueProxyImpl(ref(REGISTRYINDEX));
	}

	/**
	 * Returns a proxy object implementing the specified interface in Lua. The
	 * table at the specified stack index contains the method names from the
	 * interface as keys and the Lua functions implementing the interface
	 * methods as values. The returned object always implements the
	 * {@link LuaValueProxy} interface in addition to the specified interface.
	 * 
	 * @param index
	 *            the stack index containing the table
	 * @param interfaze
	 *            the interface
	 * @return the proxy object
	 */
	@SuppressWarnings("unchecked")
	public synchronized <T> T getProxy(int index, Class<T> interfaze) {
		return (T) getProxy(index, new Class<?>[] { interfaze });
	}

	/**
	 * Returns a proxy object implementing the specified list of interfaces in
	 * Lua. The table at the specified stack index contains the method names
	 * from the interfaces as keys and the Lua functions implementing the
	 * interface methods as values. The returned object always implements the
	 * {@link LuaValueProxy} interface in addition to the specified interfaces.
	 * 
	 * @param index
	 *            the stack index containing the table
	 * @param interfaces
	 *            the interfaces
	 * @return the proxy object
	 */
	public synchronized LuaValueProxy getProxy(int index, Class<?>[] interfaces) {
		pushValue(index);
		if (!isTable(index)) {
			throw new IllegalArgumentException(String.format(
					"index %d is not a table", index));
		}
		Class<?>[] allInterfaces = new Class<?>[interfaces.length + 1];
		System.arraycopy(interfaces, 0, allInterfaces, 0, interfaces.length);
		allInterfaces[allInterfaces.length - 1] = LuaValueProxy.class;
		int reference = ref(REGISTRYINDEX);
		try {
			Object proxy = Proxy.newProxyInstance(classLoader, allInterfaces,
					new LuaInvocationHandler(reference));
			reference = -1;
			return (LuaValueProxy) proxy;
		} finally {
			if (reference >= 0) {
				unref(REGISTRYINDEX, reference);
			}
		}
	}

	// -- Private methods
	/**
	 * Returns whether this Lua state is open.
	 */
	private boolean isOpenInternal() {
		return luaState != 0L;
	}

	/**
	 * Closes this Lua state.
	 */
	private void closeInternal() {
		if (isOpenInternal()) {
			lua_close(ownState);
			if (isOpenInternal()) {
				throw new IllegalStateException("cannot close");
			}
		}
	}

	/**
	 * Checks this Lua state.
	 */
	private void check() {
		// Check open
		if (!isOpenInternal()) {
			throw new IllegalStateException("Lua state is closed");
		}

		// Check proxy queue
		LuaValueProxyRef luaValueProxyRef;
		while ((luaValueProxyRef = (LuaValueProxyRef) proxyQueue.poll()) != null) {
			proxySet.remove(luaValueProxyRef);
			lua_unref(REGISTRYINDEX, luaValueProxyRef.getReference());
		}
	}

	/**
	 * Creates a Lua runtime exception to indicate an argument type error.
	 */
	private LuaRuntimeException getArgTypeException(int index, LuaType type) {
		return getArgException(index,
				String.format("expected %s, got %s", type.toString()
						.toLowerCase(), type(index).toString().toLowerCase()));
	}

	/**
	 * Creates a Lua runtime exception to indicate an argument error.
	 * 
	 * @param extraMsg
	 * @return
	 */
	private LuaRuntimeException getArgException(int index, String extraMsg) {
		check();
		String funcName = lua_funcname();
		index = lua_narg(index);
		String msg;
		String argument = index > 0 ? String.format("argument #%d", index)
				: "self argument";
		if (funcName != null) {
			msg = String.format("bad %s to '%s' (%s)", argument, funcName,
					extraMsg);
		} else {
			msg = String.format("bad %s (%s)", argument, extraMsg);
		}
		return new LuaRuntimeException(msg);
	}

	// -- Native methods
	private static native String lua_version();

	private native void lua_newstate(int apiversion, long luaState);

	private native void lua_close(boolean ownState);

	private native int lua_gc(int what, int data);

	private native void lua_openlib(int lib);

	private native void lua_load(InputStream inputStream, String chunkname)
			throws IOException;

	private native void lua_dump(OutputStream outputStream) throws IOException;

	private native void lua_pcall(int nargs, int nresults);

	private native void lua_getglobal(String name);

	private native void lua_setglobal(String name);

	private native void lua_pushboolean(int b);

	private native void lua_pushbytearray(byte[] b, int count);

	private native void lua_pushinteger(int n);

	private native void lua_pushjavafunction(JavaFunction f);

	private native void lua_pushjavaobject(Object object);

	private native void lua_pushnil();

	private native void lua_pushnumber(double n);

	private native void lua_pushstring(String s);

	private native int lua_isboolean(int index);

	private native int lua_iscfunction(int index);

	private native int lua_isfunction(int index);

	private native int lua_isjavafunction(int index);

	private native int lua_isjavaobject(int index);

	private native int lua_isnil(int index);

	private native int lua_isnone(int index);

	private native int lua_isnoneornil(int index);

	private native int lua_isnumber(int index);

	private native int lua_isstring(int index);

	private native int lua_istable(int index);

	private native int lua_isthread(int index);

	private native int lua_equal(int index1, int index2);

	private native int lua_lessthan(int index1, int index2);

	private native int lua_objlen(int index);

	private native int lua_rawequal(int index1, int index2);

	private native int lua_toboolean(int index);

	private native byte[] lua_tobytearray(int index);

	private native int lua_tointeger(int index);

	private native JavaFunction lua_tojavafunction(int index);

	private native Object lua_tojavaobject(int index);

	private native double lua_tonumber(int index);

	private native long lua_topointer(int index);

	private native String lua_tostring(int index);

	private native void lua_concat(int n);

	private native int lua_gettop();

	private native void lua_insert(int index);

	private native void lua_pop(int n);

	private native void lua_pushvalue(int index);

	private native void lua_remove(int index);

	private native void lua_replace(int index);

	private native void lua_settop(int index);

	private native int lua_type(int index);

	private native void lua_createtable(int narr, int nrec);

	private native String lua_findtable(int idx, String fname, int szhint);

	private native void lua_gettable(int index);

	private native void lua_getfield(int index, String k);

	private native void lua_newtable();

	private native int lua_next(int index);

	private native void lua_rawget(int index);

	private native void lua_rawgeti(int index, int n);

	private native void lua_rawset(int index);

	private native void lua_rawseti(int index, int n);

	private native void lua_settable(int index);

	private native void lua_setfield(int index, String k);

	private native int lua_getmetatable(int index);

	private native int lua_setmetatable(int index);

	private native int lua_getmetafield(int index, String k);

	private native void lua_getfenv(int index);

	private native int lua_setfenv(int index);

	private native void lua_newthread();

	private native int lua_resume(int index, int nargs);

	private native int lua_status(int index);

	private native int lua_yield(int nresults);

	private native int lua_ref(int index);

	private native void lua_unref(int index, int ref);

	private native String lua_funcname();

	private native int lua_narg(int narg);

	private native int lua_tablesize(int index);

	private native void lua_tablemove(int index, int from, int to, int count);

	// -- Enumerated types
	/**
	 * Represents a Lua library.
	 */
	public enum Library {
		/**
		 * The base library, including the coroutine functions.
		 */
		BASE,

		/**
		 * The table library.
		 */
		TABLE,

		/**
		 * The IO library.
		 */
		IO,

		/**
		 * The OS library.
		 */
		OS,

		/**
		 * The string library.
		 */
		STRING,

		/**
		 * The math library.
		 */
		MATH,

		/**
		 * The debug library.
		 */
		DEBUG,

		/**
		 * The package library.
		 */
		PACKAGE,

		/**
		 * The Java library.
		 */
		JAVA {
			@Override
			void open(LuaState luaState) {
				JavaModule.getInstance().open(luaState);
			}
		};

		// -- Methods
		/**
		 * Opens this library.
		 */
		void open(LuaState luaState) {
			luaState.lua_openlib(ordinal());
		}
	}

	/**
	 * Represents a Lua garbage collector action. See the Lua Reference Manual
	 * for an explanation of these actions.
	 */
	public enum GcAction {
		/**
		 * Stop.
		 */
		STOP,

		/**
		 * Restart.
		 */
		RESTART,

		/**
		 * Collect.
		 */
		COLLECT,

		/**
		 * Count memory in kilobytes.
		 */
		COUNT,

		/**
		 * Count reminder in bytes.
		 */
		COUNTB,

		/**
		 * Step.
		 */
		STEP,

		/**
		 * Set pause.
		 */
		SETPAUSE,

		/**
		 * Set step multiplier.
		 */
		SETSTEPMUL
	}

	// -- Nested types
	/**
	 * Phantom reference to a Lua value proxy for pre-mortem cleanup.
	 */
	private static class LuaValueProxyRef extends
			PhantomReference<LuaValueProxyImpl> {
		// -- State
		private int reference;

		// --Construction
		/**
		 * Creates a new instance.
		 */
		public LuaValueProxyRef(LuaValueProxyImpl luaProxyImpl, int reference) {
			super(luaProxyImpl, luaProxyImpl.getLuaState().proxyQueue);
			this.reference = reference;
		}

		// -- Properties
		/**
		 * Returns the reference.
		 */
		public int getReference() {
			return reference;
		}
	}

	/**
	 * Lua value proxy implementation.
	 */
	private class LuaValueProxyImpl implements LuaValueProxy {
		// -- State
		private int reference;

		// -- Construction
		/**
		 * Creates a new instance.
		 */
		public LuaValueProxyImpl(int reference) {
			this.reference = reference;
			proxySet.add(new LuaValueProxyRef(this, reference));
		}

		// -- LuaProxy methods
		@Override
		public LuaState getLuaState() {
			return LuaState.this;
		}

		@Override
		public void pushValue() {
			synchronized (LuaState.this) {
				rawGet(REGISTRYINDEX, reference);
			}
		}
	}

	/**
	 * Invocation handler for implementing Java interfaces in Lua.
	 */
	private class LuaInvocationHandler extends LuaValueProxyImpl implements
			InvocationHandler {
		// -- Construction
		/**
		 * Creates a new instance.
		 */
		public LuaInvocationHandler(int reference) {
			super(reference);
		}

		// -- InvocationHandler methods
		@Override
		public Object invoke(Object proxy, Method method, Object[] args)
				throws Throwable {
			// Handle LuaProxy methods
			if (method.getDeclaringClass() == LuaValueProxy.class) {
				return method.invoke(this, args);
			}

			// Handle Lua calls
			synchronized (LuaState.this) {
				pushValue();
				getField(-1, method.getName());
				if (!isFunction(-1)) {
					pop(2);
					throw new UnsupportedOperationException(method.getName());
				}
				insert(-2);
				int argCount = args != null ? args.length : 0;
				for (int i = 0; i < argCount; i++) {
					pushJavaObject(args[i]);
				}
				int retCount = method.getReturnType() != Void.TYPE ? 1 : 0;
				call(argCount + 1, retCount);
				try {
					return retCount == 1 ? LuaState.this.toJavaObject(-1,
							method.getReturnType()) : null;
				} finally {
					if (retCount == 1) {
						pop(1);
					}
				}
			}
		}
	}
}
