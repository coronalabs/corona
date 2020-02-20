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
	
	public void loadSound( long id, String name )
	{
		myController.getMediaManager().loadSound( id, name);
	}

	public void loadEventSound( long id, String name )
	{
		myController.getMediaManager().loadEventSound( id, name );
	}

	public void playSound( long id, String soundName, boolean loop )
	{
		MediaEvent e = new MediaEvent( myController, id, soundName, MediaEvent.Event.PlaySound );
		
		e.setLooping( loop );
		
		addEvent( e );
	}

	public void stopSound( long id )
	{
		Event e = new MediaEvent( myController, id, MediaEvent.Event.StopSound );
		
		addEvent( e );
	}

	public void pauseSound( long id )
	{
		Event e = new MediaEvent( myController, id, MediaEvent.Event.PauseSound );
		
		addEvent( e );
	}

	public void resumeSound( long id )
	{
		Event e = new MediaEvent( myController, id, MediaEvent.Event.ResumeSound );
		
		addEvent( e );
	}

	public void playVideo( long id, String name )
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
