//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.maps;


public class MapMarkerTask implements com.ansca.corona.CoronaRuntimeTask {
	MapMarker fMarker;

	public MapMarkerTask(MapMarker marker)
	{
		fMarker = marker;
	}
	
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		if (fMarker.getListener() == com.ansca.corona.CoronaLua.REFNIL ||
			fMarker.getListener() == com.ansca.corona.CoronaLua.NOREF) {

			return;
		}
		com.ansca.corona.JavaToNativeShim.mapMarkerEvent(
			runtime, fMarker.getMarkerId(), fMarker.getListener(), fMarker.getLatitude(), fMarker.getLongitude());		
	}
}
