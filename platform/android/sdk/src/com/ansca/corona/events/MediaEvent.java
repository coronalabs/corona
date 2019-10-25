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

import com.ansca.corona.Controller;

public class MediaEvent extends Event {

	String myMediaName;
	long myMediaId;
	boolean myLooping = false;
	Controller myController;

	enum Event {
		LoadSound,
		PlaySound,
		StopSound,
		PauseSound,
		ResumeSound,
		PlayVideo,
		SoundEnded,
		VideoEnded
	}
	
	Event myEvent;
	
	MediaEvent( Controller controller, String name, Event event )
	{
		myEvent = event;
		myMediaName = name;
		myController = controller;
	}

	MediaEvent( Controller controller, long id, Event event )
	{
		myEvent = event;
		myMediaId = id;
		myController = controller;
	}

	MediaEvent( Controller controller, long id, String name, Event event )
	{
		myEvent = event;
		myMediaId = id;
		myMediaName = name;
		myController = controller;
	}

	public void setLooping( boolean l )
	{
		myLooping = l;
	}
	
	@Override
	public void Send() {
		switch ( myEvent ) {
		case LoadSound:
			break;
		case PlaySound:
			myController.getMediaManager().playMedia( myMediaId, myLooping );
			break;
		case StopSound:
			myController.getMediaManager().stopMedia( myMediaId );
			break;
		case PauseSound:
			myController.getMediaManager().pauseMedia( myMediaId );
			break;
		case ResumeSound:
			myController.getMediaManager().resumeMedia( myMediaId );
			break;
		case PlayVideo:
			myController.getMediaManager().playVideo( myMediaId, myMediaName, true );
			break;
		}
	}
}
