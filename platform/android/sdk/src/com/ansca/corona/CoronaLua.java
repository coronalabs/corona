//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

import java.util.Hashtable;

import com.naef.jnlua.LuaState;
import com.naef.jnlua.LuaType;

// ----------------------------------------------------------------------------

/**
 * Provides a set of convenience functions for use with the {@link com.naef.jnlua.LuaState LuaState}.
 * <p>
 * You should call these methods inside a {@link com.ansca.corona.CoronaRuntimeTask CoronaRuntimeTask} to ensure that 
 * modifications to the {@link com.naef.jnlua.LuaState LuaState} happen on the Corona thread.
 */
public class CoronaLua {
	private static int REF_OWNER = LuaState.REGISTRYINDEX;

	/**
	 * Returned when {@link com.naef.jnlua.LuaState#ref} is called on a nil value.
	 */
	public static final int REFNIL = -1;

	/**
	 * Used to mark a references as invalid.
	 * <p>
	 * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2013/1227/">daily build 2013.1227</a></b>.
	 */
	public static final int NOREF = -2;

	/**
	 * Returns an absolute index location on the <a href="http://www.lua.org/manual/5.1/manual.html#3.1">Lua stack</a> 
	 * for the passed <font face="Courier New" color="black">index</font>. This is useful if you have negative stack indices, 
	 * as these are relative to the top of the <a href="http://www.lua.org/manual/5.1/manual.html#3.1">stack</a>.
	 * @param L The {@link com.naef.jnlua.LuaState LuaState} to operate on.
	 * @param index The index on the <a href="http://www.lua.org/manual/5.1/manual.html#3.1">lua stack</a> that we want a 
	 *				normalized value for.
	 * @return An absolute index location on the <a href="http://www.lua.org/manual/5.1/manual.html#3.1">Lua stack</a> 
	 *		   for the passed <font face="Courier New" color="black">index</font>.
	 */
	public static int normalize( LuaState L, int index ) {
		int result = index;

		// Map negative indices to positive ones
		if ( index < 0 )
		{
			result = L.getTop() + index + 1;
		}

		return result;
	}

	/**
	 * Creates a Java-side reference to a Lua object on the <a href="http://www.lua.org/manual/5.1/manual.html#3.1">Lua stack</a>
	 * at <font face="Courier New" color="black">index</font>. You can use this to store and access Lua objects from native Java code. 
	 * Typically you use this when you want to store a reference to a Lua 
	 * <a href="http://docs.coronalabs.com/daily/api/type/Listener.html">listener</a>.
	 * @param L The {@link com.naef.jnlua.LuaState LuaState} to operate on.
	 * @param index The index on the <a href="http://www.lua.org/manual/5.1/manual.html#3.1">lua stack</a> holding the object we want 
	 *				a Java-side reference for.
	 * @return Returns a reference identifier.
	 */
	public static int newRef( LuaState L, int index ) {
		L.pushValue( index );
		return L.ref( REF_OWNER );
	}

	/**
	 * Release the Java-side reference to a Lua value, which would otherwise
	 * prevent the Lua value from being <a href="http://www.lua.org/manual/5.1/manual.html#2.10">garbage collected</a>.
	 * @param L The {@link com.naef.jnlua.LuaState LuaState} to operate on.
	 * @param ref The Java-side reference to release.
	 */
	public static void deleteRef( LuaState L, int ref ) {
		if ( ref > 0 ) {
			L.unref( REF_OWNER, ref );
		}
	}

	/**
	 * Creates a Corona <a href="http://docs.coronalabs.com/daily/api/type/Event.html">event</a> with the 
	 * <font face="Courier New" color="black">name</font> property set to <font face="Courier New" color="black">eventName</font>. 
	 * <p>
	 * <b>
	 * This function <i>pushes</i> the created event table to the top of the 
	 * <a href="http://www.lua.org/manual/5.1/manual.html#3.1">Lua stack</a>!
	 * </b>
	 * @param L The {@link com.naef.jnlua.LuaState LuaState} to operate on.
	 * @param eventName The name to assign to this Corona <a href="http://docs.coronalabs.com/daily/api/type/Event.html">event</a>.
	 */
	public static void newEvent( LuaState L, String eventName ) {
		L.newTable();
		L.pushString( eventName );
		L.setField( -2, CoronaLuaEvent.NAME_KEY );
	}

