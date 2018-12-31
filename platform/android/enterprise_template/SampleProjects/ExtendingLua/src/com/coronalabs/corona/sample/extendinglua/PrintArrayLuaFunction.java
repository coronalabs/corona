package com.coronalabs.corona.sample.extendinglua;


/**
 * Implements the printArray() function in Lua.
 * <p>
 * Demonstrates how to fetch an array argument and its elements from a Lua function.
 */
public class PrintArrayLuaFunction implements com.naef.jnlua.NamedJavaFunction {
	/**
	 * Gets the name of the Lua function as it would appear in the Lua script.
	 * @return Returns the name of the custom Lua function.
	 */
	@Override
	public String getName() {
		return "printArray";
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
			// Check if the Lua function's first argument is a Lua array, which is of type table in Lua.
			// Will throw an exception if it is not an array, table, or if no argument was given.
			int luaTableStackIndex = 1;
			luaState.checkType(luaTableStackIndex, com.naef.jnlua.LuaType.TABLE);
			
			// Fetch the number of elements in the Lua array.
			int arrayLength = luaState.length(luaTableStackIndex);
			
			// Print all of the elements in the Lua array.
			// Remember that Lua array are 1-based by default.
			if (arrayLength > 0) {
				System.out.println("printArray()");
				System.out.println("{");
				for (int index = 1; index <= arrayLength; index++) {
					// Push the next Lua array value onto the Lua stack.
					luaState.rawGet(luaTableStackIndex, index);
					
					// Print the pushed value to the Android logging system based on its type.
					// A stack index of -1 means access the top most value on the Lua statck.
					StringBuilder stringBuilder = new StringBuilder("   [" + Integer.toString(index) + "] = ");
					com.naef.jnlua.LuaType valueType = luaState.type(-1);
					switch (valueType) {
						case BOOLEAN:
							stringBuilder.append(Boolean.toString(luaState.toBoolean(-1)));
							break;
						case NUMBER:
							stringBuilder.append(Double.toString(luaState.toNumber(-1)));
							break;
						case STRING:
							stringBuilder.append(luaState.toString(-1));
							break;
						default:
							stringBuilder.append(valueType.displayText());
							break;
					}
					System.out.println(stringBuilder.toString());
					
					// Pop the value that was retrieved via the luaState.rawGet() off of the Lua stack.
					luaState.pop(1);
				}
				System.out.println("}");
			}
		}
		catch (Exception ex) {
			// An exception will occur if given an invalid argument or no argument. Print the error.
			ex.printStackTrace();
		}
		
		// Return 0 since this Lua function does not return any values.
		return 0;
	}
}
