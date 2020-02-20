//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WebBrowserNavigationFailedEventArgs.h"


namespace Interop { namespace UI {

#pragma region Constructors/Destructors
WebBrowserNavigationFailedEventArgs::WebBrowserNavigationFailedEventArgs(
	const wchar_t* url, int errorCode, const wchar_t* errorMessage)
:	WebBrowserNavigatedEventArgs(url),
	fErrorCode(errorCode),
	fErrorMessage(errorMessage ? errorMessage : L"")
{
}

WebBrowserNavigationFailedEventArgs::~WebBrowserNavigationFailedEventArgs()
{
}

#pragma endregion


#pragma region Public Methods
int WebBrowserNavigationFailedEventArgs::GetErrorCode() const
{
	return fErrorCode;
}

const char* WebBrowserNavigationFailedEventArgs::GetErrorMessageAsUtf8() const
{
	return fErrorMessage.GetUTF8();
}

const wchar_t* WebBrowserNavigationFailedEventArgs::GetErrorMessageAsUtf16() const
{
	return fErrorMessage.GetUTF16();
}

#pragma endregion

} }	// namespace Interop::UI
