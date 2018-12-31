package com.coronalabs.corona.sample.extendinglua;


/**
 * Implements the printNumber() function in Lua.
 * <p>
 * Demonstrates how to fetch a numeric argument from a Lua function.
 */
public class PrintNumberLuaFunction implements com.naef.jnlua.NamedJavaFunction {
	/**
	 * Gets the name of the Lua function as it would appear in the Lua script.
	 * @return Returns the name of the custom Lua function.
	 */
	@Override
	public String getName() {
		return "printNumber";
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
			// Fetch the Lua function's first argument.
			// Will throw an exception if it is not of type number.
			double value = luaState.checkNumber(1);
			
			// Print the value to the Android logging system.
			System.out.println("printNumber(): value = " + Double.toString(value));
		}
		catch (Exception ex) {
			// An exception will occur if given an invalid argument or no argument. Print the error.
			ex.printStackTrace();
		}
		
		// Return 0 since this Lua function does not return any values.
		return 0;
	}
}
