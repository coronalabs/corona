package com.coronalabs.corona.sample.extendinglua;


/**
 * Implements the getRandomBoolean() function in Lua.
 * <p>
 * Demonstrates how to return a boolean value from Java to Lua.
 */
public class GetRandomBooleanLuaFunction implements com.naef.jnlua.NamedJavaFunction {
	/**
	 * Gets the name of the Lua function as it would appear in the Lua script.
	 * @return Returns the name of the custom Lua function.
	 */
	@Override
	public String getName() {
		return "getRandomBoolean";
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
		// Generate a random boolean value.
		java.util.Random randomizer = new java.util.Random();
		boolean value = randomizer.nextBoolean();
		
		// Push the boolean value to the Lua state's stack.
		// This is the value to be returned by the Lua function.
		luaState.pushBoolean(value);
		
		// Return 1 to indicate that this Lua function returns 1 value.
		return 1;
	}
}
