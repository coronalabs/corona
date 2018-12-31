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
#include "Control.h"


namespace Interop { namespace UI {

#pragma region Constructors/Destructors
Control::Control()
:	Control(nullptr)
{
}

Control::Control(HWND windowHandle)
:	UIComponent(windowHandle),
	fReceivedMessageEventHandler(this, &Control::OnReceivedMessage)
{
	// Add event handlers.
	GetReceivedMessageEventHandlers().Add(&fReceivedMessageEventHandler);
}

Control::~Control()
{
	// Remove event handlers.
	GetReceivedMessageEventHandlers().Remove(&fReceivedMessageEventHandler);
}

#pragma endregion


#pragma region Public Methods
Control::ResizedEvent::HandlerManager& Control::GetResizedEventHandlers()
{
	return fResizedEvent.GetHandlerManager();
}

Control::GainedFocusEvent::HandlerManager& Control::GetGainedFocusEventHandlers()
{
	return fGainedFocusEvent.GetHandlerManager();
}

Control::LostFocusEvent::HandlerManager& Control::GetLostFocusEventHandlers()
{
	return fLostFocusEvent.GetHandlerManager();
}

bool Control::HasFocus() const
{
	// Fetch the control's handle.
	auto windowHandle = GetWindowHandle();
	if (!windowHandle)
	{
		return false;
	}

	// Return this control's current focus state.
	return (windowHandle == ::GetFocus());
}

void Control::SetFocus()
{
	auto windowHandle = GetWindowHandle();
	if (windowHandle)
	{
		::SetFocus(windowHandle);
	}
}

bool Control::IsVisible() const
{
	// Fetch the control's handle.
	auto windowHandle = GetWindowHandle();
	if (!windowHandle)
	{
		return false;
	}

	// Return this control's current visibility state.
	return (::IsWindowVisible(windowHandle) != FALSE);
}

void Control::SetVisible(bool value)
{
	auto windowHandle = GetWindowHandle();
	if (windowHandle)
	{
		::ShowWindow(windowHandle, value ? SW_SHOW : SW_HIDE);
	}
}

RECT Control::GetBounds() const
{
	// Initialize bounds to all zeros.
	RECT bounds {};

	// Fetch the control's handle.
	auto windowHandle = GetWindowHandle();
	if (!windowHandle)
	{
		return bounds;
	}

	// Fetch the control's bounds.
	auto wasSuccessful = ::GetWindowRect(windowHandle, &bounds);
	if (!wasSuccessful)
	{
		return bounds;
	}

	// If the control has a parent, then convert its bounds from screen coordinates to the parent's client coordinates.
	auto parentWindowHandle = ::GetParent(windowHandle);
	if (parentWindowHandle)
	{
		POINT point;
		{
			point.x = bounds.left;
			point.y = bounds.top;
			::ScreenToClient(windowHandle, &point);
			bounds.left = point.x;
			bounds.top = point.y;
		}
		{
			point.x = bounds.right;
			point.y = bounds.bottom;
			::ScreenToClient(windowHandle, &point);
			bounds.right = point.x;
			bounds.bottom = point.y;
		}
	}

	// Return the successfully retrieved bounds.
	return bounds;
}

int Control::GetWidth() const
{
	RECT bounds = GetBounds();
	return bounds.right - bounds.left;
}

int Control::GetHeight() const
{
	RECT bounds = GetBounds();
	return bounds.bottom - bounds.top;
}

void Control::SetBounds(const RECT &bounds)
{
	auto windowHandle = GetWindowHandle();
	if (windowHandle)
	{
		WINDOWPLACEMENT windowPlacement{};
		windowPlacement.length = sizeof(windowPlacement);
		auto hasSucceeded = ::GetWindowPlacement(windowHandle, &windowPlacement);
		if (hasSucceeded)
		{
			// Update the window's position/bounds if changed.
			// Note: Doing this via SetWindowPlacement() is preferred for windows/dialogs since it'll
			//       maintain the window's restored, minimized, or maximized state.
			if (::EqualRect(&bounds, &windowPlacement.rcNormalPosition) == FALSE)
			{
				windowPlacement.rcNormalPosition = bounds;
				if (windowPlacement.rcNormalPosition.right <= windowPlacement.rcNormalPosition.left)
				{
					windowPlacement.rcNormalPosition.right = windowPlacement.rcNormalPosition.left + 1;
				}
				if (windowPlacement.rcNormalPosition.bottom <= windowPlacement.rcNormalPosition.top)
				{
					windowPlacement.rcNormalPosition.bottom = windowPlacement.rcNormalPosition.top + 1;
				}
				if (IsVisible() == false)
				{
					// The Win32 GetWindowPlacement() function above removes the window/control's hide state.
					// Add it back if applicable.
					windowPlacement.showCmd = SW_HIDE;
				}
				::SetWindowPlacement(windowHandle, &windowPlacement);
			}
		}
		else
		{
			// Update the control's position/bounds via the Win32 SetWindowPos() function as a fallback mechanism.
			int width = bounds.right - bounds.left;
			int height = bounds.bottom - bounds.top;
			if (width < 1)
			{
				width = 1;
			}
			if (height < 1)
			{
				height = 1;
			}
			::SetWindowPos(windowHandle, nullptr, bounds.left, bounds.top, width, height, SWP_NOZORDER);
		}
	}
}

RECT Control::GetClientBounds() const
{
	// Initialize bounds to all zeros.
	RECT bounds{};

	// Fetch the control's handle.
	auto windowHandle = GetWindowHandle();
	if (!windowHandle)
	{
		return bounds;
	}

	// Fetch the control's client bounds.
	auto wasSuccessful = ::GetClientRect(windowHandle, &bounds);
	if (!wasSuccessful)
	{
		return bounds;
	}

	// Return the successfully retrieved bounds.
	return bounds;
}

int Control::GetClientWidth() const
{
	RECT bounds = GetClientBounds();
	return bounds.right - bounds.left;
}

int Control::GetClientHeight() const
{
	RECT bounds = GetClientBounds();
	return bounds.bottom - bounds.top;
}

void Control::SetClientSize(const SIZE size)
{
	// Fetch the control's handle.
	auto windowHandle = GetWindowHandle();
	if (!windowHandle)
	{
		return;
	}

	// Make sure that the given size contains valid values.
	SIZE newClientSize = size;
	if (newClientSize.cx < 0)
	{
		newClientSize.cx = 0;
	}
	if (newClientSize.cy < 0)
	{
		newClientSize.cy = 0;
	}

	// Resize the control/window to fit the given client size.
	RECT clientBounds{};
	BOOL wasSuccessful = ::GetClientRect(windowHandle, &clientBounds);
	if (wasSuccessful)
	{
		RECT windowBounds{};
		wasSuccessful = ::GetWindowRect(windowHandle, &windowBounds);
		if (wasSuccessful)
		{
			windowBounds.right += newClientSize.cx - (clientBounds.right - clientBounds.left);
			windowBounds.bottom += newClientSize.cy - (clientBounds.bottom - clientBounds.top);
			SetBounds(windowBounds);
		}
	}
}

#pragma endregion


#pragma region Private Methods
void Control::OnReceivedMessage(UIComponent& sender, HandleMessageEventArgs& arguments)
{
	switch (arguments.GetMessageId())
	{
		case WM_SIZE:
			fResizedEvent.Raise(*this, EventArgs::kEmpty);
			break;
		case WM_SETFOCUS:
			fGainedFocusEvent.Raise(*this, EventArgs::kEmpty);
			break;
		case WM_KILLFOCUS:
			fLostFocusEvent.Raise(*this, EventArgs::kEmpty);
			break;
	}
}

#pragma endregion

} }	// namespace Interop::UI
