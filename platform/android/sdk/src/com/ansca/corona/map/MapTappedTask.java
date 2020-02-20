//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.maps;


public class MapTappedTask implements com.ansca.corona.CoronaRuntimeTask {
	int fId;
	double fLongitude;
	double fLatitude;

	public MapTappedTask(int id, double latitude, double longitude) {
		fId = id;
		fLongitude = longitude;
		fLatitude = latitude;
	}
	
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.mapTappedEvent(runtime, fId, fLatitude, fLongitude);
	}
}
