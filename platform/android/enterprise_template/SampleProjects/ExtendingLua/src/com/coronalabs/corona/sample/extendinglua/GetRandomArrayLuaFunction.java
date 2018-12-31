package com.coronalabs.corona.sample.extendinglua;


/**
 * Implements the getRandomArray() function in Lua.
 * <p>
 * Demonstrates how to return an array from Java to Lua.
 */
public class GetRandomArrayLuaFunction implements com.naef.jnlua.NamedJavaFunction {
	/**
	 * Gets the name of the Lua function as it would appear in the Lua script.
	 * @return Returns the name of the custom Lua function.
	 */
	@Override
	public String getName() {
		return "getRandomArray";
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
		// Generate a random array of floating point values.
		java.util.Random randomizer = new java.util.Random();
		double[] values = new double[] {
			randomizer.nextDouble(),
			randomizer.nextDouble(),
			randomizer.nextDouble()
		};
		
		// Create a new Lua array within the Lua state to copy the Java array's values to.
		// Creating a Lua array in this manner automatically pushes it on the Lua stack.
		// For best performance, you should pre-allocate the Lua array in one shot like below if the array size is known.
		// The newTable() method's first argument should be set to the array's length.
		// The newTable() method's second argument should be zero since we are not creating a key/value table in Lua.
		luaState.newTable(values.length, 0);
		int luaTableStackIndex = luaState.getTop();
		
		// Copy the Java array's values to the Lua array.
		for (int index = 0; index < values.length; index++) {
			// Push the Java array's value to the Lua stack.
			luaState.pushNumber(values[index]);
			
			// Assign the above pushed value to the next Lua array element.
			// We do an "index + 1" because arrays in Lua are 1-based by default.
			luaState.rawSet(luaTableStackIndex, index + 1);
		}
		
		// Return 1 to indicate that this Lua function returns 1 Lua array.
		return 1;
	}
}
