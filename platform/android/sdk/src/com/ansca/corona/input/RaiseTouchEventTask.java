//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.input;


/**
 * Corona runtime task used to raise a "touch" event to be received by a Lua listener.
 * <p>
 * This class should be used to raise "touch" events instead of the RaiseMultitouchEventTask class
 * if "multitouch" has not been activated.
 * <p>
 * Instances of this class are expected to be passed to the CoronaRuntimeTaskDisplatcher.send() method.
 * The dispatcher will then have this task executed by the Corona runtime in a thread safe manner.
 * @see com.ansca.corona.CoronaRuntimeTaskDispatcher
 */
public class RaiseTouchEventTask implements com.ansca.corona.CoronaRuntimeTask {
	/** Clone of the touch data to be provided by the raised touch event. */
	private TouchTracker fTouchTracker;


	/**
	 * Creates a new Corona runtime task used to raise a "touch" event to be received by a Lua listener.
	 * @param touchData Provides information about the touch event. Cannot be null.
	 */
	public RaiseTouchEventTask(TouchTracker touchData) {
		fTouchTracker = touchData.clone();
	}

	/**
	 * Called by the Corona runtime just before it renders a frame.
	 * <p>
	 * Raises a "touch" event in the Corona runtime to be received by a Lua listener.
	 * <p>
	 * Warning! This method is not called on the main thread.
	 * @param runtime Reference to the CoronaRuntime object that is currently running.
	 *                Provides access to its LuaState object.
	 */
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		if (fTouchTracker != null) {
			com.ansca.corona.JavaToNativeShim.touchEvent(runtime, fTouchTracker);
		}
	}
}
