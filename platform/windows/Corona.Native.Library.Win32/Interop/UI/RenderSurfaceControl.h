//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Macros.h"
#include "Interop\Event.h"
#include "Interop\HandledEventArgs.h"
#include "Control.h"
#include <memory>
#include <string>
#include <Windows.h>


namespace Interop { namespace UI {

/// <summary>Represents a control that can be rendered to via OpenGL.</summary>
class RenderSurfaceControl : public Control
{
	Rtt_CLASS_NO_COPIES(RenderSurfaceControl)

	public:
		#pragma region Public Event Types
		/// <summary>
		///  Defines a "RenderFrame" event type which is raised when this surface is requesting one frame to be drawn.
		/// </summary>
		typedef Event<RenderSurfaceControl&, HandledEventArgs&> RenderFrameEvent;

		#pragma endregion


		#pragma region Version Class
		/// <summary>
		///  Stores the major/minor version numbers of the rendering driver that is used to render to the control.
		/// </summary>
		class Version
		{
			public:
				/// <summary>Creates a new object with all version numbers initialized to zero.</summary>
				Version();

				/// <summary>Creates a new object initialized with the given major and minor version numbers.</summary>
				Version(int majorNumber, int minorNumber);

				/// <summary>Gets the rendering driver's version string. Can be null.</summary>
				/// <returns>
				///  <para>Returns the rendering driver's version string.</para>
				///  <para>Returns null or empty string if not assigned.</para>
				/// </returns>
				const char* GetString() const;

				/// <summary>Sets the rendering driver's version string.</summary>
				/// <param name="value">The version string. Can be null or empty.</param>
				void SetString(const char* value);

				/// <summary>Gets the rendering driver's "major" version number.</summary>
				/// <returns>Returns the driver's major version number. Returns zero if not assigned.</returns>
				int GetMajorNumber() const;

				/// <summary>Sets the rendering driver's "major" version number.</summary>
				/// <param name="value">The major version number.</param>
				void SetMajorNumber(int value);

				/// <summary>Gets the rendering driver's "minor" version number.</summary>
				/// <returns>Returns the driver's minor version number. Returns zero if not assigned.</returns>
				int GetMinorNumber() const;

				/// <summary>Sets the rendering driver's "minor" version number.</summary>
				/// <param name="value">The minor version number.</param>
				void SetMinorNumber(int value);

				/// <summary>Compares this object's version numbers with the given object's version numbers.</summary>
				/// <param name="version">The version object to be compared with.</param>
				/// <returns>
				///  <para>Returns a positive number if this version is greater than the given version.</para>
				///  <para>Returns zero if this version matches/equals the given version.</para>
				///  <para>Returns a negative number if this version is less than the given version.</para>
				/// </returns>
				int CompareTo(const Version& version) const;

			private:
				std::shared_ptr<std::string> fVersionString;
				int fMajorNumber;
				int fMinorNumber;
		};

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new render surface which wraps the given window handle.</summary>
		/// <param name="windowHandle">
		///  <para>Handle to a Windows control to render to.</para>
		///  <para>Can be null, but then this render surface object will do nothing.</para>
		/// </param>
		RenderSurfaceControl(HWND windowHandle);

		/// <summary>Destroys this object.</summary>
		virtual ~RenderSurfaceControl();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Determines if this control is currently able to render to its surface.</summary>
		/// <returns>
		///  <para>Returns true if this control is ready to render now.</para>
		///  <para>
		///   Returns false if this object is not referencing a control/window or if it failed
		///   to create a rendering context.
		///  </para>
		/// </returns>
		bool CanRender() const;

		/// <summary>
		///  Gets the &lt;major&gt;.&lt;minor&gt; version number of the rendering driver that is
		///  currently being used to render to this surface.
		/// </summary>
		/// <returns>
		///  <para>Returns the rendering driver's major and minor version numbers.</para>
		///  <para>
		///   Returns version numbers set to zero if this control is not referencing a window or if it
		///   failed to attach a rendering context to to it.
		///  </para>
		/// </returns>
		RenderSurfaceControl::Version GetRendererVersion() const;

