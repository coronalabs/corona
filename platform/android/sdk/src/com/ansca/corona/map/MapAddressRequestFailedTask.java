//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.maps;


public class MapAddressRequestFailedTask implements com.ansca.corona.CoronaRuntimeTask {
	private String fErrorMessage;
	
	
	public MapAddressRequestFailedTask(String errorMessage) {
		fErrorMessage = errorMessage;
	}
	
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.mapAddressRequestFailedEvent(runtime, fErrorMessage);
	}
}
