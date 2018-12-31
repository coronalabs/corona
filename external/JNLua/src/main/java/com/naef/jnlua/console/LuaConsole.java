/*
 * $Id: LuaConsole.java 121 2012-01-22 01:40:14Z andre@naef.com $
 * See LICENSE.txt for license terms.
 */

package com.naef.jnlua.console;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

import com.naef.jnlua.LuaException;
import com.naef.jnlua.LuaRuntimeException;
import com.naef.jnlua.LuaState;

/**
 * A simple Lua console.
 * 
 * <p>
 * The console collects input until a line with the sole content of the word
 * <i>go</i> is encountered. At that point, the collected input is run as a Lua
 * chunk. If the Lua chunk loads and runs successfully, the console displays the
 * returned values of the chunk as well as the execution time based on a
 * <code>System.nanoTime()</code> measurement. Otherwise, the console shows the
 * error that has occurred.
 * </p>
 * 
 * <p>
 * Expressions can be printed by prepending <i>=</i> to the expression at the
 * beginning of a chunk. The console translates <i>=</i> into
 * <code>return</code> followed by a space and executes the chunk immediately.
 * No separate <i>go</i> is required. Therefore, expressions printed this way
 * must be entered on a single line.
 * </p>
 */
public class LuaConsole {
	// -- Static
	private static final String[] EMPTY_ARGS = new String[0];

	/**
	 * Main routine.
	 * 
	 * @param args
	 *            the command line arguments
	 */
	public static void main(String[] args) {
		LuaConsole luaConsole = new LuaConsole(args);
		luaConsole.run();
		System.exit(0);
	}

	// -- State
	private LuaState luaState;

	// -- Construction
	/**
	 * Creates a new instance.
	 */
	public LuaConsole() {
		this(EMPTY_ARGS);
	}

	/**
	 * Creates a new instance with the specified command line arguments. The
	 * arguments are passed to Lua as the <code>argv</code> global variable.
	 * 
	 * @param args
	 */
	public LuaConsole(String[] args) {
		luaState = new LuaState();

		// Process arguments
		luaState.newTable(args.length, 0);
		for (int i = 0; i < args.length; i++) {
			luaState.pushString(args[i]);
			luaState.rawSet(-2, i + 1);
		}
		luaState.setGlobal("argv");

		// Open standard libraries
		luaState.openLibs();

		// Set buffer mode
		luaState.load("io.stdout:setvbuf(\"no\")", "setvbuf");
		luaState.call(0, 0);
		luaState.load("io.stderr:setvbuf(\"no\")", "setvbuf");
		luaState.call(0, 0);
	}

	// -- Properties
	/**
	 * Returns the Lua state of this console.
	 * 
	 * @return the Lua state
	 */
	public LuaState getLuaState() {
		return luaState;
	}

	// -- Operations
	/**
	 * Runs the console.
	 */
	public void run() {
		// Banner
		System.out.println(String.format("JNLua %s Console using Lua %s.",
				LuaState.VERSION, LuaState.LUA_VERSION));
		System.out.print("Type 'go' on an empty line to evaluate a chunk. ");
		System.out.println("Type =<expression> to print an expression.");

		// Prepare reader
		BufferedReader bufferedReader = new BufferedReader(
				new InputStreamReader(System.in));
		try {
			// Process chunks
			chunk: while (true) {
				ByteArrayOutputStream out = new ByteArrayOutputStream();
				OutputStreamWriter outWriter = new OutputStreamWriter(out,
						"UTF-8");
				boolean firstLine = true;

				// Process lines
				while (true) {
					String line = bufferedReader.readLine();
					if (line == null) {
						break chunk;
					}
					if (line.equals("go")) {
						outWriter.flush();
						InputStream in = new ByteArrayInputStream(out
								.toByteArray());
						runChunk(in);
						continue chunk;
					}
					if (firstLine && line.startsWith("=")) {
						outWriter.write("return " + line.substring(1));
						outWriter.flush();
						InputStream in = new ByteArrayInputStream(out
								.toByteArray());
						runChunk(in);
						continue chunk;
					}
					outWriter.write(line);
					outWriter.write('\n');
					firstLine = false;
				}
			}
		} catch (IOException e) {
			System.out.print("IO error: ");
			System.out.print(e.getMessage());
			System.out.println();
		}
	}

	/**
	 * Runs a chunk of Lua code from an input stream.
	 */
	protected void runChunk(InputStream in) throws IOException {
		try {
			long start = System.nanoTime();
			luaState.setTop(0);
			luaState.load(in, "console");
			luaState.call(0, LuaState.MULTRET);
			long stop = System.nanoTime();
			for (int i = 1; i <= luaState.getTop(); i++) {
				if (i > 1) {
					System.out.print(", ");
				}
				switch (luaState.type(i)) {
				case BOOLEAN:
					System.out.print(Boolean.valueOf(luaState.toBoolean(i)));
					break;
				case NUMBER:
				case STRING:
					System.out.print(luaState.toString(i));
					break;
				default:
					System.out.print(luaState.typeName(i));
				}
			}
			System.out.print("\t#msec=");
			System.out.print(String.format("%.3f", (stop - start) / 1000000.0));
			System.out.println();
		} catch (LuaRuntimeException e) {
			e.printLuaStackTrace();
			e.printStackTrace();
		} catch (LuaException e) {
			System.err.println(e.getMessage());
		}
	}
}
