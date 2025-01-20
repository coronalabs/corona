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
#include "CoronaLabs\WinRT\Utf8String.h"
#include <collection.h>


namespace CoronaLabs { namespace Corona { namespace WinRT {

/// <summary>
///  <para>Stores a string that can be transferred to/from Lua.</para>
///  <para>Instances of this class are immutable.</para>
/// </summary>
public ref class CoronaBoxedString sealed : public ICoronaBoxedComparableData
{
	private:
		/// <summary>Creates an object which boxes the given string.</summary>
		/// <remarks>This constructor was made private to force callers to use this class' static From() methods.</remarks>
		/// <param name="utf16String">The string to be boxed.</param>
		CoronaBoxedString(Platform::String^ utf16String);

		/// <summary>Creates an object which boxes the given string.</summary>
		/// <remarks>This constructor was made private to force callers to use this class' static From() methods.</remarks>
		/// <param name="utf8String">The string to be boxed. Cannot be null or else an exception will be thrown.</param>
		CoronaBoxedString(CoronaLabs::WinRT::Utf8String^ utf8String);

	public:
		/// <summary>Gets the boxed string in UTF-8 form.</summary>
		/// <returns>Returns the boxed string in UTF-8 form.</returns>
		CoronaLabs::WinRT::Utf8String^ ToUtf8String();

		/// <summary>Gets the boxed string in UTF-16 form.</summary>
		/// <returns>Returns the boxed string in UTF-16 form.</returns>
		Platform::String^ ToUtf16String();

		/// <summary>Gets the boxed string in UTF-16 form.</summary>
		/// <returns>Returns the boxed string in UTF-16 form.</returns>
		Platform::String^ ToString();

		/// <summary>Determines if this string matches the given UTF-8 string.</summary>
		/// <param name="value">The UTF-8 string to compare with.</param>
		/// <returns>
		///  <para>Returns true if the given strings exactly match.</para>
		///  <para>Returns false if not or if given null.</para>
		/// </returns>
		bool Equals(CoronaLabs::WinRT::Utf8String^ value);

		/// <summary>Determines if this string matches the given UTF-16 string.</summary>
		/// <param name="value">The UTF-16 string to compare with.</param>
		/// <returns>
		///  <para>Returns true if the given strings exactly match.</para>
		///  <para>Returns false if not or if given null.</para>
		/// </returns>
		bool Equals(Platform::String^ value);

		/// <summary>Determines if this object matches the given object.</summary>
		/// <param name="value">The object to compare with.</param>
		/// <returns>
		///  <para>Returns true if the given object's value matches this object's value.</para>
		///  <para>Returns false if they do not equal, if given null, or if the given object is of a different type.</para>
		/// </returns>
		[Windows::Foundation::Metadata::DefaultOverload]
		virtual bool Equals(Platform::Object^ value);
		
		/// <summary>Determines if this object is less than, greater than, or equal to the given object.</summary>
		/// <param name="value">The object to be compared with.</param>
		/// <returns>
		///  <para>Returns a positive value if this object is greater than the given object.</para>
		///  <para>Returns a negative value if this object is less than the given object.</para>
		///  <para>Returns zero if the both objects are equal.</para>
		/// </returns>
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

		/// <summary>Gets a re-usable empty string object.</summary>
		/// <value>Returns a pre-allocated object containing an empty UTF-16 and UTF-8 string.</value>
		static property CoronaBoxedString^ Empty { CoronaBoxedString^ get(); }

		/// <summary>Creates an object which boxes the given string.</summary>
		/// <param name="value">The string to be boxed.</param>
		/// <returns>
		///  <para>Returns an object which boxes the given string.</para>
		///  <para>Returns a <see cref="CoronaBoxedString::Empty"/> object if the given string is empty or null.</para>
		/// </returns>
		[Windows::Foundation::Metadata::DefaultOverload]
		static CoronaBoxedString^ From(Platform::String^ value);

		/// <summary>Creates an object which boxes the given UTF-8 string.</summary>
		/// <param name="value">The UTF-8 string to be boxed.</param>
		/// <returns>
		///  <para>Returns an object which boxes the given string.</para>
		///  <para>Returns a <see cref="CoronaBoxedString::Empty"/> object if the given string is empty or null.</para>
		/// </returns>
		static CoronaBoxedString^ From(CoronaLabs::WinRT::Utf8String^ value);

	private:
		/// <summary>The boxed string in UTF-16 form.</summary>
		Platform::String^ fUtf16String;

		/// <summary>The boxed string in UTF-8 form.</summary>
		CoronaLabs::WinRT::Utf8String^ fUtf8String;
};

} } }	// namespace CoronaLabs::Corona::WinRT