	/**
	 * Dispatches an <a href="http://docs.coronalabs.com/daily/api/type/Event.html">event</a> by invoking the listener corresponding to
	 * <font face="Courier New" color="black">listenerRef</font> passing the value at the top of the stack as the Corona event table. 
	 * You should use {@link com.ansca.corona.CoronaLua#newEvent(com.naef.jnlua.LuaState, java.lang.String) newEvent()} to create this 
	 * event.
	 * <p>
	 * <b>
	 * This function <i>pops</i> the value from the top of the <a href="http://www.lua.org/manual/5.1/manual.html#3.1">Lua stack</a>!
	 * </b>
	 * @param L The {@link com.naef.jnlua.LuaState LuaState} to operate on.
	 * @param listenerRef A reference identifier returned by 
	 * 					  {@link com.ansca.corona.CoronaLua#newRef(com.naef.jnlua.LuaState, int) CoronaLua.newRef()}.
	 * @param nresults The number of results to be dispatched to the 
	 *				   <a href="http://docs.coronalabs.com/daily/api/type/Event.html">event</a>.
	 */
	public static void dispatchEvent( LuaState L, int listenerRef, int nresults ) throws Exception {
		int eventIndex = L.getTop(); // event is at top of stack

		Exception e = null;

		L.getField( eventIndex, CoronaLuaEvent.NAME_KEY ); // push event.name

		if ( LuaType.STRING == L.type( -1 ) )
		{
			String eventName = L.toString( -1 );

			L.rawGet( REF_OWNER, listenerRef ); // 1: reference

			if ( L.isFunction( -1 ) )
			{
				// Corresponds to Lua code: listener( event )
			
				// 1: reference is the function
				L.pushValue( eventIndex ); // 2: event
				L.call( 1, nresults );
			}
			else if ( L.isTable( -1 ) )
			{
				// Corresponds to Lua code: listener.eventName( listener, event )
			
				// reference is a table, so look for table
				L.getField( -1, eventName ); // listener.eventName
				if ( L.isFunction( -1 ) )
				{
					// swap stack positions for reference and function
					// 1: function (listener.eventName)
					// 2: reference, i.e. table (implicit self arg to function)
					L.insert( -2 );

					L.pushValue( eventIndex ); // 3: event
					L.call( 2, nresults );
				}
				else
				{
					e = new Exception( "[Lua::DispatchEvent()] ERROR: Table listener's property '" + eventName + "' is not a function." );
				}
			}
			else if ( ! L.isNoneOrNil( -1 ) )
			{
				e = new Exception( "[Lua::DispatchEvent()] ERROR: Listener must be a function or a table, not a '" + L.typeName( -1 ) + "'." );
			}
		}
		else
		{
			e = new Exception( "[Lua::DispatchEvent()] ERROR: Attempt to dispatch malformed event. The event must have a 'name' string property." );
		}

		L.pop( 2 ); // pop event.name and event

		if ( null != e ) {
			throw e;
		}
	}

	/**
	 * Returns whether the function at <font face="Courier New" color="black">index</font> is a Corona 
	 * <a href="http://docs.coronalabs.com/daily/api/type/Listener.html">listener</a>.
	 * @param L The {@link com.naef.jnlua.LuaState LuaState} to operate on.
	 * @param index The index on the <a href="http://www.lua.org/manual/5.1/manual.html#3.1">lua stack</a> holding the function in 
	 *				question.
	 * @param eventName The name of the function at <font face="Courier New" color="black">index</font>.
	 */
	public static boolean isListener( LuaState L, int index, String eventName ) {
		boolean result = L.isFunction( index );
		if ( ! result && L.isTable( index ) )
		{
			L.getField( index, eventName );
			result = L.isFunction( -1 );
			L.pop( 1 );
		}
		return result;
	}

