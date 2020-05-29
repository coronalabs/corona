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

/// <summary>Stores the configuration for an HTTP request operation.</summary>
public ref class HttpRequestSettings sealed
{
	public:
		/// <summary>Creates a new configuration for an HTTP request operation.</summary>
		HttpRequestSettings();

		/// <summary>Copies the given HTTP request settings to this setting object.</summary>
		/// <param name="settings">
		///  <para>The settings to be copied.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		void CopyFrom(HttpRequestSettings^ settings);

		/// <summary>Gets or sets the URI/URL to send the HTTP request to.</summary>
		/// <value>
		///  <para>The URI/URL to send the HTTP request to.</para>
		///  <para>Null if this property has not been assigned yet.</para>
		/// </value>
		property Windows::Foundation::Uri^ Uri;

		/// <summary>Gets or sets the HTTP method the request should use such as "GET", "POST", etc.</summary>
		/// <value>
		///  <para>The HTTP request method to use such as "GET", "POST", etc.</para>
		///  <para>Set to empty string if this property has not been assigned yet.</para>
		///  <para>Cannot be set to null in .NET or else an exception will be thrown. (But can be set to null in C++/CX.)</para>
		/// </value>
		property Platform::String^ MethodName;

		/// <summary>
		///  Determines if the HTTP request should automatically handle a redirect response by re-sending
		///  the request to the new host.
		/// </summary>
		/// <value>
		///  <para>Set true to automatically handle a redirect response by re-sending the HTTP request to the new host.</para>
		///  <para>Set false to not automatically redirect and provide the received HTTP redirect response.</para>
		/// </value>
		property bool AutoRedirectEnabled;

		/// <summary>
		///  Determines if the HTTP request should use the system's default network timeout or the
		///  <see cref="CustomTimeout"/> assigned to this settings object.
		/// </summary>
		/// <value>
		///  <para>Set true if the HTTP request will use the system's default network timeout.</para>
		///  <para>Set false if this settings object's <see cref="CustomTimeout"/> should be used.</para>
		/// </value>
		property bool DefaultTimeoutEnabled;

		/// <summary>
		///  <para>
		///   Gets or sets how long the system should wait for a response from the network end-point until
		///   before giving up with an error.
		///  </para>
		///  <para>This timeout is only used if property <see cref="DefaultTimeoutEnabled"/> is set to false.</para>
		/// </summary>
		/// <value>
		///  <para>The amount of time to wait for a response before giving up with an error.</para>
		///  <para>Will use the system's minimum allowable timeout duration if set less than or equal to zero.</para>
		/// </value>
		property Windows::Foundation::TimeSpan CustomTimeout;

		/// <summary>Gets a collection of HTTP header name/value pairs to add to the HTTP request message.</summary>
		/// <value>Collection of HTTP header name/value pairs.</value>
		property Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^>^ Headers
		{
			Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^>^ get();
		}

		/// <summary>Determines if the HTTP request should upload the contents of a file.</summary>
		/// <value>
		///  <para>
		///   Set true if the HTTP request should upload the contents of the file reference by the
		///   <see cref="UploadFilePath"/> property.
		///  </para>
		///  <para>
		///   Set false if the HTTP request should upload the bytes provided by the
		///   <see cref="BytesToUpload"/> property.
		///  </para>
		/// </value>
		property bool UploadFileEnabled;

		/// <summary>
		///  <para>Gets or set the path to the file whose contents should be sent by the HTTP request.</para>
		///  <para>This property is only applicable if <see cref="UploadFileEnabled"/> is set to true</para>
		/// </summary>
		/// <value>
		///  <para>The path to the file whose contents will be uploaded by the HTTP request operation.</para>
		///  <para>Set to empty string if this property has not been assigned yet.</para>
		///  <para>Cannot be set to null in .NET or else an exception will be thrown. (But can be set to null in C++/CX.)</para>
		/// </value>
		property Platform::String^ UploadFilePath;

		/// <summary>
		///  <para>Gets or sets the bytes to be uploaded by the HTTP request operation.</para>
		///  <para>This property is only applicable if <see cref="UploadFileEnabled"/> is set to false</para>
		/// </summary>
		/// <value>
		///  <para>Collection of bytes to be uploaded by the HTTP request operation.</para>
		///  <para>Can be null or empty if the HTTP request you are sending should have zero bytes in its body.</para>
		/// </value>
		property CoronaLabs::WinRT::ImmutableByteBuffer^ BytesToUpload;

		/// <summary>
		///  <para>Gets or sets a path to a file that the HTTP response body should be written to.</para>
		///  <para>This is optional. If not set, then the HTTP response will be provided via an HttpResponseEventArgs.</para>
		/// </summary>
		/// <value>
		///  <para>The path to a file that the HTTP response's body should be written to.</para>
		///  <para>Set to empty string to receive the response's bytes via an HttpResponseEventArgs object.</para>
		///  <para>Cannot be set to null in .NET or else an exception will be thrown. (But can be set to null in C++/CX.)</para>
		/// </value>
		property Platform::String^ DownloadFilePath;

	private:
		/// <summary>Dictionary of HTTP header/value pairs.</summary>
		Platform::Collections::Map<Platform::String^, Platform::String^>^ fHeaderTable;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Networking
