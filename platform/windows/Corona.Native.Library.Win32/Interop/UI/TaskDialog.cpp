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

#include "stdafx.h"
#include "TaskDialog.h"
#include "WinString.h"
#include "XMessageBox.h"
#include <algorithm>
#include <CommCtrl.h>


namespace Interop { namespace UI {

#pragma region Static Variables
/// <summary>Defines a callback type for the Windows TaskDialogIndirect() function.</summary>
typedef HRESULT(WINAPI *TaskDialogIndirectCallback)(
			_In_ const TASKDIALOGCONFIG*, _Out_opt_ int*, _Out_opt_ int*, _Out_opt_ BOOL*);

/// <summary>Callback to the Windows TaskDialogIndirect() function.</summary>
static TaskDialogIndirectCallback sTaskDialogIndirectCallback = nullptr;

#pragma endregion


#pragma region Constructors/Destructors
TaskDialog::TaskDialog()
:	fDialogWindowHandle(nullptr),
	fTaskDialogPointer(nullptr),
	fXMessageBoxParamsPointer(nullptr),
	fAreButtonsReversed(false),
	fWasAbortRequested(false),
	fWasButtonPressed(false),
	fLastPressedButtonIndex(-1)
{
	// Fetch a callback to the system's TaskDialogIndirect() function, if not done already.
	// Note: This function is only available on Windows Vista and newer OS versions.
	//       It's also only available if this app has linked to version 6 or newer of the "Comctrl32.dll" library.
	if (!sTaskDialogIndirectCallback)
	{
		::InitCommonControls();
		HMODULE moduleHandle = ::LoadLibraryW(L"Comctl32");
		if (moduleHandle)
		{
			sTaskDialogIndirectCallback = (TaskDialogIndirectCallback)::GetProcAddress(moduleHandle, "TaskDialogIndirect");
		}
	}
}

TaskDialog::~TaskDialog()
{
}

#pragma endregion


#pragma region Public Methods
TaskDialogSettings& TaskDialog::GetSettings()
{
	return fSettings;
}

HWND TaskDialog::GetWindowHandle() const
{
	return fDialogWindowHandle;
}

bool TaskDialog::IsShowing() const
{
	return (fDialogWindowHandle != nullptr);
}

Interop::OperationResult TaskDialog::Show()
{
	// Do not continue if already shown.
	if (IsShowing())
	{
		return OperationResult::FailedWith(L"Task dialog has already been shown.");
	}

	// Display the task dialog modally. (This is a blocking operation.)
	auto showResult = OperationResult::kSucceeded;
	if (sTaskDialogIndirectCallback)
	{
		// *** We have access to the native Windows task dialog which provides custom button text support. ***

		// Make sure that the IDs we assign to each button is greater than IDCANCEL.
		// Otherwise, an [x] cancel button will appear in the top-right corner of the dialog.
		const int kButtonIndexOffset = IDCANCEL + 1;

		// Set up the task dialog's settings.
		TASKDIALOG_BUTTON* dialogButtonsArray = nullptr;
		TASKDIALOGCONFIG dialogConfig{};
		dialogConfig.cbSize = sizeof(dialogConfig);
		dialogConfig.hwndParent = fSettings.GetParentWindowHandle();
		dialogConfig.pszWindowTitle = fSettings.GetTitleText();
		if (L'\0' == dialogConfig.pszWindowTitle[0])
		{
			dialogConfig.pszWindowTitle = L" ";
		}
		dialogConfig.pszContent = fSettings.GetMessageText();
		dialogConfig.lpCallbackData = (LONG_PTR)this;
		dialogConfig.pfCallback = TaskDialog::OnTaskDialogNotificationReceived;
		dialogConfig.dwFlags = TDF_POSITION_RELATIVE_TO_WINDOW;
		if (L'\0' == dialogConfig.pszContent[0])
		{
			dialogConfig.dwFlags |= TDF_USE_COMMAND_LINKS_NO_ICON;
		}
		fAreButtonsReversed = false;
		if (fSettings.GetButtonLabels().size() > 0)
		{
			// Set up the buttons.
			dialogConfig.cButtons = fSettings.GetButtonLabels().size();
			dialogButtonsArray = new TASKDIALOG_BUTTON[dialogConfig.cButtons];
			bool isShowingButtonsVertically =
					((dialogConfig.dwFlags & (TDF_USE_COMMAND_LINKS_NO_ICON | TDF_USE_COMMAND_LINKS)) != 0);
			if (fSettings.IsReverseHorizontalButtonsEnabled() && !isShowingButtonsVertically)
			{
				fAreButtonsReversed = true;
			}
			for (size_t dialogButtonIndex = 0; dialogButtonIndex < dialogConfig.cButtons; dialogButtonIndex++)
			{
				size_t buttonLabelIndex = dialogButtonIndex;
				if (fAreButtonsReversed)
				{
					buttonLabelIndex = (dialogConfig.cButtons - 1) - dialogButtonIndex;
				}
				dialogButtonsArray[dialogButtonIndex].nButtonID = buttonLabelIndex + kButtonIndexOffset;
				dialogButtonsArray[dialogButtonIndex].pszButtonText =
						fSettings.GetButtonLabels().at(buttonLabelIndex).c_str();
			}
			dialogConfig.pButtons = dialogButtonsArray;
			if (fSettings.IsCancelButtonEnabled())
			{
				dialogConfig.dwFlags |= TDF_ALLOW_DIALOG_CANCELLATION;
			}
		}
		else
		{
			// No button labels were provided.
			// Set up the task dialog to show a [Close] and [x] button.
			dialogConfig.dwFlags |= TDF_ALLOW_DIALOG_CANCELLATION;
			dialogConfig.dwCommonButtons = TDCBF_CLOSE_BUTTON;
		}

		// Display the task dialog modally. (This is a blocking operation.)
		int buttonIdPressed = 0;
		fWasAbortRequested = false;
		fTaskDialogPointer = &dialogConfig;
		HRESULT result = sTaskDialogIndirectCallback(&dialogConfig, &buttonIdPressed, nullptr, nullptr);
		if (SUCCEEDED(result))
		{
			// The dialog was displayed successfully. Determine how the user closed out of the dialog.
			if (!fWasAbortRequested && (dialogConfig.cButtons > 0) && (buttonIdPressed >= kButtonIndexOffset))
			{
				// The user clicked one of the custom buttons. Fetch the button's index.
				fLastPressedButtonIndex = buttonIdPressed - kButtonIndexOffset;
				fWasButtonPressed = true;
			}
			else
			{
				// The user canceled out of the dialog.
				fLastPressedButtonIndex = -1;
				fWasButtonPressed = false;
			}
		}
		else
		{
			// Failed to display the dialog. Create an error result providing the reason why.
			std::wstring message(L"Failed to display task dialog.");
			auto errorCode = ::GetLastError();
			if (errorCode)
			{
				LPWSTR utf16Buffer = nullptr;
				::FormatMessageW(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						nullptr, errorCode,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPWSTR)&utf16Buffer, 0, nullptr);
				if (utf16Buffer)
				{
					message.append(L" Reason:\r\n   ");
					message.append(utf16Buffer);
					::LocalFree(utf16Buffer);
				}
			}
			showResult = OperationResult::FailedWith(message.c_str());
		}

		// Cleanup the dialog's resources.
		fTaskDialogPointer = nullptr;
		if (dialogButtonsArray)
		{
			delete[] dialogButtonsArray;
		}
	}
	else
	{
		// We do not have access to the native Windows task dialog. This typically happens when:
		// 1) Running on Windows XP or an older OS version.
		// 2) A "ComCtl32.dll" library older than 6.0 was loaded by mistake by the app.

		// Set up the custom message box's settings.
		fXMessageBoxParamsPointer = new XMSGBOXPARAMS;
		fXMessageBoxParamsPointer->lpCreatedFuncData = (LONG_PTR)this;
		fXMessageBoxParamsPointer->lpCreatedFunc = TaskDialog::OnXMessageBoxCreated;
		int buttonCount = 0;
		if (fSettings.GetButtonLabels().size() > 0)
		{
			fAreButtonsReversed = fSettings.IsReverseHorizontalButtonsEnabled();
			const auto kMaxCustomButtonsStringLength = sizeof(fXMessageBoxParamsPointer->szCustomButtons);
			const size_t kMaxButtonLabelIndex =
					std::min<size_t>(fSettings.GetButtonLabels().size() - 1, ID_XMESSAGEBOX_LAST_ID - IDCUSTOM1);
			for (size_t dialogButtonIndex = 0; dialogButtonIndex <= kMaxButtonLabelIndex; dialogButtonIndex++)
			{
				int buttonLabelIndex = dialogButtonIndex;
				if (fAreButtonsReversed)
				{
					buttonLabelIndex = kMaxButtonLabelIndex - buttonLabelIndex;
				}
				if (fXMessageBoxParamsPointer->szCustomButtons[0] != L'\0')
				{
					::wcscat_s(fXMessageBoxParamsPointer->szCustomButtons, kMaxCustomButtonsStringLength, L"\n");
				}
				const wchar_t* buttonLabelPointer = fSettings.GetButtonLabels().at(buttonLabelIndex).c_str();
				if (!buttonLabelPointer || (L'\0' == buttonLabelPointer[0]))
				{
					buttonLabelPointer = L" ";
				}
				::wcscat_s(fXMessageBoxParamsPointer->szCustomButtons, kMaxCustomButtonsStringLength, buttonLabelPointer);
				buttonCount++;
			}
		}
		DWORD messageBoxStyle = MB_NOSOUND;
		if (L'\0' == fXMessageBoxParamsPointer->szCustomButtons)
		{
			messageBoxStyle |= MB_OK;
		}

		// Display the task dialog modally. (This is a blocking operation.)
		fWasAbortRequested = false;
		::SetLastError(ERROR_SUCCESS);
		int buttonIdPressed = ::XMessageBox(
				fSettings.GetParentWindowHandle(), fSettings.GetMessageText(),
				fSettings.GetTitleText(), messageBoxStyle, fXMessageBoxParamsPointer);
		if (buttonIdPressed > 0)
		{
			// The dialog was displayed successfully. Determine how the user closed out of the dialog.
			buttonIdPressed &= 0xFF;
			if (!fWasAbortRequested && (buttonIdPressed >= IDCUSTOM1) && (buttonIdPressed <= ID_XMESSAGEBOX_LAST_ID))
			{
				// The user clicked one of the custom buttons. Fetch the button's index.
				fLastPressedButtonIndex = buttonIdPressed - IDCUSTOM1;
				if (fAreButtonsReversed)
				{
					fLastPressedButtonIndex = (buttonCount - 1) - fLastPressedButtonIndex;
				}
				fWasButtonPressed = true;
			}
			else
			{
				// The user canceled out of the dialog.
				fLastPressedButtonIndex = -1;
				fWasButtonPressed = false;
			}
		}
		else
		{
			// Failed to display the dialog. Create an error result providing the reason why.
			std::wstring message(L"Failed to display task dialog.");
			auto errorCode = ::GetLastError();
			if (errorCode)
			{
				LPWSTR utf16Buffer = nullptr;
				::FormatMessageW(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						nullptr, errorCode,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPWSTR)&utf16Buffer, 0, nullptr);
				if (utf16Buffer)
				{
					message.append(L" Reason:\r\n   ");
					message.append(utf16Buffer);
					::LocalFree(utf16Buffer);
				}
			}
			showResult = OperationResult::FailedWith(message.c_str());
		}

