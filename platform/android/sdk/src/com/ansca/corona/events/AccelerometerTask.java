//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;


public class AccelerometerTask implements com.ansca.corona.CoronaRuntimeTask {
	private double myX, myY, myZ, myDeltaTime;
	
	public AccelerometerTask(double x, double y, double z, double deltaTime)
	{
		myX = x;
		myY = y;
		myZ = z;
		myDeltaTime = deltaTime;
	}
	
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
    	com.ansca.corona.JavaToNativeShim.accelerometerEvent(runtime, myX, myY, myZ, myDeltaTime);
	}

}
