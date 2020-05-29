//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>
#include <Windows.h>


namespace Interop { namespace UI {

/// <summary>
///  Provides the title bar text, content text, button labels, and other settings to be used by a "TaskDialog" object.
/// </summary>
class TaskDialogSettings
{
	public:
		/// <summary>Creates a new settings object.</summary>
		TaskDialogSettings();

		/// <summary>Destroys this object and its resources.</summary>
		virtual ~TaskDialogSettings();

		/// <summary>Gets a handle to the window the task dialog will use as its parent.</summary>
		/// <returns>
		///  <para>Returns a handle to the task dialog's parent window.</para>
		///  <para>Returns null if the task dialog is parentless.</para>
		/// </returns>
		HWND GetParentWindowHandle() const;

		/// <summary>Sets the window to be used as the task dialog's parent.</summary>
		/// <param name="windowHandle">
		///  <para>Handle to a window or conrol to be used as this task dialog's parent.</para>
		///  <para>Set to null to make this a parentless task dialog.</para>
		/// </param>
		void SetParentWindowHandle(HWND windowHandle);

		/// <summary>Set a handle to the top-most window to be used as the task dialog's parent.</summary>
		/// <param name="windowHandle">
		///  <para>Handle to the window to be used to find the top-most window.</para>
		///  <para>Set to null to make this a parentless task dialog.</para>
		/// </param>
		void SetParentToTopMostWindowOf(HWND windowHandle);

		/// <summary>Gets the text to be displayed on the task dialog's title bar.</summary>
		/// <returns>
		///  <para>Returns the task dialog's title text.</para>
		///  <para>Returns an empty string if no title bar text is to be shown.</para>
		/// </returns>
		const wchar_t* GetTitleText() const;

		/// <summary>Sets the text to be displayed on the task dialog's title bar.</summary>
		/// <param name="text">
		///  <para>The text to be shown in the task dialog's title bar.</para>
		///  <para>Set to null or empty string to not show any title bar text.</para>
		/// </param>
		void SetTitleText(const wchar_t* text);

		/// <summary>Gets the main text to be displayed in the task dialog.</summary>
		/// <returns>
		///  <para>Returns the main content text to be displayed in the task dialog.</para>
		///  <para>Returns an empty string if no content text is to be shown.</para>
		/// </returns>
		const wchar_t* GetMessageText() const;

		/// <summary>Sets the main text to be displayed in the task dialog.</summary>
		/// <param name="text">
		///  <para>The main text to be displayed within the task dialog.</para>
		///  <para>Set to null or empty string to not display any text within the task dialog.</para>
		/// </param>
		void SetMessageText(const wchar_t* text);

		/// <summary>Gets a modifiable collection of labels to be displayed as buttons in the task dialog.</summary>
		/// <returns>
		///  <para>Returns a modifiable collection of strings used to generate buttons within the task dialog.</para>
		///  <para>Returns an empty collection if no buttons have been set up yet.</para>
		/// </returns>
		std::vector<std::wstring>& GetButtonLabels();

		/// <summary>
		///  <para>Determines if the dialog's [x] close/cancel button will be shown or hidden.</para>
		///  <para>
		///   Clicking this button will cancel out of the dialog, causing the TaskDialog's WasButtonPressed() method
		///   to return false since it's not one of the main buttons.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns true if the dialog's [x] close button will be shown to the user.</para>
		///  <para>Returns false if it'll be hidden or disabled.</para>
		/// </returns>
		bool IsCancelButtonEnabled() const;

		/// <summary>
		///  <para>Sets whether or not the task dialog's [x] close/cancel button will be shown.</para>
		///  <para>
		///   Clicking this button will cancel out of the dialog, causing the TaskDialog's WasButtonPressed() method
		///   to return false since it's not one of the main buttons.
		///  </para>
		/// </summary>
		/// <param name="value">
		///  <para>Set true to show this close/cancel button.</para>
		///  <para>Set false to hide/disable it, forcing the user to click on one of the main buttons.</para>
		/// </param>
		void SetCancelButtonEnabled(bool value);

		/// <summary>
		///  <para>
		///   Determines whether or not buttons should be shown using Microsoft's default left-to-right order for
		///   non-Arabic systems or the reverse right-to-left order like Apple/Android systems.
		///  </para>
		///  <para>Intended to be used by the Corona Simulator to mimic Apple/Android's right-to-left button order.</para>
		///  <para>
		///   Not intended to do right-to-left ordering for Arabic systems since Microsoft's native dialogs
		///   already reverse the button order for you.
		///  </para>
		///  <para>This setting does not affect the dialog's button order when it lists buttons vertically.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns true if the dialog button order is opposite to Microsoft' default, making it right-to-left.</para>
		///  <para>Returns false if the dialog button order is according to Microsoft's default left-to-right order.</para>
		/// </returns>
		bool IsReverseHorizontalButtonsEnabled() const;

		/// <summary>
		///  <para>
		///   Sets whether or not the buttons should be shown using Microsoft's default left-to-right order for non-Arabic
		///   systems or the reverse right-to-left order like Apple/Android systems.
		///  </para>
		///  <para>Intended to be used by the Corona Simulator to mimic Apple/Android's right-to-left button order.</para>
		///  <para>
		///   Not intended to do right-to-left ordering for Arabic systems since Microsoft's native dialogs
		///   already reverse the button order for you.
		///  </para>
		///  <para>This setting does not affect the dialog's button order when it lists buttons vertically.</para>
		/// </summary>
		/// <param name="value">
		///  <para>Set true to show the button order opposite to Microsoft' default, making it right-to-left.</para>
		///  <para>Set false to show the button order according to Microsoft's default left-to-right order.</para>
		/// </param>
		void SetReverseHorizontalButtonsEnabled(bool value);

	private:
		HWND fParentWindowHandle;
		std::wstring fTitle;
		std::wstring fMessage;
		std::vector<std::wstring> fButtonLabels;
		bool fIsCancelButtonEnabled;
		bool fIsReverseHorizontalButtonsEnabled;
};

} }	// namespace Interop::UI
