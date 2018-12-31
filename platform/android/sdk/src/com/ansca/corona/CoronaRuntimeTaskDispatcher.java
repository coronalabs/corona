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
 * Allows you to send tasks to be executed on the {@link com.ansca.corona.CoronaRuntime CoronaRuntime's} thread, just before Corona 
 * renders a frame. These tasks provide thread safe access to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} and its 
 * {@link com.naef.jnlua.LuaState LuaState}.
 * <p>
 * The main intention of this class is to provide a means for other threads to invoke Lua functions or
 * manipulate Lua global variables belonging to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime's} 
 * {@link com.naef.jnlua.LuaState LuaState}. This is done by implementing the {@link com.naef.jnlua.LuaState LuaState} manipulation code 
 * within a {@link com.ansca.corona.CoronaRuntimeTask CoronaRuntimeTask} object and then sending that task object
 * to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} via this class' 
 * {@link com.ansca.corona.CoronaRuntimeTaskDispatcher#send(com.ansca.corona.CoronaRuntimeTask) send()} method. This class will handle 
 * this in a thread safe manner and make sure that the task object is executed on the 
 * {@link com.ansca.corona.CoronaRuntime CoronaRuntime's} thread, just before it renders a frame.
 * <p>
 * Instances of this class are immutable.
 * @see CoronaRuntime
 * @see CoronaRuntimeTask
 */
public class CoronaRuntimeTaskDispatcher {
	/** Reference to the Corona runtime that this dispatcher will send tasks to. */
	private CoronaRuntime fRuntime;
	
	
	/**
	 * Creates a new dispatcher which will send tasks to the given {@link com.ansca.corona.CoronaRuntime CoronaRuntime}.
	 * @param runtime Reference to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} that this dispatcher will send tasks to.
	 *                <p>
	 *                Setting this to null is allowed, but will create a dispatcher that is permanently inactive.
	 *                <p>
	 *                The created dispatcher will also be permanently inactive if the given 
	 *                {@link com.ansca.corona.CoronaRuntime CoronaRuntime} has already been disposed/destroyed, which you can check by
	 *                calling the {@link com.ansca.corona.CoronaRuntime CoronaRuntime's} 
	 *                {@link com.ansca.corona.CoronaRuntime#wasDisposed() wasDiposed()} method.
	 *                The {@link com.ansca.corona.CoronaRuntime CoronaRuntime} will be disposed when its associated 
	 *                {@link com.ansca.corona.CoronaActivity CoronaActivity} has been destroyed.
	 *                <p>
	 *                You can check if the created dispatcher is capable of sending tasks to the targeted 
	 *                {@link com.ansca.corona.CoronaRuntime CoronaRuntime} by calling its 
	 *                {@link com.ansca.corona.CoronaRuntimeTaskDispatcher#isRuntimeAvailable() isRuntimeAvailable()} method.
	 */
	public CoronaRuntimeTaskDispatcher(CoronaRuntime runtime) {
		fRuntime = runtime;
	}
	
	/**
	 * Creates a new dispatcher which will send tasks to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} that owns the given 
	 * {@link com.naef.jnlua.LuaState LuaState}.
	 * @param luaState Reference to the {@link com.naef.jnlua.LuaState LuaState} that is owned by the 
	 *                 {@link com.ansca.corona.CoronaRuntime CoronaRuntime} that you want to target.
	 *                 <p>
	 *                 Setting this to null is allowed, but will create a dispatcher that is permanently inactive.
	 *                 <p>
	 *                 The created dispatcher will also be permanently inactive if the given 
	 *                 {@link com.ansca.corona.CoronaRuntime CoronaRuntime} has already been disposed/destroyed, which you can check by
	 *                 calling the {@link com.ansca.corona.CoronaRuntime CoronaRuntime's} 
	 *                 {@link com.ansca.corona.CoronaRuntime#wasDisposed() wasDiposed()} method.
	 *                 The {@link com.ansca.corona.CoronaRuntime CoronaRuntime} will be disposed when its associated 
	 *                 {@link com.ansca.corona.CoronaActivity CoronaActivity} has been destroyed.
	 *                 <p>
	 *                 You can check if the created dispatcher is capable of sending tasks to the targeted 
	 *                 {@link com.ansca.corona.CoronaRuntime CoronaRuntime} by calling its 
	 *                 {@link com.ansca.corona.CoronaRuntimeTaskDispatcher#isRuntimeAvailable() isRuntimeAvailable()} method.
	 */
	public CoronaRuntimeTaskDispatcher(com.naef.jnlua.LuaState luaState) {
		// Fetch a reference to the Corona runtime that owns the given Lua state.
		fRuntime = null;
		if (luaState != null) {
			CoronaRuntime runtime = CoronaRuntimeProvider.getRuntimeByLuaState(luaState);
			fRuntime = runtime;
		}
	}
	
	/**
	 * Determines if the targeted {@link com.ansca.corona.CoronaRuntime CoronaRuntime} is still available for this dispatcher to send 
	 * tasks to.
	 * @return Returns true if the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} is still available for this dispatcher to send 
	 *         tasks to.
	 *         <p>
	 *         Returns false if the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} has been disposed/destroyed, which happens when
	 *         its associated {@link com.ansca.corona.CoronaActivity CoronaActivity} has been destroyed. This permanently disables this 
	 *         dispatcher.
	 */
	public boolean isRuntimeAvailable() {
		// Do not continue if this dispatcher was never given a runtime object to send tasks to.
		if (fRuntime == null) {
			return false;
		}
		
		// Returns true if the targeted runtime is still active and has not been destroyed yet.
		return fRuntime.wasNotDisposed();
	}
	
	/**
	 * Determines if the targeted {@link com.ansca.corona.CoronaRuntime CoronaRuntime} is permanently unavailable, meaning that this 
	 * dispatcher no longer has a {@link com.ansca.corona.CoronaRuntime CoronaRuntime} to send tasks to.
	 * @return Returns true if the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} has been disposed/destroyed, which happens when 
	 *         its associated {@link com.ansca.corona.CoronaActivity CoronaActivity} has been destroyed. This permanently disables this 
	 *         dispatcher.
	 *         <p>
	 *         Returns false if the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} is still available.
	 */
	public boolean isRuntimeUnavailable() {
		return !isRuntimeAvailable();
	}
	
	/**
	 * Sends the given task to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} to be executed just before it renders a frame.
	 * This task will be executed on the same thread that the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} runs in.
	 * <p>
	 * The main intent of this method is to allow another thread to call a Lua function or
	 * manipulate a global variable in the {@link com.naef.jnlua.LuaState LuaState} on the same thread that Lua is running in.
	 * <p>
	 * Tasks will be executed by the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} in the order that they were sent by this method.
	 * @param task The task to be executed by the {@link com.ansca.corona.CoronaRuntime CoronaRuntime}.
	 *             Setting this to null will cause an {@link java.lang.Exception exception} to be thrown.
	 */
	public void send(CoronaRuntimeTask task) {
		// Validate argument.
		if (task == null) {
			throw new NullPointerException();
		}
		
		// Do not continue if the targeted Corona runtime is no longer available.
		if (isRuntimeUnavailable()) {
			android.util.Log.i("Corona", "CoronaRuntimeTaskDispatcher.send() doesn't have an available " + 
				"CoronaRuntime to run this task on! Abort!");
			return;
		}
		
		// Push the given task to the Corona event manager's queue.
		com.ansca.corona.events.EventManager eventManager = fRuntime.getController().getEventManager();
		if (eventManager != null) {
			eventManager.addEvent(new CoronaRuntimeTaskDispatcher.TaskEvent(task, fRuntime));
		} else {
			android.util.Log.i("Corona", "CoronaRuntimeTaskDispatcher.send() doesn't have an event manager to use! Abort!");
		}
	}
	
	/** Execute a Corona runtime task via the EventManager. */
	private static class TaskEvent extends com.ansca.corona.events.Event {
		/** The task to be executed by the Corona runtime. */
		private CoronaRuntimeTask fTask;
		
		private CoronaRuntime fCoronaRuntime;

		/**
		 * Creates an event object used to execute the given task via the Corona EventManager.
		 * @param task The Corona runtime task to be executed by the Corona runtime. Cannot be null.
		 */
		public TaskEvent(CoronaRuntimeTask task, CoronaRuntime runtime) {
			if (task == null) {
				throw new NullPointerException();
			}
			fTask = task;
			fCoronaRuntime = runtime;
		}
		
		/** Called by the Corona EventManager to execute the CoronaRuntimeTask object. */
		@Override
		public void Send() {
			// Make sure the runtime is actually valid
			if (fCoronaRuntime.wasDisposed()) {
				return;
			}
			
			// Execute the task.
			fTask.executeUsing(fCoronaRuntime);
		}
	}
}
