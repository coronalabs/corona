//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Interop\EventArgs.h"
#include "WebBrowserNavigatedEventArgs.h"
#include "WinString.h"


namespace Interop { namespace UI {

/// <summary>Event arguments providing a WebBrowser control's URL loading error information.</summary>
class WebBrowserNavigationFailedEventArgs : public WebBrowserNavigatedEventArgs
{
	public:
		/// <summary>Creates a new event arguments object with the given information.</summary>
		/// <param name="url">A UTF-16 encoded URL.</param>
		/// <param name="errorCode">Native error code provided by the web browser.</param>
		/// <param name="errorMessage">Error message provided by the web browser.</param>
		WebBrowserNavigationFailedEventArgs(const wchar_t* url, int errorCode, const wchar_t* errorMessage);

		/// <summary>Destroys this object.</summary>
		virtual ~WebBrowserNavigationFailedEventArgs();

		/// <summary>Gets the native error code provided by the web browser control.</summary>
		/// <returns>Returns the native error code provided by the web browser control.</returns>
		int GetErrorCode() const;

		/// <summary>Gets a UTF-8 encoded error message provided by the web browser control.</summary>
		/// <returns>Returns a UTF-8 encoded error message provided by the web browser control.</returns>
		const char* GetErrorMessageAsUtf8() const;

		/// <summary>Gets a UTF-16 encoded error message provided by the web browser control.</summary>
		/// <returns>Returns a UTF-16 encoded error message provided by the web browser control.</returns>
		const wchar_t* GetErrorMessageAsUtf16() const;

	private:
		int fErrorCode;
		WinString fErrorMessage;
};

} }	// namespace Interop::UI