		/// <summary>
		///  <para>Sets an event handler to be invoked when the surface is requesting a frame to be rendered to it.</para>
		///  <para>Note that this handler will typically be invoked after a call to the RequestRender() method.</para>
		/// </summary>
		/// <param name="handlerPointer">
		///  <para>The handler to be invoked when the surface is ready to have something rendered to it.</para>
		///  <para>Can be null, in which case the surface will draw a black screen when requested to render.</para>
		/// </param>
		void SetRenderFrameHandler(RenderFrameEvent::Handler *handlerPointer);

		/// <summary>
		///  <para>Makes this surface's rendering context the calling thread's current context.</para>
		///  <para>All subsequent rendering function calls will then be made to this surface's context.</para>
		/// </summary>
		void SelectRenderingContext();

		/// <summary>
		///  Swaps the rendering surface's back buffer with the front buffer, the last rendered content appear onscreen.
		/// </summary>
		void SwapBuffers();

		/// <summary>Requests this surface to render another frame.</summary>
		/// <remarks>
		///  Calling this method wil cause this surface to invoke the handler given to the SetRenderFramHandler()
		///  method once the surface is ready to have something drawn to it.
		/// </remarks>
		void RequestRender();

		#pragma endregion

	protected:
		/// <summary>
		///  <para>Called just after the "Destroying" event was raised.</para>
		///  <para>
		///   Performs final cleanup after the component's event handlers have performed their final operations on it.
		///  </para>
		/// </summary>
		virtual void OnRaisedDestroyingEvent();

	private:
		#pragma region FetchMultisampleFormatResult Struct
		/// <summary>Provides multisample format information returned by the FetchMultisampleFormat() method.</summary>
		struct FetchMultisampleFormatResult
		{
			/// <summary>Set true if the rendering driver/hardware supports multisampling. False if not.</summary>
			bool IsSupported;

			/// <summary>
			///  <para>
			///   Index to the best multisampling pixel format to be given to the Win32 ::SetPixelFormat() function.
			///  </para>
			///  <para>
			///   This field should be ignored if the "IsSupported" field is set false,
			///   indicating that multisampling is not supported.
			///  </para>
			/// </summary>
			int PixelFormatIndex;

			/// <summary>Creates a new result object initialized to "not supported".</summary>
			FetchMultisampleFormatResult()
			:	IsSupported(false),
				PixelFormatIndex(-1)
			{ }
		};

		#pragma endregion


		#pragma region Private Methods
		/// <summary>
		///  Determines if the rendering driver supports multisampling, and if so, provides the best pixel format.
		/// </summary>
		/// <returns>Returns the requested multisample format information.</returns>
		FetchMultisampleFormatResult FetchMultisampleFormat();

		/// <summary>
		///  <para>Creates a new rendering context for the currently referenced control.</para>
		///  <para>Will destroy the last context if still active.</para>
		/// </summary>
		void CreateContext();

		/// <summary>Destroys the last created rendering context.</summary>
		void DestroyContext();

		/// <summary>Called when a Windows message has been dispatched to this control.</summary>
		/// <param name="sender">Reference to this control.</param>
		/// <param name="arguments">
		///  <para>Provides the Windows message information.</para>
		///  <para>Call its SetHandled() and SetReturnValue() methods if this handler will be handling the message.</para>
		/// </param>
		void OnReceivedMessage(UIComponent& sender, HandleMessageEventArgs& arguments);

		/// <summary>Called when the control is requesting to have its surface re-painted.</summary>
		void OnPaint();

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Handler to be invoked when the "ReceivedMessage" event has been raised.</summary>
		UIComponent::ReceivedMessageEvent::MethodHandler<RenderSurfaceControl> fReceivedMessageEventHandler;

		/// <summary>Pointer to one "RenderFrame" event handler.</summary>
		RenderFrameEvent::Handler* fRenderFrameEventHandlerPointer;

		/// <summary>Handle to the control's main device context that OpenGL will render to.</summary>
		HDC fMainDeviceContextHandle;

		/// <summary>Handle to OpenGL's rendering context.</summary>
		HGLRC fRenderingContextHandle;

		/// <summary>Stores the major/minor version number of the OpenGL driver that is rendering to this surface.</summary>
		RenderSurfaceControl::Version fRendererVersion;

		#pragma endregion
};

} }	// namespace Interop::UI
