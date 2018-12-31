package com.coronalabs.corona.sample.extendinglua;


/**
 * Implements the printTableValuesXY() function in Lua.
 * <p>
 * Demonstrates how to fetch a table argument from a Lua function and how to retrieve particular entries.
 */
public class PrintTableValuesXYLuaFunction implements com.naef.jnlua.NamedJavaFunction {
	/**
	 * Gets the name of the Lua function as it would appear in the Lua script.
	 * @return Returns the name of the custom Lua function.
	 */
	@Override
	public String getName() {
		return "printTableValuesXY";
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
			
			// Print only the entry's "x" and "y" in the Lua table.
			// See the private method below on how to access an entry within a Lua table.
			System.out.println("printTableValuesXY()");
			System.out.println("{");
			System.out.println("   [x] = " + getLuaTableEntryValueFrom(luaState, luaTableStackIndex, "x"));
			System.out.println("   [y] = " + getLuaTableEntryValueFrom(luaState, luaTableStackIndex, "y"));
			System.out.println("}");
		}
		catch (Exception ex) {
			// An exception will occur if given an invalid argument or no argument. Print the error.
			ex.printStackTrace();
		}
		
		// Return 0 since this Lua function does not return any values.
		return 0;
	}
	
	/**
	 * Private method used to fetch a Lua table entry's value in string form.
	 * @param luaState Reference to the Lua state that contains the table.
	 * @param luaTableStackIndex Index to the Lua table within the Lua stack.
	 * @param keyName String key of the Lua table entry to access.
	 * @return Returns the specified Lua table entry's value in string form.
	 *         Will return "nil" if the key was not found in the Lua table.
	 */
	private String getLuaTableEntryValueFrom(com.naef.jnlua.LuaState luaState, int luaTableStackIndex, String keyName) {
		// Fetch the value for the given key from the Lua table.
		// The getField() method will push the entry's value on to the Lua stack.
		// It will push nil if the value was not found.
		luaState.getField(luaTableStackIndex, keyName);
		
		// The table entry's value can now be accessed on the Lua stack with an index of -1.
		// Determine the type of value it is and then convert it to a string.
		String valueString = null;
		com.naef.jnlua.LuaType luaType = luaState.type(-1);
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
			valueString = com.naef.jnlua.LuaType.NIL.displayText();
		}
		
		// Pop the table entry's value off of the stack that was pushed by the getField() method call up above.
		luaState.pop(1);
		
		// Return the table entry's value string.
		return valueString;
	}
}
