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

/// <summary>
///  <para>Event arguments providing the URL that a WebBrowser control is about to load.</para>
///  <para>Provides a SetCanceled() method, which when called, tells the WebBrowser to not load the event's URL.</para>
/// </summary>
class WebBrowserNavigatingEventArgs : public EventArgs
{
	public:
		/// <summary>Creates a new event arguments object with the given information.</summary>
		/// <param name="url">A UTF-16 encoded URL.</param>
		WebBrowserNavigatingEventArgs(const wchar_t* url);

		/// <summary>Destroys this object.</summary>
		virtual ~WebBrowserNavigatingEventArgs();

		/// <summary>Gets the URL as a UTF-8 encoded string.</summary>
		/// <returns>Returns the URL as a UTF-8 encoded string.</returns>
		const char* GetUrlAsUtf8() const;

		/// <summary>Gets the URL as a UTF-16 encoded string.</summary>
		/// <returns>Returns the URL as a UTF-16 encoded string.</returns>
		const wchar_t* GetUrlAsUtf16() const;

		/// <summary>
		///  <para>Determines if this object's SetCanceled() method was called.</para>
		///  <para>If true, then this informs the WebBrowser to not load the event's URL.</para>
		/// </summary>
		/// <returns>Returns true if the SetCanceled() method was called. Returns false if not.</returns>
		bool WasCanceled() const;

		/// <summary>
		///  <para>Flags the event as canceled, which causes the WasCanceled() method to return true.</para>
		///  <para>This informs the WebBrowser control that raised this event to cancel loading the event's URL.</para>
		/// </summary>
		void SetCanceled();

	private:
		WinString fUrl;
		bool fWasCanceled;
};

} }	// namespace Interop::UI
