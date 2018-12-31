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
 * Corona runtime task used to raise multiple "touch" events to be received by a Lua listener.
 * <p>
 * This class should be used to raise "touch" events instead of the RaiseTouchEventTask class
 * if "multitouch" has been activated.
 * <p>
 * Instances of this class are expected to be passed to the CoronaRuntimeTaskDisplatcher.send() method.
 * The dispatcher will then have this task executed by the Corona runtime in a thread safe manner.
 * @see com.ansca.corona.CoronaRuntimeTaskDispatcher
 */
public class RaiseMultitouchEventTask implements com.ansca.corona.CoronaRuntimeTask {
	/** Clone of the touch event data from multiple pointers to be provided by the raised touch event. */
	private TouchTrackerCollection fTouchTrackers;


	/**
	 * Creates a new Corona runtime task used to raise a "touch" event to be received by a Lua listener.
	 * @param multitouchData Provides information about multitouch event. Cannot be null.
	 */
	public RaiseMultitouchEventTask(TouchTrackerCollection multitouchData) {
		fTouchTrackers = multitouchData.clone();
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
		com.ansca.corona.JavaToNativeShim.multitouchEventBegin(runtime);
		for (TouchTracker tracker : fTouchTrackers) {
			com.ansca.corona.JavaToNativeShim.multitouchEventAdd(runtime, tracker);
		}
		com.ansca.corona.JavaToNativeShim.multitouchEventEnd(runtime);
	}
}
