//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
