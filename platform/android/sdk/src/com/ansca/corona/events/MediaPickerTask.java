//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;


/** Provides the result of the media picker window to be passed over to the C++ side of Corona. */
public abstract class MediaPickerTask implements com.ansca.corona.CoronaRuntimeTask {
	/** Set to the path\file name of the media that was selected by the Picker window. */
	protected String fSelectedMediaFileName;
	
	public MediaPickerTask() {
		fSelectedMediaFileName = "";
	}

	public MediaPickerTask(String selectedMediaFileName) {
		fSelectedMediaFileName = (selectedMediaFileName != null) ? selectedMediaFileName : "";
	}

	/**
	 * Sends this event's data to Corona.
	 */
	@Override
	abstract public void executeUsing(com.ansca.corona.CoronaRuntime runtime);

}
