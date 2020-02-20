//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;


public class GyroscopeTask implements com.ansca.corona.CoronaRuntimeTask {
	private double myXRotation, myYRotation, myZRotation, myDeltaTime;
	
	public GyroscopeTask(double xRotation, double yRotation, double zRotation, double deltaTime)
	{
		myXRotation = xRotation;
		myYRotation = yRotation;
		myZRotation = zRotation;
		myDeltaTime = deltaTime;
	}
	
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
    	com.ansca.corona.JavaToNativeShim.gyroscopeEvent(runtime, myXRotation, myYRotation, myZRotation, myDeltaTime);
	}

}
