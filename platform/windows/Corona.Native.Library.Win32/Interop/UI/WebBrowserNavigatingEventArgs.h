//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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
