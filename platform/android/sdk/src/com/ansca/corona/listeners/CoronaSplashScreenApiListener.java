//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.listeners;

/** The interface has all the funcations that are activity specific and aren't implemented by default. */
public interface CoronaSplashScreenApiListener{
	/**
	 * Called when the splash screen should be shown.
	 */
	public void showSplashScreen();

	/**
	 * Called when the splash screen should be hidden.
	 */
	public void hideSplashScreen();
}
