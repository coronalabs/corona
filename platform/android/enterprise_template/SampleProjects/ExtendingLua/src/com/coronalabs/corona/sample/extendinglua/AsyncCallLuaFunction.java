package com.coronalabs.corona.sample.extendinglua;


/**
 * Implements the asyncCallLuaFunction() function in Lua.
 * <p>
 * Demonstrates how to fetch a "Lua function argument" and then call that Lua function from another thread.
 * <p>
 * You must never manipulate the Lua state from another thread or else race conditions and exceptions that
 * can crash the application can occur. This example demonstrates how to do this in a thread safe manner.
 */
public class AsyncCallLuaFunction implements com.naef.jnlua.NamedJavaFunction {
	/**
	 * Gets the name of the Lua function as it would appear in the Lua script.
	 * @return Returns the name of the custom Lua function.
	 */
	@Override
	public String getName() {
		return "asyncCallLuaFunction";
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
		// Check if the first argument is a function.
		// Will print a stack trace if not or if no argument was given.
		int luaFunctionStackIndex = 1;
		try {
			luaState.checkType(luaFunctionStackIndex, com.naef.jnlua.LuaType.FUNCTION);
		}
		catch (Exception ex) {
			ex.printStackTrace();
			return 0;
		}
		
		// Store the given Lua function in the Lua registry to be accessed later. We must do this because
		// the given Lua function argument will be popped off the Lua stack when we leave this Java method.
		// Note that the ref() method expects the value to be stored is at the top of the Lua stack.
		// So, we must first push the Lua function to the top. The ref() method will automatically pop off
		// the push Lua function afterwards.
		luaState.pushValue(luaFunctionStackIndex);
		final int luaFunctionReferenceKey = luaState.ref(com.naef.jnlua.LuaState.REGISTRYINDEX);
		
		// Set up a dispatcher which allows us to send a task to the Corona runtime thread from another thread.
		// This way we can call the given Lua function on the same thread that Lua runs in.
		// This dispatcher will only send tasks to the Corona runtime that owns the given Lua state object.
		// Once the Corona runtime is disposed/destroyed, which happens when the Corona activy is destroyed,
		// then this dispatcher will no longer be able to send tasks.
		final com.ansca.corona.CoronaRuntimeTaskDispatcher dispatcher =
					new com.ansca.corona.CoronaRuntimeTaskDispatcher(luaState);
		
		// Post a Runnable object on the UI thread that will call the given Lua function.
		com.ansca.corona.CoronaEnvironment.getCoronaActivity().runOnUiThread(new Runnable() {
			@Override
			public void run() {
				// *** We are now running in the main UI thread. ***
				
				// Create a task that will call the given Lua function.
				// This task's execute() method will be called on the Corona runtime thread, just before rendering a frame.
				com.ansca.corona.CoronaRuntimeTask task = new com.ansca.corona.CoronaRuntimeTask() {
					@Override
					public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
						// *** We are now running on the Corona runtime thread. ***
						try {
							// Fetch the Corona runtime's Lua state.
							com.naef.jnlua.LuaState luaState = runtime.getLuaState();
							
							// Fetch the Lua function stored in the registry and push it to the top of the stack.
							luaState.rawGet(com.naef.jnlua.LuaState.REGISTRYINDEX, luaFunctionReferenceKey);
							
							// Remove the Lua function from the registry.
							luaState.unref(com.naef.jnlua.LuaState.REGISTRYINDEX, luaFunctionReferenceKey);
							
							// Call the Lua function that was just pushed to the top of the stack.
							// The 1st argument indicates the number of arguments that we are passing to the Lua function.
							// The 2nd argument indicates the number of return values to accept from the Lua function.
							// In this case, we are calling this Lua function without arguments and accepting no return values.
							// Note: If you want to call the Lua function with arguments, then you need to push each argument
							//       value to the luaState object's stack.
							luaState.call(0, 0);
						}
						catch (Exception ex) {
							ex.printStackTrace();
						}
					}
				};
				
				// Send the above task to the Corona runtime asynchronously.
				// The send() method will do nothing if the Corona runtime is no longer available, which can
				// happen if the runtime was disposed/destroyed after the user has exited the Corona activity.
				dispatcher.send(task);
			}
		});
		
		// Return 0 since this Lua function does not return any values.
		return 0;
	}
}
