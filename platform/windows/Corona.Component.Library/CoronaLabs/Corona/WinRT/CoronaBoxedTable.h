// ----------------------------------------------------------------------------
// 
// CoronaBoxedTable.h
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
#include "ICoronaBoxedComparableData.h"
#include <collection.h>


namespace CoronaLabs { namespace Corona { namespace WinRT {

/// <summary>Stores a dictionary of key/value pairs that can be transferred to/from Lua as a table.</summary>
public ref class CoronaBoxedTable sealed
:	public ICoronaBoxedData,
	public Windows::Foundation::Collections::IIterable<
				Windows::Foundation::Collections::IKeyValuePair<ICoronaBoxedComparableData^, ICoronaBoxedData^>^>
{
	public:
		/// <summary>Creates an empty table.</summary>
		CoronaBoxedTable();

		/// <summary>
		///  <para>Adds the given key/value pair to the table.</para>
		///  <para>If the given key already exists in the table, then its value will be replaced.</para>
		/// </summary>
		/// <param name="key">
		///  <para>Unique key to store the value under in the table.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		/// <param name="value">
		///  <para>The value to store in the table. to the property in the collection.</para>
		///  <para>
		///   Setting this to null will remove its entry from the table.
		///   This matches Lua's behavior where Lua tables do not support nil values.
		///  </para>
		/// </param>
		void Set(ICoronaBoxedComparableData^ key, ICoronaBoxedData^ value);

		/// <summary>Fetches a value from the table by its unique key.</summary>
		/// <param name="key">Unique key to search for.</param>
		/// <returns>
		///  <para>Returns the value belonging to the given key.</para>
		///  <para>Returns null if the key was not found in the table or the given key was null.</para>
		/// </returns>
		ICoronaBoxedData^ GetValueByKey(ICoronaBoxedComparableData^ key);

		/// <summary>Determines if the given key exists in the table.</summary>
		/// <param name="key">Unique key to search for in the table.</param>
		/// <returns>
		///  <para>Returns true if the given key was found in the table.</para>
		///  <para>Returns false if the key was not found or if the given key was null.</para>
		/// </returns>
		bool ContainsKey(ICoronaBoxedComparableData^ key);

		/// <summary>Removes an entry from the table by its unique key.</summary>
		/// <param name="key">Unique key to search for in the table.</param>
		/// <returns>
		///  <para>Returns true if the key's entry was successfully removed from the table.</para>
		///  <para>Returns false if the key was not found in the table or if the given key was null.</para>
		/// </returns>
		bool RemoveByKey(ICoronaBoxedComparableData^ key);

		/// <summary>Removes all entries from the table. This makes the table count zero.</summary>
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

		/// <summary>Gets an iterator used to iterate through all of the key/value pairs in the table in a foreach loop.</summary>
		/// <returns>Returns an iterator to be used by a foreach loop.</returns>
		virtual Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<ICoronaBoxedComparableData^, ICoronaBoxedData^>^>^ First();

	private:
		/// <summary>Less-than comparer used to find/sort ICoronaBoxedComparableData types in a Plaform::Map object.</summary>
		class KeyLessThanComparer : public std::less<ICoronaBoxedComparableData^>
		{
			public:
				/// <summary>Creates a new less-than comparer.</summary>
				KeyLessThanComparer();

				/// <summary>Determines if x is less than y.</summary>
				/// <param name="x">Value to be compared with y.</param>
				/// <param name="y">Value to be compared with x.</param>
				/// <returns>Returns true if x is less than y. Returns false if x is greater than or equal to y.</returns>
				bool operator()(const ICoronaBoxedComparableData^ x, const ICoronaBoxedComparableData^ y) const;
		};

		/// <summary>Stores a dictionary of data.</summary>
		Platform::Collections::Map<ICoronaBoxedComparableData^, ICoronaBoxedData^, KeyLessThanComparer>^ fMap;
};

} } }	// namespace CoronaLabs::Corona::WinRT
