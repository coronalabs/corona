package com.coronalabs.corona.sample.customerrorhandler;


/**
 * Implements the throwException() function in Lua.
 * <p>
 * Throws an exception to be caught by the UnhandledExcpetionHandler.
 */
public class ThrowExceptionLuaFunction implements com.naef.jnlua.NamedJavaFunction {
	/**
	 * Gets the name of the Lua function as it would appear in the Lua script.
	 * @return Returns the name of the custom Lua function.
	 */
	@Override
	public String getName() {
		return "throwException";
	}
	
	/**
	 * This method is called when a Lua error occurs int he Corona runtime.
	 * <p>
	 * Warning! This method is not called on the main UI thread.
	 * @param luaState Reference to the Lua state that the error occurred in.
	 *                 Typically provides an error string at the top of the stack to be returned by
	 *                 this function. The Lua state can also provide access to a stack trace, if available.
	 * @return Returns the number of values to be returned by this error function.
	 *         An error function would typically return one string value describing the error.
	 */
	@Override
	public int invoke(com.naef.jnlua.LuaState luaState) {
		// Trigger a NullPointerException to happen before the next render pass on the Corona runtime thread.
		com.ansca.corona.CoronaRuntimeTaskDispatcher dispatcher;
		dispatcher = new com.ansca.corona.CoronaRuntimeTaskDispatcher(luaState);
		dispatcher.send(new com.ansca.corona.CoronaRuntimeTask() {
			@Override
			public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
				String test = null;
				test.toString();
			}
		});
		return 0;
	}
}