	/**
	 * Returns an {@link java.lang.Object Object} for the item at the specified index. Supports Lua 
	 * <a href="http://docs.coronalabs.com/daily/api/type/String.html">Strings</a>, 
	 * <a href="http://docs.coronalabs.com/daily/api/type/Number.html">Numbers</a>, 
	 * <a href="http://docs.coronalabs.com/daily/api/type/Boolean.html">Booleans</a>, and 
	 * <a href="http://docs.coronalabs.com/daily/api/type/Table.html">Tables</a>.
	 * <p>
	 * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2014/2125/">daily build 2014.2125</a></b>.
	 * @param L The {@link com.naef.jnlua.LuaState LuaState} where the object is located.
	 * @param index The index on the <a href="http://www.lua.org/manual/5.1/manual.html#3.1">lua stack</a> where the object is located.
	 * @return An {@link java.lang.Object Object} with the value of the lua object at the designated index on the 
	 *		   <a href="http://www.lua.org/manual/5.1/manual.html#3.1">lua stack</a>.
	 */
	public static Object toValue(LuaState L, int index) {
		if (index < 0) {
			index = L.getTop() + index + 1;
		}

		Object result = null;

		com.naef.jnlua.LuaType luaType = L.type(index);
		switch(luaType) {
			case STRING:
				result = L.toString(index);
				break;
			case NUMBER:
				result = Double.valueOf(L.toNumber(index));
				break;
			case BOOLEAN:
				result = Boolean.valueOf(L.toBoolean(index));
				break;
			case TABLE: {
				/*
				The following is equivlant to:
				system.pathForTable(tableAtIndex)
				*/
				L.getGlobal("system");
				L.getField(-1, "pathForTable");
				L.remove(-2);
				L.pushValue(index);
				L.call(1, 1);

				com.naef.jnlua.LuaType pathForTableResult = L.type(-1);
				switch(pathForTableResult) {
					case STRING:
						result = L.toString(-1);
						break;
					case NIL:
						result = toHashtable(L, index);
						break;
				}
				L.pop(1);
				break;
			}
		}
		return result;
	}

	/**
	 * Creates a {@link java.util.Hashtable Hashtable} from a 
	 * <a href="http://docs.coronalabs.com/daily/api/type/Table.html">lua table</a> at the specified index on the 
	 * <a href="http://www.lua.org/manual/5.1/manual.html#3.1">lua stack</a>.
	 * <p>
	 * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2014/2125/">daily build 2014.2125</a></b>.
	 * @param L The {@link com.naef.jnlua.LuaState LuaState} where the 
	 *			<a href="http://docs.coronalabs.com/daily/api/type/Table.html">table</a> is located.
	 * @param index The index on the <a href="http://www.lua.org/manual/5.1/manual.html#3.1">lua stack</a> where the 
	 *				<a href="http://docs.coronalabs.com/daily/api/type/Table.html">lua table</a> is located.
	 * @return A {@link java.util.Hashtable Hashtable} with the values of the 
	 *		   <a href="http://docs.coronalabs.com/daily/api/type/Table.html">lua table</a>.
	 */
	public static Hashtable<Object, Object> toHashtable(LuaState L, int index) {
		// This is used to convert negative lua stack indexies into positive ones.
		// which is needed because the top can constantly change
		if (index < 0) {
			index = L.getTop() + index + 1;
		}

		Hashtable<Object, Object> result = new Hashtable<Object, Object>();
		
		L.checkType(index, com.naef.jnlua.LuaType.TABLE);
		for (L.pushNil(); L.next(index); L.pop(1)) {
			// Fetch the table entry's string key.
			// An index of -2 accesses the key that was pushed into the Lua stack by luaState.next() up above.
			Object key = toValue(L, -2);
			if (key == null) {
				// A valid key was not found. Skip this table entry.
				continue;
			}
			
			// Fetch the table entry's value in string form.
			// An index of -1 accesses the entry's value that was pushed into the Lua stack by luaState.next() above.
			Object value = toValue(L, -1);
			if (value == null) {
				continue;
			}
			
			result.put(key, value);
		}
		return result;
	}

