//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;


/**
 * Represents a single task to be executed by the {@link com.ansca.corona.CoronaRuntime CoronaRuntime}, 
 * just before it renders a frame. The task will be executed on the same thread that the 
 * {@link com.ansca.corona.CoronaRuntime CoronaRuntime} runs in.
 * <p>
 * The purpose of this class is to allow another thread to send a task to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime}
 * that will manipulate its {@link com.naef.jnlua.LuaState LuaState}, such as calling a Lua function, on the same thread that
 * the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} and Lua is running on. This is important because you should never manipulate
 * the {@link com.naef.jnlua.LuaState LuaState} on another thread since it could cause race conditions to occur and exceptions
 * to be thrown, which could crash the application.
 * <p>
 * Instances of this interface are to be given to a 
 * {@link com.ansca.corona.CoronaRuntimeTaskDispatcher#send(com.ansca.corona.CoronaRuntimeTask) CoronaRuntimeTaskDispatcher object's send()} 
 * method. That class will send tasks to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} in a thread safe manner.
 * <p>
 * <b>WARNING!</b> This task will not be executed by the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} on the main UI thread.
 * The {@link com.ansca.corona.CoronaRuntime CoronaRuntime} and its Lua scripts run on a separate thread.
 * This means that you should never manipulate an
 * <a href="http://developer.android.com/reference/android/app/Activity.html">activity's</a> UI elements from this class' method.
 * @see CoronaRuntime
 * @see CoronaRuntimeTaskDispatcher
 */
public interface CoronaRuntimeTask {
	/**
	 * Called by the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} just before it renders a frame.
	 * This is the application's opportunity to call a Lua function or update global variables in Lua.
	 * <p>
	 * <b>WARNING!</b> This method is not called on the main UI thread.
	 * @param runtime Reference to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} object that is currently running.
	 *                Provides access to its {@link com.naef.jnlua.LuaState LuaState} object.
	 */
	public void executeUsing(CoronaRuntime runtime);
}
