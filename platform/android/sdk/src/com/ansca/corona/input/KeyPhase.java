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
 * Indicates if a key is currently in the "down" or "up" position.
 * Provides unique IDs matching constants in Corona and Android for easy conversion.
 * <p>
 * Instances of this class are immutable.
 * <p>
 * You cannot create instances of this class.
 * Instead, you use the pre-allocated objects from this class' static methods and fields.
 */
public class KeyPhase {
	/** Unique integer ID of the key phase defined by Android's "KeyEvent" class. */
	private int fAndroidNumericId;

	/** Unique integer ID of the key phase defined by Corona's "KeyEvent.Phase" enum in C++. */
	private int fCoronaNumericId;

	/** Unique string ID of the key phase defined by Corona's "KeyEvent" class in C++. */
	private String fCoronaStringId;


	/**
	 * Creates a new key phase object.
	 * @param androidNumericId Android's unique integer ID for the key phase.
	 * @param coronaNumericId Corona's unique integer ID for the key phase.
	 * @param coronaStringId Corona's unique string ID for the key phase.
	 */
	private KeyPhase(int androidNumericId, int coronaNumericId, String coronaStringId) {
		fAndroidNumericId = androidNumericId;
		fCoronaNumericId = coronaNumericId;
		fCoronaStringId = coronaStringId;
	}


	/** Indicates that a key has been pressed down. */
	public static final KeyPhase DOWN = new KeyPhase(android.view.KeyEvent.ACTION_DOWN, 0, "down");

	/** Indicates that a key has been released. */
	public static final KeyPhase UP = new KeyPhase(android.view.KeyEvent.ACTION_UP, 1, "up");
	

	/**
	 * Gets Android's unique integer ID for this key phase.
	 * @return Returns Android constant "KeyEvent.ACTION_DOWN" or "KeyEvent.ACTION_UP".
	 */
	public int toKeyEventAction() {
		return fAndroidNumericId;
	}

	/**
	 * Gets Corona's unique integer ID for this key phase.
	 * @return Returns a unique integer ID used by Corona's KeyEvent C++ class.
	 */
	public int toCoronaNumericId() {
		return fCoronaNumericId;
	}

	/**
	 * Gets Corona's unique string ID for this key phase.
	 * @return Returns a unique string ID used by Corona's key event in Lua.
	 */
	public String toCoronaStringId() {
		return fCoronaStringId;
	}

	/**
	 * Gets Corona's unique string ID for this key phase.
	 * @return Returns a unique string ID used by Corona's key event in Lua.
	 */
	@Override
	public String toString() {
		return fCoronaStringId;
	}

	/**
	 * Gets the key phase from the given Android key event object.
	 * @param event The Android key event object providing a phase via its getAction() method.
	 *              <p>
	 *              Setting this to null will cause an exception to occur.
	 * @return Returns the given key event's current phase.
	 */
	public static KeyPhase from(android.view.KeyEvent event) {
		if (event == null) {
			throw new NullPointerException();
		}
		return fromKeyEventAction(event.getAction());
	}

	/**
	 * Gets the key phase from the given key event action.
	 * @param value The key event action which must match one of the Android constants "KeyEvent.ACTION_DOWN",
	 *              "KeyEvent.ACTION_UP", or "KeyEvent.ACTION_MULTIPLE".
	 *              <p>
	 *              Corona does not support Android's "KeyEvent.ACTION_MULTIPLE", so it will be converted
	 *              to a DOWN phase. This makes sense since the multiple action occurs when holding down
	 *              a key, causing a key repeat event to occur.
	 * @return Returns the given key event's current phase.
	 */
	public static KeyPhase fromKeyEventAction(int value) {
		return (value == android.view.KeyEvent.ACTION_UP) ? KeyPhase.UP : KeyPhase.DOWN;
	}
}
