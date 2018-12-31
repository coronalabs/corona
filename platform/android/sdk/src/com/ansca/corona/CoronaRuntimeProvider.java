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

import java.util.Collection;
import java.util.concurrent.ConcurrentHashMap;

import com.naef.jnlua.LuaState;

/** An internal use only class used to store all active CoronaRuntimes objects */
public class CoronaRuntimeProvider {
	private static ConcurrentHashMap<LuaState, CoronaRuntime> sRuntimesByLuaState = new ConcurrentHashMap<LuaState, CoronaRuntime>();

	/**
	 * Adds the CoronaRUntime to the list of active runtimes
	 * @param runtime The runtime to add
	 */
	public static void addRuntime(CoronaRuntime runtime) {
		LuaState luaState = runtime.getLuaState();
		sRuntimesByLuaState.put(luaState, runtime);
	}

	/**
	 * Removes the CoronaRUntime to the list of active runtimes
	 * @param runtime The runtime to remove
	 */
	public static void removeRuntime(CoronaRuntime runtime) {
		LuaState luaState = runtime.getLuaState();
		if (luaState != null) {
			sRuntimesByLuaState.remove(luaState);
		}
	}

	/**
	 * Gets the runtime associated with the LuaState object
	 * @param runtime The lua state associated with the runtime
	 * @return The runtime associated with the lua state
	 */
	public static CoronaRuntime getRuntimeByLuaState(LuaState L) {
		return sRuntimesByLuaState.get(L);
	}

	/**
	 * Retrieves all the active CoronaRuntimes
	 * @return A collection of all the active runtimes
	 */
	public static Collection<CoronaRuntime> getAllCoronaRuntimes() {
		return sRuntimesByLuaState.values();
	}

	/**
	 * Gets the memory address of a LuaState object
	 * @param luaState The LuaState you want the memory address of
	 * @return The memory address if found.  If not the memory address will be 0.
	 */
	public static long getLuaStateMemoryAddress(LuaState luaState) {
		long memoryAddress = 0;
		try {
			java.lang.reflect.Field luaStateMemoryAddressField = luaState.getClass().getDeclaredField("luaState");
			luaStateMemoryAddressField.setAccessible(true);
			memoryAddress = luaStateMemoryAddressField.getLong(luaState);
		}
		catch (Exception ex) { }

		return memoryAddress;
	}
}