		// Cleanup the dialog's resources.
		delete fXMessageBoxParamsPointer;
		fXMessageBoxParamsPointer = nullptr;
	}

	// Null out the handle of the dialog that was just opened and closed up above.
	fDialogWindowHandle = nullptr;

	// Returns whether or not we were able to display the dialog.
	return showResult;
}

void TaskDialog::Close()
{
	if (fDialogWindowHandle)
	{
		fWasAbortRequested = true;
		if (fTaskDialogPointer)
		{
			if (fTaskDialogPointer->cButtons > 0)
			{
				WPARAM wParam = MAKEWPARAM(fTaskDialogPointer->cButtons + IDCANCEL, BN_CLICKED);
				::SendMessage(fDialogWindowHandle, TDM_CLICK_BUTTON, wParam, 0);
			}
			else
			{
				WPARAM wParam = MAKEWPARAM(IDCANCEL, BN_CLICKED);
				::SendMessage(fDialogWindowHandle, WM_COMMAND, wParam, 0);
			}
		}
		else if (fXMessageBoxParamsPointer)
		{
			if (fXMessageBoxParamsPointer->szCustomButtons[0] != L'\0')
			{
				WPARAM wParam = MAKEWPARAM(IDCUSTOM1, BN_CLICKED);
				::SendMessage(fDialogWindowHandle, WM_COMMAND, wParam, 0);
			}
			else
			{
				WPARAM wParam = MAKEWPARAM(IDCANCEL, BN_CLICKED);
				::SendMessage(fDialogWindowHandle, WM_COMMAND, wParam, 0);
			}
		}
	}
}

