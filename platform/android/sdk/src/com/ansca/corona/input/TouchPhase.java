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
 * Indicates if touch input has began, moved/dragged, ended, or has been canceled.
 * Provides unique IDs matching constants in Corona and Android for easy conversion.
 * <p>
 * Instances of this class are immutable.
 * <p>
 * You cannot create instances of this class.
 * Instead, you use the pre-allocated objects from this class' static methods and fields.
 */
public class TouchPhase {
	/** Unique integer ID of the touch phase defined by Corona's "TouchEvent.Phase" enum in C++. */
	private int fCoronaNumericId;

	/** Unique string ID of the key phase defined by Corona's "TouchEvent" class in C++. */
	private String fCoronaStringId;


	/**
	 * Creates a new touch input phase object.
	 * @param coronaNumericId Corona's unique integer ID for the touch phase.
	 * @param coronaStringId Corona's unique string ID for the touch phase.
	 */
	private TouchPhase(int coronaNumericId, String coronaStringId) {
		fCoronaNumericId = coronaNumericId;
		fCoronaStringId = coronaStringId;
	}


	/** Indicates that touch input has began. */
	public static final TouchPhase BEGAN = new TouchPhase(0, "began");

	/** Indicates that the touch input pointer has changed coordinates. */
	public static final TouchPhase MOVED = new TouchPhase(1, "moved");

//Note: This is defined in "TouchEvent.Phase" in C++, but not currently used by Corona.
//	/** Indicates that touch input has began, but the pointer is not currently moving. */
//	public static final TouchPhase STATIONARY = new TouchPhase(2, "stationary");

	/** Indicates that touch input has ended. */
	public static final TouchPhase ENDED = new TouchPhase(3, "ended");

	/** Indicates that the system has aborted tracking touch input. */
	public static final TouchPhase CANCELED = new TouchPhase(4, "cancelled");

	
	/**
	 * Gets Corona's unique integer ID for this touch input phase.
	 * @return Returns a unique integer ID used by Corona's TouchEvent C++ class.
	 */
	public int toCoronaNumericId() {
		return fCoronaNumericId;
	}

	/**
	 * Gets Corona's unique string ID for this touch input phase.
	 * @return Returns a unique string ID used by Corona's touch event in Lua.
	 */
	public String toCoronaStringId() {
		return fCoronaStringId;
	}

	/**
	 * Gets Corona's unique string ID for this touch input phase.
	 * @return Returns a unique string ID used by Corona's touch event in Lua.
	 */
	@Override
	public String toString() {
		return fCoronaStringId;
	}

	/**
	 * Gets the touch phase from the given Android motion event.
	 * @param event The motion event received from an activity or view's onMotionEvent() method.
	 * @return Returns the touch input phase extracted from the given event.
	 *         <p>
	 *         Returns null if given a null argument.
	 *         <p>
	 *         Returns null if the given event is not touch related, such as for an analog axis.
	 */
	public static TouchPhase from(android.view.MotionEvent event) {
		// Validate argument.
		if (event == null) {
			return null;
		}

		// Fetch the matching touch phase from this class.
		int action = event.getAction() & android.view.MotionEvent.ACTION_MASK;
		switch (action) {
			case android.view.MotionEvent.ACTION_DOWN:
			case android.view.MotionEvent.ACTION_POINTER_DOWN:
				return TouchPhase.BEGAN;

			case android.view.MotionEvent.ACTION_MOVE:
				return TouchPhase.MOVED;

			case android.view.MotionEvent.ACTION_UP:
			case android.view.MotionEvent.ACTION_POINTER_UP:
				return TouchPhase.ENDED;

			case android.view.MotionEvent.ACTION_CANCEL:
				return TouchPhase.CANCELED;
		}

		// The given motion event's action is unknown.
		return null;
	}
}
