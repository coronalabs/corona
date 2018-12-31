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

#pragma once

#include "CoronaLua.h"
extern "C"
{
#	include "lua.h"
#	include "lauxlib.h"
}


namespace Interop {

template<class TClass>
/**
  Registers a Lua function to invoke a given C++ object method.
 
  Provides an easy means of pushing the function to Lua and adding/removing it as a Corona Runtime event listener.
 
  Will safely unregister the object's pointer from the Lua listener after releasing or destroying callback instances,
  preventing crashes in case the Lua function reference out-lives the callback.
 */
class LuaMethodCallback
{
	public:
		/** Defines a class method signature to be passed into the handler's constructor. */
		typedef int(TClass::*MethodType)(lua_State*);

		/** Creates a null callback which no-ops. */
		LuaMethodCallback()
		:	fObjectPointer(NULL),
			fMethodPointer(NULL),
			fLuaStatePointer(NULL),
			fLuaRegistryReferenceId(LUA_NOREF)
		{
		}

		/**
		  Creates a C++ object method callback that can be later registered to a Lua state via the RegisterTo() method.

		  @param objectPointer
		  Pointer to the object that argument "methodPointer" belongs to.
		 
		  Can be null, causing this callback to no-op and not register a function into Lua.

		  @param methodPointer
		  Pointer to the C++ class' method to be invoked by the registered Lua function.
		 
		  Can be null, causing this callback to no-op and not register a function into Lua.
		 */
		LuaMethodCallback(TClass* objectPointer, MethodType methodPointer)
		:	fObjectPointer(objectPointer),
			fMethodPointer(methodPointer),
			fLuaStatePointer(NULL),
			fLuaRegistryReferenceId(LUA_NOREF)
		{
		}

		/** Releases this callback's function from Lua and then destroys this object. */
		virtual ~LuaMethodCallback()
		{
			Unregister();
		}

		/**
		  Determines if this callback's function is currently registered into Lua and can be invoked.

		  @return
		  Returns true if this callback's function is currently registered into Lua.
		  This means that the object method given to this callback can be invoked when its associated
		  Lua function gets invoked.
		
		  Returns false if this callback's function is not registered into Lua.
		  This can happen if this callback's Unregister() method was called or if the callback's constructor
		  was given null arguments.
		 */
		bool IsRegistered() const
		{
			return (fLuaRegistryReferenceId != LUA_NOREF);
		}

		/**
		  Gets a pointer to the Lua state this callback's Lua function was registered to.

		  @return
		  Returns a pointer to the Lua state this callback's Lua function was registered to.
		
		  Returns null if this callback's Lua function is no longer registered into Lua.
		 */
		lua_State* GetLuaState() const
		{
			return fLuaStatePointer;
		}

		/**
		  Creates and registers a Lua function which will invoke this callback's C++ object method.

		  @param luaStatePointer
		  Pointer to the Lua state this callback will register a Lua function to.

		  @return
		  Returns true if this callback's Lua function was successfully created and registered in Lua.

		  Returns false if this callback has already been registered to Lua.
		  Will also return false if given a null argument, if the constructor was given null pointers,
		  or if failed to create/register a Lua function for this callback.
		 */
		bool RegisterTo(lua_State* luaStatePointer)
		{
			// Validate.
			if (!fObjectPointer || !fMethodPointer || !luaStatePointer)
			{
				return false;
			}

			// Do not continue if this callback is already registered to a Lua state.
			if (IsRegistered())
			{
				return false;
			}

			// If the given Lua state belongs to a coroutine, then use the main Lua state instead.
			{
				lua_State* mainLuaStatePointer = CoronaLuaGetCoronaThread(luaStatePointer);
				if (mainLuaStatePointer && (mainLuaStatePointer != luaStatePointer))
				{
					luaStatePointer = mainLuaStatePointer;
				}
			}

			// Create a Lua closure to this class' private static OnInvoked() function.
			// Add the closure to the Lua registry to prevent it from being garbage collected.
			// Our OnInvoked() function will then call the give object method safely when invoked.
			lua_pushlightuserdata(luaStatePointer, (void*)this);
			lua_pushcclosure(luaStatePointer, OnInvoked, 1);
			const int referenceId = luaL_ref(luaStatePointer, LUA_REGISTRYINDEX);
			if ((referenceId == LUA_REFNIL) || (referenceId == LUA_NOREF))
			{
				return false;
			}

			// Lua closure was successfully configured. Update member variables.
			fLuaStatePointer = luaStatePointer;
			fLuaRegistryReferenceId = referenceId;
			return true;
		}

