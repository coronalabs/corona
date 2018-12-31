// ----------------------------------------------------------------------------
// 
// IRenderSurface.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif

#include "CoronaLabs\Corona\WinRT\Interop\UI\PageProxy.h"
#include "CoronaLabs\Corona\WinRT\Interop\RelativeOrientation2D.h"
#include "CoronaLabs\WinRT\EmptyEventArgs.h"
#include "RenderFrameHandler.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

/// <summary>Interface used to manage rendering to a Direct3D surface.</summary>
public interface class IRenderSurface
{
	#pragma region Events
	/// <summary>Raised just before the render surface attempts to fetch the rendering context.</summary>
	/// <remarks>
	///  This event is raised just before every time the surface invokes its <see cref="RenderFrameHandler"/>
	///  and is the owner's opportunity to load any image or shader resources before rendering.
	///  Note that it's possible for the surface to fail to acquire the render context, in which case, this
	///  event will be followed up by a LostRenderContext event.
	/// </remarks>
	event Windows::Foundation::TypedEventHandler<IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>^ AcquiringRenderContext;

	/// <summary>Raised when a rendering context has been received from the surface.</summary>
	/// <remarks>This event indicates that the surface is ready to be rendered to.</remarks>
	event Windows::Foundation::TypedEventHandler<IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>^ ReceivedRenderContext;

	/// <summary>Raised when the rendering surface has been destroyed and its context has been lost.</summary>
	/// <remarks>
	///  This event indicates that the surface can no longer be rendered to. If a ReceivedRenderContext event
	///  is raised after this event, then the system may need to reload resources such as images and shaders.
	/// </remarks>
	event Windows::Foundation::TypedEventHandler<IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>^ LostRenderContext;

	/// <summary>Raised when the surface's width and/or height has changed.</summary>
	event Windows::Foundation::TypedEventHandler<IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>^ Resized;

	#pragma endregion


	#pragma region Functions/Properties
	/// <summary>Determines if the surface and its rendering context are ready to be rendered to.</summary>
	/// <returns>
	///  <para>Returns true if the surface has been created and is ready to be rendered to.</para>
	///  <para>Returns false if this controller does not have a surface to render to you.</para>
	/// </returns>
	property bool IsReadyToRender { bool get(); }

	/// <summary>Gets the width of the surface in pixels, relative to the application's current orientations.</summary>
	/// <value>The surface's width in pixels.</value>
	property int WidthInPixels { int get(); }

	/// <summary>Gets the height of the surface in pixels, relative to the application's current orientations.</summary>
	/// <value>The surface's height in pixels.</value>
	property int HeightInPixels { int get(); }

	/// <summary>Gets the surface's orientation relative to the application's orientation.</summary>
	/// <remarks>
	///  <para>For a Xaml DrawingSurface control, this will always be "upright".</para>
	///  <para>
	///   For a Xaml DrawingSurfaceBackgroundGrid control, the surface always renders its content in a fixed portrait
	///   orientation, relative to the upright position of the hardware. So, this property will indicate the
	///   orientation relative to the app, such as sidweways left/right when the app is displayed landscape.
	///   The rendering system is then expected to rotate its content relative to the surface orientation.
	///  </para>
	/// </remarks>
	/// <value>
	///  <para>The orientatation the surface draws its content in, relative to the application's current orientation.</para>
	///  <para>
	///   An "upright" orientation indicates that the surface's content is displayed in the same relative orientation as
	///   the application. This means that the rendered content does not need to be rotated.
	///  </para>
	///  <para>
	///   Any other orientation such as sideways right/left or upside down indicates that the rendering system needs to
	///   rotate its content to be relative to the application's orientation.
	///  </para>
	/// </value>
	property Corona::WinRT::Interop::RelativeOrientation2D^ Orientation
	{
		Corona::WinRT::Interop::RelativeOrientation2D^ get();
	}

	/// <summary>Requests this surface to render another frame.</summary>
	/// <remarks>
	///  <para>
	///   Calling this method wil cause this surface to invoke the delegate given to the SetRenderFramHandler() method
	///   once the surface is ready to have something drawn to it.
	///  </para>
	///  <para>If the IsReadyToRender property is false, then calling this method will do nothing.</para>
	/// </remarks>
	void RequestRender();

	/// <summary>Blocking call which attempts to force the surface to render another frame immediately.</summary>
	/// <remarks>
	///  <para>
	///   Calling this method wil cause this surface to invoke the delegate given to the SetRenderFramHandler() method
	///   once the surface is ready to have something drawn to it.
	///  </para>
	///  <para>If the IsReadyToRender property is false, then calling this method will do nothing.</para>
	/// </remarks>
	void ForceRender();

	/// <summary>
	///  <para>Sets a delegate to be invoked by the surface when it is requesting a frame to be rendered to it.</para>
	///  <para>Note that this delegate will typically be invoked after a call to the RequestRender() method.</para>
	/// </summary>
	/// <param name="handler">
	///  <para>The handler to be invoked when the surface is ready to have something rendered to it.</para>
	///  <para>Can be null, in which case the surface will draw a black screen when requested to render.</para>
	/// </param>
	void SetRenderFrameHandler(RenderFrameHandler^ handler);

	#pragma endregion
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
