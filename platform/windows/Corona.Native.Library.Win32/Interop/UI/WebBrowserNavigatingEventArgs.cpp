//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WebBrowserNavigatingEventArgs.h"


namespace Interop { namespace UI {

#pragma region Constructors/Destructors
WebBrowserNavigatingEventArgs::WebBrowserNavigatingEventArgs(const wchar_t* url)
:	fUrl(url ? url : L""),
	fWasCanceled(false)
{
}

WebBrowserNavigatingEventArgs::~WebBrowserNavigatingEventArgs()
{
}

#pragma endregion


#pragma region Public Methods
const char* WebBrowserNavigatingEventArgs::GetUrlAsUtf8() const
{
	return fUrl.GetUTF8();
}

const wchar_t* WebBrowserNavigatingEventArgs::GetUrlAsUtf16() const
{
	return fUrl.GetUTF16();
}

bool WebBrowserNavigatingEventArgs::WasCanceled() const
{
	return fWasCanceled;
}

void WebBrowserNavigatingEventArgs::SetCanceled()
{
	fWasCanceled = true;
}

#pragma endregion

} }	// namespace Interop::UI
