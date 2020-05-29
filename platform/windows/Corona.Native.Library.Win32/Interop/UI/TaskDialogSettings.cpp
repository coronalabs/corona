//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TaskDialogSettings.h"


namespace Interop { namespace UI {

#pragma region Constructors/Destructors
TaskDialogSettings::TaskDialogSettings()
:	fParentWindowHandle(nullptr),
	fIsCancelButtonEnabled(false),
	fIsReverseHorizontalButtonsEnabled(false)
{
}

TaskDialogSettings::~TaskDialogSettings()
{
}

#pragma endregion


#pragma region Public Methods
HWND TaskDialogSettings::GetParentWindowHandle() const
{
	return fParentWindowHandle;
}

void TaskDialogSettings::SetParentWindowHandle(HWND windowHandle)
{
	fParentWindowHandle = windowHandle;
}

void TaskDialogSettings::SetParentToTopMostWindowOf(HWND windowHandle)
{
	if (windowHandle)
	{
#if 1
		HWND topMostWindowHandle = ::GetTopWindow(windowHandle);
		if (topMostWindowHandle)
		{
			windowHandle = topMostWindowHandle;
		}
#else
		HWND topMostWindowHandle = ::GetLastActivePopup(windowHandle);
		if (topMostWindowHandle)
		{
			windowHandle = topMostWindowHandle;
		}
#endif
	}
	fParentWindowHandle = windowHandle;
}

const wchar_t* TaskDialogSettings::GetTitleText() const
{
	return fTitle.c_str();
}

void TaskDialogSettings::SetTitleText(const wchar_t* text)
{
	if (!text)
	{
		text = L"";
	}
	fTitle = text;
}

const wchar_t* TaskDialogSettings::GetMessageText() const
{
	return fMessage.c_str();
}

void TaskDialogSettings::SetMessageText(const wchar_t* text)
{
	if (!text)
	{
		text = L"";
	}
	fMessage = text;
}

std::vector<std::wstring>& TaskDialogSettings::GetButtonLabels()
{
	return fButtonLabels;
}

bool TaskDialogSettings::IsCancelButtonEnabled() const
{
	return fIsCancelButtonEnabled;
}

void TaskDialogSettings::SetCancelButtonEnabled(bool value)
{
	fIsCancelButtonEnabled = value;
}

bool TaskDialogSettings::IsReverseHorizontalButtonsEnabled() const
{
	return fIsReverseHorizontalButtonsEnabled;
}

void TaskDialogSettings::SetReverseHorizontalButtonsEnabled(bool value)
{
	fIsReverseHorizontalButtonsEnabled = value;
}

#pragma endregion

} }	// namespace Interop::UI
