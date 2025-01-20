//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoronaLabs\Corona\WinRT\Interop\Graphics\IRenderSurface.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\RenderFrameHandler.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\IDispatcher.h"
#include "CoronaLabs\WinRT\NativeThreadBlocker.h"
#include "IXamlRenderSurfaceAdapter.h"
#include <atomic>
#include <windows.h>
#include <wrl/module.h>
#include <Windows.Phone.Graphics.Interop.h>
#include <DrawingSurfaceNative.h>
#include <d3d11_1.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <../external/Angle/Project/src/common/winrtangle.h>


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Phone { namespace Interop { namespace Graphics {

/// <summary>Rendering surface adapter implementation for Windows Phone 8 and Direct3D.</summary>
[Windows::Foundation::Metadata::WebHostHidden]
public ref class Direct3DSurfaceAdapter sealed
:	public Windows::Phone::Input::Interop::IDrawingSurfaceManipulationHandler,
	public CoronaLabs::Corona::WinRT::Interop::Graphics::IRenderSurface
{
	private:
		#pragma region Private Destructor
		/// <summary>Releases the resources consumed by this adapter.</summary>
		~Direct3DSurfaceAdapter();

		#pragma endregion

	public:
		#pragma region Events
		/// <summary>Raised just before the render surface attempts to fetch the rendering context.</summary>
		/// <remarks>
		///  This event is raised just before every time the surface invokes its
		///  <see cref="CoronaLabs::Corona::WinRT::Interop::Graphics::RenderFrameHandler"/>
		///  and is the owner's opportunity to load any image or shader resources before rendering.
		///  Note that it's possible for the surface to fail to acquire the render context, in which case, this
		///  event will be followed up by a LostRenderContext event.
		/// </remarks>
		virtual event Windows::Foundation::TypedEventHandler<CoronaLabs::Corona::WinRT::Interop::Graphics::IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>^ AcquiringRenderContext;

		/// <summary>Raised when a rendering context has been received from the surface.</summary>
		/// <remarks>This event indicates that the surface is ready to be rendered to.</remarks>
		virtual event Windows::Foundation::TypedEventHandler<CoronaLabs::Corona::WinRT::Interop::Graphics::IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>^ ReceivedRenderContext;

		/// <summary>Raised when the rendering surface has been destroyed and its context has been lost.</summary>
		/// <remarks>
		///  This event indicates that the surface can no longer be rendered to. If a ReceivedRenderContext event
		///  is raised after this event, then the system may need to reload resources such as images and shaders.
		/// </remarks>
		virtual event Windows::Foundation::TypedEventHandler<CoronaLabs::Corona::WinRT::Interop::Graphics::IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>^ LostRenderContext;

		/// <summary>Raised when the surface's width and/or height has changed.</summary>
		virtual event Windows::Foundation::TypedEventHandler<CoronaLabs::Corona::WinRT::Interop::Graphics::IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>^ Resized;

		#pragma endregion


		#pragma region Constructors
		/// <summary>Creates a new adapter used to manage and render to a Direct3D surface.</summary>
		/// <param name="xamlSurfaceAdapter">
		///  <para>Adapter referencing either a Xaml "DrawingSurface" or "DrawingSurfaceBackgroundGrid" control.</para>
		///  <para>The given adapter is needed to provide the Xaml surface's current pixel width, height, and orientation.</para>
		///  <para>
		///   This class keeps a weak reference to the given adapter.
		///   So, the caller is responsible for keeping the given adapter reference alive.
		///  </para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		Direct3DSurfaceAdapter(IXamlRenderSurfaceAdapter^ xamlSurfaceAdapter);

		#pragma endregion


		#pragma region Public Methods/Properties
		/// <summary>Provides access to a Xaml surface's touch/mouse events.</summary>
		/// <remarks>
		///  You should not call this method directly.
		///  This method is only expected to be called by Microsoft's "DrawingSurface" or "DrawingSurfaceBackgroundGrid"
		///  Xaml controls after passing this adapter to them via their SetManipulationHandler() method.
		/// </remarks>
		/// <param name="manipulationHost">
		///  Object which provides the touch/mouse events that have occurred on the Direct3D surface.
		/// </param>
		virtual void SetManipulationHost(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ manipulationHost);

		/// <summary>Gets an object used to provide content to a Direct3D surface.</summary>
		/// <remarks>
		///  This object is intended to be passed to a "DrawingSurface" or "DrawingSurfaceBackgroundGrid"
		///  Xaml control's SetContentProvider() method. It allows this adapter to render content
		///  to the Direct3D surface.
		/// </remarks>
		/// <value>Object used to render content to a Direct3D surface.</value>
		property Platform::Object^ ContentProvider { Platform::Object^ get(); }

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
		///   Calling this method wil cause this surface to invoke the delegate given to the SetRenderFramHandler() method
		///   once the surface is ready to have something drawn to it.
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
		virtual void SetRenderFrameHandler(Corona::WinRT::Interop::Graphics::RenderFrameHandler^ handler);

		#pragma endregion


		#pragma region Static Functions
		/// <summary>Determines if Corona is currently blocking the Direct3D rendering thread.</summary>
		/// <value>
		///  <para>
		///   Set to true if Corona is currently blocking the Direct3D thread. In this case, you should avoid calling
		///   any Xaml class' methods and properties to avoid deadlock which will hang the application. This is because
		///   the Xaml framework tends to block the main UI thread until it obtains access to the Direct3D thread.
		///  </para>
		///  <para>
		///   Set to false if Corona is not currently synchronized with the Direct3D thread, meaning that the
		///   Direct3D thread is not currently blocked by Corona.
		///  </para>
		/// </value>
		static property bool IsSynchronizedWithRenderingThread { bool get(); }

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>
		///  <para>Attempts to acquire a Direct3D/OpenGL context (if changed) and render content to the surface.</para>
		///  <para>This method must be called on the main UI thread.</para>
		/// </summary>
		/// <param name="isRequestingRender">
		///  <para>Set to true to force this adapter to render content to the surface.</para>
		///  <para>Set to false to not render content. In which case, only a context will be acquired, if changed.</para>
		/// </param>
		/// <returns>
		///  <para>Returns true if successfully updated the render thread and its surface.</para>
		///  <para>Returns false if unable to access the rendering thread or if the surface is currently unavailable.</para>
		/// </returns>
		bool UpdateRenderThread(bool isRequestingRender);

		/// <summary>
		///  <para>Attempts to acquire a Direct3D/OpenGL context (if changed) and render content to the surface.</para>
		///  <para>This method must be called on the main UI thread.</para>
		/// </summary>
		/// <param name="isRequestingRender">
		///  <para>Set to true to force this adapter to render content to the surface.</para>
		///  <para>Set to false to not render content. In which case, only a context will be acquired, if changed.</para>
		/// </param>
		/// <param name="timeoutInMilliseconds">
		///  Maximum amount of time to wait for the rendering thread in milliseconds before giving up.
		/// </param>
		/// <returns>
		///  <para>Returns true if successfully updated the render thread and its surface.</para>
		///  <para>Returns false if unable to access the rendering thread or if the surface is currently unavailable.</para>
		/// </returns>
		bool UpdateRenderThread(bool isRequestingRender, const std::chrono::milliseconds &timeoutInMilliseconds);

		/// <summary>Called by the IXamlRenderSurfaceAdapter when the surface has changed width/height.</summary>
		/// <param name="sender">The Xaml surface adapter that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnSurfaceResized(IXamlRenderSurfaceAdapter^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args);

		/// <summary>Called on the main UI thread when its time to attempt to render.</summary>
		void OnRender();

		/// <summary>
		///  <para>Called when the content provider has successfully connected with the surface.</para>
		///  <para>This surface adapter's IsReadyToRender property will return true when this is called.</para>
		/// </summary>
		/// <param name="host">The surface's host which allows this adapter to request it to render.</param>
		void OnDirect3DHostReceived(_In_ IDrawingSurfaceRuntimeHostNative* host);

		/// <summary>
		///  <para>Called when the content provider has lost its connection with the surface.</para>
		///  <para>This surface adapter's IsReadyToRender property will return false when this is called.</para>
		/// </summary>
		void OnDirect3DHostLost();

		/// <summary>
		///  <para>To be called when a new Direct3D/OpenGL context has been received.</para>
		///  <para>This function must be called on the main UI thread.</para>
		/// </summary>
		void OnDirect3DNewContextReceived();

		/// <summary>
		///  <para>To be called when a new Direct3D/OpenGL context has been received.</para>
		///  <para>This function must be called on the Direct3D thread.</para>
		/// </summary>
		void OnDirect3DThreadNewContextReceived();

		/// <summary>
		///  <para>To be called when a new Direct3D context has been received.</para>
		///  <para>Will create a new OpenGL context attached to the given Direct3D context.</para>
		/// </summary>
		/// <param name="device">Direct3D device the surface is attached to.</param>
		/// <param name="context">The new Direct3D context that has been received from the system.</param>
		/// <param name="renderTargetView">The view to be rendered to.</param>
		/// <returns>
		///  <para>Returns S_OK if the OpenGL context was successfully created using the given Direct3D context.</para>
		///  <para>Returns an error result if failed to create an OpenGL context.</para>
		/// </returns>
		HRESULT OnDirect3DThreadAcquiredContext(
					_In_ ID3D11Device1* device, _In_ ID3D11DeviceContext1* context,
					_In_ ID3D11RenderTargetView* renderTargetView);
		
		/// <summary>Creates a new OpenGL context for the currently selected Direct3D context, if necessary.</summary>
		/// <returns>
		///  <para>Returns S_OK if the OpenGL context was successfully created using the given Direct3D context.</para>
		///  <para>Returns an error result if failed to create an OpenGL context.</para>
		/// </returns>
		HRESULT UpdateOpenGLContext();

		/// <summary>
		///  <para>
		///   Creates a new OpenGL surface and assigns it to member variable "fEglSurface" for the current
		///   Direct3D render target and OpenGL context, if necessary.
		///  </para>
		///  <para>This method is expected to be called after calling the UpdateOpenGLContext() method.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns S_OK if the OpenGL surface was created/updated successfully.</para>
		///  <para>Returns an error result if failed to create the OpenGL surface.</para>
		/// </returns>
		HRESULT UpdateOpenGLSurface();

		/// <summary>
		///  <para>Destroys the OpenGL context that was created via the UpdateOpenGLContext() method.</para>
		///  <para>It is okay to call this method if an OpenGL context was not created or already destroyed.</para>
		/// </summary>
		void DestroyOpenGLContext();

		#pragma endregion


		#pragma region RenderThreadTask Enum
		/// <summary>Indicates the current task the main UI thread is imposing on the Direct3D rendering thread.</summary>
		enum RenderThreadTask
		{
			/// <summary>Indicates that the main UI thread does not have a task for the rendering thread.</summary>
			kRenderThreadTask_None,

			/// <summary>
			///  Indicates that the main UI thread is requesting the rendering thread to block and wait for further
			///  insructions/tasks from the main thread.
			/// </summary>
			kRenderThreadTask_Wait,

			/// <summary>
			///  Indicates that the main UI thread is requesting the rendering thread to obtain a Direct3D/OpenGL context.
			/// </summary>
			kRenderThreadTask_AcquireContext,

			/// <summary>
			///  Indicates that the main UI thread has queued Direct3D operations onto the "fDirect3DDeferredCommandList"
			///  member variable and is requesting the rendering thread to execute them.
			/// </summary>
			kRenderThreadTask_ExecuteDeferredCommands
		};

		#pragma endregion


		#pragma region BaseContentProvider Class
		/// <summary>
		///  <para>Base class which the BackgroundContentProvider and SurfaceContentProvider derive from.</para>
		///  <para>Used to store a weak reference and provide access to the surface adapter that owns the content provider.</para>
		/// </summary>
		class BaseContentProvider
		{
			public:
				/// <summary>Creates a new content provider.</summary>
				/// <param name="surfaceAdapter">
				///  <para>Reference to the surface adapter that owns this content provider.</para>
				///  <para>Cannot be null or else an exception will be thrown.</para>
				/// </param>
				BaseContentProvider(Direct3DSurfaceAdapter^ surfaceAdapter);

				/// <summary>Gets the surface adapter that owns this content provider.</summary>
				/// <returns>
				///  <para>Returns a reference to the surface adapter that owns this content provider.</para>
				///  <para>Returns null if the surface adapter is no longer available (ie: garbage collected).</para>
				/// </returns>
				Direct3DSurfaceAdapter^ GetSurfaceAdapter();

			private:
				/// <summary>Weak reference to a Direct3DSurfaceAdapter object that owns this content provider.</summary>
				Platform::WeakReference fSurfaceAdapterWeakReference;
		};
		#pragma endregion


		#pragma region BackgroundContentProvider Class
		/// <summary>Content provider for a "DrawingSurfaceBackgroundGrid" Xaml control.</summary>
		class BackgroundContentProvider
		:	public BaseContentProvider,
			public Microsoft::WRL::RuntimeClass<
				Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
				ABI::Windows::Phone::Graphics::Interop::IDrawingSurfaceBackgroundContentProvider,
				IDrawingSurfaceBackgroundContentProviderNative>
		{
			public:
				/// <summary>Creates a new content provider.</summary>
				/// <param name="surfaceAdapter">
				///  <para>Reference to the surface adapter that owns this content provider.</para>
				///  <para>Cannot be null or else an exception will be thrown.</para>
				/// </param>
				BackgroundContentProvider(Direct3DSurfaceAdapter^ surfaceAdapter);

				/// <summary>
				///  Called by Microsoft's Direct3D system when this content provider has established a connection
				///  with a rendering surface.
				/// </summary>
				/// <param name="host">Host used to request the surface to render a frame.</param>
				/// <param name="device">Direct3D device adapter which provides the Direct3D context.</param>
				/// <returns>Returns S_OK if successful. Returns an error result if failed.</returns>
				HRESULT STDMETHODCALLTYPE Connect(_In_ IDrawingSurfaceRuntimeHostNative* host, _In_ ID3D11Device1* device);

				/// <summary>
				///   Called by Microsoft's Direct3D system when the content provider can no longer render to the surface.
				/// </summary>
				/// <remarks>
				///  <para>A disconnect can happen for the following reasons:</para>
				///  <list type="number">
				///   <item><description>The content provider has been removed from the surface.</description></item>
				///   <item><description>The surface's Xaml control has been removed from the page.</description></item>
				///   <item><description>The application has been suspended.</description></item>
				///  </list>
				/// </remarks>
				void STDMETHODCALLTYPE Disconnect();

				/// <summary>
				///  Called by Microsoft's Direct3D system when asking this content provider to update its content.
				/// </summary>
				/// <param name="presentTargetTime">Timestamp when this function has been called.</param>
				/// <param name="desiredRenderTargetSize">The width and height of the surface.</param>
				/// <returns>Returns S_OK if successful. Returns an error result if failed.</returns>
				HRESULT STDMETHODCALLTYPE PrepareResources(_In_ const LARGE_INTEGER* presentTargetTime, _Inout_ DrawingSurfaceSizeF* desiredRenderTargetSize);

				/// <summary>
				///  Called by Microsoft's Direct3D system when requesting this content provider to render its content.
				/// </summary>
				/// <param name="device">The Direct3D device the surface is associated with.</param>
				/// <param name="context">The Direct3D context needed to render content.</param>
				/// <param name="renderTargetView">The view to be rendered to.</param>
				/// <returns>Returns S_OK if successful. Returns an error result if failed.</returns>
				HRESULT STDMETHODCALLTYPE Draw(_In_ ID3D11Device1* device, _In_ ID3D11DeviceContext1* context, _In_ ID3D11RenderTargetView* renderTargetView);
		};

		#pragma endregion


		#pragma region SurfaceContentProvider Class
		/// <summary>Content provider for a "DrawingSurface" Xaml control.</summary>
		class SurfaceContentProvider
		:	public BaseContentProvider,
			public Microsoft::WRL::RuntimeClass<
				Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::WinRtClassicComMix>,
				ABI::Windows::Phone::Graphics::Interop::IDrawingSurfaceContentProvider,
				IDrawingSurfaceContentProviderNative>
		{
			public:
				/// <summary>Creates a new content provider.</summary>
				/// <param name="surfaceAdapter">
				///  <para>Reference to the surface adapter that owns this content provider.</para>
				///  <para>Cannot be null or else an exception will be thrown.</para>
				/// </param>
				SurfaceContentProvider(Direct3DSurfaceAdapter^ surfaceAdapter);

				/// <summary>
				///  Called by Microsoft's Direct3D system when this content provider has established a connection
				///  with a rendering surface.
				/// </summary>
				/// <param name="host">Host used to request the surface to render a frame.</param>
				/// <returns>Returns S_OK if successful. Returns an error result if failed.</returns>
				HRESULT STDMETHODCALLTYPE Connect(_In_ IDrawingSurfaceRuntimeHostNative* host);

				/// <summary>
				///   Called by Microsoft's Direct3D system when the content provider can no longer render to the surface.
				/// </summary>
				/// <remarks>
				///  <para>A disconnect can happen for the following reasons:</para>
				///  <list type="number">
				///   <item><description>The content provider has been removed from the surface.</description></item>
				///   <item><description>The surface's Xaml control has been removed from the page.</description></item>
				///   <item><description>The application has been suspended.</description></item>
				///  </list>
				/// </remarks>
				void STDMETHODCALLTYPE Disconnect();

				/// <summary>
				///  Called by Microsoft's Direct3D system when asking this content provider to update its content.
				/// </summary>
				/// <param name="presentTargetTime">Timestamp when this function has been called.</param>
				/// <param name="isContentDirty">
				///  <para>Pointer to a boolean.</para>
				///  <para>
				///   To be set true by this function if it has new content to render.
				///   When set true, the Direct3D system will call this content provider's GetTexture() function.
				///  </para>
				///  <para>
				///   To be set false by this function if content does not need to be updated, in which case,
				///   the Direct3D system will not call this content provider's GetTexture() function.
				///  </para>
				/// </param>
				/// <returns>Returns S_OK if successful. Returns an error result if failed.</returns>
				HRESULT STDMETHODCALLTYPE PrepareResources(_In_ const LARGE_INTEGER* presentTargetTime, _Out_  BOOL *isContentDirty);

				/// <summary>
				///  Called by Microsoft's Direct3D system when requesting this content provider to render its content.
				/// </summary>
				/// <param name="surfaceSize">Provides the width and height of the surface.</param>
				/// <param name="textureSubRectangle">The texture to be rendered to.</param>
				/// <param name="textureSubRectangle">The portion of the "textureSubRectangle" that needs updating.</param>
				/// <returns>Returns S_OK if successful. Returns an error result if failed.</returns>
				HRESULT STDMETHODCALLTYPE GetTexture(_In_  const DrawingSurfaceSizeF *surfaceSize, _Outptr_  IDrawingSurfaceSynchronizedTextureNative **synchronizedTexture, _Out_  DrawingSurfaceRectF *textureSubRectangle);

			private:
				Microsoft::WRL::ComPtr<ID3D11Device1> fDirect3DDevice;
				Microsoft::WRL::ComPtr<ID3D11DeviceContext1> fDirect3DMainContext;
				Microsoft::WRL::ComPtr<ID3D11Texture2D> fDirect3DRenderTarget;
				Microsoft::WRL::ComPtr<ID3D11RenderTargetView> fDirect3DRenderTargetView;
				Microsoft::WRL::ComPtr<IDrawingSurfaceSynchronizedTextureNative> fSynchronizedTexture;
				FLOAT fLastSurfaceWidth;
				FLOAT fLastSurfaceHeight;
		};
		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Set to true if this surface adapter is currently blocking the Direct3D thread. Set to false if not.</summary>
		static bool sIsSynchronizedWithRenderingThread;

		/// <summary>
		///  Weak reference to an IXamlRenderSurfaceAdapter that provides access to the .NET Xaml control hosting
		///  the rendering surface.
		/// </summary>
		Platform::WeakReference fXamlSurfaceAdapterWeakReference;
		
		/// <summary>Delegate/callback to be invoked when this surface is requesting its owner to render content.</summary>
		Corona::WinRT::Interop::Graphics::RenderFrameHandler^ fRenderFrameHandler;

		/// <summary>
		///  <para>The orientation of the surface relative to the application page's orientation.</para>
		///  <para>Cached for fast access and to make it available to the content provider on the Direct3D thread.</para>
		/// </summary>
		Corona::WinRT::Interop::RelativeOrientation2D^ fOrientation;

		/// <summary>
		///  <para>Stores an instance of either the BackgroundContentProvider or the SurfaceContentProvider.</para>
		///  <para>Returned by this surface's ContentProvider property, to be given to a Xaml surface control.</para>
		/// </summary>
		Platform::Object^ fContentProvider;

		/// <summary>Direct3D host provided by a content provider's Connect() function.</summary>
		Microsoft::WRL::ComPtr<IDrawingSurfaceRuntimeHostNative> fSurfaceRuntimeHost;

		/// <summary>Direct3D device that the rendering surface is connected to.</summary>
		Microsoft::WRL::ComPtr<ID3D11Device1> fDirect3DDevice;

		/// <summary>The main Direct3D context. Can only be accessed on the Direct3D thread.</summary>
		Microsoft::WRL::ComPtr<ID3D11DeviceContext1> fDirect3DMainContext;

		/// <summary>Deferred context which is used to render content on the main UI thread.</summary>
		Microsoft::WRL::ComPtr<ID3D11DeviceContext1> fDirect3DDeferredContext;

		/// <summary>
		///  <para>Queues all Direct3D operations performed on "fDirect3DDeferredContext" on the main UI thread.</para>
		///  <para>Once queued, they are to be executed on the Direct3D thread in order to render to the surface.</para>
		/// </summary>
		Microsoft::WRL::ComPtr<ID3D11CommandList> fDirect3DDeferredCommandList;

		/// <summary>Direct3D texture that "Angle" needs to render content to via OpenGL.</summary>
		Microsoft::WRL::ComPtr<ID3D11Texture2D> fDirect3DRenderTarget;

		/// <summary>The Direct3D surface view to render to.</summary>
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> fDirect3DRenderTargetView;

		/// <summary>The Angle library's interface to the Xaml surface control.</summary>
		Microsoft::WRL::ComPtr<IWinPhone8XamlD3DWindow> fDirect3DWindow;

		/// <summary>The Angle library's OpenGL window that is bound to "fDirect3DWindow".</summary>
		Microsoft::WRL::ComPtr<IWinrtEglWindow> fEglWindow;

		/// <summary>OpenGL display that is bound to "fEglWindow".</summary>
		EGLDisplay fEglDisplay;

		/// <summary>Context used to render content to the surface via OpenGL functions.</summary>
		EGLContext fEglContext;

		/// <summary>OpenGL surface that is bound to "fEglWindow" and "fEglDisplay".</summary>
		EGLSurface fEglSurface;

		/// <summary>OpenGL configuration used by "fEglContext" and "fEglSurface".</summary>
		EGLConfig fEglConfig;

		/// <summary>Set true if the RequestRender() method was called. Set false if there is nothing new to render.</summary>
		std::atomic_bool fAtomicIsWaitingToSynchronize;

		/// <summary>Stores the next task that the main UI thread wants to the Direct3D thread to perform.</summary>
		std::atomic<RenderThreadTask> fAtomicRenderThreadTask;

		/// <summary>
		///  <para>Used to communicate between the main UI thread and the Direct3D thread.</para>
		///  <para>Intended to block the main thread until the rendering thread unblocks it.</para>
		/// </summary>
		CoronaLabs::WinRT::NativeThreadBlocker fMainThreadBlocker;

		/// <summary>
		///  <para>Used to communicate between the main UI thread and the Direct3D thread.</para>
		///  <para>Intended to block the rendering thread until the main thread unblocks it.</para>
		/// </summary>
		CoronaLabs::WinRT::NativeThreadBlocker fRenderThreadBlocker;

		#pragma endregion
};

} } } } } }	// namespace CoronaLabs::Corona::WinRT::Phone::Interop::Graphics
