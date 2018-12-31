// ----------------------------------------------------------------------------
// 
// CoronaBoxedData.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif


#include "ICoronaBoxedData.h"
extern "C"
{
	struct lua_State;
}


namespace CoronaLabs { namespace Corona { namespace WinRT {

/// <summary>
///  <para>Provides an easy means of converting Lua data to a Corona boxed data object.</para>
///  <para>You cannot create instances of this class. You can only use this class' static methods.</para>
/// </summary>
public ref class CoronaBoxedData sealed
{
	private:
		/// <summary>Constructor made private to prevent instances from being made.</summary>
		CoronaBoxedData();

	public:
		/// <summary>Creates a boxed data object containing a copy of the value(s) indexed in Lua.</summary>
		/// <param name="luaStateMemoryAddress">Memory address to a lua_State object, convertible to a C/C++ pointer.</param>
		/// <param name="luaStackIndex">Index to a Lua object in the Lua stack such as a boolean, number, string, or table.</param>
		/// <returns>
		///  <para>Returns a boxed data object containing a copy of the indexed Lua object's data.</para>
		///  <para>Returns null if unable to copy the indexed data in Lua or if given invalid arguments.</para>
		/// </returns>
		static ICoronaBoxedData^ FromLua(int64 luaStateMemoryAddress, int luaStackIndex);

	internal:
		/// <summary>Creates a boxed data object containing a copy of the value(s) indexed in Lua.</summary>
		/// <param name="luaStatePointer">Pointer to the Lua state to copy the indexed value(s) from.</param>
		/// <param name="luaStackIndex">Index to a Lua object in the Lua stack such as a boolean, number, string, or table.</param>
		/// <returns>
		///  <para>Returns a boxed data object containing a copy of the indexed Lua object's data.</para>
		///  <para>Returns null if unable to copy the indexed data in Lua or if given invalid arguments.</para>
		/// </returns>
		static ICoronaBoxedData^ FromLua(lua_State *luaStatePointer, int luaStackIndex);
};

} } }	// namespace CoronaLabs::Corona::WinRT
