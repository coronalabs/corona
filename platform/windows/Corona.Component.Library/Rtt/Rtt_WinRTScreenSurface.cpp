// ----------------------------------------------------------------------------
// 
// Rtt_WinRTScreenSurface.cpp
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "Rtt_WinRTScreenSurface.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\IRenderSurface.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\ICoronaControlAdapter.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\IPage.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\IUserInterfaceServices.h"
#include "CoronaLabs\Corona\WinRT\Interop\InteropServices.h"
#include "CoronaLabs\Corona\WinRT\Interop\RelativeOrientation2D.h"
#include "CoronaLabs\Corona\WinRT\CoronaRuntimeEnvironment.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core\Rtt_Build.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region WinRTScreenSurface Class

#pragma region Constructors/Destructors
WinRTScreenSurface::WinRTScreenSurface(CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ environment)
:	fEnvironment(environment),
	fRenderSurface(nullptr),
	fPage(nullptr)
{
	// Validate.
	if (nullptr == fEnvironment)
	{
		throw ref new Platform::NullReferenceException("environment");
	}

	// Fetch the Corona control's render surface and parent page.
	auto coronaControl = fEnvironment->CoronaControlAdapter;
	if (coronaControl)
	{
		fRenderSurface = coronaControl->RenderSurface;
		fPage = coronaControl->ParentPageProxy;
	}
	if (nullptr == fRenderSurface)
	{
		throw ref new Platform::NullReferenceException("renderSurface");
	}
	if (nullptr == fPage)
	{
		throw ref new Platform::NullReferenceException("page");
	}
}

WinRTScreenSurface::~WinRTScreenSurface()
{
}

#pragma endregion


#pragma region Public Member Functions
void WinRTScreenSurface::SetCurrent() const
{
}

void WinRTScreenSurface::Flush() const
{
}

S32 WinRTScreenSurface::Width() const
{
	// Return a pixel width relative to the surface's current orientation.
	S32 portraitWidth;
	if (fRenderSurface->Orientation->IsSideways)
	{
		portraitWidth = fRenderSurface->HeightInPixels;
	}
	else
	{
		portraitWidth = fRenderSurface->WidthInPixels;
	}
	return portraitWidth;
}

S32 WinRTScreenSurface::Height() const
{
	// Return a pixel height relative to the surface's current orientation.
	S32 portraitHeight;
	if (fRenderSurface->Orientation->IsSideways)
	{
		portraitHeight = fRenderSurface->WidthInPixels;
	}
	else
	{
		portraitHeight = fRenderSurface->HeightInPixels;
	}
	return portraitHeight;
}

DeviceOrientation::Type WinRTScreenSurface::GetOrientation() const
{
	// Return an orientation relative to portrait.
	auto portraitOrientation = CoronaLabs::Corona::WinRT::Interop::UI::PageOrientation::PortraitUpright;
	auto relativePageOrientation = fPage->Orientation->ToOrientationRelativeTo(portraitOrientation);
	auto degreesClockwiseFromPortrait =
			(relativePageOrientation->DegreesClockwise + fRenderSurface->Orientation->DegreesClockwise) % 360;
	return Rtt::DeviceOrientation::OrientationForAngle(degreesClockwiseFromPortrait);
}

S32 WinRTScreenSurface::DeviceWidth() const
{
	// Return the surface's pixel width relative to a portrait orientation.
	S32 portraitWidth;
	if (fPage->Orientation->IsLandscape)
	{
		portraitWidth = fRenderSurface->HeightInPixels;
	}
	else
	{
		portraitWidth = fRenderSurface->WidthInPixels;
	}
	return portraitWidth;
}

S32 WinRTScreenSurface::DeviceHeight() const
{
	// Return the surface's pixel height relative to a portrait orientation.
	S32 portraitHeight;
	if (fPage->Orientation->IsLandscape)
	{
		portraitHeight = fRenderSurface->WidthInPixels;
	}
	else
	{
		portraitHeight = fRenderSurface->HeightInPixels;
	}
	return portraitHeight;
}

S32 WinRTScreenSurface::AdaptiveWidth() const
{
//TODO: Need to re-work this to return a scaled length relative to iOS' base resolution of 320x480 and 163 DPI.
#if 0
	// Calculate a standard scaled "width" based on device resolution and DPI.
	// Microsoft recommends the following resolutions on Windows Phone:
	// |-----------------|------------------|-------------------|
	// | Resolution Name | Pixel Resolution | Scaled Resolution |
	// |-----------------|------------------|-------------------|
	// | WVGA            | 480x800          | 480x800           |
	// | WXGA            | 768x1280         | 480x800           |
	// | 720P            | 720x1280         | 480x853           |
	// | 1080P           | 1080x1920        | 480x853           |
	// |-----------------|------------------|-------------------|

	// Attempt to use the system's recommended scale, if provided.
	auto userInterfaceServices = fEnvironment->InteropServices->UserInterfaceServices;
	if (userInterfaceServices)
	{
		double scaleFactor = userInterfaceServices->ScaleFactor;
		if (scaleFactor > 0)
		{
			return (S32)(((double)DeviceWidth() / scaleFactor) + 0.5);
		}
	}

	// Could not find the system information needed to calculate a good scale.
	// So, just return an unscaled pixel width instead.
	return DeviceWidth();
#else
	Rtt_LogException("The \"adaptive\" content scaling feature is not currently supported on WP8.");
	return DeviceWidth();
#endif
}

S32 WinRTScreenSurface::AdaptiveHeight() const
{
//TODO: Need to re-work this to return a scaled length relative to iOS' base resolution of 320x480 and 163 DPI.
#if 0
	// Calculate a standard scaled "height" based on device resolution and DPI.
	// Microsoft recommends the following resolutions on Windows Phone:
	// |-----------------|------------------|-------------------|
	// | Resolution Name | Pixel Resolution | Scaled Resolution |
	// |-----------------|------------------|-------------------|
	// | WVGA            | 480x800          | 480x800           |
	// | WXGA            | 768x1280         | 480x800           |
	// | 720P            | 720x1280         | 480x853           |
	// | 1080P           | 1080x1920        | 480x853           |
	// |-----------------|------------------|-------------------|

	// Attempt to use the system's recommended scale, if provided.
	auto userInterfaceServices = fEnvironment->InteropServices->UserInterfaceServices;
	if (userInterfaceServices)
	{
		double scaleFactor = userInterfaceServices->ScaleFactor;
		if (scaleFactor > 0)
		{
			return (S32)(((double)DeviceHeight() / scaleFactor) + 0.5);
		}
	}

	// Could not find the system information needed to calculate a good scale.
	// So, just return an unscaled pixel height instead.
	return DeviceHeight();
#else
	return DeviceHeight();
#endif
}

#pragma endregion

#pragma endregion


#pragma region WinRTOffscreenSurface Class

#pragma region Constructors/Destructors
WinRTOffscreenSurface::WinRTOffscreenSurface(const PlatformSurface& parent)
:	fWidth(parent.Width()),
	fHeight(parent.Height())
{
}

WinRTOffscreenSurface::~WinRTOffscreenSurface()
{
}

#pragma endregion


#pragma region Public Member Functions
void WinRTOffscreenSurface::SetCurrent() const
{
}

void WinRTOffscreenSurface::Flush() const
{
}

S32 WinRTOffscreenSurface::Width() const
{
	return fWidth;
}

S32 WinRTOffscreenSurface::Height() const
{
	return fHeight;
}

#pragma endregion

#pragma endregion

} // namespace Rtt
