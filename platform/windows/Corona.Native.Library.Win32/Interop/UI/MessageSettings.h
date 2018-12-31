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

#include <Windows.h>


namespace Interop { namespace UI {

/// <summary>Stores information for one Windows message.</summary>
struct MessageSettings
{
	/// <summary>Gets the handle to the window that the message was dispatched to.</summary>
	HWND WindowHandle;

	/// <summary>Unique integer ID of the message suchas WM_CLOSE, WM_SIZE, etc.</summary>
	UINT MessageId;

	/// <summary>
	///  <para>Additional information associated with the message.</para>
	///  <para>What it is used for depends on the message.</para>
	/// </summary>
	WPARAM WParam;

	/// <summary>
	///  <para>Additional information associated with the message.</para>
	///  <para>What it is used for depends on the message.</para>
	/// </summary>
	LPARAM LParam;

	/// <summary>Creates a new Windows message.</summary>
	MessageSettings()
	:	WindowHandle(0),
		MessageId(0),
		WParam(0),
		LParam(0)
	{
	}
};

} }	// namespace Interop::UI
