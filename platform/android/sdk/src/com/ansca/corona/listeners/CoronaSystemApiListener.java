//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.listeners;

import android.content.Intent;
import com.ansca.corona.CoronaRuntime;

/** The interface has all the functions that are activity specific and aren't implemented by default. */
public interface CoronaSystemApiListener{	
	public boolean requestSystem(CoronaRuntime runtime, String actionName, long luaStateMemoryAddress, int luaStackIndex);
	
	public Intent getIntent();
	public Intent getInitialIntent();
}
