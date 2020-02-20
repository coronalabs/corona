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
#include "WinString.h"


namespace Interop { namespace UI {

/// <summary>Event arguments providing the URL that a WebBrowser control finished navigating to.</summary>
class WebBrowserNavigatedEventArgs : public EventArgs
{
	public:
		/// <summary>Creates a new event arguments object with the given information.</summary>
		/// <param name="url">A UTF-16 encoded URL.</param>
		WebBrowserNavigatedEventArgs(const wchar_t* url);

		/// <summary>Destroys this object.</summary>
		virtual ~WebBrowserNavigatedEventArgs();

		/// <summary>Gets the URL as a UTF-8 encoded string.</summary>
		/// <returns>Returns the URL as a UTF-8 encoded string.</returns>
		const char* GetUrlAsUtf8() const;

		/// <summary>Gets the URL as a UTF-16 encoded string.</summary>
		/// <returns>Returns the URL as a UTF-16 encoded string.</returns>
		const wchar_t* GetUrlAsUtf16() const;

	private:
		WinString fUrl;
};

} }	// namespace Interop::UI
