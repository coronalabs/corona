//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MLuaTableBridge_H__
#define _Rtt_MLuaTableBridge_H__

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

// Bridge to wrap C++ object to have Lua table semantics
// 
// This simplifies bridge code where the Lua code is accessing a property of
// a userdata object and on the C/C++ side, we need to access the corresponding
// "property" (a.k.a. the instance variable) inside a C++ object. For example,
// this interface is designed to wrap C++ objects so that LuaProxy classes
// can access properties of these C++ objects as if they were Lua objects.
class MLuaTableBridge
{
	public:
		// If property exists in receiver, then push the property value onto 
		// Lua stack and return 1 (or the number of values pushed if more than 1;
		// otherwise return 0.
		virtual int ValueForKey( lua_State *L, const char key[] ) const = 0;

		// If property exists and can be set in receiver, then fetch value from 
		// Lua stack, update the property value, and return true; otherwise return false.
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex ) = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MLuaTableBridge_H__
