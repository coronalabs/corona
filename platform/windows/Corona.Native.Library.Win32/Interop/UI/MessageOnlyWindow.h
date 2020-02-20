//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Macros.h"
#include "UIComponent.h"
#include <string>
#include <unordered_set>
#include <Windows.h>


namespace Interop { namespace UI {

/// <summary>
///  <para>Creates a Win32 "message-only window" used to send private/custom Windows messages.</para>
///  <para>A message-only window has no UI and only provides a Windows message pump.</para>
///  <para>Messages sent/posted to this window can be received via the "ReceivedMessage" event.</para>
/// </summary>
class MessageOnlyWindow : public UIComponent
{
	Rtt_CLASS_NO_COPIES(MessageOnlyWindow)

	public:
		/// <summary>Creates a new message-only window object.</summary>
		MessageOnlyWindow();

		/// <summary>
		///  <para>Creates a new message-only window object registered with the given class name in Windows.</para>
		///  <para>
		///   This allows the caller to find the created message-only window by its registered class name via
		///   the Win32 FindWindowEx() function for Inter-Process Communications.
		///  </para>
		/// </summary>
		/// <param name="className">
		///  <para>The unique class name this object will pass to the Win32 RegisterClassEx() function with.</para>
		///  <para>Can be null or empty, in which case a default name will be assigned.</para>
		/// </param>
		MessageOnlyWindow(const wchar_t* className);

		/// <summary>Destroys this object.</summary>
		virtual ~MessageOnlyWindow();

		/// <summary>Gets the registered class name the message-only window was created with.</summary>
		/// <returns>Returns the registered class name the message-only window was created with.</returns>
		const wchar_t* GetRegisteredClassName() const;

		/// <summary>
		///  <para>Generates a unique Windows message ID within the WM_USER or WM_APP range.</para>
		///  <para>This message ID can then be used when sending/posting messages to this message-only window.</para>
		///  <para>Note: This message ID will only be unique for this message-only window object.</para>
		/// </summary>
		/// <returns>
		///  Returns a message ID unique to this message-only window that you can use to send/post messages to.
		/// </returns>
		UINT ReserveMessageId();

		/// <summary>
		///  <para>Releases a message ID returned by the ReserveMessageId() method to be.</para>
		///  <para>This makes this message ID available again to future calls to the ReserveMessageId() method.</para>
		/// </summary>
		/// <param name="id">
		///  ID returned by the ReserveMesssageId() method that the caller will no longer be using.
		/// </param>
		void UnreserveMessageId(UINT id);

		/// <summary>
		///  Removes all Windows messages assigned the given unique integer ID posted to this message-only window.
		/// </summary>
		/// <param name="messageId">Unique integer ID of the message to be removed from the queue.</param>
		/// <returns>
		///  <para>Returns true if at least 1 message was removed from this message-only window's queue.</para>
		///  <para>Returns false if the given message ID was not found in the queue.</para>
		/// </returns>
		bool RemoveMessagesById(UINT messageId);

		/// <summary>
		///  <para>Fetches a global message-only window to be shared on the calling thread.</para>
		///  <para>This instance is created upon demand and will persist until application exit.</para>
		///  <para>Intended to be used by features that need a message pump that outlives the main window.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns a pointer to shared/global message-only window instance for the current thread.</para>
		///  <para>Returns null if failed to create the message-only window. Unlikely to happen unless low on memory.</para>
		/// </returns>
		static MessageOnlyWindow* GetSharedInstanceForCurrentThread();

	private:
		/// <summary>
		///  A WndProc C callback that gets invoked when a Windows message has been received from the assigned window.
		/// </summary>
		/// <param name="windowHandle">Handle to the window or control that is receiving the message.</param>
		/// <param name="messageId">Unique integer ID of the message such as WM_CLOSE, WM_SiZE, etc.</param>
		/// <param name="wParam">Additional information assigned to the message.</para>
		/// <param name="lParam">Additional information assigned to the message.</para>
		/// <returns>Returns a value to the source of the Windows message.</returns>
		static LRESULT CALLBACK OnProcessMessage(HWND windowHandle, UINT messageId, WPARAM wParam, LPARAM lParam);

		/// <summary>Stores a set of unique Windows message IDs reserved by the ReserveMessageId() method.</summary>
		std::unordered_set<UINT> fReservedMessageIdSet;

		/// <summary>The class name registered with the OS that the message-only window was created with.</summary>
		std::wstring fRegisteredClassName;
};

} }	// namespace Interop::UI
