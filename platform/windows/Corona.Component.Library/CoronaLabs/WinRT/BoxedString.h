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
	#error This header file cannot be included by an external library.
#endif


namespace CoronaLabs { namespace WinRT {

/// <summary>
///  <para>Nullable type used to store a string object.</para>
///  <para>
///   This is needed because the C++/CX <see cref="Platform::String"/> class is not a nullable type,
///   where setting it to null makes it an empty string instead.
///  </para>
///  <para>Instances of this type are immutable.</para>
/// </summary>
public ref class BoxedString sealed
{
	public:
		/// <summary>Creates a new object boxing the given string.</summary>
		/// <param name="value">
		///  <para>The string to be boxed.</para>
		///  <para>
		///   Cannot be set to null in .NET or else an exception will be thrown.
		///   However, it can be set to null in C++/CX, which is really an empty string.
		///  </para>
		/// </param>
		BoxedString(Platform::String^ value);

		/// <summary>Gets the boxed string.</summary>
		/// <value>The boxed string.</value>
		property Platform::String^ Value { Platform::String^ get(); }

		/// <summary>Gets the boxed string.</summary>
		/// <returns>Returns the boxed string.</returns>
		Platform::String^ ToString();

	private:
		/// <summary>The boxed string.</summary>
		Platform::String^ fValue;
};

} }	// namespace CoronaLabs::WinRT
