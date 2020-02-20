//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.maps;


public class MapRequestLocationFailedTask implements com.ansca.corona.CoronaRuntimeTask {
	private String fErrorMessage;
	private String fOriginalRequest;
	private int fListener;
	
	public MapRequestLocationFailedTask(int listener, String errorMessage, String originalRequest) {
		fErrorMessage = errorMessage;
		fListener = listener;
		fOriginalRequest = originalRequest;
	}
	
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.mapRequestLocationFailed(runtime, fListener, fErrorMessage, fOriginalRequest);
	}
}
