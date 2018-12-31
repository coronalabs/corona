// ----------------------------------------------------------------------------
// 
// ImmutableByteBuffer.h
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

#include <collection.h>


namespace CoronaLabs { namespace WinRT {

/// <summary>Stores a static collection of bytes which cannot be changed.</summary>
/// <remarks>
///  <para>You can only create instances of this class via its static From() methods.</para>
///  <para>
///   You can also access a pre-allocated "empty" version of this buffer via its static <see cref="Empty"/> property.
///  </para>
/// </remarks>
public ref class ImmutableByteBuffer sealed
:	public Windows::Foundation::Collections::IIterable<byte>
{
	private:
		/// <summary>Creates a new immutable byte buffer which wraps the given byte collection.</summary>
		/// <remarks>Constructor made private to force external code to use this class' static From() functions.</remarks>
		/// <param name="bytes">
		///  <para>Read-only byte collection that the new byte buffer will wrap and store a reference to.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		ImmutableByteBuffer(Windows::Foundation::Collections::IVectorView<byte>^ bytes);

	public:
		/// <summary>Fetches a byte from the buffer by its zero based index.</summary>
		/// <param name="index">
		///  <para>Zero based index to the byte in the buffer.</para>
		///  <para>Will throw an exception if the index is out of bounds. (ie: less than zero or greater than Count.)</para>
		/// </param>
		/// <returns>Returns the indexed byte.</returns>
		byte GetByIndex(int index);

		/// <summary>Determins if this buffer contains zero bytes.</summary>
		/// <value>
		///  <para>True if this byte buffer does not contain any bytes.</para>
		///  <para>False if this buffer contains at least 1 byte.</para>
		/// </value>
		property bool IsEmpty { bool get(); }

		/// <summary>Determines if this buffer contains at least 1 byte.</summary>
		/// <value>
		///  <para>True if this buffer contains at least 1 byte.</para>
		///  <para>False if this buffer does not contain any bytes.</para>
		/// </value>
		property bool IsNotEmpty { bool get(); }

		/// <summary>Gets the number of bytes stored in this buffer.</summary>
		/// <value>
		///  <para>The number of bytes stored in this buffer.</para>
		///  <para>Zero if this byte buffer is empty.</para>
		/// </value>
		property int Count { int get(); }

		/// <summary>Gets an iterator used to iterate through all of the bytes in the buffer in a foreach loop.</summary>
		/// <returns>Returns an iterator to be used by a foreach loop.</returns>
		virtual Windows::Foundation::Collections::IIterator<byte>^ First();

		/// <summary>
		///  <para>Gets a pre-allocated ImmutableByteBuffer instance which contains zero bytes.</para>
		///  <para>You can use this object to optimize member variable initialization of this type.</para>
		/// </summary>
		static property ImmutableByteBuffer^ Empty { ImmutableByteBuffer^ get(); }

		/// <summary>Creates a new immutable byte buffer containing a copy of the given byte collection.</summary>
		/// <param name="bytes">Byte collection to be copied to the new immutable byte buffer.</param>
		/// <returns>
		///  <para>Returns a new immutable byte buffer containing a copy of the given byte collection.</para>
		///  <para>Returns null if the given byte collection is null.</para>
		///  <para>Returns the <see cref="Empty"/> immutable byte buffer instance if the given collection is empty.</para>
		/// </returns>
		[Windows::Foundation::Metadata::DefaultOverload]
		static ImmutableByteBuffer^ From(Windows::Foundation::Collections::IIterable<byte>^ bytes);

		/// <summary>Creates a new immutable byte buffer containing a copy of the given byte collection.</summary>
		/// <param name="bytes">Byte collection to be copied to the new immutable byte buffer.</param>
		/// <returns>
		///  <para>Returns a new immutable byte buffer containing a copy of the given byte collection.</para>
		///  <para>Returns null if the given byte collection is null.</para>
		///  <para>Returns the <see cref="Empty"/> immutable byte buffer instance if the given collection is empty.</para>
		/// </returns>
		static ImmutableByteBuffer^ From(Windows::Foundation::Collections::IVector<byte>^ bytes);

		/// <summary>Creates a new immutable byte buffer containing a copy of the given byte array.</summary>
		/// <param name="bytes">Byte array to be copied to the new immutable byte buffer.</param>
		/// <returns>
		///  <para>Returns a new immutable byte buffer containing a copy of the given byte array.</para>
		///  <para>Returns null if the given byte array is null.</para>
		///  <para>Returns the <see cref="Empty"/> immutable byte buffer instance if the given array is empty.</para>
		/// </returns>
		static ImmutableByteBuffer^ From(const Platform::Array<byte>^ bytes);

	internal:
		/// <summary>
		///  Creates a new immutable byte buffer containing a copy of the given string's characters,
		///  excluding the null termination character.
		/// </summary>
		/// <param name="stringPointer">Pointer to the string to copy characters from. Can be null.</param>
		/// <returns>
		///  <para>Returns a new immutable byte buffer containing a copy of the given string's characters.</para>
		///  <para>Returns null if the given string is null.</para>
		///  <para>Returns the <see cref="Empty"/> immutable byte buffer instance if the given string is empty.</para>
		/// </returns>
		static ImmutableByteBuffer^ From(const char *stringPointer);

		/// <summary>
		///  <para>Creates a new immutable byte buffer containing a copy of the specified string's characters.</para>
		///  <para>Will copy null characters if they're within the "count" parameter's range.</para>
		/// </summary>
		/// <param name="stringPointer">Pointer to the string to copy characters from. Can be null.</param>
		/// <param name="count">
		///  <para>Number of characters to copy from the given string, including null characters.</para>
		///  <para>Must be greater or equal to zero or else an exception will be thrown.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a new immutable byte buffer containing a copy of the given string's characters.</para>
		///  <para>Returns null if the given string is null.</para>
		///  <para>Returns the <see cref="Empty"/> immutable byte buffer instance if the "count" parameter is zero.</para>
		/// </returns>
		static ImmutableByteBuffer^ From(const char *stringPointer, int count);

	private:
		/// <summary>Read-only container wrapping this class' byte collection.</summary>
		Windows::Foundation::Collections::IVectorView<byte>^ fBytes;
};

} }	// namespace CoronaLabs::WinRT
