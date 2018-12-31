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

#include "Core\Rtt_Macros.h"
#include "Interop\OperationResult.h"
#include "TaskDialogSettings.h"
#include <CommCtrl.h>
#include <Windows.h>

struct XMSGBOXPARAMS;


namespace Interop { namespace UI {

/// <summary>
///  <para>Displays a Windows task dialog, which is a message box supporting customizable button text.</para>
///  <para>Will use Microsoft's built-in task dialog on Windows Vista or newer operating systems.</para>
///  <para>Will use an open source XMessageBox class for Windows XP that'll provide similar functionality.</para>
/// </summary>
class TaskDialog
{
	Rtt_CLASS_NO_COPIES(TaskDialog)

	public:
		#pragma region Constructors/Destructors
		/// <summary>Creates a new task dialog.</summary>
		TaskDialog();

		/// <summary>Destroys this object.</summary>
		virtual ~TaskDialog();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>
		///  <para>
		///   Gets a modifiable settings object used to customize the dialog's title text, message text,
		///   button labels, and other settings.
		///  </para>
		///  <para>
		///   You are expected to customize these settings before calling the Show() method. Any changes made to this
		///   object while the task dialog is shown will not be used until the next time the task dialog is shown.
		///  </para>
		/// </summary>
		/// <returns>Returns modifiable settings used to customize the task dialog.</returns>
		TaskDialogSettings& GetSettings();

		/// <summary>
		///  <para>Fetches the dialog's window handle.</para>
		///  <para>Only provided while a dialog is being shown. This handle may change everytime it is shown.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns the dialog window handle.</para>
		///  <para>Returns null if the dialog is not currently being displayed.</para>
		/// </returns>
		HWND GetWindowHandle() const;

		/// <summary>Determines if this task dialog is currently being shown onscreen.</summary>
		/// <returns>Returns true if the task dialog is currently being shown. Returns false if not.</returns>
		bool IsShowing() const;

		/// <summary>
		///  <para>Displays this task dialog using the assigned title, message, and button labels.</para>
		///  <para>
		///   The task dialog is displayed modally, meaning that this method blocks until the task dialog has been closed.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns a success result if the task dialog was successfully shown.</para>
		///  <para>
		///   Returns a failure result if the meesage box is currently showing or if failed to show the task dialog.
		///   The returned result object's message will provide details as to why it failed.
		///  </para>
		/// </returns>
		Interop::OperationResult Show();
		
		/// <summary>
		///  <para>Closes the task dialog if currently shown.</para>
		///  <para>The WasButtonPressed() method will return false in this case.</para>
		/// </summary>
		void Close();

		/// <summary>Closes the task dialog, if currently shown, by simulating a button press.</summary>
		/// <param name="index">Zero based index to a button in GetButtonLabels() method's returned collection.</param>
		/// <returns>
		///  <para>Returns true if successfully closed the task dialog with the given button index.</para>
		///  <para>Returns false if the task dialog is not currently shown or if given an invalid button index.</para>
		/// </returns>
		bool CloseWithButtonIndex(int index);

		/// <summary>Determines if the task dialog was closed via one of its displayed buttons.</summary>
		/// <returns>
		///  <para>
		///   Returns true if the task dialog was closed via a button press, in which case, the
		///   GetPressedButtonIndex() method will indicate which button was pressed. Note that this will return
		///   true if you call the CloseWithButtonIndex() method as well.
		///  </para>
		///  <para>
		///   Returns false if the task dialog was closed via the [x] button, if the Close() method was called,
		///   or if the task dialog hasn't been shown yet.
		///  </para>
		/// </returns>
		bool WasButtonPressed() const;

		/// <summary>
		///  <para>Determines which button was pressed to close the task dialog.</para>
		///  <para>Note that this method is only applicable if the WasButtonPressed() method return true.</para>
		/// </summary>
		/// <returns>
		///  <para>
		///   Returns a zero based index of the button that was pressed in the task dialog. This index matches a
		///   button label in the GetButtonLabels() method's returned collection.
		///  </para>
		///  <para>
		///   Returns -1 if a button was not pressed to close the task dialog, which is indicated by the WasButtonPressed()
		///   method. In this case, the task dialog was closed via the [x] button or by the Close() method.
		///  </para>
		/// </returns>
		int GetLastPressedButtonIndex() const;

		#pragma endregion

