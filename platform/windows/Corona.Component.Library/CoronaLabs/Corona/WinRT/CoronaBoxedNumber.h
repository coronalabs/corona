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
///  <para>Stores a floating point value that can be transferred to/from Lua.</para>
///  <para>Instances of this class are immutable.</para>
/// </summary>
public ref class CoronaBoxedNumber sealed : public ICoronaBoxedComparableData
{
	public:
		/// <summary>Creates an object which boxes the given floating point value.</summary>
		/// <param name="value">The value to be boxed.</param>
		CoronaBoxedNumber(double value);

		/// <summary>Gets the boxed floating point value.</summary>
		/// <value>The boxed floating point value.</value>
		property double Value { double get(); }

		/// <summary>Gets the boxed floating point value as a localized string.</summary>
		/// <returns>Returns the boxed floating point value as a localized string.</returns>
		Platform::String^ ToString();

		/// <summary>Determines if the given value matches this boxed object's stored value.</summary>
		/// <param name="value">The value to compare with.</param>
		/// <returns>Returns true if the given value matches this boxed object's value. Returns false if not.</returns>
		bool Equals(double value);

		/// <summary>Determines if this object matches the given object.</summary>
		/// <param name="value">The object to compare with.</param>
		/// <returns>
		///  <para>Returns true if the given object's value matches this object's value.</para>
		///  <para>Returns false if they do not equal, if given null, or if the given object is of a different type.</para>
		/// </returns>
		[Windows::Foundation::Metadata::DefaultOverload]
		virtual bool Equals(Platform::Object^ value);
		
		/// <summary>Determines if this object is less than, greater than, or equal to the given value.</summary>
		/// <param name="value">The value to be compared with.</param>
		/// <returns>
		///  <para>Returns a positive value if this object is greater than the given object.</para>
		///  <para>Returns a negative value if this object is less than the given object.</para>
		///  <para>Returns zero if the both objects are equal.</para>
		/// </returns>
		int CompareTo(double value);

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

	private:
		/// <summary>The boxed floating point value.</summary>
		double fValue;
};

} } }	// namespace CoronaLabs::Corona::WinRT
