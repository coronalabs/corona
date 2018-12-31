// ----------------------------------------------------------------------------
// 
// IXamlRenderSurfaceAdapter.h
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

#include "CoronaLabs\Corona\WinRT\Interop\RelativeOrientation2D.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\IDispatcher.h"
#include "CoronaLabs\WinRT\EmptyEventArgs.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Phone { namespace Interop { namespace Graphics {

/// <summary>
///  <para>Interface to a Xaml DrawingSurface or DrawingSurfaceBackgroundGrid Direct3D control.</para>
///  <para>
///   An instance of this class is intended to be given to a C++ Direct3DSurfaceAdapter object so that it can
///   acquire the Xaml control's size and orientation from the .NET side of the application.
///  </para>
/// </summary>
[Windows::Foundation::Metadata::WebHostHidden]
public interface class IXamlRenderSurfaceAdapter
{
	/// <summary>Raised when the surface's width and/or height has changed.</summary>
	event Windows::Foundation::TypedEventHandler<IXamlRenderSurfaceAdapter^, CoronaLabs::WinRT::EmptyEventArgs^>^ Resized;

	/// <summary>Gets a dispatcher used to queue operations to be executed on the main UI thread.</summary>
	/// <value>The main UI thread dispatcher this control is associated with.</value>
	property Corona::WinRT::Interop::UI::IDispatcher^ Dispatcher { Corona::WinRT::Interop::UI::IDispatcher^ get(); }

	/// <summary>
	///  Determines if this is a background surface which renders fullscreen or if the surface is rendering
	///  to a resizable control.
	/// </summary>
	/// <value>
	///  <para>Returns true if this is a background surface. This type of surface has the best performance.</para>
	///  <para>
	///   Returns false if this is a resizable surface. This has the worst performance because it renders to a texture.
	///  </para>
	/// </value>
	property bool IsRenderingToBackground { bool get(); }

	/// <summary>Gets the width of the surface in pixels, relative to the application's orientations.</summary>
	/// <value>The surface's width in pixels.</value>
	property int WidthInPixels { int get(); }

	/// <summary>Gets the height of the surface in pixels, relative to the application's orientations.</summary>
	/// <value>The surface's height in pixels.</value>
	property int HeightInPixels { int get(); }

	/// <summary>Gets the surface's orientation relative to the application's orientation.</summary>
	/// <remarks>
	///  <para>For a Xaml DrawingSurface control, this will always be "upright".</para>
	///  <para>
	///   For a Xaml DrawingSurfaceBackgroundGrid control, the surface always renders its content in a fixed portrait
	///   orientation, relative to the upright position of the hardware. So, this property will indicate the relative
	///   orientation in relative to the app, such as sidweways left/right when the app is displayed in landscape form.
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
};

} } } } } }	// namespace CoronaLabs::Corona::WinRT::Phone::Interop::Graphics
