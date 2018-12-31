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

package com.ansca.corona;


/**
 * Generates recurring "onTimerElapsed" events using the standard Android message queue.
 * <p>
 * Timer elapse events can be received by implementing the MessageBasedTimer.Listener interface
 * and assigning that listener to the timer object's setListener() method.
 * <p>
 * This timer requires a "Handler" object to be assigned to it via the setHandler() method before
 * it can start running. This "Handler" object is used to post timer messages to the message queue.
 */
public class MessageBasedTimer {
	/** Handler object used to post the timer's "onTimerElapsed" messages to. */
	private android.os.Handler fHandler;
	
	/** Runnable object to be added to fHandler's message queue used to raise the "onTimerElapsed" event. */
	private Runnable fRunnable;
	
	/** Listener object which receives this timer's events. */
	private MessageBasedTimer.Listener fListener;
	
	/** The amount of time to wait between timer "elapsed" events. */
	private TimeSpan fInterval;
	
	/**
	 * The next timer elapse time which is used while this timer is running.
	 * Stores time in system ticks to make this timer immune to system clock changes.
	 */
	private Ticks fNextElapseTimeInTicks;
	
	/** Set true if the timer is running. Set false if not. */
	private boolean fIsRunning;
	
	
	/** The minimum interval allowed by this timer. */
	public static final TimeSpan MIN_INTERVAL = TimeSpan.fromMilliseconds(5);
	
	
	/**
	 * Listener used to receive a MessageBoasedTimer object's "onTimerElapsed" events.
	 * Will receive events when assigned to the timer via its MessageBoasedTimer.setListener() method.
	 */
	public interface Listener {
		/** Called when a timer interval has elapsed. */
		public void onTimerElapsed();
	}
	
	
	/** Creates a new timer object initialized to its defaults.. */
	public MessageBasedTimer() {
		fHandler = null;
		fRunnable = null;
		fListener = null;
		fInterval = TimeSpan.fromMinutes(1);
		fNextElapseTimeInTicks = Ticks.fromCurrentTime();
		fIsRunning = false;
	}
	
	/**
	 * Sets the handler to be used to post timer messages to.
	 * <p>
	 * This handler is required or else this timer cannot be started.
	 * <p>
	 * If the timer's listener will be manipulating any UI, then you should give this timer a
	 * handler that belongs to the main UI thread to prevent exceptions from occurring.
	 * <p>
	 * You can change the handler while the timer is currently running. This will cause this timer
	 * to remove posted message from the last handler and restart itself on the given handler.
	 * @param handler Reference to the handler to be used to post timer messages to.
	 *                Set to null to remove the handler, which will also automatically stop the timer.
	 */
	public void setHandler(android.os.Handler handler) {
		// Do not continue if the handler has not changed.
		if (handler == fHandler) {
			return;
		}
		
		// Stop the timer if it is currently running.
		boolean wasRunning = fIsRunning;
		if (fIsRunning) {
			stop();
		}
		
		// Store the given handler.
		fHandler = handler;
		
		// Restart the timer if it was running before.
		if (wasRunning && (handler != null)) {
			start();
		}
	}
	
	/**
	 * Gets the handler that this timer uses to post messages to.
	 * @return Returns the handler that this timer uses to post messages to.
	 *         Returns null if a handler has not been assigned to this timer.
	 */
	public android.os.Handler getHandler() {
		return fHandler;
	}
	
	/**
	 * Sets the listener object that will receive this timer's events.
	 * @param listener The listener to receive this timer's events. Set to null to remove the listener.
	 */
	public void setListener(MessageBasedTimer.Listener listener) {
		fListener = listener;
	}
	
	/**
	 * Gets the listener object that this timer will send events to.
	 * @return Returns the listener object that will receive this timer's events.
	 *         Returns null if a listener has not been assigned.
	 */
	public MessageBasedTimer.Listener getListener() {
		return fListener;
	}
	
	/**
	 * Sets the amount of time to wait between "onTimerElapsed" events.
	 * @param interval The amount of time to wait between timer events.
	 *                 Cannot be less than "MessageBasedTimer.MIN_INTERVAL".
	 *                 Setting this to null will cause an exception to be thrown.
	 */
	public void setInterval(TimeSpan interval) {
		// Validate.
		if (interval == null) {
			throw new NullPointerException();
		}
		
		// Do not allow the interval to be set to less than the minimum.
		// If this occurs, then floor the interval to the minimum instead.
		if (interval.compareTo(MIN_INTERVAL) <= 0) {
			interval = MIN_INTERVAL;
		}
		
		// Do not continue if the interval has not changed.
		if (interval.equals(fInterval)) {
			return;
		}
		
		// Store the given interval.
		fInterval = interval;
	}
	
	/**
	 * Gets the interval used by the timer.
	 * @return Returns the interval used by this timer.
	 */
	public TimeSpan getInterval() {
		return fInterval;
	}
	
	/**
	 * Determines if this timer has been started and is currently running.
	 * @return Returns true if this timer is enabled and running. Returns false if not.
	 */
	public boolean isRunning() {
		return fIsRunning;
	}
	
	/**
	 * Determines if this timer is not running.
	 * @return Returns true if this timer is currently stopped. Returns false if it is running.
	 */
	public boolean isNotRunning() {
		return !fIsRunning;
	}
	
	/** Starts the timer. */
	public void start() {
		// Do not continue if already running.
		if (fIsRunning) {
			return;
		}
		
		// Do not continue if not given a handler to post timer messages to.
		if (fHandler == null) {
			return;
		}
		
		// Start the timer.
		fRunnable = new Runnable() {
			@Override
			public void run() {
				onElapsed();
			}
		};
		
		// Start the timer.
		fIsRunning = true;
		fNextElapseTimeInTicks = Ticks.fromCurrentTime().add(fInterval);
		fHandler.postDelayed(fRunnable, fInterval.getTotalMilliseconds());
	}
	
	/** Stops the timer. */
	public void stop() {
		// Do not continue if already stopped.
		if (!fIsRunning) {
			return;
		}
		
		// Stop the timer.
		if ((fHandler != null) && (fRunnable != null)) {
			fHandler.removeCallbacks(fRunnable);
			fRunnable = null;
		}
		fIsRunning = false;
	}
	
	/**
	 * Called when this timer's Runnable object has been processed by the message queue.
	 * Notifies this timer's assigned listener and then schedules the next timer elapse event.
	 */
	private void onElapsed() {
		// Do not continue if the timer was just stopped.
		if (!fIsRunning) {
			return;
		}
		
		// Notify the timer's listener.
		if (fListener != null) {
			fListener.onTimerElapsed();
		}
		
		// Do not continue if the above listener just stopped the timer.
		if (!fIsRunning) {
			return;
		}
		
		// Post the next timer event to the handler's message queue.
		Ticks currentTimeInTicks = Ticks.fromCurrentTime();
		do {
			fNextElapseTimeInTicks = fNextElapseTimeInTicks.add(fInterval);
		} while (fNextElapseTimeInTicks.compareTo(currentTimeInTicks) <= 0);
		long delayInMilliseconds = fNextElapseTimeInTicks.subtract(currentTimeInTicks).getTotalMilliseconds();
		fHandler.postDelayed(fRunnable, delayInMilliseconds);
	}
}
