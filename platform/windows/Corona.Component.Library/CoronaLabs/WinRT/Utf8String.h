// ----------------------------------------------------------------------------
// 
// Utf8String.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif

#include <collection.h>
#include "NativeStringServices.h"


namespace CoronaLabs { namespace WinRT {

/// <summary>Represents a UTF-8 encoded string.</summary>
/// <remarks>
///  <para>
///   This class provides an easy means of converting a UTF-16 string to a UTF-8 string and vice-versa.
///   This class also supports converting to/from a standard .NET String or a C++/CX String object,
///   which are UTF-16 string objects.
///  </para>
///  <para>
///   This UTF-8 string uses a "single byte" array used to store multibyte unicode characters. That is,
///   a single character can range between 1 to 6 bytes depending on the character. Native C++/CX code
///   can access this internal array as a "const char*" via the
///   <see cref="NativeStringServices::GetStringPointerFrom(Utf8String)"/> method.
///  </para>
///  <para>Instances of this class are immutable.</para>
/// </remarks>
/// <seealso cref="NativeStringServices"/>
public ref class Utf8String sealed
{
	public:
		/// <summary>Creates an empty string.</summary>
		Utf8String();

		/// <summary>Creates a new UTF-8 encoded string using the given UTF-16 encoded string.</summary>
		/// <param name="utf16String">Reference to a UTF-16 encoded string. Can be null or empty.</param>
		Utf8String(Platform::String^ utf16String);


		/// <summary>Gets the number of characters in this string, excluding the null termination character.</summary>
		/// <remarks>
		///  Note that you cannot assume that 1 byte represents 1 character in a UTF-8 encoded string.
		///  A UTF-8 encoded character can range between 1 to 6 bytes. An ASCII character will always be 1 byte,
		///  but a unicode character will range between 2 and 6 bytes.
		/// </remarks>
		/// <value>
		///  <para>The number of characters stored in this string, excluding the null termination character.</para>
		///  <para>Returns zero if this string is empty.</para>
		/// </value>
		property int CharacterCount { int get(); }

		/// <summary>
		///  Gets the number of bytes used to store this UTF-8 encoded string, including the null termination character.
		/// </summary>
		/// <remarks>
		///  Note that you cannot assume that 1 byte represents 1 character in a UTF-8 encoded string.
		///  A UTF-8 encoded character can range between 1 to 6 bytes. An ASCII character will always be 1 byte,
		///  but a unicode character will range between 2 and 6 bytes.
		/// </remarks>
		/// <value>
		///  <para>The number of bytes used to store this UTF-8 encoded string, including the null termination character.</para>
		///  <para>
		///   Returns 1 if this is an empty string. The 1 byte is used by the null termination character.
		///   Note that this property will never return zero.
		///  </para>
		/// </value>
		property int ByteCount { int get(); }

		/// <summary>Determines if this string does not contain any characters.</summary>
		/// <remarks>This check excludes the null termination character.</remarks>
		/// <value>
		///  <para>Returns true if this string contains no character.</para>
		///  <para>Returns false if this string contains at least one character.</para>
		/// </value>
		property bool IsEmpty { bool get(); }
		
		/// <summary>Determines if this string contains any characters.</summary>
		/// <remarks>This check excludes the null termination character.</remarks>
		/// <value>
		///  <para>Returns true if this string contains at least one character.</para>
		///  <para>Returns false if this string contains no character.</para>
		/// </value>
		property bool IsNotEmpty { bool get(); }
		
		/// <summary>Creates and returns a copy of this string converted to lowercase.</summary>
		/// <returns>Returns a new string object having all lowercased characters.</returns>
		Utf8String^ ToLower();

		/// <summary>Creates and returns a copy of this string converted to uppercase.</summary>
		/// <returns>Returns a new string object having all uppercased characters.</returns>
		Utf8String^ ToUpper();

		/// <summary>Converts this UTF-8 encoded string to a UTF-16 encoded standard string object.</summary>
		/// <returns>Returns this string converted to UTF-16 form.</returns>
		Platform::String^ ToString();

		/// <summary>Determines if the given string matches this string.</summary>
		/// <param name="value">The string to be compared with.</param>
		/// <returns>
		///  <para>Returns true if the given string matches this string.</para>
		///  <para>Returns false if they do no match or if the given string is null.</para>
		/// </returns>
		bool Equals(Utf8String^ value);

		/// <summary>Determines if this object matches the given object.</summary>
		/// <param name="value">The object to compare with.</param>
		/// <returns>
		///  <para>Returns true if the given object's value matches this object's value.</para>
		///  <para>Returns false if they do not equal, if given null, or if the given object is of a different type.</para>
		/// </returns>
		[Windows::Foundation::Metadata::DefaultOverload]
		bool Equals(Platform::Object^ value);

		/// <summary>Represents an empty "" UTF-8 encoded string.</summary>
		/// <remarks>
		///  This property provides a pre-allocated immutable empty string object. You can use this object to
		///  initialize Utf8String objects, which is more efficient than allocating new empty string objects.
		///  This is the equivalent to the .NET String.Empty object.
		/// </remarks>
		/// <value>An empty "" UTF-8 encoded string. This object has zero characters and one byte for the null character.</value>
		static property Utf8String^ Empty { Utf8String^ get(); }

		/// <summary>Converts the given UTF-16 string to a UTF-8 encoded string object.</summary>
		/// <param name="utf16String">The UTF-16 string to be converted. Can be null or empty.</param>
		/// <returns>
		///  <para>Returns an object containing the UTF-8 equivlanet of the given UTF-16 string.</para>
		///  <para>Returns an empty string if the given string is null or empty.</para>
		/// </returns>
		static Utf8String^ From(Platform::String^ utf16String);

	internal:
		/// <summary>Creates a new string object with the given UTF-8 encoded string.</summary>
		/// <param name="utf8String">
		///  Pointer to a UTF-8 encoded string to be copied to this string object. Can be null or empty.
		/// </param>
		Utf8String(const char* utf8String);

		/// <summary>Creates a new UTF-8 encoded string using the given UTF-16 encoded string.</summary>
		/// <param name="utf16String">Pointer to a UTF-16 encoded string. Can be null or empty.</param>
		Utf8String(const wchar_t* utf16String);

		/// <summary>Gets a native C/C++ string pointer to this object's internal single byte character array.</summary>
		/// <remarks>
		///  This is an internal property that can only be accessed publicly via the
		///  <see cref="NativeStringServices::GetStringPointerFrom(Utf8String)"/> method, which is only available
		///  to C++/CX written libraries.
		/// </remarks>
		/// <value>The native C/C++ pointer to this object's UTF-8 encoded string array.</value>
		property const char* Data { const char* get(); }

		/// <summary>Converts the given UTF-16 string to a UTF-8 encoded string object.</summary>
		/// <param name="utf16String">The UTF-16 string to be converted. Can be null or empty.</param>
		/// <returns>
		///  <para>Returns an object containing the UTF-8 equivlanet of the given UTF-16 string.</para>
		///  <para>Returns an empty string if the given string is empty.</para>
		///  <para>Returns null if the given string was null.</para>
		/// </returns>
		static Utf8String^ From(const wchar_t* utf16String);

		/// <summary>Less-than comparer used to find/sort Utf8String types in a Plaform::Map object.</summary>
		class LessThanComparer : public std::less<Utf8String^>
		{
			public:
				/// <summary>Creates a new less-than comparer.</summary>
				LessThanComparer();

				/// <summary>Determines if x is less than y.</summary>
				/// <param name="x">Value to be compared with y.</param>
				/// <param name="y">Value to be compared with x.</param>
				/// <returns>Returns true if x is less than y. Returns false if x is greater than or equal to y.</returns>
				bool operator()(const Utf8String^ x, const Utf8String^ y) const;
		};

	private:
		/// <summary>
		///  Gets a pre-allocated array containing 1 null character.
		///  This array is intended to be shared by all empty string instances of this class.
		/// </summary>
		/// <value>Character array containing only 1 null character.</value>
		static property Platform::Array<unsigned char>^ EmptyStringBuffer { Platform::Array<unsigned char>^ get(); }

		/// <summary>Creates a byte array containing the UTF-8 equivalent of the given UTF-16 string.</summary>
		/// <returns>
		///  <para>Returns a byte array containing the UTF-8 equivalent of the given UTF-16 string.</para>
		///  <para>Returns a reference to the Utf8String::Empty object's "fStringBuffer" if given a null or empty argument.</para>
		///  <para>This function will never return null.</para>
		/// </returns>
		static Platform::Array<unsigned char>^ CreateUtf8ArrayFrom(const wchar_t* utf16String);


		/// <summary>The collection storing the UTF-8 encoded characters.</summary>
		Platform::Array<unsigned char>^ fStringBuffer;

		/// <summary>
		///  The number of characters (not bytes) stored in "fStringBuffer", excluding the NULL termination character.
		/// </summary>
		/// <remarks>Note that a UTF-8 encoded character can range from 1 to 6 bytes.</remarks>
		int fCharacterCount;
};

} }	// namespace CoronaLabs::WinRT