bool TaskDialog::CloseWithButtonIndex(int index)
{
	if (fTaskDialogPointer)
	{
		if (fTaskDialogPointer->cButtons > 0)
		{
			if ((index >= 0) && (index < (int)fTaskDialogPointer->cButtons))
			{
				WPARAM wParam = MAKEWPARAM(index + IDCANCEL + 1, BN_CLICKED);
				::SendMessage(fDialogWindowHandle, TDM_CLICK_BUTTON, wParam, 0);
				return true;
			}
		}
	}
	else if (fXMessageBoxParamsPointer)
	{
		if (fXMessageBoxParamsPointer->szCustomButtons[0] != L'\0')
		{
			if (fAreButtonsReversed)
			{
				int lastButtonIndex = ID_XMESSAGEBOX_LAST_ID - IDCUSTOM1;
				for (; !::GetDlgItem(fDialogWindowHandle, IDCUSTOM1 + lastButtonIndex); lastButtonIndex--);
				index = lastButtonIndex - index;
			}
			int buttonId = IDCUSTOM1 + index;
			auto buttonHandle = ::GetDlgItem(fDialogWindowHandle, buttonId);
			if (buttonHandle)
			{
				WPARAM wParam = MAKEWPARAM(buttonId, BN_CLICKED);
				::SendMessage(fDialogWindowHandle, WM_COMMAND, wParam, 0);
				return true;
			}
		}
	}
	return false;
}

