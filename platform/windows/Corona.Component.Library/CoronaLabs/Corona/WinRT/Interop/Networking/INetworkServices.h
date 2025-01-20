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


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Networking {

interface class IHttpRequestOperation;
ref class NetworkConnectionStatus;

/// <summary>
///  Provides networking features such as the ability to transmit/receive HTTP messages,
///  fetch the device's network status, etc.
/// </summary>
public interface class INetworkServices
{
	/// <summary>Fetches the device's current network connection status.</summary>
	/// <returns>Returns the device's current network connection status.</returns>
	NetworkConnectionStatus^ FetchConnectionStatus();

	/// <summary>Creates an object used to send an HTTP request and receive a response.</summary>
	/// <returns>Returns an object used to communicate with an HTTP server.</returns>
	IHttpRequestOperation^ CreateHttpRequest();
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Networking
