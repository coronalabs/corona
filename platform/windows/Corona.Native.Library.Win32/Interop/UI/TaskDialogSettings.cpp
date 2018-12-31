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
