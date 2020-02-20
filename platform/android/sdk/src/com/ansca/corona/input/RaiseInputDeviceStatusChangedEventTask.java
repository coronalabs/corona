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
 * Corona runtime task used to raise an "inputDeviceStatus" event to be received by a Lua listener.
 * <p>
 * Instances of this class are expected to be passed to the CoronaRuntimeTaskDisplatcher.send() method.
 * The dispatcher will then have this task executed by the Corona runtime in a thread safe manner.
 * @see com.ansca.corona.CoronaRuntimeTaskDispatcher
 */
public class RaiseInputDeviceStatusChangedEventTask implements com.ansca.corona.CoronaRuntimeTask {
	/** Reference to the input device whose status has been changed. */
	private InputDeviceInterface fDevice;

	/** Provides information indicating what aspects of the device has changed. */
	private InputDeviceStatusEventInfo fEventInfo;


	/**
	 * Creates a new Corona runtime task used to raise an "inputDeviceStatus" event in Lua.
	 * @param device Reference to the input device whose status has changed. Cannot be null.
	 * @param eventInfo Provides information about the event. Cannot be null.
	 */
	public RaiseInputDeviceStatusChangedEventTask(
		InputDeviceInterface device, InputDeviceStatusEventInfo eventInfo)
	{
		if ((device == null) || (eventInfo == null)) {
			throw new NullPointerException();
		}
		fDevice = device;
		fEventInfo = eventInfo;
	}

	/**
	 * Called by the Corona runtime just before it renders a frame.
	 * <p>
	 * Raises an "inputDeviceStatus" event in the Corona runtime to be received by a Lua listener.
	 * <p>
	 * Warning! This method is not called on the main thread.
	 * @param runtime Reference to the CoronaRuntime object that is currently running.
	 *                Provides access to its LuaState object.
	 */
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.inputDeviceStatusEvent(runtime, fDevice, fEventInfo);
	}
}