	private:
		#pragma region Private Static Functions
		/// <summary>
		///  <para>Called by a native Win32 task dialog for various notifications.</para>
		///  <para>
		///   A pointer to this function is expected to be given to the "TASKDIALOGCONFIG.pfCallback" struct entry.
		///  </para>
		/// </summary>
		/// <param name="windowHandle">Handle to the task dialog that is invoking this function.</param>
		/// <param name="notificationId">
		///  The notification's unique integer ID such as TDN_CREATED, TDN_DESTROYED, TDN_HYPERLINK_CLICKED, etc.
		/// </param>
		/// <param name="wParam">Additional information. What this is used for depends on the notification ID.</param>
		/// <param name="lParam">Additional information. What this is used for depends on the notification ID.</param>
		/// <param name="lpCallbackData">
		///  <para>Data assigned to the "TASKDIALOGCONFIG.lpCallbackData" struct entry.</para>
		///  <para>Expected to be set to a pointer to the TaskDialog instance that owns the Win32 dialog.</para>
		/// </param>
		/// <returns>
		///  Returns a success/failure result for the notification. This depends on the notfication ID.
		/// </returns>
		static HRESULT CALLBACK OnTaskDialogNotificationReceived(
				HWND windowHandle, UINT notificationId, WPARAM wParam, LPARAM lParam, LONG_PTR lpCallbackData);

		/// <summary>
		///  <para>Called by the XMessageBox class when its dialog has just been created.</para>
		///  <para>
		///   A pointer to this function is expected to be given to the "XMSGBOXPARAMS.lpCreatedFunc" struct entry.
		///  </para>
		/// </summary>
		/// <param name="windowHandle">Handle to the dialog that is invoking this function.</param>
		/// <param name="callbackData">
		///  <para>Data assigned to the "XMSGBOXPARAMS.lpCreatedFuncData" struct entry.</para>
		///  <para>Expected to be set to a pointer to the TaskDialog instance that owns the newly created dialog.</para>
		/// </param>
		static void OnXMessageBoxCreated(HWND windowHandle, LONG_PTR callbackData);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Provides the dialog's title text, message text, button labels, etc.</summary>
		TaskDialogSettings fSettings;

		/// <summary>
		///  <para>Handle to the dialog that is currently be shown.</para>
		///  <para>Null if a dialog is not currently being shown.</para>
		/// </summary>
		HWND fDialogWindowHandle;

		/// <summary>
		///  <para>Pointer to a native Win32 task dialog configuration that is currently being shown.</para>
		///  <para>
		///   This object is needed to figure out how to close the dialog externally via the
		///   Close() and CloseWithButtonIndex() methods.
		///  </para>
		///  <para>Set to null if a Microsoft TaskDialogIndirect() dialog is not currently being shown.</para>
		/// </summary>
		TASKDIALOGCONFIG* fTaskDialogPointer;

		/// <summary>
		///  <para>Pointer to parameters object used to display an XMessageBox that is currently being shown.</para>
		///  <para>
		///   This object is needed to figure out how to close the dialog externally via the
		///   Close() and CloseWithButtonIndex() methods.
		///  </para>
		///  <para>Set to null if an XMessageBox is not currently being shown.</para>
		/// </summary>
		XMSGBOXPARAMS* fXMessageBoxParamsPointer;

		/// <summary>
		///  <para>
		///   Set true if the buttons in the dialog are shown in reverse order compared to the button labels listed
		///   in the dialog's settings. Used to mimic Apple's right-to-left button order.
		///  </para>
		///  <para>Set false if the button order is not reversed and follows Microsoft's left-to-right order.</para>
		/// </summary>
		bool fAreButtonsReversed;

		/// <summary>
		///  <para>Set to true if the Close() or CloseWithButtonIndex() were called.</para>
		///  <para>This indicates that the dialog was closed via code instead of by the end-user.</para>
		/// </summary>
		bool fWasAbortRequested;

		/// <summary>
		///  <para>
		///   Set to true if a dialog button was clicked by the end-user or via the CloseWithButtonIndex() method.
		///  </para>
		///  <para>
		///   Set to false if the dialog was closed via the dialog [x] button or if the Close() method was called.
		///  </para>
		/// </summary>
		bool fWasButtonPressed;

		/// <summary>Zero based index of the button that was clicked. -1 if no button was clicked (ie: canceled).</summary>
		int fLastPressedButtonIndex;

		#pragma endregion
};

} }	// namespace Interop::UI
