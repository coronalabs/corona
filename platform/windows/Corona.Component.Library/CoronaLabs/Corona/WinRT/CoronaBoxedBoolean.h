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


#include "ICoronaBoxedComparableData.h"


namespace CoronaLabs { namespace Corona { namespace WinRT {

/// <summary>
///  <para>Stores a boolean value that can be transferred to/from Lua.</para>
///  <para>Instances of this class are immutable.</para>
/// </summary>
public ref class CoronaBoxedBoolean sealed : public ICoronaBoxedComparableData
{
	private:
		/// <summary>Creates an object which boxes the given boolean value.</summary>
		/// <remarks>
		///  This constructor was made private to force callers to acquire pre-allocated instances of this class
		///  via its static methods/properties.
		/// </remarks>
		/// <param name="value">The boolean value to be boxed.</param>
		CoronaBoxedBoolean(bool value);

	public:
		/// <summary>Gets the boxed boolean value.</summary>
		/// <value>The boxed boolean value.</value>
		property bool Value { bool get(); }

		/// <summary>Gets the boxed boolean value in string form.</summary>
		/// <returns>Returns "true" or "false".</returns>
		Platform::String^ ToString();

		/// <summary>Determines if the given value matches this boxed object's stored value.</summary>
		/// <param name="value">The value to compare with.</param>
		/// <returns>Returns true if the given value matches this boxed object's value. Returns false if not.</returns>
		bool Equals(bool value);

		/// <summary>Determines if this object matches the given object.</summary>
		/// <param name="value">The object to compare with.</param>
		/// <returns>
		///  <para>Returns true if the given object's value matches this object's value.</para>
		///  <para>Returns false if they do not equal, if given null, or if the given object is of a different type.</para>
		/// </returns>
		[Windows::Foundation::Metadata::DefaultOverload]
		virtual bool Equals(Platform::Object^ value);
		
		/// <summary>Determines if this object is less than, greater than, or equal to the given boolean value.</summary>
		/// <param name="value">The boolean value to be compared with.</param>
		/// <returns>
		///  <para>Returns a positive value if this object is greater than the given object. (True is greater than false.)</para>
		///  <para>Returns a negative value if this object is less than the given object. (False is less than true.)</para>
		///  <para>Returns zero if the both objects are equal.</para>
		/// </returns>
		int CompareTo(bool value);

		/// <summary>Determines if this object is less than, greater than, or equal to the given object.</summary>
		/// <param name="value">The object to be compared with.</param>
		/// <returns>
		///  <para>Returns a positive value if this object is greater than the given object.</para>
		///  <para>Returns a negative value if this object is less than the given object.</para>
		///  <para>Returns zero if the both objects are equal.</para>
		/// </returns>
		[Windows::Foundation::Metadata::DefaultOverload]
		virtual int CompareTo(Platform::Object^ value);

		/// <summary>Gets an integer hash code for this object.</summary>
		/// <returns>Returns this object's hash code.</returns>
		virtual int GetHashCode();

		/// <summary>Pushes this object's data to the top of the Lua stack.</summary>
		/// <param name="luaStateMemoryAddress">Memory address to a lua_State object, convertible to a C/C++ pointer.</param>
		/// <returns>
		///  <para>Returns true if the push was successful.</para>
		///  <para>Returns false if it failed or if given a memory address of zero (aka: null pointer).</para>
		/// </returns>
		virtual bool PushToLua(int64 luaStateMemoryAddress);

		/// <summary>Pre-allocated instance of this class set to "true".</summary>
		static property CoronaBoxedBoolean^ True { CoronaBoxedBoolean^ get(); }

		/// <summary>Pre-allocated instance of this class set to "false".</summary>
		static property CoronaBoxedBoolean^ False { CoronaBoxedBoolean^ get(); }

		/// <summary>Fetches a pre-allocated instance of this class based on the given value.</summary>
		/// <param name="value">The boolean value to be boxed.</param>
		/// <returns>Returns a True or False instance of this class.</returns>
		static CoronaBoxedBoolean^ From(bool value);

	private:
		/// <summary>The boxed boolean value.</summary>
		bool fValue;
};

} } }	// namespace CoronaLabs::Corona::WinRT
