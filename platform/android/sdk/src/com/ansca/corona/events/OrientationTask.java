//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;


public class OrientationTask implements com.ansca.corona.CoronaRuntimeTask {
	int myNewOrientation, myOldOrientation;
	
	public OrientationTask( int newOrientation, int oldOrientation ) {
		myNewOrientation = newOrientation;
		myOldOrientation = oldOrientation;
	}

	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
    	com.ansca.corona.JavaToNativeShim.orientationChanged(runtime, myNewOrientation, myOldOrientation);
	}

}