	/**
	 * Pushes the {@link java.lang.Object Object} onto the top of the 
	 * <a href="http://www.lua.org/manual/5.1/manual.html#3.1">lua stack</a>. Supports {@link java.lang.String String}, 
	 * {@link java.lang.Integer Integer}, {@link java.lang.Double Double}, {@link java.lang.Boolean Boolean}, and 
	 * {@link java.util.Hashtable Hashtable} objects.
	 * <p>
	 * <b>This function <i>pushes</i> the desired object to the top of the 
	 * <a href="http://www.lua.org/manual/5.1/manual.html#3.1">Lua stack</a>!</b>
	 * <p>
	 * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2014/2125/">daily build 2014.2125</a></b>.
	 * @param L The {@link com.naef.jnlua.LuaState LuaState} to operate on.
	 * @param object The {@link java.lang.Object Object} to be pushed onto the top of the 
	 *				 <a href="http://www.lua.org/manual/5.1/manual.html#3.1">lua stack</a>.
	 */
	public static void pushValue(LuaState L, Object object) {
		if (object instanceof String) {
			L.pushString((String)object);
		} else if (object instanceof Integer) {
			L.pushInteger(((Integer)object).intValue());
		} else if (object instanceof Double) {
			L.pushNumber(((Double)object).doubleValue());
		} else if (object instanceof Boolean) {
			L.pushBoolean(((Boolean)object).booleanValue());
		} else if (object instanceof Hashtable) {
			pushHashtable(L, (Hashtable)object);
		} else {
			L.pushNil();
		}
	}

	/**
	 * Pushes a <a href="http://docs.coronalabs.com/daily/api/type/Table.html">lua table</a> with the values in the 
	 * {@link java.util.Hashtable Hashtable} onto the top of the <a href="http://www.lua.org/manual/5.1/manual.html#3.1">lua stack</a>.
	 * <p>
	 * <b>This function <i>pushes</i> a <a href="http://docs.coronalabs.com/daily/api/type/Table.html">lua table</a> to the top of the 
	 * <a href="http://www.lua.org/manual/5.1/manual.html#3.1">Lua stack</a>!</b>
	 * <p>
	 * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2014/2125/">daily build 2014.2125</a></b>.
	 * @param L The {@link com.naef.jnlua.LuaState LuaState} where the <a href="http://docs.coronalabs.com/daily/api/type/Table.html">table</a> should be pushed to.
	 * @param hashtable The {@link java.util.Hashtable Hashtable} which contains the values to be pushed onto the 
	 * <a href="http://www.lua.org/manual/5.1/manual.html#3.1">lua stack</a> as a <a href="http://docs.coronalabs.com/daily/api/type/Table.html">lua table</a>.
	 */
	public static void pushHashtable(LuaState L, Hashtable<Object, Object> hashtable) {
		if (hashtable == null) {
			L.newTable();
			return;
		}

		L.newTable(0, hashtable.size());
		
		int tableIndex = L.getTop();

		// // Copy the Java hashtable's entries to the Lua table.
		for (java.util.Map.Entry<Object, Object> entry : hashtable.entrySet()) {
			pushValue(L, entry.getKey());
			pushValue(L, entry.getValue());
			L.setTable(tableIndex);
		}
	}

