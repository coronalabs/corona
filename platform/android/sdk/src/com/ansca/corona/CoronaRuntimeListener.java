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
 * Receives events from the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} to detect when it is loaded, started, suspended, etc.
 * <p>
 * Instances of this interface are to be given to the 
 * {@link com.ansca.corona.CoronaEnvironment#addRuntimeListener(com.ansca.corona.CoronaRuntimeListener) CoronaEnvironment.addRuntimeListener()} 
 * method. If you want to receive the {@link com.ansca.corona.CoronaRuntime runtime's} "loaded" and "started" events, 
 * then you should assign the listener before displaying the {@link com.ansca.corona.CoronaActivity CoronaActivity}, 
 * such as in a derived version of 
 * <a href="http://developer.android.com/reference/android/app/Application.html">Android's Application class</a>.
 * <p>
 * <b>WARNING!</b> All methods in this interface are not called on the main UI thread.
 * The {@link com.ansca.corona.CoronaRuntime CoronaRuntime} and its Lua scripts run on a separate thread. 
 * This means that you should not manipulate an 
 * <a href="http://developer.android.com/reference/android/app/Activity.html">activity's</a> UI elements from these methods.
 * @see CoronaEnvironment
 * @see CoronaRuntime
 */
public interface CoronaRuntimeListener {
	/**
	 * Called after the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} 
	 * has been created and just before executing the "main.lua" file.
	 * This is the application's opportunity to register custom APIs into Lua.
	 * <p>
	 * <b>WARNING!</b> This method is not called on the main UI thread.
	 * @param runtime Reference to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} 
	 *				  object that has just been loaded/initialized.
	 *                Provides a {@link com.naef.jnlua.LuaState LuaState} object that allows the application to extend the Lua API.
	 */
	public void onLoaded(CoronaRuntime runtime);
	
	/**
	 * Called just after the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} has executed the "main.lua" file.
	 * <p>
	 * <b>WARNING!</b> This method is not called on the main UI thread.
	 * @param runtime Reference to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} object that has just been started.
	 */
	public void onStarted(CoronaRuntime runtime);
	
	/**
	 * Called just after the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} has been suspended 
	 * which pauses all rendering, audio, timers, and other Corona related operations. This can happen when another 
	 * <a href="http://developer.android.com/reference/android/app/Activity.html">Android activity</a> (ie: window) has
	 * been displayed, when the screen has been powered off, or when the screen lock is shown.
	 * <p>
	 * <b>WARNING!</b> This method is not called on the main UI thread.
	 * @param runtime Reference to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} object that has just been suspended.
	 */
	public void onSuspended(CoronaRuntime runtime);
	
	/**
	 * Called just after the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} has been resumed after a suspend.
	 * <p>
	 * <b>WARNING!</b> This method is not called on the main UI thread.
	 * @param runtime Reference to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} object that has just been resumed.
	 */
	public void onResumed(CoronaRuntime runtime);
	
	/**
	 * Called just before the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} terminates.
	 * <p>
	 * This happens when the {@link com.ansca.corona.CoronaActivity CoronaActivity} is being destroyed which happens 
	 * when the user presses the Back button on the 
	 * <a href="http://developer.android.com/reference/android/app/Activity.html">activity</a>, when the 
	 * <a href="http://docs.coronalabs.com/daily/api/library/native/requestExit.html">native.requestExit()</a> method is called in Lua,
	 * or when the <a href="http://developer.android.com/reference/android/app/Activity.html#finish()">activity's finish()</a>
	 * method is called. This does not mean that the application is exiting.
	 * <p>
	 * <b>WARNING!</b> This method is not called on the main UI thread.
	 * @param runtime Reference to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} object that is being terminated.
	 */
	public void onExiting(CoronaRuntime runtime);
}
