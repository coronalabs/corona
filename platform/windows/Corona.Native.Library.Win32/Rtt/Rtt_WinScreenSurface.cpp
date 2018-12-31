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
#include "Rtt_WinScreenSurface.h"
#include "Core\Rtt_Build.h"
#include "Interop\UI\RenderSurfaceControl.h"
#include "Interop\UI\Window.h"
#include "Interop\MDeviceSimulatorServices.h"
#include "Interop\RuntimeEnvironment.h"
#include "Rtt_NativeWindowMode.h"


namespace Rtt
{

WinScreenSurface::WinScreenSurface(Interop::RuntimeEnvironment& environment)
:	Super(),
	fEnvironment(environment),
	fPreviousClientWidth(0),
	fPreviousClientHeight(0)
{
}

WinScreenSurface::~WinScreenSurface()
{
}

DeviceOrientation::Type WinScreenSurface::GetOrientation() const
{
	// If we're simulating a device, then use the current simulated orientation.
	auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
	if (deviceSimulatorServicesPointer)
	{
		return deviceSimulatorServicesPointer->GetOrientation();
	}

	// Use the Corona project's default orientation, if provided.
	auto orientation = fEnvironment.GetProjectSettings().GetDefaultOrientation();
	if (Rtt::DeviceOrientation::IsInterfaceOrientation(orientation))
	{
		return orientation;
	}

	// If all else fails, default to an upright orientation.
	return Rtt::DeviceOrientation::kUpright;
}

void WinScreenSurface::SetCurrent() const
{
	auto surfaceControlPointer = fEnvironment.GetRenderSurface();
	if (surfaceControlPointer)
	{
		surfaceControlPointer->SelectRenderingContext();
	}
}

void WinScreenSurface::Flush() const
{
	auto surfaceControlPointer = fEnvironment.GetRenderSurface();
	if (surfaceControlPointer)
	{
		surfaceControlPointer->SwapBuffers();
	}
}

S32 WinScreenSurface::Width() const
{
	// Return zero if we do not have a surface to render to.
	auto renderSurfacePointer = fEnvironment.GetRenderSurface();
	if (!renderSurfacePointer)
	{
		return 0;
	}

	// Fetch the surface's client width in pixels.
	int length = 0;
	auto windowPointer = fEnvironment.GetMainWindow();
	if (windowPointer && windowPointer->GetWindowMode().Equals(Rtt::NativeWindowMode::kMinimized))
	{
		// The window hosting the surface has been minimized, causing the client area to have a zero width/height.
		// Use the client length before it was minimized to avoid triggering an unnecessary "resize" event in Corona.
		length = fPreviousClientWidth;
	}
	else
	{
		// Fetch the requested length and store it in case the window gets minimized later.
		length = renderSurfacePointer->GetClientWidth();
		fPreviousClientWidth = length;
	}

	// Corona's rendering system will assert if given a zero length. So, floor it to 1.
	if (length <= 0)
	{
		length = 1;
	}

	// Return a pixel width relative to the surface's current orientation.
	return length;
}

S32 WinScreenSurface::Height() const
{
	// Return zero if we do not have a surface to render to.
	auto renderSurfacePointer = fEnvironment.GetRenderSurface();
	if (!renderSurfacePointer)
	{
		return 0;
	}

	// Fetch the surface's client height in pixels.
	int length = 0;
	auto windowPointer = fEnvironment.GetMainWindow();
	if (windowPointer && windowPointer->GetWindowMode().Equals(Rtt::NativeWindowMode::kMinimized))
	{
		// The window hosting the surface has been minimized, causing the client area to have a zero width/height.
		// Use the client length before it was minimized to avoid triggering an unnecessary "resize" event in Corona.
		length = fPreviousClientHeight;
	}
	else
	{
		// Fetch the requested length and store it in case the window gets minimized later.
		length = renderSurfacePointer->GetClientHeight();
		fPreviousClientHeight = length;
	}

	// Corona's rendering system will assert if given a zero length. So, floor it to 1.
	if (length <= 0)
	{
		length = 1;
	}

	// Return a pixel height relative to the surface's current orientation.
	return length;
}

S32 WinScreenSurface::DeviceWidth() const
{
	// Return the surface's pixel width relative to a portrait orientation.
	S32 length;
	auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
	if (deviceSimulatorServicesPointer)
	{
		length = deviceSimulatorServicesPointer->GetScreenWidthInPixels();
	}
	else if (Rtt::DeviceOrientation::IsSideways(GetOrientation()))
	{
		length = Height();
	}
	else
	{
		length = Width();
	}
	return length;
}

S32 WinScreenSurface::DeviceHeight() const
{
	// Return the surface's pixel height relative to a portrait orientation.
	S32 length;
	auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
	if (deviceSimulatorServicesPointer)
	{
		length = deviceSimulatorServicesPointer->GetScreenHeightInPixels();
	}
	else if (Rtt::DeviceOrientation::IsSideways(GetOrientation()))
	{
		length = Width();
	}
	else
	{
		length = Height();
	}
	return length;
}

S32 WinScreenSurface::AdaptiveWidth() const
{
	// If we're simulating a device, then use its hard coded adaptive length.
	auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
	if (deviceSimulatorServicesPointer)
	{
		return deviceSimulatorServicesPointer->GetAdaptiveScreenWidthInPixels();
	}

	// Return an adaptive length for a windows desktop app.
	return Super::AdaptiveWidth();
}

S32 WinScreenSurface::AdaptiveHeight() const
{
	// If we're simulating a device, then use its hard coded adaptive length.
	auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
	if (deviceSimulatorServicesPointer)
	{
		return deviceSimulatorServicesPointer->GetAdaptiveScreenHeightInPixels();
	}

	// Return an adaptive length for a windows desktop app.
	return Super::AdaptiveHeight();
}

}	// namespace Rtt
