//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;

public class SoundEndedTask implements com.ansca.corona.CoronaRuntimeTask {
	private long fMediaId;

	public SoundEndedTask( long mediaId )
	{
		fMediaId = mediaId;
	}

	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
	    com.ansca.corona.JavaToNativeShim.soundEndCallback( runtime, fMediaId );
	}
}
