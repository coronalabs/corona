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
#include "Rtt_WinDisplayObject.h"
#include "Core\Rtt_Build.h"
#include "Core\Rtt_String.h"
#include "Corona\CoronaLua.h"
#include "Display\Rtt_Display.h"
#include "Interop\UI\Control.h"
#include "Interop\MDeviceSimulatorServices.h"
#include "Interop\RuntimeEnvironment.h"
#include "Renderer\Rtt_Renderer.h"
#include "Rtt_Lua.h"
#include "Rtt_Runtime.h"
#include "Rtt_WinDevice.h"
#include "Rtt_WinInputDeviceManager.h"
#include "Rtt_WinPlatform.h"
#include <Windows.h>


namespace Rtt
{

WinDisplayObject::WinDisplayObject(Interop::RuntimeEnvironment& environment, const Rect& bounds)
:	PlatformDisplayObject(),
	fReceivedMessageEventHandler(this, &WinDisplayObject::OnReceivedMessage),
	fEnvironment(environment),
	fSelfBounds(bounds)
{
	// Convert the given x/y coordinates from a top-left reference point to a center reference point.
	Translate(bounds.xMin + Rtt_RealDiv2(bounds.Width()), bounds.yMin + Rtt_RealDiv2(bounds.Height()));
	fSelfBounds.MoveCenterToOrigin();
}

WinDisplayObject::~WinDisplayObject()
{
}

Interop::RuntimeEnvironment& WinDisplayObject::GetRuntimeEnvironment() const
{
	return fEnvironment;
}

void WinDisplayObject::SetFocus() const
{
	auto controlPointer = GetControl();
	if (controlPointer)
	{
		controlPointer->SetFocus();
	}
}

bool WinDisplayObject::Initialize()
{
	// Fetch this object's native UI control.
	auto controlPointer = GetControl();
	if (!controlPointer)
	{
		Rtt_ASSERT(0);
		return false;
	}

	// Add event handlers.
	controlPointer->GetReceivedMessageEventHandlers().Add(&fReceivedMessageEventHandler);
	return true;
}

void WinDisplayObject::DidMoveOffscreen()
{
}

void WinDisplayObject::WillMoveOnscreen()
{
}

bool WinDisplayObject::CanCull() const
{
	// Disable culling for all native display objects.
	// Note: This is needed so that the Build() function will get called when a native object
	//       is being moved partially or completely offscreen.
	return false;
}

void WinDisplayObject::Prepare(const Display& display)
{
	PlatformDisplayObject::Prepare(display);
	if (ShouldPrepare())
	{
		// First, update this object's cached scale factors in case the app window has been resized.
		auto runtimePointer = fEnvironment.GetRuntime();
		Preinitialize(runtimePointer->GetDisplay());

		// Update the native object's screen bounds.
		auto controlPointer = GetControl();
		if (controlPointer)
		{
			Rect coronaBounds{};
			RECT nativeBounds{};
			GetScreenBounds(coronaBounds);
			nativeBounds.left = (LONG)Rtt_RealToInt(coronaBounds.xMin);
			nativeBounds.top = (LONG)Rtt_RealToInt(coronaBounds.yMin);
			nativeBounds.right = (LONG)Rtt_RealToInt(coronaBounds.xMax);
			nativeBounds.bottom = (LONG)Rtt_RealToInt(coronaBounds.yMax);
			controlPointer->SetBounds(nativeBounds);
		}
	}
}

void WinDisplayObject::Draw( Renderer& renderer ) const
{
}

void WinDisplayObject::GetSelfBounds(Rect& rect) const
{
	rect = fSelfBounds;
}

void WinDisplayObject::SetSelfBounds(Real width, Real height)
{
	// Update the bounds.
	if (width > Rtt_REAL_0)
	{
		fSelfBounds.Initialize(Rtt_RealDiv2(width), Rtt_RealDiv2(GetGeometricProperty(kHeight)));
	}
	if (height > Rtt_REAL_0)
	{
		fSelfBounds.Initialize(Rtt_RealDiv2(GetGeometricProperty(kWidth)), Rtt_RealDiv2(height));
	}

	// Flag the Prepare() method to be called, which will resize the native UI control.
	Invalidate(kGeometryFlag | kStageBoundsFlag | kTransformFlag);
}

int WinDisplayObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	auto controlPointer = GetControl();
	int result = 1;

