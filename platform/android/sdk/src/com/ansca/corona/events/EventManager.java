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


/**
 * Manages Events. This kind of events are things that happen during threads and callbacks that need
 * to be serialized in a main event loop kind of way. We process them during rendering.
 * 
 * @author Eric
 */
public class EventManager {

	private java.util.LinkedList<Event> myEvents = new java.util.LinkedList<Event>();
	private com.ansca.corona.Controller myController;

	public EventManager(Controller controller) {
		myController = controller;
	}

	public void addEvent( Event e ) {
		synchronized (this) {
			myEvents.add( e );
		}
		myController.requestEventRender();
	}
	
	public synchronized void removeAllEvents() {
		myEvents.clear();
	}
	
	public void loadSound( int id, String name )
	{
		myController.getMediaManager().loadSound( id, name);
	}

	public void loadEventSound( int id, String name )
	{
		myController.getMediaManager().loadEventSound( id, name );
	}

	public void playSound( int id, String soundName, boolean loop )
	{
		MediaEvent e = new MediaEvent( myController, id, soundName, MediaEvent.Event.PlaySound );
		
		e.setLooping( loop );
		
		addEvent( e );
	}

	public void stopSound( int id )
	{
		Event e = new MediaEvent( myController, id, MediaEvent.Event.StopSound );
		
		addEvent( e );
	}

	public void pauseSound( int id )
	{
		Event e = new MediaEvent( myController, id, MediaEvent.Event.PauseSound );
		
		addEvent( e );
	}

	public void resumeSound( int id )
	{
		Event e = new MediaEvent( myController, id, MediaEvent.Event.ResumeSound );
		
		addEvent( e );
	}

	public void playVideo( int id, String name )
	{
		Event e = new MediaEvent( myController, id, name, MediaEvent.Event.PlayVideo );
		
		addEvent( e );
	}

	public void sendEvents()
	{
		// Make a shallow copy of the main event queue and then clear it.
		// We do this to avoid race conditions in case an event gets added while in the middle of dispatching events.
		java.util.LinkedList<Event> clonedEventQueue;
		synchronized (this) {
			clonedEventQueue = (java.util.LinkedList<Event>)myEvents.clone();
			myEvents.clear();
		}

		// Dispatch all queued events.
		for (Event event : clonedEventQueue) {
			event.Send();
		}
	}
}