		/**
		  Releases this callback object's reference to its Lua function and prevents that Lua function from
		  invoking the callback's given object method.
		
		  This callback object's IsRegistered() method will return false after this and its other methods
		  will fail to push its function to Lua.
		 */
		void Unregister()
		{
			// Do not continue if already released.
			if (LUA_NOREF == fLuaRegistryReferenceId)
			{
				return;
			}

			// Release this object's reference to its closure from Lua.
			if (fLuaStatePointer)
			{
				// Fetch the current Lua stack count.
				int luaStackCount = lua_gettop(fLuaStatePointer);

				// First, set the closure's upvalue to a null pointer. (Was set to a pointer to this object.)
				// This is in case the Lua closure/function gets invoked after this object has been destroyed.
				lua_rawgeti(fLuaStatePointer, LUA_REGISTRYINDEX, fLuaRegistryReferenceId);
				lua_pushlightuserdata(fLuaStatePointer, NULL);
				lua_setupvalue(fLuaStatePointer, -2, 1);    // <- Will only pop user data if upvalue still exists.

				// Remove this object's closure from the Lua registry.
				luaL_unref(fLuaStatePointer, LUA_REGISTRYINDEX, fLuaRegistryReferenceId);

				// Restore the stack, popping off the objects pushed in above.
				lua_settop(fLuaStatePointer, luaStackCount);
			}

			// Clear Lua related member variables.
			fLuaStatePointer = NULL;
			fLuaRegistryReferenceId = LUA_NOREF;
		}

		/**
		  Pushes this callback's Lua function to the top of the stack for the given Lua state.

		  @param luaStatePointer
		  Pointer to the Lua state to push the function to. Must be the same Lua state that this
		  callback has been registered to or a coroutine of the same Lua state.

		  @return
		  Returns true if this callback has successfully pushed its function to Lua.
		
		  Returns false if given a null argument or if this callback's function is no longer registered into Lua.
		 */
		bool PushTo(lua_State* luaStatePointer)
		{
			// Validate.
			if (!luaStatePointer)
			{
				return false;
			}
			if (LUA_NOREF == fLuaRegistryReferenceId)
			{
				return false;
			}

			// Push this callback's closure to Lua.
			lua_rawgeti(luaStatePointer, LUA_REGISTRYINDEX, fLuaRegistryReferenceId);
			return true;
		}

		/**
		  Calls the Lua Runtime:addEventListener() function, passing in the given event name
		  and this callback's Lua function as the listener.
		
		  Note that you should call the RemoveFromRuntimeEventListners() method before releasing or destroying this
		  callback to remove its Lua listener reference. The Lua listener will no-op if you don't, which is harmless,
		  but its the caller's responsibility to handle this.

		  @param eventName The name of the event, such as "enterFrame".

		  @return
		  Returns true if the Lua Runtime function was successfully called.
		
		  Returns false if given a null "eventName" argument, if this callback's Lua function is no longer registered,
		  or if failed to access the Lua Runtime object.
		 */
		bool AddToRuntimeEventListeners(const char* eventName)
		{
			// Validate.
			if (!fLuaStatePointer || !eventName || (LUA_NOREF == fLuaRegistryReferenceId))
			{
				return false;
			}

			// Call the Lua Runtime:addEventListener() method with the given event name.
			// Passes this class' OnInvoked() closure as the Lua listener.
			CoronaLuaPushRuntime(fLuaStatePointer);
			if (!lua_istable(fLuaStatePointer, -1))
			{
				lua_pop(fLuaStatePointer, 1);
				return false;
			}
			lua_getfield(fLuaStatePointer, -1, "addEventListener");
			if (!lua_isfunction(fLuaStatePointer, -1))
			{
				lua_pop(fLuaStatePointer, 2);
				return false;
			}
			lua_insert(fLuaStatePointer, -2);
			lua_pushstring(fLuaStatePointer, eventName);
			lua_rawgeti(fLuaStatePointer, LUA_REGISTRYINDEX, fLuaRegistryReferenceId);
			CoronaLuaDoCall(fLuaStatePointer, 3, 0);
			return true;
		}

