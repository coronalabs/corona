//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.listeners;

import com.ansca.corona.CoronaStatusBarSettings;

/** The interface has all the funcations that are activity specific and aren't implemented by default. */
public interface CoronaStatusBarApiListener{	
	/**
	 * This should return the current status bar mode.
	 */
	public CoronaStatusBarSettings getStatusBarMode();

	/**
	 * Sets the status bar mode.
	 * @param mode The settings for the status bar
	 */
	public void setStatusBarMode(CoronaStatusBarSettings mode);

	/**
	 * Gets the height of the status bar
	 * @return The height of the status bar in pixels.
	 */
	public int getStatusBarHeight();

	/**
	 * Gets the height of the navigation bar
	 * @return The height of the navigation bar in pixels.
	 */
	public int getNavigationBarHeight();

	/**
	 * Helps to understand is the device is TV
	 * @return true if it is TV
	 */
	public boolean IsAndroidTV();

	/**
	 * Indicates wherether device has harware navigation buttons
	 * @return true if it has
	 */
	public boolean HasSoftwareKeys();
}