bool TaskDialog::WasButtonPressed() const
{
	return fWasButtonPressed;
}

int TaskDialog::GetLastPressedButtonIndex() const
{
	return fLastPressedButtonIndex;
}

#pragma endregion


#pragma region Private Static Functions
HRESULT CALLBACK TaskDialog::OnTaskDialogNotificationReceived(
	HWND windowHandle, UINT notificationId, WPARAM wParam, LPARAM lParam, LONG_PTR lpCallbackData)
{
	// Fetch a pointer to the TaskDialog object that owns the window invoking this callback.
	if (!lpCallbackData)
	{
		return S_OK;
	}
	auto taskDialogPointer = (TaskDialog*)lpCallbackData;

	// Handle the dialog's notification.
	switch (notificationId)
	{
		case TDN_CREATED:
			// The dialog was just created. Store a handle to the dialog so it can be
			// closed externally via the Close() and CloseWithButtonIndex() methods.
			taskDialogPointer->fDialogWindowHandle = windowHandle;
			break;
	}

	// Return a success result to the dialog.
	return S_OK;
}

void TaskDialog::OnXMessageBoxCreated(HWND windowHandle, LONG_PTR callbackData)
{
	// Fetch a pointer to the TaskDialog object that owns the window invoking this callback.
	if (!callbackData)
	{
		return;
	}
	auto taskDialogPointer = (TaskDialog*)callbackData;

	// The dialog was just created. Store a handle to the dialog so it can be
	// closed externally via the Close() and CloseWithButtonIndex() methods.
	taskDialogPointer->fDialogWindowHandle = windowHandle;

	// Show the dialog's [x] close button for canceling the dialog if configured to do so.
	if (taskDialogPointer->fSettings.IsCancelButtonEnabled())
	{
		auto systemMenuHandle = ::GetSystemMenu(windowHandle, FALSE);
		if (systemMenuHandle)
		{
			::EnableMenuItem(systemMenuHandle, SC_CLOSE, MF_ENABLED);
		}
	}
}

#pragma endregion

} }	// namespace Interop::UI
