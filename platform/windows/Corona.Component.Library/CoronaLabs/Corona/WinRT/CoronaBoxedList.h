// ----------------------------------------------------------------------------
// 
// CoronaBoxedList.h
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
#include <collection.h>


namespace CoronaLabs { namespace Corona { namespace WinRT {

/// <summary>Stores a collection of boxed data that can be transferred to/from Lua as an array.</summary>
public ref class CoronaBoxedList sealed
:	public ICoronaBoxedData,
	public Windows::Foundation::Collections::IIterable<ICoronaBoxedData^>
{
	public:
		/// <summary>Creates an empty list.</summary>
		CoronaBoxedList();

		/// <summary>Adds the given item to the end of the collection.</summary>
		/// <param name="item">
		///  <para>The item to be added to the collection.</para>
		///  <para>Cannot be null because Lua arrays cannot contain nil elements.</para>
		/// </param>
		void Add(ICoronaBoxedData^ item);

		/// <summary>Removes all items in the collection.</summary>
		void Clear();

		/// <summary>Gets the number of items in the collection.</summary>
		/// <value>
		///  <para>Returns the number of items in the collection.</para>
		///  <para>Returns zero if the collection is empty.</para>
		/// </value>
		property int Count { int get(); }

		/// <summary>Pushes this object's data to the top of the Lua stack.</summary>
		/// <param name="luaStateMemoryAddress">Memory address to a lua_State object, convertible to a C/C++ pointer.</param>
		/// <returns>
		///  <para>Returns true if the push was successful.</para>
		///  <para>Returns false if it failed or if given a memory address of zero (aka: null pointer).</para>
		/// </returns>
		virtual bool PushToLua(int64 luaStateMemoryAddress);

		/// <summary>Gets an iterator used to iterate through all of the elements in the collection in a foreach loop.</summary>
		/// <returns>Returns an iterator to be used by a foreach loop.</returns>
		virtual Windows::Foundation::Collections::IIterator<ICoronaBoxedData^>^ First();

	private:
		/// <summary>Stores a collection of boxed data.</summary>
		Platform::Collections::Vector<ICoronaBoxedData^>^ fCollection;
};

} } }	// namespace CoronaLabs::Corona::WinRT
