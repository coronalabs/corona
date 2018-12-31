// ----------------------------------------------------------------------------
// 
// RenderFrameHandler.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

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
