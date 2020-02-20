//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

interface class IRenderSurface;

/// <summary>Delegate to be invoked when an IRenderSurface is requesting one frame to be drawn to it.</summary>
/// <param name="surface">Reference to the surface to be rendered to.</param>
public delegate void RenderFrameHandler(IRenderSurface^ surface);

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