	if ( Rtt_StringCompare( "isVisible", key ) == 0 )
	{
		bool isVisible = controlPointer ? controlPointer->IsVisible() : false;
		lua_pushboolean(L, isVisible ? 1 : 0);
	}
	else if ( Rtt_StringCompare( "alpha", key ) == 0 )
	{
		lua_pushnumber(L, 1.0);
	}
	else if ( Rtt_StringCompare( "hasBackground", key ) == 0 )
	{
		lua_pushboolean(L, 1);
	}
	else
	{
		result = 0;
	}
	return result;
}

bool WinDisplayObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	auto controlPointer = GetControl();
	bool result = true;

	if ( Rtt_StringCompare( "isVisible", key ) == 0 )
	{
		if (lua_type(L, valueIndex) == LUA_TBOOLEAN)
		{
			if (controlPointer)
			{
				controlPointer->SetVisible(lua_toboolean(L, valueIndex) ? true : false);
			}
		}
	}
	else if ( Rtt_StringCompare( "alpha", key ) == 0 )
	{
		CoronaLuaWarning(L, "Native UI objects on Windows do not support the \"alpha\" property.");
	}
	else if ( Rtt_StringCompare( "hasBackground", key ) == 0 )
	{
		CoronaLuaWarning(L, "Native UI objects on Windows do not support the \"hasBackground\" property.");
	}
	else
	{
		result = false;
	}

	return result;
}

void WinDisplayObject::OnReceivedMessage(
	Interop::UI::UIComponent& sender, Interop::UI::HandleMessageEventArgs& arguments)
{
	// Do not continue if the received message was already handled by another callback.
	if (arguments.WasHandled())
	{
		return;
	}

	// Handle the received message.
	switch (arguments.GetMessageId())
	{
		case WM_CHAR:
		{
			// Block keyboard and mouse input while a "wait" cursor is enabled.
			WinInputDeviceManager& inputDeviceManager =
					(WinInputDeviceManager&)fEnvironment.GetPlatform()->GetDevice().GetInputDeviceManager();
			if (inputDeviceManager.IsWaitCursorEnabled())
			{
				arguments.SetReturnResult(0);
				arguments.SetHandled();
				break;
			}

			// Handle tabbing to other native UI controls ourselves in case the main window didn't.
			// Note: If the main window handled the tab key, then we wouldn't have received it here.
			if (VK_TAB == arguments.GetWParam())
			{
				bool isShiftDown = ((::GetKeyState(VK_LSHIFT) | ::GetKeyState(VK_RSHIFT)) & 0x80) ? true : false;
				HWND currentFocusWindowHandle = ::GetFocus();
				HWND parentWindowHandle = ::GetParent(arguments.GetWindowHandle());
				if (currentFocusWindowHandle && parentWindowHandle)
				{
					HWND nextFocusWindowHandle = ::GetNextDlgTabItem(
							parentWindowHandle, currentFocusWindowHandle, isShiftDown ? TRUE : FALSE);
					if (nextFocusWindowHandle)
					{
						::SetFocus(nextFocusWindowHandle);
					}
					arguments.SetReturnResult(0);
					arguments.SetHandled();
				}
			}
			break;
		}
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			// Block keyboard and mouse input while a "wait" cursor is enabled.
			WinInputDeviceManager& inputDeviceManager =
					(WinInputDeviceManager&)fEnvironment.GetPlatform()->GetDevice().GetInputDeviceManager();
			if (inputDeviceManager.IsWaitCursorEnabled())
			{
				arguments.SetReturnResult(0);
				arguments.SetHandled();
			}
			break;
		}
		case WM_SETCURSOR:
		{
			// Display a "wait" mouse cursor if enabled.
			WinInputDeviceManager& inputDeviceManager =
					(WinInputDeviceManager&)fEnvironment.GetPlatform()->GetDevice().GetInputDeviceManager();
			if (inputDeviceManager.IsWaitCursorEnabled())
			{
				auto cursorHandle = ::LoadCursor(nullptr, IDC_WAIT);
				if (cursorHandle)
				{
					::SetCursor(cursorHandle);
					arguments.SetReturnResult(0);
					arguments.SetHandled();
				}
			}
			break;
		}
	}
}

}	// namespace Rtt
