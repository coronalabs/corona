//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif

#include "CoronaLuaEventProperties.h"
#include <collection.h>

#pragma region Forward Declarations
namespace CoronaLabs { namespace WinRT {
	interface class IOperationResult;
} }
extern "C" struct lua_State;

#pragma endregion


namespace CoronaLabs { namespace Corona { namespace WinRT {

interface class ICoronaBoxedData;

/// <summary>
///  <para>
///   Wraps an existing <see cref="CoronaLuaEventProperties"/> object, providing read-only access to the
///   collection's properties.
///  </para>
///  <para>Note that this read-only collection cannot prevent modifications of mutables property values.</para>
/// </summary>
public ref class ReadOnlyCoronaLuaEventProperties sealed
:	public Windows::Foundation::Collections::IIterable<
				Windows::Foundation::Collections::IKeyValuePair<Platform::String^, ICoronaBoxedData^>^>
{
	public:
		/// <summary>Creates a new read-only wrapper around the given mutable properties collection.</summary>
		/// <param name="properties">
		///  <para>The property collection to be wrapped by the new read-only wrapper.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		ReadOnlyCoronaLuaEventProperties(CoronaLuaEventProperties^ properties);

		/// <summary>Fetches a property value from the collection by its property name.</summary>
		/// <param name="name">The unique name of the property to fetch from.</param>
		/// <returns>
		///  <para>Returns the value of the specified property in boxed object form.</para>
		///  <para>Returns null if the property name was not found.</para>
		/// </returns>
		ICoronaBoxedData^ Get(Platform::String^ name);

		/// <summary>Determines if the given property name exists in the collection.</summary>
		/// <param name="name">The unique name of the property to search for.</param>
		/// <returns>
		///  <para>Returns true if the given property name exists in the collection.</para>
		///  <para>Returns false if not or if given a null/emptry string.</para>
		/// </returns>
		bool Contains(Platform::String^ name);

		/// <summary>Fetches the number of properties stored in the collection.</summary>
		/// <value>
		///  <para>Number of properties that are currently contained in the collection.</para>
		///  <para>Returns zero if the collection is currently empty.</para>
		/// </value>
		property int Count { int get(); }

		/// <summary>Gets an iterator that can be used to traverse all properties in the collection via a foreach loop.</summary>
		/// <returns>Returns an iterator that can be used to traverse all properties in the collection.</returns>
		virtual Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<Platform::String^, ICoronaBoxedData^>^>^ First();

	internal:
		/// <summary>Pushes this object's property collection to Lua as a table.</summary>
		/// <remarks>This is an internal method that is only accessible to Corona Labs.</remarks>
		/// <param name="luaStatePointer">Pointer to a Lua state to push this object's properties to.</param>
		/// <returns>
		///  <para>Returns a success result if this object successfully pushed its properties into Lua.</para>
		///  <para>Returns a failure result if given a null Lua state pointer.</para>
		/// </returns>
		CoronaLabs::WinRT::IOperationResult^ PushTo(lua_State *luaStatePointer);

	private:
		/// <summary>Reference to the property collection wrapped by this read-only object.</summary>
		CoronaLuaEventProperties^ fProperties;
};

} } }	// namespace CoronaLabs::Corona::WinRT
