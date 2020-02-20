//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_LuaResource_H__
#define _Rtt_LuaResource_H__

// ----------------------------------------------------------------------------

#include "Rtt_Lua.h"

#include "Core/Rtt_ResourceHandle.h"

namespace Rtt
{

class MEvent;

// ----------------------------------------------------------------------------

// TODO: Move this
// LuaResource is a way to encapsulate a Lua resource to platform-specific code.
// For example, imagine a Corona developer writes an arbitrary Lua function
// to be used as a listener/callback. We can use LuaResource to store a reference
// to that function so we can later invoke it when the appropriate event occurs.
class LuaResource
{
	public:
		// index is the location on the Lua stack of the element you want to hold onto
		LuaResource( const ResourceHandle< lua_State >& handle, int index );
		~LuaResource();

	public:
		// Push encapsulated function or table listener on the Lua stack
		// If resource is a table t, then interpret as a table listener
		// meaning if t.key, where key = e.Name(), is a function, push it
		// on the stack, then push t (the implicit self arg). After all that,
		// push the event on top of stack. Returns number of arguments that are 
		// on the stack *after* the function. Add 1 to result, for actual number  
		// of objects pushed on the stack.
		int PushListenerAndEvent( const MEvent& e ) const;

		// Dispatches event. No arguments left on stack
		// Return standard Lua status code for invoking Lua methods (0 is success)
		int DispatchEvent( const MEvent& e ) const;

		// Disptaches event. Leaves 'nresults' on stack. Caller must pop.
		// Return standard Lua status code for invoking Lua methods (0 is success)
		int DispatchEvent( const MEvent& e, int nresults ) const;

		void SetRef( int ref );
		
	protected:
		// Push encapsulated resource on the Lua stack
		int Push( lua_State *L ) const;
		int PushListener( lua_State *L, const char *key ) const;

	public:
		// Returns L if fHandle is still valid; otherwise NULL
		lua_State* L() const;

	private:
		ResourceHandle< lua_State > fHandle;
		int fRef;
};

/*
// TODO: Move this
class LuaObject
{
	Rtt_CLASS_NO_DYNAMIC_ALLOCATION

	public:
		// index is the location on the Lua stack of the element you want to hold onto
		LuaObject( lua_State *L, int index );

	public:
		int GetType() const;
		bool IsBoolean() const;
		bool IsCFunction() const;
		bool IsFunction() const;
		bool IsLightuserdata() const;
		bool IsNil() const;
		bool IsNumber() const;
		bool IsString() const;
		bool IsTable() const;
		bool IsUserdata() const;

		bool ToBoolean() const;
		lua_CFunction ToCFunction() const;
		lua_Integer ToInteger() const;
		double ToNumber() const;
		const void* ToPointer() const;
		const char* ToString() const;
		void* ToUserdata() const;

		LuaObject GetField( const char *key ) const;

	private:
		LuaResource fResource;
};
*/

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_LuaResource_H__
