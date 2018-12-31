package com.coronalabs.corona.sample.extendinglua;


/**
 * Implements the callLuaFunction() function in Lua.
 * <p>
 * Demonstrates how to fetch a "Lua function argument" and then call that Lua function.
 */
public class CallLuaFunction implements com.naef.jnlua.NamedJavaFunction {
	/**
	 * Gets the name of the Lua function as it would appear in the Lua script.
	 * @return Returns the name of the custom Lua function.
	 */
	@Override
	public String getName() {
		return "callLuaFunction";
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
		try {
			// Check if the first argument is a function.
			// Will throw an exception if not or if no argument is given.
			int luaFunctionStackIndex = 1;
			luaState.checkType(luaFunctionStackIndex, com.naef.jnlua.LuaType.FUNCTION);
			
			// Push the given Lua function to the top of the Lua state's stack. We need to do this because the
			// because the call() method below expects this Lua function to be a the top of the stack.
			luaState.pushValue(luaFunctionStackIndex);
			
			// Call the given Lua function.
			// The first argument indicates the number of arguments that we are passing to the Lua function.
			// The second argument indicates the number of return values to accept from the Lua function.
			// In this case, we are calling this Lua function with no arguments and are accepting no return values.
			// Note: If you want to call the Lua function with arguments, then you need to push each argument
			//       value to the luaState object's stack.
			luaState.call(0, 0);
		}
		catch (Exception ex) {
			// An exception will occur if the following happens:
			// 1) No argument was given.
			// 2) The argument was not a Lua function.
			// 3) The Lua function call failed, likely because the Lua function could not be found.
			ex.printStackTrace();
		}
		
		// Return 0 since this Lua function does not return any values.
		return 0;
	}
}
