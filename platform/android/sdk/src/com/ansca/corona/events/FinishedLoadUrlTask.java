//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;


public class FinishedLoadUrlTask implements com.ansca.corona.CoronaRuntimeTask {
	private int fId;
	private String fUrl;
	
	
	public FinishedLoadUrlTask(int id, String url) {
		fId = id;
		fUrl = url;
	}
	
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.webViewFinishedLoadUrl(runtime, fId, fUrl);
	}
}
