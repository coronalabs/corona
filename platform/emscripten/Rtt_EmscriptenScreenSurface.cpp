//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <SDL2/SDL.h>
#include "Core/Rtt_Build.h"
#include "Rtt_EmscriptenScreenSurface.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

#pragma region EmscriptenScreenSurface Class

#pragma region Constructors/Destructors
EmscriptenScreenSurface::EmscriptenScreenSurface()
	: fWindow(NULL)
	, fOrientation(DeviceOrientation::kUpright)
{
}

EmscriptenScreenSurface::~EmscriptenScreenSurface()
{
}

#pragma endregion


#pragma region Public Member Functions
void EmscriptenScreenSurface::SetCurrent() const
{
}

void EmscriptenScreenSurface::Flush() const
{
	SDL_GL_SwapWindow(fWindow);
}

S32 EmscriptenScreenSurface::Width() const
{
	if (fWindow)
	{
		int w, h;
		SDL_GetWindowSize(fWindow, &w, &h);
		return w;
	}
	Rtt_ASSERT_NOT_REACHED();
	return 0;
}

S32 EmscriptenScreenSurface::Height() const
{
	if (fWindow)
	{
		int w, h;
		SDL_GetWindowSize(fWindow, &w, &h);
		return h;
	}
	Rtt_ASSERT_NOT_REACHED();
	return 0;
}

S32 EmscriptenScreenSurface::DeviceWidth() const
{
	// Return the surface's pixel width relative to a portrait orientation.
	if (fWindow)
	{
		int w, h;
		SDL_GetWindowSize(fWindow, &w, &h);
		return (fOrientation == DeviceOrientation::kUpright  || fOrientation == DeviceOrientation::kUpsideDown) ? w : h;
	}
	Rtt_ASSERT_NOT_REACHED();
	return 0;	
}

S32 EmscriptenScreenSurface::DeviceHeight() const
{
	// Return the surface's pixel height relative to a portrait orientation.
	if (fWindow)
	{
		int w, h;
		SDL_GetWindowSize(fWindow, &w, &h);
		return (fOrientation == DeviceOrientation::kUpright  || fOrientation == DeviceOrientation::kUpsideDown) ? h : w;
	}
	Rtt_ASSERT_NOT_REACHED();
	return 0;	
}

void EmscriptenScreenSurface::getWindowSize(int* w, int* h)
{
	SDL_GetWindowSize(fWindow, w, h);
}

DeviceOrientation::Type EmscriptenScreenSurface::GetOrientation() const
{
	return fOrientation;
}


#pragma endregion

#pragma endregion


#pragma region EmscriptenOffscreenSurface Class

#pragma region Constructors/Destructors
EmscriptenOffscreenSurface::EmscriptenOffscreenSurface(const PlatformSurface& parent)
:	fWidth(parent.Width()),
	fHeight(parent.Height())
{
}

EmscriptenOffscreenSurface::~EmscriptenOffscreenSurface()
{
}

#pragma endregion


#pragma region Public Member Functions
void EmscriptenOffscreenSurface::SetCurrent() const
{
}

void EmscriptenOffscreenSurface::Flush() const
{
}

S32 EmscriptenOffscreenSurface::Width() const
{
	return fWidth;
}

S32 EmscriptenOffscreenSurface::Height() const
{
	return fHeight;
}

#pragma endregion

#pragma endregion

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
