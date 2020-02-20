//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Interop\HandledEventArgs.h"
#include "MessageSettings.h"
#include <Windows.h>


namespace Interop { namespace UI {

/// <summary>
///  <para>Event arguments providing a Windows message that was received.</para>
///  <para>Provides a SetHandled() and SetReturnValue() methods to be used by the event handler to "handle" the message.</para>
/// </summary>
class HandleMessageEventArgs : public HandledEventArgs
{
	public:
		/// <summary>Creates a new event arguments object with the given information.</summary>
		/// <param name="settings">The Windows message information that was received.</param>
		HandleMessageEventArgs(const MessageSettings &settings);

		/// <summary>Destroys this object.</summary>
		virtual ~HandleMessageEventArgs();

		/// <summary>Gets the handle to the window that the message was dispatched to.</summary>
		/// <returns>Returns a handle to the window that the message was dispatched to.</returns>
		HWND GetWindowHandle() const;

		/// <summary>Unique integer ID assigned to the message such as WM_CLOSE, WM_SIZE, etc.</summary>
		/// <returns>Returns the message's unique integer ID.</returns>
		UINT GetMessageId() const;

		/// <summary>Gets the message's WPARAM information. What this is used for depends on the message.</summary>
		/// <returns>Returns the message's WPARAM information.</returns>
		WPARAM GetWParam() const;

		/// <summary>Gets the message's LPARAM information. What this is used for depends on the message.</summary>
		/// <returns>Returns the message's LPARAM information.</returns>
		LPARAM GetLParam() const;

		/// <summary>
		///  <para>Gets the value to be returned by the WndProc callback if this message has been handled.</para>
		///  <para>Note: This value will be ignored unless the SetHandled() method has been called.</para>
		/// </summary>
		/// <returns>Returns the value to be returned by the WndProc callback if the message has been handled.</returns>
		LRESULT GetReturnResult() const;

		/// <summary>
		///  <para>Sets the value to be returned by the WndProc callback if the message should be handled.</para>
		///  <para>This value will be ignored unless the event handler also calls the SetHandled() method.</para>
		/// </summary>
		/// <param name="value">The value to be returned by the WndProc callback.</param>
		void SetReturnResult(LRESULT value);

	private:
		MessageSettings fMessageSettings;
		LRESULT fReturnResult;
};

} }	// namespace Interop::UI
