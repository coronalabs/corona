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


#include "ICoronaBoxedData.h"
#include <collection.h>


#pragma region Forward Declarations
namespace CoronaLabs { namespace WinRT {
	interface class IOperationResult;
} }
extern "C" struct lua_State;

#pragma endregion


namespace CoronaLabs { namespace Corona { namespace WinRT {

/// <summary>
///  Stores a collection of name/value property pairs for one Corona event that is intended to be dispatched to/from Lua.
/// </summary>
public ref class CoronaLuaEventProperties sealed
:	public Windows::Foundation::Collections::IIterable<
				Windows::Foundation::Collections::IKeyValuePair<Platform::String^, ICoronaBoxedData^>^>
{
	private:
		/// <summary>Creates an empty event properties table.</summary>
		/// <remarks>Constructor made private to force callers to create a properties object via its static methods.</remarks>
		CoronaLuaEventProperties();

	public:
		/// <summary>
		///  <para>Adds the given property name/value pair to the collection.</para>
		///  <para>If the given property name already exists in the collection, then its value will be replaced.</para>
		/// </summary>
		/// <param name="name">
		///  <para>Unique name of the property to add or update.</para>
		///  <para>Cannot be null/empty or else an exception will be thrown.</para>
		/// </param>
		/// <param name="value">
		///  <para>The value to be assigned to the property in the collection.</para>
		///  <para>
		///   Setting this to null will remove the property from the collection.
		///   This is because Lua tables do not support nil values.
		///  </para>
		/// </param>
		[Windows::Foundation::Metadata::DefaultOverload]
		void Set(Platform::String^ name, ICoronaBoxedData^ value);
		
		/// <summary>
		///  <para>Adds the given property name/value pair to the collection.</para>
		///  <para>If the given property name already exists in the collection, then its value will be replaced.</para>
		/// </summary>
		/// <param name="name">
		///  <para>Unique name of the property to add or update.</para>
		///  <para>Cannot be null/empty or else an exception will be thrown.</para>
		/// </param>
		/// <param name="value">The value to be assigned to the property.</param>
		void Set(Platform::String^ name, bool value);

		/// <summary>
		///  <para>Adds the given property name/value pair to the collection.</para>
		///  <para>If the given property name already exists in the collection, then its value will be replaced.</para>
		/// </summary>
		/// <param name="name">
		///  <para>Unique name of the property to add or update.</para>
		///  <para>Cannot be null/empty or else an exception will be thrown.</para>
		/// </param>
		/// <param name="value">The value to be assigned to the property.</param>
		void Set(Platform::String^ name, int value);

		/// <summary>
		///  <para>Adds the given property name/value pair to the collection.</para>
		///  <para>If the given property name already exists in the collection, then its value will be replaced.</para>
		/// </summary>
		/// <param name="name">
		///  <para>Unique name of the property to add or update.</para>
		///  <para>Cannot be null/empty or else an exception will be thrown.</para>
		/// </param>
		/// <param name="value">The value to be assigned to the property.</param>
		void Set(Platform::String^ name, double value);

		/// <summary>
		///  <para>Adds the given property name/value pair to the collection.</para>
		///  <para>If the given property name already exists in the collection, then its value will be replaced.</para>
		/// </summary>
		/// <param name="name">
		///  <para>Unique name of the property to add or update.</para>
		///  <para>Cannot be null/empty or else an exception will be thrown.</para>
		/// </param>
		/// <param name="value">The string to be assigned to the property.</param>
		void Set(Platform::String^ name, Platform::String^ value);

		/// <summary>Fetches a property value from the collection by its property name.</summary>
		/// <param name="name">The unique name of the property to fetch from.</param>
		/// <returns>
		///  <para>Returns the value of the specified property as a boxed object.</para>
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

		/// <summary>Removes a property from the collection by its unique name.</summary>
		/// <param name="name">The unique name of the property to remove from the collection.</param>
		/// <returns>
		///  <para>Returns true if the specified property was removed from the collection.</para>
		///  <para>Returns false if the given property name was not found or if given a null/empty string.</para>
		/// </returns>
		bool Remove(Platform::String^ name);

		/// <summary>Removes all properties from the collection, making the collection empty.</summary>
		void Clear();

		/// <summary>Fetches the number of properties stored in the collection.</summary>
		/// <value>
		///  <para>Number of properties that are currently contained in the collection.</para>
		///  <para>Returns zero if the collection is currently empty.</para>
		/// </value>
		property int Count { int get(); }

		/// <summary>Gets an iterator that can be used to traverse all properties in the collection via a foreach loop.</summary>
		/// <returns>Returns an iterator that can be used to traverse all properties in the collection.</returns>
		virtual Windows::Foundation::Collections::IIterator<Windows::Foundation::Collections::IKeyValuePair<Platform::String^, ICoronaBoxedData^>^>^ First();

		/// <summary>Creates a new Corona event properties table using the given event name.</summary>
		/// <param name="eventName">
		///  <para>The Corona event "name" to be added to the event properties table.</para>
		///  <para>
		///   Expected to be one of the Corona event names defined here:
		///   <a href="http://docs.coronalabs.com/api/event/index.html">http://docs.coronalabs.com/api/event/index.html</a>
		///  </para>
		///  <para>
		///   Alternatively, you can use your own custom event name that you can use to communicate between
		///   native code and Lua, such as how it works with Corona plugins.
		///  </para>
		///  <para>This name is required. You cannot dispatch an event to Corona without a name.</para>
		///  <para>Cannot be null/empty or else an exception will be thrown.</para>
		/// </param>
		/// <returns>Returns a new properties collection assigned the given event name.</returns>
		static CoronaLuaEventProperties^ CreateWithName(Platform::String^ eventName);

	internal:
		/// <summary>Pushes this object's property collection to Lua as a table.</summary>
		/// <remarks>This is an internal method that is only accessible to Corona Labs.</remarks>
		/// <param name="luaStatePointer">Pointer to a Lua state to push this object's properties to.</param>
		/// <returns>
		///  <para>Returns a success result if this object successfully pushed its properties into Lua.</para>
		///  <para>Returns a failure result if given a null Lua state pointer.</para>
		/// </returns>
		CoronaLabs::WinRT::IOperationResult^ PushTo(lua_State *luaStatePointer);

		/// <summary>Creates a new Corona event properties object containing a copy of the index Lua event table.</summary>
		/// <param name="luaStatePointer">Pointer to the Lua state to copy the indexed Lua event table from.</param>
		/// <param name="luaEventTableIndex">Index to the event table in the Lua stack.</param>
		/// <returns>
		///  <para>Returns a new properties collection containing a copy of the indexed event table's values in Lua.</para>
		///  <para>Returns null if given invalid argument or if the indexed Lua object does not reference an event table.</para>
		/// </returns>
		static CoronaLuaEventProperties^ From(lua_State *luaStatePointer, int luaEventTableIndex);

	private:
		/// <summary>Dictionary used to store all properties.</summary>
		Platform::Collections::Map<Platform::String^, ICoronaBoxedData^>^ fProperties;
};

} } }	// namespace CoronaLabs::Corona::WinRT
