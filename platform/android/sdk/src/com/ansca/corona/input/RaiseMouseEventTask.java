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
 * Corona runtime task used to raise a "mouse" event to be received by a Lua listener.
 * <p>
 * Instances of this class are expected to be passed to the CoronaRuntimeTaskDisplatcher.send() method.
 * The dispatcher will then have this task executed by the Corona runtime in a thread safe manner.
 * @see com.ansca.corona.CoronaRuntimeTaskDispatcher
 */
public class RaiseMouseEventTask implements com.ansca.corona.CoronaRuntimeTask {
	/** The mouse cursor's X coordinate on the screen in pixels. */
	private float fPointX;

	/** The mouse cursor's Y coordinate on the screen in pixels. */
	private float fPointY;

	/** Number of pixels to travel in the X direction as requested by the horizontal scroll wheel. */
	private float fScrollX;

	/** Number of pixels to travel in the Y direction as requested by the vertical scroll wheel. */
	private float fScrollY;

	/** The time the mouse event update occurred in milliseconds since bootup. */
	private long fTimestamp;

	/** Set to true if the primary/left mouse button was down while the mouse event occurred. */
	private boolean fIsPrimaryButtonDown;

	/** Set to true if the secondary/right mouse button was down while the mouse event occurred. */
	private boolean fIsSecondaryButtonDown;

	/** Set to true if the middle/tertiary mouse button was down while the mouse event occurred. */
	private boolean fIsMiddleButtonDown;


	/**
	 * Creates a new Corona runtime task used to raise a "mouse" event to be received by a Lua listener.
	 * @param x The mouse cursor's X coordinate on screen in pixels.
	 * @param y The mouse cursor's Y coordinate on screen in pixels.
	 * @param scrollX The number of pixels to scroll in the X direction as requested by the horizontal scroll wheel.
	 * @param scrollY The number of pixels to scroll in the Y direction as requested by the vertical scroll wheel.
	 * @param timestamp The time the mouse event occurred in milliseconds since bootup.
	 *                  <p>
	 *                  This value should be taken from MotionEvent.getEventTime() or SystemClock.uptimeMillis().
	 * @param isPrimaryButtonDown Set true if the primary/left mouse button was pressed down.
	 * @param isSecondaryButtonDown Set true if the secondary/right mouse button was pressed down.
	 * @param isMiddleButtonDown Set true if the middle/tertiary mouse button was pressed down.
	 */
	public RaiseMouseEventTask(
		float x, float y, float scrollX, float scrollY, long timestamp,
		boolean isPrimaryButtonDown, boolean isSecondaryButtonDown, boolean isMiddleButtonDown)
	{
		fPointX = x;
		fPointY = y;
		fScrollX = scrollX;
		fScrollY = scrollY;
		fTimestamp = timestamp;
		fIsPrimaryButtonDown = isPrimaryButtonDown;
		fIsSecondaryButtonDown = isSecondaryButtonDown;
		fIsMiddleButtonDown = isMiddleButtonDown;
	}

	/**
	 * Called by the Corona runtime just before it renders a frame.
	 * <p>
	 * Raises a "mouse" event in the Corona runtime to be received by a Lua listener.
	 * <p>
	 * Warning! This method is not called on the main thread.
	 * @param runtime Reference to the CoronaRuntime object that is currently running.
	 *                Provides access to its LuaState object.
	 */
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.mouseEvent( runtime,
				(int)fPointX, (int)fPointY, (int)fScrollX, (int)fScrollY, fTimestamp,
				fIsPrimaryButtonDown, fIsSecondaryButtonDown, fIsMiddleButtonDown);
	}
}
