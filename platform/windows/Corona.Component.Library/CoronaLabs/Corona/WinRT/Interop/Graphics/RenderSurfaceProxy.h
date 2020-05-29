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

#include "IRenderSurface.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

/// <summary>Proxy which passes through all events, commands, and property accessors to an a real rendering surface.</summary>
/// <remarks>
///  <para>
///   This proxy allows a rendering surface to be hot swapped with another surface so that the owner of this proxy surface
///   does not have to worry about remove event handlers from the old surface and add event handlers to the new surface.
///  </para>
///  <para>
///   Corona uses this proxy on Windows Phone 8.0 so that its CoronPanel control can switch rendering from a
///   DrawingSurface to a DrawingSurfaceBackgroundGrid Xaml control or vice-versa dynamically.
///  </para>
/// </remarks>
public ref class RenderSurfaceProxy sealed : public IRenderSurface
{
	public:
		#pragma region Events
		/// <summary>Raised just before the render surface attempts to fetch the rendering context.</summary>
		/// <remarks>
		///  This event is raised just before every time the surface invokes its <see cref="RenderFrameHandler"/>
		///  and is the owner's opportunity to load any image or shader resources before rendering.
		///  Note that it's possible for the surface to fail to acquire the render context, in which case, this
		///  event will be followed up by a LostRenderContext event.
		/// </remarks>
		virtual event Windows::Foundation::TypedEventHandler<CoronaLabs::Corona::WinRT::Interop::Graphics::IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>^ AcquiringRenderContext;

		/// <summary>Raised when a rendering context has been received from the surface.</summary>
		/// <remarks>This event indicates that the surface is ready to be rendered to.</remarks>
		virtual event Windows::Foundation::TypedEventHandler<IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>^ ReceivedRenderContext;

		/// <summary>Raised when the rendering surface has been destroyed and its context has been lost.</summary>
		/// <remarks>
		///  This event indicates that the surface can no longer be rendered to. If a ReceivedRenderContext event
		///  is raised after this event, then the system may need to reload resources such as images and shaders.
		/// </remarks>
		virtual event Windows::Foundation::TypedEventHandler<IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>^ LostRenderContext;

		/// <summary>Raised when the surface's width and/or height has changed.</summary>
		virtual event Windows::Foundation::TypedEventHandler<IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>^ Resized;

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new proxy that does not reference rendering surface yet.</summary>
		RenderSurfaceProxy();

		#pragma endregion


		#pragma region Public Methods/Properties
		property IRenderSurface^ RenderSurface { IRenderSurface^ get(); void set(IRenderSurface^ surface); }

		/// <summary>Determines if the surface and its rendering context are ready to be rendered to.</summary>
		/// <returns>
		///  <para>Returns true if the surface has been created and is ready to be rendered to.</para>
		///  <para>Returns false if this controller does not have a surface to render to you.</para>
		/// </returns>
		virtual property bool IsReadyToRender { bool get(); }

		/// <summary>Gets the width of the surface in pixels, relative to the application's current orientations.</summary>
		/// <value>The surface's width in pixels.</value>
		virtual property int WidthInPixels { int get(); }

		/// <summary>Gets the height of the surface in pixels, relative to the application's current orientations.</summary>
		/// <value>The surface's height in pixels.</value>
		virtual property int HeightInPixels { int get(); }

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
		virtual property Corona::WinRT::Interop::RelativeOrientation2D^ Orientation
		{
			Corona::WinRT::Interop::RelativeOrientation2D^ get();
		}

		/// <summary>Requests this surface to render another frame.</summary>
		/// <remarks>
		///  <para>
		///   Calling this method will cause this controller to raise a "RequestingRenderFrame" event when the surface is ready.
		///  </para>
		///  <para>If the IsReadyToRender property is false, then calling this method will do nothing.</para>
		/// </remarks>
		virtual void RequestRender();

		/// <summary>Blocking call which attempts to force the surface to render another frame immediately.</summary>
		/// <remarks>
		///  <para>
		///   Calling this method wil cause this surface to invoke the delegate given to the SetRenderFramHandler() method
		///   once the surface is ready to have something drawn to it.
		///  </para>
		///  <para>If the IsReadyToRender property is false, then calling this method will do nothing.</para>
		/// </remarks>
		virtual void ForceRender();

		/// <summary>
		///  <para>Sets a delegate to be invoked by the surface when it is requesting a frame to be rendered to it.</para>
		///  <para>Note that this delegate will typically be invoked after a call to the RequestRender() method.</para>
		/// </summary>
		/// <param name="handler">
		///  <para>The handler to be invoked when the surface is ready to have something rendered to it.</para>
		///  <para>Can be null, in which case the surface will draw a black screen when requested to render.</para>
		/// </param>
		virtual void SetRenderFrameHandler(RenderFrameHandler^ handler);

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>Subscribes to the "fSurface" member variable's events.</summary>
		void AddEventHandlers();

		/// <summary>Unsubscribes from the "fSurface" member variable's events.</summary>
		void RemoveEventHandlers();

		/// <summary>
		///  <para>Called when this proxy's surface has raised a "AcquiringRenderContext" event.</para>
		///  <para>Relays this event to this proxy's external event handlers.</para>
		/// </summary>
		/// <param name="sender">The rendering surface that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnAcquiringRenderContext(IRenderSurface^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args);

		/// <summary>
		///  <para>Called when this proxy's surface has raised a "ReceivedRenderContext" event.</para>
		///  <para>Relays this event to this proxy's external event handlers.</para>
		/// </summary>
		/// <param name="sender">The rendering surface that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnReceivedRenderContext(IRenderSurface^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args);
		
		/// <summary>
		///  <para>Called when this proxy's surface has raised a "LostRenderContext" event.</para>
		///  <para>Relays this event to this proxy's external event handlers.</para>
		/// </summary>
		/// <param name="sender">The rendering surface that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnLostRenderContext(IRenderSurface^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args);

		/// <summary>
		///  <para>Called when this proxy's surface has raised a "Resized" event.</para>
		///  <para>Relays this event to this proxy's external event handlers.</para>
		/// </summary>
		/// <param name="sender">The rendering surface that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnResized(IRenderSurface^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>The render surface that this proxy references.</summary>
		IRenderSurface^ fSurface;

		/// <summary>
		///  <para>Stores the last orientation the surface was rendering in, relative to the app's orientation.</para>
		///  <para>This is used to detect orientation changes when switching between 2 different rendering surfaces.</para>
		/// </summary>
		Corona::WinRT::Interop::RelativeOrientation2D^ fLastOrientation;

		/// <summary>Delegate reference given to this proxy's SetRenderFrameHandler() method.</summary>
		RenderFrameHandler^ fRenderFrameHandler;

		/// <summary>Token received when adding a handler to the surface's "AcquiringRenderContext" event.</summary>
		Windows::Foundation::EventRegistrationToken fAcquiringRenderContextEventToken;

		/// <summary>Token received when adding a handler to the surface's "ReceivedRenderContext" event.</summary>
		Windows::Foundation::EventRegistrationToken fReceivedRenderContextEventToken;

		/// <summary>Token received when adding a handler to the surface's "LostRenderContext" event.</summary>
		Windows::Foundation::EventRegistrationToken fLostRenderContextEventToken;

		/// <summary>Token received when adding a handler to the surface's "Resized" event.</summary>
		Windows::Foundation::EventRegistrationToken fResizedEventToken;

		#pragma endregion
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
