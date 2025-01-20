//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;


public class DidFailLoadUrlTask implements com.ansca.corona.CoronaRuntimeTask {
	private int myId;
	private String myFailingUrl;
	private String myDescription;
	private int myErrorCode;
	
	public DidFailLoadUrlTask( int id, String failingUrl, String description, int errorCode ) {
		myId = id;
		myFailingUrl = failingUrl;
		myDescription = description;
		myErrorCode = errorCode;
	}
	
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.webViewDidFailLoadUrl(
					runtime, myId, myFailingUrl, myDescription, myErrorCode);
	}

}
