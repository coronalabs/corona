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

#include <collection.h>

#pragma region Forward Declarations
namespace CoronaLabs { namespace WinRT {
	ref class ImmutableByteBuffer;
} }
#pragma endregion


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Networking {

/// <summary>Provides the HTTP response data that was received.</summary>
public ref class HttpResponseEventArgs sealed
{
	public:
		/// <summary>Creates a new event arguments object providing HTTP response data.</summary>
		/// <param name="statusCode">
		///  The HTTP status code that was received such as 200 for "OK", 404 for "Not Found", and etc.
		/// </param>
		/// <param name="headers">
		///  <para>Table of HTTP header name/value pairs received in the response.</para>
		///  <para>Can be empty or null.</para>
		/// </param>
		/// <param name="body">
		///  <para>The HTTP response's body/content in byte form.</para>
		///  <para>Can be empty or null.</para>
		/// </param>
		HttpResponseEventArgs(
				int statusCode,
				Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^>^ headers,
				CoronaLabs::WinRT::ImmutableByteBuffer^ body);

		/// <summary>Gets the HTTP response's status code such as 200 for "OK", 404 for "Not Found, and etc.</summary>
		/// <value>The HTTP status code received.</value>
		property int StatusCode { int get(); }

		/// <summary>Gets a read-only collection of HTTP header name/value pairs received in the response.</summary>
		/// <value>Read-only collection of HTTP header name/value pairs.</value>
		property Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^>^ Headers
		{
			Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^>^ get();
		}

		/// <summary>Gets the HTTP response's content body in byte form.</summary>
		/// <value>Immutable collection of bytes making of the HTTP response's body. Can be empty.</value>
		property CoronaLabs::WinRT::ImmutableByteBuffer^ Body { CoronaLabs::WinRT::ImmutableByteBuffer^ get(); }

	private:
		/// <summary>The HTTP status code received such as 200 for "OK" or 404 for "Not Found".</summary>
		int fStatusCode;

		/// <summary>Dictionary of HTTP header/value pairs in the response.</summary>
		Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^>^ fHeaders;

		/// <summary>The HTTP response's body in byte form.</summary>
		CoronaLabs::WinRT::ImmutableByteBuffer^ fBodyBytes;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Networking
