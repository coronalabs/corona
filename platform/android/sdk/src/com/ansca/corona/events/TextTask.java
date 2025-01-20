//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;

import com.ansca.corona.ViewManager;


public class TextTask implements com.ansca.corona.CoronaRuntimeTask {
	private int myEditTextId;
	private boolean myHasFocus;
	private boolean myIsDone;
	
	public TextTask( int id, boolean f, boolean done )
	{
		myEditTextId = id;
		myHasFocus = f;
		myIsDone = done;
	}
	
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.textEvent( runtime, myEditTextId, myHasFocus, myIsDone );
	}
}
