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


/** Provides information that was received after successfully registering for push notifications. */
public class NotificationRegistrationTask implements com.ansca.corona.CoronaRuntimeTask {
	/** The unique string assigned to this application by the push notification service. */
	private String fRegistrationId;


	/**
	 * Creates a new notification registration event to be sent to the native side of Corona.
	 * @param registrationId The unique string ID assigned to this application by the push notification service.
	 */
	public NotificationRegistrationTask(String registrationId) {
		// Validate arguments.
		if (registrationId == null) {
			throw new NullPointerException();
		}

		// Store settings.
		fRegistrationId = registrationId;
	}
	
	/** Sends the event to the native side of Corona. */
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		// Fetch the Corona runtime's Lua state.
		com.naef.jnlua.LuaState luaState = null;
		if (runtime != null) {
			luaState = runtime.getLuaState();
		}
		if (luaState == null) {
			return;
		}

		// Push this event's information to a new Lua table.
		luaState.newTable();
		int luaTableStackIndex = luaState.getTop();
		luaState.pushString(NotificationReceivedTask.NAME);
		luaState.setField(luaTableStackIndex, "name");
		luaState.pushString(com.ansca.corona.notifications.NotificationType.REMOTE_REGISTRATION.toInvariantString());
		luaState.setField(luaTableStackIndex, "type");
		luaState.pushString(fRegistrationId);
		luaState.setField(luaTableStackIndex, "token");

		// Dispatch the event at the top of the Lua stack.
		com.ansca.corona.JavaToNativeShim.dispatchEventInLua(runtime);

		// Pop the event from the Lua stack.
		luaState.pop(1);
	}
}
