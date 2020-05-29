//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WebBrowserNavigatedEventArgs.h"


namespace Interop { namespace UI {

#pragma region Constructors/Destructors
WebBrowserNavigatedEventArgs::WebBrowserNavigatedEventArgs(const wchar_t* url)
:	fUrl(url ? url : L"")
{
}

WebBrowserNavigatedEventArgs::~WebBrowserNavigatedEventArgs()
{
}

#pragma endregion


#pragma region Public Methods
const char* WebBrowserNavigatedEventArgs::GetUrlAsUtf8() const
{
	return fUrl.GetUTF8();
}

const wchar_t* WebBrowserNavigatedEventArgs::GetUrlAsUtf16() const
{
	return fUrl.GetUTF16();
}

#pragma endregion

} }	// namespace Interop::UI
