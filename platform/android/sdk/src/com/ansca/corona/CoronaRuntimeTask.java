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
 * Represents a single task to be executed by the {@link com.ansca.corona.CoronaRuntime CoronaRuntime}, 
 * just before it renders a frame. The task will be executed on the same thread that the 
 * {@link com.ansca.corona.CoronaRuntime CoronaRuntime} runs in.
 * <p>
 * The purpose of this class is to allow another thread to send a task to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime}
 * that will manipulate its {@link com.naef.jnlua.LuaState LuaState}, such as calling a Lua function, on the same thread that
 * the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} and Lua is running on. This is important because you should never manipulate
 * the {@link com.naef.jnlua.LuaState LuaState} on another thread since it could cause race conditions to occur and exceptions
 * to be thrown, which could crash the application.
 * <p>
 * Instances of this interface are to be given to a 
 * {@link com.ansca.corona.CoronaRuntimeTaskDispatcher#send(com.ansca.corona.CoronaRuntimeTask) CoronaRuntimeTaskDispatcher object's send()} 
 * method. That class will send tasks to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} in a thread safe manner.
 * <p>
 * <b>WARNING!</b> This task will not be executed by the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} on the main UI thread.
 * The {@link com.ansca.corona.CoronaRuntime CoronaRuntime} and its Lua scripts run on a separate thread.
 * This means that you should never manipulate an
 * <a href="http://developer.android.com/reference/android/app/Activity.html">activity's</a> UI elements from this class' method.
 * @see CoronaRuntime
 * @see CoronaRuntimeTaskDispatcher
 */
public interface CoronaRuntimeTask {
	/**
	 * Called by the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} just before it renders a frame.
	 * This is the application's opportunity to call a Lua function or update global variables in Lua.
	 * <p>
	 * <b>WARNING!</b> This method is not called on the main UI thread.
	 * @param runtime Reference to the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} object that is currently running.
	 *                Provides access to its {@link com.naef.jnlua.LuaState LuaState} object.
	 */
	public void executeUsing(CoronaRuntime runtime);
}
