//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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
