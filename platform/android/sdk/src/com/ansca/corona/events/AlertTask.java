//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;


public class AlertTask implements com.ansca.corona.CoronaRuntimeTask {
	private int myButtonIndex;
	private boolean mCancelled;

	public AlertTask( int which, boolean cancelled )
	{
		myButtonIndex = which;
		mCancelled = cancelled;
	}
	
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.alertCallback( runtime, myButtonIndex, mCancelled );
	}

}
