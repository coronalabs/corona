//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
