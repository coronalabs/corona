//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;


public class LocationTask implements com.ansca.corona.CoronaRuntimeTask {
	private double myLatitude, myLongitude, myAltitude, myAccuracy, mySpeed, myBearing, myTime;
	
	public LocationTask( double latitude, double longitude, double altitude, double accuracy, double speed, double bearing, double time )
	{
		myLatitude = latitude;
		myLongitude = longitude;
		myAltitude = altitude;
		myAccuracy = accuracy;
		mySpeed = speed;
		myBearing = bearing;
		myTime = time;
	}
	
	public LocationTask( double bearing )
	{
		myLatitude = 0;
		myLongitude = 0;
		myAltitude = 0;
		myAccuracy = 0;
		mySpeed = 0;
		myBearing = bearing;
		myTime = 0;
	}

	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
    	com.ansca.corona.JavaToNativeShim.locationEvent( runtime, myLatitude, myLongitude, myAltitude, myAccuracy, mySpeed, myBearing, myTime );
	}

}
