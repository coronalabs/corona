// ----------------------------------------------------------------------------
// 
// HttpRequestSettings.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "HttpRequestSettings.h"
#include "CoronaLabs\WinRT\ImmutableByteBuffer.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Networking {

#pragma region Consructors/Destructors
HttpRequestSettings::HttpRequestSettings()
{
	// Set the default custom timeout to 30 seconds.
	Windows::Foundation::TimeSpan defaultCustomTimeout;
	defaultCustomTimeout.Duration = 300000000LL;

	// Initialize all settings.
	this->Uri = nullptr;
	this->MethodName = L"GET";
	this->AutoRedirectEnabled = true;
	this->DefaultTimeoutEnabled = true;
	this->CustomTimeout = defaultCustomTimeout;
	this->UploadFileEnabled = false;
	this->UploadFilePath = nullptr;
	this->BytesToUpload = CoronaLabs::WinRT::ImmutableByteBuffer::Empty;
	this->DownloadFilePath = nullptr;
	fHeaderTable = ref new Platform::Collections::Map<Platform::String^, Platform::String^>();
}

#pragma endregion


#pragma region Public Methods/Properties
void HttpRequestSettings::CopyFrom(HttpRequestSettings^ settings)
{
	// Validate.
	if (nullptr == settings)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Do not continue if given a reference to this object. Nothing different to copy.
	if (Platform::Object::ReferenceEquals(this, settings))
	{
		return;
	}

	// Copy the given settings to this object.
	this->Uri = settings->Uri;
	this->MethodName = settings->MethodName;
	this->AutoRedirectEnabled = settings->AutoRedirectEnabled;
	this->DefaultTimeoutEnabled = settings->DefaultTimeoutEnabled;
	this->CustomTimeout = settings->CustomTimeout;
	this->UploadFileEnabled = settings->UploadFileEnabled;
	this->UploadFilePath = settings->UploadFilePath;
	this->BytesToUpload = settings->BytesToUpload;
	this->DownloadFilePath = settings->DownloadFilePath;

	// Copy the header table's entries.
	fHeaderTable->Clear();
	for (auto&& headerPair : settings->fHeaderTable)
	{
		fHeaderTable->Insert(headerPair->Key, headerPair->Value);
	}
}

Windows::Foundation::Collections::IMap<Platform::String^, Platform::String^>^ HttpRequestSettings::Headers::get()
{
	return fHeaderTable;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Networking
