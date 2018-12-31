package com.coronalabs.corona.sample.extendinglua;


/**
 * Implements the getRandomTable() function in Lua.
 * <p>
 * Demonstrates how to return a table of key/value pairs from Java to Lua.
 */
public class GetRandomTableLuaFunction implements com.naef.jnlua.NamedJavaFunction {
	/**
	 * Gets the name of the Lua function as it would appear in the Lua script.
	 * @return Returns the name of the custom Lua function.
	 */
	@Override
	public String getName() {
		return "getRandomTable";
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
		// Generate a Java hashtable containing random values.
		java.util.Random randomizer = new java.util.Random();
		java.util.Hashtable<String, Double> hashtable = new java.util.Hashtable<String, Double>();
		hashtable.put("x", Double.valueOf(randomizer.nextDouble()));
		hashtable.put("y", Double.valueOf(randomizer.nextDouble()));
		hashtable.put("z", Double.valueOf(randomizer.nextDouble()));
		
		// Create a new Lua table within the Lua state to copy the Java hashtable's entries to.
		// Creating a Lua table in this manner automatically pushes it on the Lua stack.
		// For best performance, you should pre-allocate the Lua table like below if the number of entries is known.
		// The newTable() method's first argument should be set to zero since we are not creating a Lua array.
		// The newTable() method's second argument should be set to the number of entries in the Java dictionary.
		luaState.newTable(0, hashtable.size());
		int luaTableStackIndex = luaState.getTop();
		
		// Copy the Java hashtable's entries to the Lua table.
		for (java.util.Map.Entry<String, Double> entry : hashtable.entrySet()) {
			// Push the value to be inserted into the Lua table onto the Lua stack.
			luaState.pushNumber(entry.getValue().doubleValue());
			
			// Insert the above value into the Lua table with the given key.
			// This does the equivalent of "table[key] = value" in Lua.
			// The setField() method automatically pops the value off of the Lua stack that was pushed up above.
			luaState.setField(luaTableStackIndex, entry.getKey());
		}
		
		// Return 1 to indicate that this Lua function returns 1 Lua table.
		return 1;
	}
}
