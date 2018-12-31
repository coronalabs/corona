// ----------------------------------------------------------------------------
// 
// ICoronaBoxedData.h
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


namespace CoronaLabs { namespace Corona { namespace WinRT {

/// <summary>Stores data that can be transferred to/from Lua.</summary>
public interface class ICoronaBoxedData
{
	/// <summary>Pushes this object's data to the top of the Lua stack.</summary>
	/// <param name="luaStateMemoryAddress">Memory address to a lua_State object, convertible to a C/C++ pointer.</param>
	/// <returns>
	///  <para>Returns true if the push was successful.</para>
	///  <para>Returns false if it failed or if given a memory address of zero (aka: null pointer).</para>
	/// </returns>
	bool PushToLua(int64 luaStateMemoryAddress);
};

} } }	// namespace CoronaLabs::Corona::WinRT
