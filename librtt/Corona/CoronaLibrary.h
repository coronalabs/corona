//-----------------------------------------------------------------------------
//
// Corona Labs
//
// easing.lua
//
// Code is MIT licensed; see https://www.coronalabs.com/links/code/license
//
//-----------------------------------------------------------------------------


#ifndef _CoronaLibrary_H__
#define _CoronaLibrary_H__

#include "CoronaLua.h"
#include "CoronaMacros.h"
#include <stdarg.h>

// ----------------------------------------------------------------------------

CORONA_API const char *CoronaLibraryClassName(void) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

// Initializes the base service systems for the Lua state 'L'.
CORONA_API int CoronaLibraryInitialize( lua_State *L ) CORONA_PUBLIC_SUFFIX;

// Constructor for creating a Lua library from native code.
//
// Creates a new CoronaLibrary object and leaves it at the top of the stack.
// This object is a Lua table that is an instance of CoronaLibrary. It initializes
// the table with the closures in 'libFuncs' and 'context'.
CORONA_API int CoronaLibraryNew(
	lua_State *L,
	const char *libName, const char *publisherId, int version, int revision,
	const luaL_Reg libFuncs[], void *context ) CORONA_PUBLIC_SUFFIX;

// Constructor for creating a CoronaLibrary object from a Lua file, passing in
// the Lua chunk corresponding to the Lua file as 'factory'.
//
// Creates a new CoronaLibrary object by invoking 'factory', which is a Lua function
// that returns a CoronaLibrary object. This object is left at the top of the stack.
// If 'factory' succeeds, then the functions in 'libFuncs' are added.
//
// The factory should return a CoronaLibrary object where the 'name' and
// 'publisherId' properties are initialized.
CORONA_API int CoronaLibraryNewWithFactory(
	lua_State *L, lua_CFunction factory, const luaL_Reg libFuncs[], void *context ) CORONA_PUBLIC_SUFFIX;

// For the library at 'index', sets the function/closure at top of the stack to be
// called for '__index' metamethod accesses. If your function/closure returns nil,
// then the library's original '__index' metamethod is called.
//
// In this way, you can intercept the library's '__index' metamethod accesses so
// that it routes to a function defined in your native code before calling the
// original metamethod.
CORONA_API void CoronaLibrarySetExtension( lua_State *L, int index ) CORONA_PUBLIC_SUFFIX;

// Defines a new provider base class called: "CoronaProvider" .. "." .. libName
// and registers it as a module so it's accessible via 'require'.
CORONA_API void CoronaLibraryProviderDefine( lua_State *L, const char *libName ) CORONA_PUBLIC_SUFFIX;

// Some libraries rely on multiple service providers for functionality. Such libraries
// will define a particular provider type unique to that library. Use this function to
// create a Lua provider object for the library called 'libraryName'. This will instantiate
// a provider instance by looking for a class called: "CoronaProvider" .. "." .. libName
CORONA_API int CoronaLibraryProviderNew( lua_State *L, const char *libName, const char *providerName, const char *publisherId ) CORONA_PUBLIC_SUFFIX;

// Invokes a function in the library, equivalent to the following Lua code:
//		function CoronaLibraryPushProperty( libName, name )
//			local library = require libName
//			return library[name]
//		end
CORONA_API void CoronaLibraryPushProperty( lua_State *L, const char *libName, const char *name ) CORONA_PUBLIC_SUFFIX;

/*
Signatures are a string. The parameters (inputs) and the results (outputs) are separated by '>'
For example: "t>s" means that a table is passed in 
{
	boolean = b,
	string = s,
	integer = d,		-- 'd' for %d (int) in printf 
	number = f,			-- 'f' for %f (double) in printf
	self = o,			-- 'o' for object
	table = t,			-- the value should be an index on the Lua stack
	userdata = p,		-- 'p' for pointer (the value should be an index on the Lua stack)
	lightuserdata = l,
}
*/

// Invokes a function in the library, equivalent to the following Lua code:
//		local library = require 'libName'
//		library[name]( ... )
CORONA_API int CoronaLibraryCallFunction( lua_State *L, const char *libName, const char *name, const char *sig, ... ) CORONA_PUBLIC_SUFFIX;

// Invokes a function in the library, equivalent to the following Lua code:
//		local library = require 'libName'
//		library[name]( ... )
CORONA_API int CoronaLibraryCallFunctionV( lua_State *L, const char *libName, const char *name, const char *sig, va_list arguments ) CORONA_PUBLIC_SUFFIX;

// Invokes an object method in the library, equivalent to the following Lua code:
//		local library = require 'libName'
//		library[name]:( ... )
CORONA_API int CoronaLibraryCallMethod( lua_State *L, const char *libName, const char *name, const char *sig, ... ) CORONA_PUBLIC_SUFFIX;

// Invokes an object method in the library, equivalent to the following Lua code:
//		local library = require 'libName'
//		library[name]:( ... )
CORONA_API int CoronaLibraryCallMethodV( lua_State *L, const char *libName, const char *name, const char *sig, va_list arguments ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

#endif // _CoronaLibrary_H__
