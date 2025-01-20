//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;


public class ShouldLoadUrlTask implements com.ansca.corona.CoronaRuntimeTask {
	private String myUrl;
	private int myId;
	private int mySourceType;
	
	public ShouldLoadUrlTask(int id, String url, int sourceType) {
		myId = id;
		myUrl = url;
		mySourceType = sourceType;
	}

	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.webViewShouldLoadUrl(runtime, myId, myUrl, mySourceType);
	}
}
