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
