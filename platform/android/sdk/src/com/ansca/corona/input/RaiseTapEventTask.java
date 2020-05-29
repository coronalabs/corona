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
 * Corona runtime task used to raise a "tap" event to be received by a Lua listener.
 * <p>
 * Instances of this class are expected to be passed to the CoronaRuntimeTaskDisplatcher.send() method.
 * The dispatcher will then have this task executed by the Corona runtime in a thread safe manner.
 * @see com.ansca.corona.CoronaRuntimeTaskDispatcher
 */
public class RaiseTapEventTask implements com.ansca.corona.CoronaRuntimeTask {
	/** The coordinates of where the tap occurred. */
	private TouchPoint fTapPoint;

	/** The number of consecutive taps that have occurred at the same coordinates. */
	private int fTapCount;


	/**
	 * Creates a new Corona runtime task used to raise a "tap" event to be received by a Lua listener.
	 * @param tapPoint Indicates where onscreen the tap occurred. Cannot be null.
	 * @param tapCount The number of consecutive taps that have occurred at the given tap point.
	 */
	public RaiseTapEventTask(TouchPoint tapPoint, int tapCount) {
		if (tapPoint == null) {
			throw new NullPointerException();
		}
		fTapPoint = tapPoint;
		fTapCount = tapCount;
	}

	/**
	 * Called by the Corona runtime just before it renders a frame.
	 * <p>
	 * Raises a "tap" event in the Corona runtime to be received by a Lua listener.
	 * <p>
	 * Warning! This method is not called on the main thread.
	 * @param runtime Reference to the CoronaRuntime object that is currently running.
	 *                Provides access to its LuaState object.
	 */
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.tapEvent(runtime, (int)fTapPoint.getX(), (int)fTapPoint.getY(), fTapCount);
	}
}
