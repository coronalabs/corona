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
 * KeyEvent derived class used to indicate a key sent from Corona to the Android activity.
 * <p>
 * Instances of this key event type are ignored by Corona's InputHandler and are never sent
 * to Corona's key event listener in Lua.
 * <p>
 * This class is typically used when a key event received from Android is not overriden by
 * Corona's key event listener in Lua. In this case, the original key event object is wrapped
 * by an instance of this class and sent back to the Corona activity so that Android will
 * invoke its default Android handling.
 */
public class CoronaKeyEvent extends android.view.KeyEvent {
	/**
	 * Creates a copy of the given key event object.
	 * @param event The event settings to be copied into the new key event object.
	 */
	public CoronaKeyEvent(android.view.KeyEvent event) {
		super(event);
	}
}
