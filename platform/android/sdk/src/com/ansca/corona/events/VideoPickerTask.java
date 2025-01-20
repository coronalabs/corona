//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;


/** Provides the result of the video picker window to be passed over to the C++ side of Corona. */
public class VideoPickerTask extends MediaPickerTask {
	private int fDuration;
	private long fSize;

	public VideoPickerTask() {
		super();
		fDuration = -1;
		fSize = -1;
	}

	public VideoPickerTask(String selectedFileName) {
		super(selectedFileName);
		fDuration = -1;
		fSize = -1;
	}

	public VideoPickerTask(String selectedFileName, int duration, long size) {
		super(selectedFileName);
		fDuration = duration;
		fSize = size;
	}

	/**
	 * Sends this event's data to Corona.
	 */
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		java.io.File file = new java.io.File(super.fSelectedMediaFileName);
		if (file.exists()) {
			com.ansca.corona.JavaToNativeShim.videoPickerEvent(runtime, android.net.Uri.decode(android.net.Uri.fromFile(file).toString()), fDuration, fSize);
		} else {
			com.ansca.corona.JavaToNativeShim.videoPickerEvent(runtime, super.fSelectedMediaFileName, fDuration, fSize);
		}
	}

}
