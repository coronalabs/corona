//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_LinuxScreenSurface.h"

namespace Rtt
{
	#pragma region LinuxScreenSurface Class

	#pragma region Constructors/Destructors
	LinuxScreenSurface::LinuxScreenSurface()
		: fContext(NULL)
	{
	}

	LinuxScreenSurface::~LinuxScreenSurface()
	{
	}

	#pragma endregion

	#pragma region Public Member Functions
	void LinuxScreenSurface::SetCurrent() const
	{
	}

	void LinuxScreenSurface::Flush() const
	{
		fContext->Flush();
	}

	S32 LinuxScreenSurface::Width() const
	{
		return fContext->GetWidth();
	}

	S32 LinuxScreenSurface::Height() const
	{
		return fContext->GetHeight();
	}

	S32 LinuxScreenSurface::DeviceWidth() const
	{
		return (fContext->GetOrientation() == DeviceOrientation::kUpright  || fContext->GetOrientation() == DeviceOrientation::kUpsideDown) ? Width() : Height();
	}

	S32 LinuxScreenSurface::DeviceHeight() const
	{
		return (fContext->GetOrientation() == DeviceOrientation::kUpright  || fContext->GetOrientation() == DeviceOrientation::kUpsideDown) ? Height() : Width();
	}

	void LinuxScreenSurface::getWindowSize(int* w, int* h)
	{
		*w = Width();
		*h = Height();
	}

	DeviceOrientation::Type LinuxScreenSurface::GetOrientation() const
	{
		return fContext->GetOrientation();
	}

	#pragma endregion
	#pragma endregion

	#pragma region LinuxOffscreenSurface Class

	#pragma region Constructors/Destructors
	LinuxOffscreenSurface::LinuxOffscreenSurface(const PlatformSurface &parent)
		:	fWidth(parent.Width()),
		  fHeight(parent.Height())
	{
	}

	LinuxOffscreenSurface::~LinuxOffscreenSurface()
	{
	}

	#pragma endregion

	#pragma region Public Member Functions
	void LinuxOffscreenSurface::SetCurrent() const
	{
	}

	void LinuxOffscreenSurface::Flush() const
	{
	}

	S32 LinuxOffscreenSurface::Width() const
	{
		return fWidth;
	}

	S32 LinuxOffscreenSurface::Height() const
	{
		return fHeight;
	}

	#pragma endregion
	#pragma endregion
}; // namespace Rtt