		/**
		  Calls the Lua Runtime:removeEventListener() function, passing in the given event name
		  and this callback's Lua function as the listener.

		  @param eventName The name of the event, such as "enterFrame".

		  @return
		  Returns true if the Lua Runtime function was successfully called.
		
		  Returns false if given a null "eventName" argument, if this callback's Lua function is no longer registered,
		  or if failed to access the Lua Runtime object.
		 */
		bool RemoveFromRuntimeEventListeners(const char* eventName)
		{
			// Validate.
			if (!fLuaStatePointer || !eventName || (LUA_NOREF == fLuaRegistryReferenceId))
			{
				return false;
			}

			// Call the Lua Runtime:removeEventListener() method with the given event name.
			// Passes this class' OnInvoked() closure as the Lua listener.
			CoronaLuaPushRuntime(fLuaStatePointer);
			if (!lua_istable(fLuaStatePointer, -1))
			{
				lua_pop(fLuaStatePointer, 1);
				return false;
			}
			lua_getfield(fLuaStatePointer, -1, "removeEventListener");
			if (!lua_isfunction(fLuaStatePointer, -1))
			{
				lua_pop(fLuaStatePointer, 2);
				return false;
			}
			lua_insert(fLuaStatePointer, -2);
			lua_pushstring(fLuaStatePointer, eventName);
			lua_rawgeti(fLuaStatePointer, LUA_REGISTRYINDEX, fLuaRegistryReferenceId);
			CoronaLuaDoCall(fLuaStatePointer, 3, 0);
			return true;
		}

	private:
		/** Copy constructor not supported. */
		LuaMethodCallback(const LuaMethodCallback<TClass>&) {}

		/** Copy operation not supported. */
		void operator=(const LuaMethodCallback<TClass>&) {}


		/**
		  Called from Lua when a LuaMethodCallback instance's closure has been invoked.
		  @param luaStatePointer Pointer to the Lua state that invoked this callback's closure.
		  @return
		  Returns the number of "return values" pushed to Lua by this callback.
		
		  Returns zero if this callback has not returned any values to Lua.
		 */
		static int OnInvoked(lua_State* luaStatePointer)
		{
			// Validate.
			if (!luaStatePointer)
			{
				return 0;
			}

			// Fetch the Lua closure/function's associated LuaMethodCallback instance from its upvalue.
			// Will be null if the callback's Unregister() method was called.
			LuaMethodCallback<TClass>* callbackPointer =
					static_cast<LuaMethodCallback<TClass>*>(lua_touserdata(luaStatePointer, lua_upvalueindex(1)));
			if (!callbackPointer)
			{
				return 0;
			}

			// Invoke the object method this callback wraps.
			if (callbackPointer->fObjectPointer && callbackPointer->fMethodPointer)
			{
				return (*callbackPointer->fObjectPointer.*callbackPointer->fMethodPointer)(luaStatePointer);
			}
			return 0;
		}


		/** Pointer to the C++ class instance that member variable "fMethodPointer" belongs to. */
		TClass *fObjectPointer;

		/** Pointer to the C++ class method belonging to "fObjectPointer" to be invoked. */
		MethodType fMethodPointer;

		/** Pointer to the Lua state that this callback has registered its closure to. */
		lua_State* fLuaStatePointer;

		/**
		  The unique ID assigned to the callback's closure in the Lua registry.
		  Set to LUA_NOREF if the callback's closure has been released or was never assigned to Lua.
		 */
		int fLuaRegistryReferenceId;
};

}	// namespace Interop
