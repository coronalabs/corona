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
