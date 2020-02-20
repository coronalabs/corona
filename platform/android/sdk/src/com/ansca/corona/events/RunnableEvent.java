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


/** A generic event class that executes a Runnable object when its Send() method is called. */
public class RunnableEvent extends Event {
	/** The Runnable object to be executed when the Send() method is called. */
	private java.lang.Runnable fRunnable;
	
	/**
	 * Creates a new event object that will execute the given Runnable object when Send() is called.
	 * @param runnable The runnable object to be executed.
	 */
	public RunnableEvent(java.lang.Runnable runnable) {
		fRunnable = runnable;
	}
	
	/** Sends the event by executing the provided Runnable object. */
	@Override
	public void Send() {
		if (fRunnable != null) {
			fRunnable.run();
		}
	}

}
