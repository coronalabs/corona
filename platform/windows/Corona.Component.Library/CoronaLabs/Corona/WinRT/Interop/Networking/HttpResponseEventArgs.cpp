// ----------------------------------------------------------------------------
// 
// HttpResponseEventArgs.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "HttpResponseEventArgs.h"
#include "CoronaLabs\WinRT\ImmutableByteBuffer.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Networking {

#pragma region Consructors/Destructors
HttpResponseEventArgs::HttpResponseEventArgs(
	int statusCode,
	Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^>^ headers,
	CoronaLabs::WinRT::ImmutableByteBuffer^ body)
{
	// Ensure the given arguments are set to valid values.
	if (nullptr == headers)
	{
		headers = ref new Platform::Collections::MapView<Platform::String^, Platform::String^>();
	}
	if (nullptr == body)
	{
		body = CoronaLabs::WinRT::ImmutableByteBuffer::Empty;
	}

	// Initialize member variables.
	fStatusCode = statusCode;
	fHeaders = headers;
	fBodyBytes = body;
}

#pragma endregion


#pragma region Public Methods/Properties
int HttpResponseEventArgs::StatusCode::get()
{
	return fStatusCode;
}

Windows::Foundation::Collections::IMapView<Platform::String^, Platform::String^>^ HttpResponseEventArgs::Headers::get()
{
	return fHeaders;
}

CoronaLabs::WinRT::ImmutableByteBuffer^ HttpResponseEventArgs::Body::get()
{
	return fBodyBytes;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Networking
