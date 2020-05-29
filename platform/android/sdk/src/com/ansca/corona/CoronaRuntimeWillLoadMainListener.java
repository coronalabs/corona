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
 * Receives events from the Corona runtime to detect when it is about to run main.lua
 * <p>
 * Instances of this interface are to be given to the CoronaEnvironment.addRuntimeListener() method.
 * <p>
 * Warning! All methods in this interface are not called on the main UI thread.
 * The Corona runtime and its Lua scripts run on a separate thread.
 * This means that you should not manipulate an activity's UI elements from these methods.
 * @see CoronaEnvironment
 * @see CoronaRuntime
 */
interface CoronaRuntimeWillLoadMainListener {
	/**
	 * Called after the Corona runtime has been created and just before executing the "main.lua" file.
	 * <p>
	 * Warning! This method is not called on the main thread.
	 * @param runtime Reference to the CoronaRuntime object that has just been loaded/initialized.
	 *                Provides a LuaState object that allows the application to extend the Lua API.
	 */
	public void onWillLoadMain(CoronaRuntime runtime);
}
