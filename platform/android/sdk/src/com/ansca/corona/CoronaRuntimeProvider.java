//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
	 * @param L The lua state associated with the runtime
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
