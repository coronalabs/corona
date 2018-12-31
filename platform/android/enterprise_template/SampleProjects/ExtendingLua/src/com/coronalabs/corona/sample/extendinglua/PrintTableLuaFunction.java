package com.coronalabs.corona.sample.extendinglua;


/**
 * Implements the printTable() function in Lua.
 * <p>
 * Demonstrates how to fetch a table argument from a Lua function and how to iterate over all of its key/value pairs.
 */
public class PrintTableLuaFunction implements com.naef.jnlua.NamedJavaFunction {
	/**
	 * Gets the name of the Lua function as it would appear in the Lua script.
	 * @return Returns the name of the custom Lua function.
	 */
	@Override
	public String getName() {
		return "printTable";
	}
	
	/**
	 * This method is called when the Lua function is called.
	 * <p>
	 * Warning! This method is not called on the main UI thread.
	 * @param luaState Reference to the Lua state.
	 *                 Needed to retrieve the Lua function's parameters and to return values back to Lua.
	 * @return Returns the number of values to be returned by the Lua function.
	 */
	@Override
	public int invoke(com.naef.jnlua.LuaState luaState) {
		// Print the Lua function's argument to the Android logging system.
		try {
			// Check if the Lua function's first argument is a Lua table.
			// Will throw an exception if it is not a table or if no argument was given.
			int luaTableStackIndex = 1;
			luaState.checkType(luaTableStackIndex, com.naef.jnlua.LuaType.TABLE);
			
			// Print all of the key/value paris in the Lua table.
			System.out.println("printTable()");
			System.out.println("{");
			for (luaState.pushNil(); luaState.next(luaTableStackIndex); luaState.pop(1)) {
				// Fetch the table entry's string key.
				// An index of -2 accesses the key that was pushed into the Lua stack by luaState.next() up above.
				String keyName = null;
				com.naef.jnlua.LuaType luaType = luaState.type(-2);
				switch (luaType) {
					case STRING:
						// Fetch the table entry's string key.
						keyName = luaState.toString(-2);
						break;
					case NUMBER:
						// The key will be a number if the given Lua table is really an array.
						// In this case, the key is an array index. Do not call luaState.toString() on the
						// numeric key or else Lua will convert the key to a string from within the Lua table.
						keyName = Integer.toString(luaState.toInteger(-2));
						break;
				}
				if (keyName == null) {
					// A valid key was not found. Skip this table entry.
					continue;
				}
				
				// Fetch the table entry's value in string form.
				// An index of -1 accesses the entry's value that was pushed into the Lua stack by luaState.next() above.
				String valueString = null;
				luaType = luaState.type(-1);
				switch (luaType) {
					case STRING:
						valueString = luaState.toString(-1);
						break;
					case BOOLEAN:
						valueString = Boolean.toString(luaState.toBoolean(-1));
						break;
					case NUMBER:
						valueString = Double.toString(luaState.toNumber(-1));
						break;
					default:
						valueString = luaType.displayText();
						break;
				}
				if (valueString == null) {
					valueString = "";
				}
				
				// Print the table entry to the Android logging system.
				System.out.println("   [" + keyName + "] = " + valueString);
			}
			System.out.println("}");
		}
		catch (Exception ex) {
			// An exception will occur if given an invalid argument or no argument. Print the error.
			ex.printStackTrace();
		}
		
		// Return 0 since this Lua function does not return any values.
		return 0;
	}
}
