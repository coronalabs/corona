//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.maps;


public class MapRequestLocationTask implements com.ansca.corona.CoronaRuntimeTask {
	private double fLatitude;
	private double fLongitude;
	private String fOriginalRequest;
	private int fListener;

	public MapRequestLocationTask(int listener, double latitude, double longitude, String originalRequest)
	{
		fLatitude = latitude;
		fLongitude = longitude;
		fOriginalRequest = originalRequest;
		fListener = listener;
	}
	
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.mapRequestLocationEvent(runtime, fListener, fLatitude, fLongitude, fOriginalRequest);
	}
}
