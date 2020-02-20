//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;


public class WebHistoryUpdatedTask implements com.ansca.corona.CoronaRuntimeTask {
	private int fId;
	private boolean fCanGoBack;
	private boolean fCanGoForward;
	
	
	public WebHistoryUpdatedTask(int id, boolean canGoBack, boolean canGoForward) {
		fId = id;
		fCanGoBack = canGoBack;
		fCanGoForward = canGoForward;
	}
	
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.webViewHistoryUpdated(runtime, fId, fCanGoBack, fCanGoForward);
	}
}