	/**
	 * Dispatches the <a href="http://docs.coronalabs.com/daily/api/type/Event.html">event</a> at the top of the 
	 * <a href="http://www.lua.org/manual/5.1/manual.html#3.1">lua stack</a> to the global 
	 * <a href="http://docs.coronalabs.com/daily/api/type/Runtime/index.html">Runtime</a> object.
	 * <p>
	 * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2014/2125/">daily build 2014.2125</a></b>.
	 * @param L The {@link com.naef.jnlua.LuaState LuaState} of the 
	 *			<a href="http://docs.coronalabs.com/daily/api/type/Runtime/index.html">Runtime</a> object.
	 * @param nResults The number of results returned from the Lua side.
	 */
	public static void dispatchRuntimeEvent(LuaState L, int nResults) {
		L.getGlobal("Runtime");
		L.getField(-1, "dispatchEvent");
		L.insert(-3);
		L.insert(-2);
		L.call(2, nResults);
	}

/*
	public static void PushRuntime( LuaState L )
	{
		Rtt::Lua::PushRuntime( L );
	}

	public static void RuntimeDispatchEvent( LuaState L, int index )
	{
		if ( Rtt_VERIFY( lua_istable( L, index ) ) )
		{
			int top = lua_gettop( L );
			int indexNormalized = ( index > 0 ? index : top + index + 1 );

			PushRuntime( L ); Rtt_ASSERT( lua_istable( L, -1 ) );
			lua_getfield( L, -1, "dispatchEvent" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
			lua_insert( L, -2 ); // swap stack positions for "Runtime" and "dispatchEvent"
			int nargs = lua_gettop( L ) - top; Rtt_ASSERT( 2 == nargs );
			lua_pushvalue( L, indexNormalized ); 
			DoCall( L, nargs, 0 );
		}
		else
		{
			Rtt_LogException( "[ERROR] Lua::RuntimeDispatchEvent() expected the 'event' at index (%d) to be a Lua table.\n", index );
		}
	}
	public static void NewGCMetatable( LuaState L, const char name[], lua_CFunction __gc1 )
	{
		Rtt::Lua::NewGCMetatable( L, name, __gc1 );
	}

	public static void NewMetatable( LuaState L, const char name[], const luaL_Reg vtable[] )
	{
		Rtt::Lua::NewMetatable( L, name, vtable );
	}

	public static void InitializeGCMetatable( LuaState L, const char name[], lua_CFunction __gc1 )
	{
		Rtt::Lua::InitializeGCMetatable( L, name, __gc1 );
	}

	public static void InitializeMetatable( LuaState L, const char name[], const luaL_Reg vtable[] )
	{
		Rtt::Lua::InitializeMetatable( L, name, vtable );
	}

	public static void PushUserdata( LuaState L, void *ud, const char metatableName[] )
	{
		Rtt::Lua::PushUserdata( L, ud, metatableName );
	}

	public static void *ToUserdata( LuaState L, int index )
	{
		return Rtt::Lua::ToUserdata( L, index );
	}

	public static void *CheckUserdata( LuaState L, int index, const char metatableName[] )
	{
		return Rtt::Lua::CheckUserdata( L, index, metatableName );
	}

	public static void RegisterModuleLoader( LuaState L, const char *name, lua_CFunction loader, int nupvalues )
	{
		Rtt::Lua::RegisterModuleLoader( L, name, loader, nupvalues );
	}

	public static void RegisterModuleLoaders( LuaState L, const luaL_Reg moduleLoaders[], int nupvalues )
	{
		Rtt::Lua::RegisterModuleLoaders( L, moduleLoaders, nupvalues );
	}

	public static int OpenModule( LuaState L, lua_CFunction loader )
	{
		return Rtt::Lua::OpenModule( L, loader );
	}

	public static void InsertPackageLoader( LuaState L, lua_CFunction loader, int index )
	{
		Rtt::Lua::InsertPackageLoader( L, loader, index );
	}

	public static lua_CFunction GetErrorHandler()
	{
		return Rtt::Lua::GetErrorHandler( NULL );
	}

	public static void SetErrorHandler( lua_CFunction newValue )
	{
		Rtt::Lua::SetErrorHandler( newValue );
	}

	public static int DoCall( LuaState L, int narg, int nresults )
	{
		return Rtt::Lua::DoCall( L, narg, nresults );
	}

	public static int DoBuffer( LuaState L, lua_CFunction loader, lua_CFunction pushargs )
	{
		return Rtt::Lua::DoBuffer( L, loader, pushargs );
	}

	public static int DoFile( LuaState L, const char* file, int narg, int clear )
	{
		return Rtt::Lua::DoFile( L, file, narg, clear );
	}

	public static int PushModule( LuaState L, const char *name )
	{
		return Rtt::Lua::PushModule( L, name );
	}
*/
}


// ----------------------------------------------------------------------------
