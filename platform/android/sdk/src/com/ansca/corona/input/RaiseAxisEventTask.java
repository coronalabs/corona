//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.input;


/**
 * Corona runtime task used to raise an "axis" event to be received by a Lua listener.
 * <p>
 * Instances of this class are expected to be passed to the CoronaRuntimeTaskDisplatcher.send() method.
 * The dispatcher will then have this task executed by the Corona runtime in a thread safe manner.
 * @see com.ansca.corona.CoronaRuntimeTaskDispatcher
 */
public class RaiseAxisEventTask implements com.ansca.corona.CoronaRuntimeTask {
	/** Reference to the input device that the axis data came from. */
	private InputDeviceInterface fDevice;

	/** Provides the received data and information about its axis. */
	private AxisDataEventInfo fDataEventInfo;


	/**
	 * Creates a new Corona runtime task used to raise an "axis" event to be received by a Lua listener.
	 * @param device Reference to the device that the axis data came from. Cannot be null.
	 * @param eventInfo Provides information about the axis and its received data. Cannot be null.
	 */
	public RaiseAxisEventTask(InputDeviceInterface device, AxisDataEventInfo eventInfo) {
		if ((device == null) || (eventInfo == null)) {
			throw new NullPointerException();
		}
		fDevice = device;
		fDataEventInfo = eventInfo;
	}

	/**
	 * Called by the Corona runtime just before it renders a frame.
	 * <p>
	 * Raises an "axis" event in the Corona runtime to be received by a Lua listener.
	 * <p>
	 * Warning! This method is not called on the main thread.
	 * @param runtime Reference to the CoronaRuntime object that is currently running.
	 *                Provides access to its LuaState object.
	 */
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.axisEvent(runtime, fDevice, fDataEventInfo);
	}
}
