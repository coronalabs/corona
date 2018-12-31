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
#include "Window.h"
#include "Core\Rtt_Build.h"
#include "Rtt_NativeWindowMode.h"


namespace Interop { namespace UI {

#pragma region Constructors/Destructors
Window::Window(HWND windowHandle)
:	Control(windowHandle),
	fPreviousWindowStyles(0)
{
}

Window::~Window()
{
}

#pragma endregion


#pragma region Public Methods
void Window::SetWindowMode(const Rtt::NativeWindowMode& windowMode)
{
	// Fetch the Win32 handle.
	HWND windowHandle = this->GetWindowHandle();
	if (!windowHandle)
	{
		return;
	}

	// Fetch the window's current mode.
	int currentWindowMode = GetNormalizedWin32ShowState();

	// Determine if the window currently has a border.
	const LONG kBorderStyles = WS_BORDER | WS_DLGFRAME | WS_THICKFRAME;
	LONG currentWindowStyles = ::GetWindowLongPtrW(windowHandle, GWL_STYLE);
	bool hasBorder = (currentWindowStyles & kBorderStyles) ? true : false;

	// Determine if the window is currently being displayed fullscreen.
	bool isFullscreen = !hasBorder && (SW_MAXIMIZE == currentWindowMode);

	// Fetch the requested window mdoe from Lua.
	int newWindowMode = SW_RESTORE;
	bool isRequestingFullscreenMode = false;
	if (windowMode.Equals(Rtt::NativeWindowMode::kNormal))
	{
		newWindowMode = SW_RESTORE;
	}
	else if (windowMode.Equals(Rtt::NativeWindowMode::kMinimized))
	{
		newWindowMode = SW_MINIMIZE;
	}
	else if (windowMode.Equals(Rtt::NativeWindowMode::kMaximized))
	{
		newWindowMode = SW_MAXIMIZE;
	}
	else if (windowMode.Equals(Rtt::NativeWindowMode::kFullscreen))
	{
		newWindowMode = SW_MAXIMIZE;
		isRequestingFullscreenMode = true;
	}
	else
	{
		Rtt_LogException("\"windowMode\" name \"%s\" is not supported on this platform.", windowMode.GetStringId());
		return;
	}

	// Update the window mode, but only if it is changing.
	bool isBorderChanging = (isRequestingFullscreenMode != isFullscreen) && (newWindowMode != SW_MINIMIZE);
	if ((newWindowMode != currentWindowMode) || isBorderChanging)
	{
		// Update the window's border if switching to/from fullscreen mode.
		if (isBorderChanging)
		{
			// Add/remove the window's border.
			if (isRequestingFullscreenMode)
			{
				fPreviousWindowStyles = currentWindowStyles;
				::SetWindowLongPtrW(windowHandle, GWL_STYLE, currentWindowStyles & ~kBorderStyles);
				::SetWindowPos(
						windowHandle, nullptr, 0, 0, 0, 0,
						SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

#if 0
//NOTE: In the future, if we want to do *real* fullscreen support, then it would be done like the below.
//      What we're doing above is a borderless maximized window, which is how normal desktop apps do fullscreen.
//      The advantage of the below is you get better GPU performance and Nvidia SLI and ATI Crossfire support.
				auto monitorHandle = ::MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);
				if (monitorHandle)
				{
					MONITORINFOEXW monitorInfo{};
					monitorInfo.cbSize = sizeof(monitorInfo);
					::GetMonitorInfoW(monitorHandle, &monitorInfo);
					LONG screenWidth = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
					LONG screenHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
					if ((screenWidth > 0) && (screenHeight > 0))
					{
						DEVMODEW deviceMode{};
						deviceMode.dmSize = sizeof(deviceMode);
						deviceMode.dmPelsWidth = screenWidth;
						deviceMode.dmPelsHeight = screenHeight;
						deviceMode.dmBitsPerPel = 32;
						deviceMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
						auto changeResult = ::ChangeDisplaySettingsExW(
							monitorInfo.szDevice, &deviceMode, nullptr, CDS_FULLSCREEN, nullptr);
						if (changeResult != DISP_CHANGE_SUCCESSFUL)
						{
							bool uhOh = true;
						}
					}
				}
#endif
			}
			else
			{
				::SetWindowLongPtrW(windowHandle, GWL_STYLE, currentWindowStyles | fPreviousWindowStyles);
#if 0
				MONITORINFOEXW monitorInfo{};
				monitorInfo.cbSize = sizeof(monitorInfo);
				auto monitorHandle = ::MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);
				if (monitorHandle)
				{
					::GetMonitorInfoW(monitorHandle, &monitorInfo);
				}
				::ChangeDisplaySettingsExW(monitorInfo.szDevice, nullptr, nullptr, 0, nullptr);
#endif
			}

			// This works-around a Windows bug where a window's border won't change correctly when switching
			// from maximized to fullscreen or vice-versa. We must restore to "normal" window mode first.
			if (SW_MAXIMIZE == currentWindowMode)
			{
				::ShowWindow(windowHandle, SW_RESTORE);
			}
		}

		// Display the window in the requested mode.
		::ShowWindow(windowHandle, newWindowMode);
	}
}

const Rtt::NativeWindowMode& Window::GetWindowMode() const
{
	HWND windowHandle = this->GetWindowHandle();
	if (windowHandle)
	{
		int showState = GetNormalizedWin32ShowState();
		switch (showState)
		{
			case SW_MAXIMIZE:
			{
				// On Windows, a maximized window without a border is displayed "fullscreen".
				const LONG kBorderStyles = WS_BORDER | WS_DLGFRAME;
				LONG currentWindowStyles = ::GetWindowLongW(windowHandle, GWL_STYLE);
				bool hasBorder = (currentWindowStyles & kBorderStyles) ? true : false;
				if (!hasBorder && (SW_MAXIMIZE == showState))
				{
					return Rtt::NativeWindowMode::kFullscreen;
				}
				else
				{
					return Rtt::NativeWindowMode::kMaximized;
				}
			}
			case SW_MINIMIZE:
			{
				return Rtt::NativeWindowMode::kMinimized;
			}
			case SW_RESTORE:
			default:
			{
				return Rtt::NativeWindowMode::kNormal;
			}
		}
	}
	return Rtt::NativeWindowMode::kNormal;
}

SIZE Window::GetNormalModeSize() const
{
	auto bounds = GetNormalModeBounds();
	SIZE size{};
	size.cx = std::abs(bounds.right - bounds.left);
	size.cy = std::abs(bounds.bottom - bounds.top);
	return size;
}

RECT Window::GetNormalModeBounds() const
{
	// Fetch the window's handle.
	auto windowHandle = GetWindowHandle();
	if (!windowHandle)
	{
		return RECT{};
	}

	// Attempt to fetch the window's normal mode bounds.
	WINDOWPLACEMENT windowPlacement{};
	windowPlacement.length = sizeof(windowPlacement);
	auto wasSuccessful = ::GetWindowPlacement(windowHandle, &windowPlacement);

	// If the above failed, then fallback to using the window's current bounds.
	if (!wasSuccessful)
	{
		windowPlacement.rcNormalPosition = GetBounds();
	}

	// Return the request bounds.
	return windowPlacement.rcNormalPosition;
}

void Window::SetNormalModeBounds(const RECT& bounds)
{
	SetBounds(bounds);
}

RECT Window::GetNormalModeClientBoundsInWorkspace() const
{
	// Fetch the window's handle.
	auto windowHandle = GetWindowHandle();
	if (!windowHandle)
	{
		return RECT{};
	}

	// Fetch info about this window needed for the calculations below.
	// Note: The border style is removed when in fullscreen mode. So, use the previous normal mode styles in this case.
	LONG styleFlags;
	if (GetWindowMode().Equals(Rtt::NativeWindowMode::kFullscreen) && fPreviousWindowStyles)
	{
		styleFlags = fPreviousWindowStyles;
	}
	else
	{
		styleFlags = ::GetWindowLongW(windowHandle, GWL_STYLE);
	}
	auto exStyleFlags = ::GetWindowLongW(windowHandle, GWL_EXSTYLE);
	BOOL hasMenu = ::GetMenu(windowHandle) ? TRUE : FALSE;

	// Fetch the size of the window's borders.
	RECT adjustedBounds{};
	::AdjustWindowRectEx(&adjustedBounds, styleFlags, hasMenu, exStyleFlags);

	// Calculate and return this window's client bounds.
	auto clientBounds = GetNormalModeBounds();
	clientBounds.left -= adjustedBounds.left;
	clientBounds.top -= adjustedBounds.top;
	clientBounds.right -= adjustedBounds.right;
	clientBounds.bottom -= adjustedBounds.bottom;
	return clientBounds;
}

SIZE Window::GetNormalModeClientSize() const
{
	auto bounds = GetNormalModeClientBoundsInWorkspace();
	SIZE size{};
	size.cx = std::abs(bounds.right - bounds.left);
	size.cy = std::abs(bounds.bottom - bounds.top);
	return size;
}

void Window::SetNormalModeClientSize(const SIZE& size)
{
	// Fetch the window's handle.
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

	// Fetch the current client bounds.
	auto bounds = GetNormalModeClientBoundsInWorkspace();

	// Do not continue if the client size isn't changing.
	if (((bounds.right - bounds.left) == newClientSize.cx) && ((bounds.bottom - bounds.top) == newClientSize.cy))
	{
		return;
	}

	// Fetch info about this window needed for the calculations below.
	// Note: The border style is removed when in fullscreen mode. So, use the previous normal mode styles in this case.
	LONG styleFlags;
	if (GetWindowMode().Equals(Rtt::NativeWindowMode::kFullscreen) && fPreviousWindowStyles)
	{
		styleFlags = fPreviousWindowStyles;
	}
	else
	{
		styleFlags = ::GetWindowLongW(windowHandle, GWL_STYLE);
	}
	auto exStyleFlags = ::GetWindowLongW(windowHandle, GWL_EXSTYLE);
	BOOL hasMenu = ::GetMenu(windowHandle) ? TRUE : FALSE;

	// Resize the window's normal mode client bounds.
	bounds.right = bounds.left + newClientSize.cx;
	bounds.bottom = bounds.top + newClientSize.cy;
	auto wasAdjusted = ::AdjustWindowRectEx(&bounds, styleFlags, hasMenu, exStyleFlags);
	if (wasAdjusted)
	{
		SetNormalModeBounds(bounds);
	}
}

#pragma endregion


#pragma region Private Methods
int Window::GetNormalizedWin32ShowState() const
{
	// Fetch the window's current mode.
	WINDOWPLACEMENT currentWindowPlacement{};
	currentWindowPlacement.length = sizeof(currentWindowPlacement);
	currentWindowPlacement.showCmd = SW_RESTORE;
	HWND windowHandle = this->GetWindowHandle();
	if (windowHandle)
	{
		::GetWindowPlacement(windowHandle, &currentWindowPlacement);
	}

	// Normalize the different window modes to their simpler states.
	// This makes it easier for SetWindowMode() and GetWindowMode() to recognize the window's current state.
	switch (currentWindowPlacement.showCmd)
	{
		case SW_SHOWMINIMIZED:
		case SW_SHOWMINNOACTIVE:
			currentWindowPlacement.showCmd = SW_MINIMIZE;
			break;
		case SW_SHOWMAXIMIZED:
			currentWindowPlacement.showCmd = SW_MAXIMIZE;
			break;
		case SW_SHOW:
		case SW_SHOWNA:
		case SW_SHOWNOACTIVATE:
		case SW_SHOWNORMAL:
		default:
			currentWindowPlacement.showCmd = SW_RESTORE;
			break;
	}
	return currentWindowPlacement.showCmd;
}

#pragma endregion

} }	// namespace Interop::UI
