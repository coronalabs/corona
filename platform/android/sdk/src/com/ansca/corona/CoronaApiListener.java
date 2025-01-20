//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

/** The interface has all the functions that are activity specific and shouldn't be exposed in corona kit */
public interface CoronaApiListener{
	public void onScreenLockStateChanged(boolean isScreenLocked);

	public void addKeepScreenOnFlag();
	public void removeKeepScreenOnFlag();
}
