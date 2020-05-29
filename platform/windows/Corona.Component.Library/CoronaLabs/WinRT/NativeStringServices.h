//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


#ifdef CORONALABS_CORONA_API_EXPORT
	#define CORONALABS_CORONA_API __declspec( dllexport )
#else
	#define CORONALABS_CORONA_API __declspec( dllimport )
#endif


namespace CoronaLabs { namespace WinRT {

// Forward declarations.
ref class Utf8String;


/// <summary>Provides simple conversion functions between UTF-8 and UTF-16 strings.</summary>
/// <remarks>
///  <para>This class is only available to native C++/CX libraries.</para>
///  <para>You cannot create instances of this class. You can only use this class' static functions.</para>
/// </remarks>
class CORONALABS_CORONA_API NativeStringServices
{
	private:
		/// <summary>Constructor made private to prevent instances from being made.</summary>
		/// <remarks>This class' functionality is made available via its static functions.</remarks>
		NativeStringServices();

	public:
		/// <summary>Converts the given UTF-16 encoded string to UTF-8.</summary>
		/// <param name="utf16String">The UTF-16 string to be converted. Can be null or empty.</param>
		/// <returns>
		///  <para>Returns a UTF-8 encoded string object matching the given UTF-16 string.</para>
		///  <para>Returns null if the given string was null.</para>
		/// </returns>
		static Utf8String^ Utf8From(Platform::String^ utf16String);

		/// <summary>Converts the given UTF-16 encoded string to UTF-8.</summary>
		/// <param name="utf16String">The UTF-16 string to be converted. Can be null or empty.</param>
		/// <returns>
		///  <para>Returns a UTF-8 encoded string object matching the given UTF-16 string.</para>
		///  <para>Returns null if the given string was null.</para>
		/// </returns>
		static Utf8String^ Utf8From(const wchar_t* utf16String);

		/// <summary>Converts the given UTF-8 encoded string to UTF-16.</summary>
		/// <remarks>
		///  Note that the given string is expected to be UTF-8 or ASCII encoded. If you provide a string with a
		///  different encoding, then the returned result will likely be wrong.
		/// </remarks>
		/// <param name="utf8String">The UTF-8 encoded string to be converted. Can be null or empty.</param>
		/// <returns>
		///  <para>Returns a UTF-16 encoded string object matching the given UTF-8 string.</para>
		///  <para>Returns null if the given string was null.</para>
		/// </returns>
		static Platform::String^ Utf16FromUtf8(const char* utf8String);

		/// <summary>Converts the given UTF-8 string to UTF-16.</summary>
		/// <param name="utf8String">The UTF-8 encoded string to be converted. Can be null or empty.</param>
		/// <returns>
		///  <para>Returns a UTF-16 encoded string object matching the given UTF-8 string.</para>
		///  <para>Returns null if the given string was null.</para>
		/// </returns>
		static Platform::String^ Utf16From(Utf8String^ utf8String);

		/// <summary>
		///  Fetches a native C/C++ pointer to the Utf8String object's character buffer used to store its string.
		/// </summary>
		/// <param name="utf8String">Reference to the string object to fetch the string pointer from.</param>
		/// <returns>
		///  <para>Returns a C/C++ pointer to the given string object's character array.</para>
		///  <para>Returns NULL if given a nullptr argument or if the given string object does not have a string allocated.</para>
		/// </returns>
		static const char* GetStringPointerFrom(Utf8String^ utf8String);
};

} }	// namespace CoronaLabs::WinRT
