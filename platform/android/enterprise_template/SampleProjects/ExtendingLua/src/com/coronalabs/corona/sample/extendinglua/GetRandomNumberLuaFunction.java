package com.coronalabs.corona.sample.extendinglua;


/**
 * Implements the getRandomNumber() function in Lua.
 * <p>
 * Demonstrates how to return a numeric value from Java to Lua.
 */
public class GetRandomNumberLuaFunction implements com.naef.jnlua.NamedJavaFunction {
	/**
	 * Gets the name of the Lua function as it would appear in the Lua script.
	 * @return Returns the name of the custom Lua function.
	 */
	@Override
	public String getName() {
		return "getRandomNumber";
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
		// Generate a random floating point value.
		java.util.Random randomizer = new java.util.Random();
		double value = randomizer.nextDouble();
		
		// Push the floating point value to the Lua state's stack.
		// This is the value to be returned by the Lua function.
		// Note: Lua numbers are of type "double" on Android. Integer types are not supported.
		luaState.pushNumber(value);
		
		// Return 1 to indicate that this Lua function returns 1 value.
		return 1;
	}
}
