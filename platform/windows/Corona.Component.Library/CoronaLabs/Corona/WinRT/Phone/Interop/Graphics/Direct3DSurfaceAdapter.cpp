// ----------------------------------------------------------------------------
// 
// Direct3DSurfaceAdapter.cpp
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "Direct3DSurfaceAdapter.h"
#include <stdio.h>
#include <wchar.h>
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core\Rtt_Time.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


#define LOG_RENDER_THREAD_UPDATE_DURATIONS 0


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Phone { namespace Interop { namespace Graphics {

#pragma region Constructors/Destructors
Direct3DSurfaceAdapter::Direct3DSurfaceAdapter(IXamlRenderSurfaceAdapter^ xamlSurfaceAdapter)
:	fXamlSurfaceAdapterWeakReference(xamlSurfaceAdapter),
	fRenderFrameHandler(nullptr),
	fOrientation(Corona::WinRT::Interop::RelativeOrientation2D::Unknown),
	fContentProvider(nullptr),
	fSurfaceRuntimeHost(nullptr),
	fDirect3DDevice(nullptr),
	fDirect3DMainContext(nullptr),
	fDirect3DDeferredContext(nullptr),
	fDirect3DDeferredCommandList(nullptr),
	fDirect3DRenderTarget(nullptr),
	fDirect3DRenderTargetView(nullptr),
	fDirect3DWindow(nullptr),
	fEglWindow(nullptr),
	fEglDisplay(nullptr),
	fEglContext(nullptr),
	fEglSurface(nullptr),
	fEglConfig(nullptr),
	fAtomicRenderThreadTask(kRenderThreadTask_None)
{
	// Validate.
	if (nullptr == xamlSurfaceAdapter)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Create the content provider.
	if (xamlSurfaceAdapter->IsRenderingToBackground)
	{
		// Create a content provider for a "DrawingSurfaceBackgroundGrid" Xaml control.
		auto contentProvider = Microsoft::WRL::Make<Direct3DSurfaceAdapter::BackgroundContentProvider>(this);
		fContentProvider = reinterpret_cast<Object^>(contentProvider.Get());
	}
	else
	{
		// Create a content provider for a "DrawingSurface" Xaml control.
		auto contentProvider = Microsoft::WRL::Make<Direct3DSurfaceAdapter::SurfaceContentProvider>(this);
		fContentProvider = reinterpret_cast<Object^>(contentProvider.Get());
	}

	// Subscribe to the Xaml surface's events.
	xamlSurfaceAdapter->Resized += ref new Windows::Foundation::TypedEventHandler<IXamlRenderSurfaceAdapter^, CoronaLabs::WinRT::EmptyEventArgs^>(this, &Direct3DSurfaceAdapter::OnSurfaceResized);

	// Finish initializing all other member variables.
	fAtomicIsWaitingToSynchronize = false;
}

Direct3DSurfaceAdapter::~Direct3DSurfaceAdapter()
{
	// Destroy all OpenGL resources, if created.
	DestroyOpenGLContext();
}

#pragma endregion


#pragma region Public Methods/Properties
void Direct3DSurfaceAdapter::SetManipulationHost(
	Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ manipulationHost)
{
}

Platform::Object^ Direct3DSurfaceAdapter::ContentProvider::get()
{
	return fContentProvider;
}

bool Direct3DSurfaceAdapter::IsReadyToRender::get()
{
	return (fSurfaceRuntimeHost.Get() && fEglContext);
}

int Direct3DSurfaceAdapter::WidthInPixels::get()
{
	auto xamlSurfaceAdapter = fXamlSurfaceAdapterWeakReference.Resolve<IXamlRenderSurfaceAdapter>();
	if (nullptr == xamlSurfaceAdapter)
	{
		return 0;
	}
	return xamlSurfaceAdapter->WidthInPixels;
}

int Direct3DSurfaceAdapter::HeightInPixels::get()
{
	auto xamlSurfaceAdapter = fXamlSurfaceAdapterWeakReference.Resolve<IXamlRenderSurfaceAdapter>();
	if (nullptr == xamlSurfaceAdapter)
	{
		return 0;
	}
	return xamlSurfaceAdapter->HeightInPixels;
}

Corona::WinRT::Interop::RelativeOrientation2D^ Direct3DSurfaceAdapter::Orientation::get()
{
	auto xamlSurfaceAdapter = fXamlSurfaceAdapterWeakReference.Resolve<IXamlRenderSurfaceAdapter>();
	if (nullptr == xamlSurfaceAdapter)
	{
		return Corona::WinRT::Interop::RelativeOrientation2D::Unknown;
	}
	return xamlSurfaceAdapter->Orientation;
}

void Direct3DSurfaceAdapter::RequestRender()
{
	// Do not continue if this adapter does not have a surface to render to yet.
	if (this->IsReadyToRender == false)
	{
		return;
	}

	// Do not continue if a render pass was already requested.
	if (fAtomicIsWaitingToSynchronize.load())
	{
		return;
	}

	// Flag the rendering thread that the UI thread is ready to render to it.
	// This will cause the rendering thread to block until the below dispatched handler invokes.
	fSurfaceRuntimeHost->RequestAdditionalFrame();
	fAtomicIsWaitingToSynchronize = true;

	// Attempt to render now if the rendering thread is already waiting.
	OnRender();
}

void Direct3DSurfaceAdapter::ForceRender()
{
	// Do not continue if this adapter does not have a surface to render to yet.
	if (this->IsReadyToRender == false)
	{
		return;
	}

	// Do a blocking render.
	fSurfaceRuntimeHost->RequestAdditionalFrame();
	fAtomicIsWaitingToSynchronize = true;
	UpdateRenderThread(true, std::chrono::milliseconds(100));
}

void Direct3DSurfaceAdapter::SetRenderFrameHandler(Corona::WinRT::Interop::Graphics::RenderFrameHandler^ handler)
{
	fRenderFrameHandler = handler;
}

#pragma endregion


#pragma region Public Static Functions
bool Direct3DSurfaceAdapter::sIsSynchronizedWithRenderingThread = false;

bool Direct3DSurfaceAdapter::IsSynchronizedWithRenderingThread::get()
{
	return sIsSynchronizedWithRenderingThread;
}

#pragma endregion


#pragma region Private Methods
bool Direct3DSurfaceAdapter::UpdateRenderThread(bool isRequestingRender)
{
	return UpdateRenderThread(isRequestingRender, std::chrono::milliseconds(3));
}

bool Direct3DSurfaceAdapter::UpdateRenderThread(
	bool isRequestingRender, const std::chrono::milliseconds &timeoutInMilliseconds)
{
	// Do not continue if we're already in the middle of updating this render thread.
	if (sIsSynchronizedWithRenderingThread)
	{
		return false;
	}

	// Do not continue if we've lost our connection to the rendering surface.
	if (fSurfaceRuntimeHost.Get() == nullptr)
	{
		return false;
	}

	// Fetch the Xaml control's current rendering orientation.
	// Note: Accessing the Xaml's properties from the Direct3D thread will cause an exception.
	//       We must store this value to a member variable to make it safely accessible to other threads.
	auto xamlSurfaceAdapter = fXamlSurfaceAdapterWeakReference.Resolve<IXamlRenderSurfaceAdapter>();
	if (nullptr == xamlSurfaceAdapter)
	{
		return false;
	}
	fOrientation = xamlSurfaceAdapter->Orientation;

	// Do not continue if the rendering surface is currently collapsed. Nothing to render in this case.
	if ((xamlSurfaceAdapter->WidthInPixels < 1) || (xamlSurfaceAdapter->HeightInPixels < 1))
	{
		return false;
	}

#if LOG_RENDER_THREAD_UPDATE_DURATIONS
	auto startTime = Rtt_GetAbsoluteTime();
#endif

	// Notify the system that we're about to synchronize with the rendering thread.
	// Note: This is the owner's opportunity to load images such as text bitmaps that can
	//       only be generated when we're not synchronized/blocking the rendering thread.
	this->AcquiringRenderContext(this, CoronaLabs::WinRT::EmptyEventArgs::Instance);

	// Attempt to synchronize with the rendering thread, if available.
	if (fAtomicIsWaitingToSynchronize.load())
	{
		fMainThreadBlocker.Enable();
		fAtomicRenderThreadTask = kRenderThreadTask_Wait;
		fRenderThreadBlocker.Disable();
		auto unblockResult = fMainThreadBlocker.BlockUntil(timeoutInMilliseconds);
		if (unblockResult.WasUnblockedManually())
		{
			// Successfully synchronized with the rendering thread.
			// The rendering thread is now blocking forever, waiting for commands from this thread.
			sIsSynchronizedWithRenderingThread = true;
		}
		else
		{
			// Failed to synchronize with the rendering thread. (We just missed it.)
			fAtomicRenderThreadTask = kRenderThreadTask_None;
			fRenderThreadBlocker.Disable();
		}
	}

	// Update the rendering surface if synchronized.
	bool hasUpdatedRenderThread = sIsSynchronizedWithRenderingThread;
	if (sIsSynchronizedWithRenderingThread)
	{
		// Clearing the synchronize flag tells the rendering thread to stop blocking until next time we need to render.
		fAtomicIsWaitingToSynchronize = false;

		// Request the rendering thread to update this adapter's Direct3D/OpenGL context info, if changed.
		EGLContext lastEglContext = fEglContext;
		fAtomicRenderThreadTask = kRenderThreadTask_AcquireContext;
		fMainThreadBlocker.Enable();
		fRenderThreadBlocker.Disable();
		fMainThreadBlocker.Block();

		// Raise an event if the last Direct3D/OpenGL context has been lost.
		if (lastEglContext && (lastEglContext != fEglContext))
		{
			eglMakeCurrent(EGL_NO_DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			try { LostRenderContext(this, CoronaLabs::WinRT::EmptyEventArgs::Instance); }
			catch (...) {}
		}

		// Update the rendering surface, if available.
		if (fEglContext)
		{
			// Set up Angle to route OpenGL function calls to the deferred context.
			// This allows us to render outside of the Direct3D thread.
			fDirect3DWindow->Update(fDirect3DDevice.Get(), fDirect3DDeferredContext.Get(), fDirect3DRenderTargetView.Get());

			// Ensure that OpenGL has this surface's context currently selected.
			// Note: The eglMakeCurrent() function is expensive. So, only call it when necessary.
			if (eglGetCurrentContext() != fEglContext)
			{
				eglMakeCurrent(fEglDisplay, fEglSurface, fEglSurface, fEglContext);
			}

			// Determine if a new Direct3D/OpenGL context has been received.
			if (!lastEglContext || (lastEglContext != fEglContext))
			{
				// Raise an event informing the system that a new context has been received.
				// This is needed so that the system will know it needs to reload textures and shaders.
				// Note: This event must be raised after making the OpenGL context current.
				try { ReceivedRenderContext(this, CoronaLabs::WinRT::EmptyEventArgs::Instance); }
				catch (...) {}

				// Force the system to render to the new context.
				isRequestingRender = true;
			}

			// Render to the surface.
			if (isRequestingRender)
			{
				// If the owner of this surface has provided a RenderFrameHandler, then use it to draw the next frame.
				// Otherwise, draw a black screen until a handler has been given to this surface.
				if (fRenderFrameHandler)
				{
					try { fRenderFrameHandler->Invoke(this); }
					catch (...) {}
				}
				else
				{
					glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
					glClear(GL_COLOR_BUFFER_BIT);
				}

				// Fetch the queued rendering operations.
				fDirect3DDeferredContext->FinishCommandList(FALSE, &fDirect3DDeferredCommandList);

				// Request the rendering thread to execute the queued render operations above.
				fAtomicRenderThreadTask = kRenderThreadTask_ExecuteDeferredCommands;
				fMainThreadBlocker.Enable();
				fRenderThreadBlocker.Disable();
				fMainThreadBlocker.Block();
			}
		}

		// Release the Direct3D thread.
		fAtomicRenderThreadTask = kRenderThreadTask_None;
		fRenderThreadBlocker.Disable();
	}

#if LOG_RENDER_THREAD_UPDATE_DURATIONS
	int durationInMilliseconds = (int)Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime() - startTime);
	wchar_t message[256];
	swprintf_s(message, 256, L"Direct3DSurfaceAdapter::UpdateRenderThread() Duration = %d ms\r\n", durationInMilliseconds);
	OutputDebugStringW(message);
#endif

	// Flag that we're done updating the Direct3D thread.
	sIsSynchronizedWithRenderingThread = false;

	return hasUpdatedRenderThread;
}

void Direct3DSurfaceAdapter::OnSurfaceResized(IXamlRenderSurfaceAdapter^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args)
{
	// Relay this event to the owner(s) of this object.
	Resized(this, args);
}

void Direct3DSurfaceAdapter::OnRender()
{
	// Attempt to render to the surface.
	if (fSurfaceRuntimeHost)
	{
		fSurfaceRuntimeHost->RequestAdditionalFrame();
	}
	bool isRequestingRender = true;
	bool hasUpdatedRenderThread = UpdateRenderThread(isRequestingRender);

	// If we failed to synchronize with the rendering thread, then try again later.
	if (!hasUpdatedRenderThread && fAtomicIsWaitingToSynchronize.load())
	{
		CoronaLabs::Corona::WinRT::Interop::UI::IDispatcher^ dispatcher = nullptr;
		auto xamlSurfaceAdapter = fXamlSurfaceAdapterWeakReference.Resolve<IXamlRenderSurfaceAdapter>();
		if (xamlSurfaceAdapter && xamlSurfaceAdapter->Dispatcher)
		{
			auto handler = ref new Windows::UI::Core::DispatchedHandler(this, &Direct3DSurfaceAdapter::OnRender);
			xamlSurfaceAdapter->Dispatcher->InvokeAsync(handler);
		}
	}
}

void Direct3DSurfaceAdapter::OnDirect3DHostReceived(_In_ IDrawingSurfaceRuntimeHostNative* hostPointer)
{
	// Validate argument.
	if (!hostPointer)
	{
		OnDirect3DHostLost();
		return;
	}

	// Do not continue if the host hasn't changed.
	if (fSurfaceRuntimeHost.Get() == hostPointer)
	{
		return;
	}

	// Store the given host.
	fSurfaceRuntimeHost = hostPointer;
}

void Direct3DSurfaceAdapter::OnDirect3DHostLost()
{
	// Do not continue if the host was never received by this adapter.
	if (fSurfaceRuntimeHost.Get() == nullptr)
	{
		return;
	}

	// Determine if we've ever had a Direct3D/OpenGL context set up.
	bool hadContext = (fEglContext != nullptr);

	// Destroy the OpenGL context associated with the Direct3D context we lost.
	DestroyOpenGLContext();

	// Give up all references to the last Direct3D context.
	fSurfaceRuntimeHost = nullptr;
	fDirect3DRenderTargetView = nullptr;
	fDirect3DRenderTarget = nullptr;
	fDirect3DDeferredCommandList = nullptr;
	fDirect3DDeferredContext = nullptr;
	fDirect3DMainContext = nullptr;
	fDirect3DDevice = nullptr;

	// Raise an event if the Direct3D/OpenGL context has been lost.
	// Note: This must be done after clearing all rendering object references above in case
	//       this event's handlers attempt request this adapter to render when it can't.
	if (hadContext)
	{
		eglMakeCurrent(EGL_NO_DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		LostRenderContext(this, CoronaLabs::WinRT::EmptyEventArgs::Instance);
	}
}

void Direct3DSurfaceAdapter::OnDirect3DNewContextReceived()
{
	bool isRequestingRender = false;
	UpdateRenderThread(isRequestingRender);

	if (fAtomicIsWaitingToSynchronize.load())
	{
		auto xamlSurfaceAdapter = fXamlSurfaceAdapterWeakReference.Resolve<IXamlRenderSurfaceAdapter>();
		if (xamlSurfaceAdapter && xamlSurfaceAdapter->Dispatcher)
		{
			auto handler = ref new Windows::UI::Core::DispatchedHandler(this, &Direct3DSurfaceAdapter::OnDirect3DNewContextReceived);
			xamlSurfaceAdapter->Dispatcher->InvokeAsync(handler);
		}
	}
}

void Direct3DSurfaceAdapter::OnDirect3DThreadNewContextReceived()
{
	// Inform this adapter about the context change on the main UI thread.
	auto xamlSurfaceAdapter = fXamlSurfaceAdapterWeakReference.Resolve<IXamlRenderSurfaceAdapter>();
	if (xamlSurfaceAdapter && xamlSurfaceAdapter->Dispatcher)
	{
		fAtomicIsWaitingToSynchronize = true;
		auto handler = ref new Windows::UI::Core::DispatchedHandler(this, &Direct3DSurfaceAdapter::OnDirect3DNewContextReceived);
		xamlSurfaceAdapter->Dispatcher->InvokeAsync(handler);
	}
}

HRESULT Direct3DSurfaceAdapter::OnDirect3DThreadAcquiredContext(
	_In_ ID3D11Device1* devicePointer, _In_ ID3D11DeviceContext1* contextPointer,
	_In_ ID3D11RenderTargetView* renderTargetViewPointer)
{
	HRESULT result = S_OK;

	// Store the given Direct3D device information.
	bool hasContextChanged = false;
	if (fDirect3DDevice.Get() != devicePointer)
	{
		fDirect3DDevice = devicePointer;
		fDirect3DDeferredContext = nullptr;
		hasContextChanged = true;
	}
	if (fDirect3DMainContext.Get() != contextPointer)
	{
		fDirect3DMainContext = contextPointer;
		hasContextChanged = true;
	}
	if (fDirect3DRenderTargetView.Get() != renderTargetViewPointer)
	{
		fDirect3DRenderTargetView = renderTargetViewPointer;
		if (false == hasContextChanged)
		{
			UpdateOpenGLSurface();
		}
	}

	// Do not continue if no change has been detected.
	if (false == hasContextChanged)
	{
		return S_OK;
	}

	// Create a new OpenGL context configured for the given Direct3D device.
	DestroyOpenGLContext();
	if (devicePointer)
	{
		result = UpdateOpenGLContext();
		if (FAILED(result))
		{
			return result;
		}
	}

	// Create a deferred context used to queue render operations outside of the Direct3D thread.
	if (devicePointer)
	{
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> deferredContext;
		result = devicePointer->CreateDeferredContext(0, &deferredContext);
		if (FAILED(result))
		{
			return result;
		}
		deferredContext.As(&fDirect3DDeferredContext);
	}

	// This adapter has successfully updated its Direct3D and OpenGL context information.
	return S_OK;
}

HRESULT Direct3DSurfaceAdapter::UpdateOpenGLContext()
{
	// Destroy the last OpenGL context, if created.
	DestroyOpenGLContext();

	// Stop here if we do not have a Direct3D context.
	if (!fDirect3DDevice.Get() || !fDirect3DMainContext.Get() || !fDirect3DRenderTargetView.Get())
	{
		return S_OK;
	}

	// Select a Direct 3D feature level that this device and the Angle library supports.
	// Throw an exception if the graphics hardware does not meet Angle's minimum requirements.
	ANGLE_D3D_FEATURE_LEVEL featureLevel;
	D3D_FEATURE_LEVEL supportedFeatureLevel = fDirect3DDevice->GetFeatureLevel();
	if (supportedFeatureLevel >= D3D_FEATURE_LEVEL_9_3)
	{
		featureLevel = ANGLE_D3D_FEATURE_LEVEL_9_3;
	}
	else if (D3D_FEATURE_LEVEL_9_2 == supportedFeatureLevel)
	{
		featureLevel = ANGLE_D3D_FEATURE_LEVEL_9_2;
	}
	else if (D3D_FEATURE_LEVEL_9_1 == supportedFeatureLevel)
	{
		featureLevel = ANGLE_D3D_FEATURE_LEVEL_9_1;
	}
	else
	{
		const int MAX_MESSAGE_LENGTH = 256;
		wchar_t message[MAX_MESSAGE_LENGTH];
		int majorVersion = supportedFeatureLevel / 0xFFF;
		int minorVersion = (supportedFeatureLevel / 0xFF) & 0xF;
		_snwprintf_s(
				message, MAX_MESSAGE_LENGTH, MAX_MESSAGE_LENGTH,
				L"Unable to initialize rendering system. Corona requires a Direct 3D device that supports feature level 9.1 or higher. This device only supports feature level %d.%d.",
				majorVersion, minorVersion);
		OutputDebugStringW(message);
		return E_FAIL;
	}

	// Create the Direct 3D window that Angle will render to.
	HRESULT result = CreateWinPhone8XamlWindow(&fDirect3DWindow);
	if (FAILED(result))
	{
		return result;
	}
	fDirect3DWindow->Update(fDirect3DDevice.Get(), fDirect3DMainContext.Get(), fDirect3DRenderTargetView.Get());

	// Create an OpenGL window that is bound to the above Direct 3D window.
	result = CreateWinrtEglWindow(WINRT_EGL_IUNKNOWN(fDirect3DWindow.Get()), featureLevel, fEglWindow.GetAddressOf());
	if (FAILED(result))
	{
		return result;
	}
	fEglDisplay = eglGetDisplay(fEglWindow);
	if (EGL_NO_DISPLAY == fEglDisplay)
	{
		return E_FAIL;
	}

	// Initialize the OpenGL display and fetch the version of API it supports.
	EGLint eglMajorVersion = 0;
	EGLint eglMinorVersion = 0;
	if (!eglInitialize(fEglDisplay, &eglMajorVersion, &eglMinorVersion))
	{
		return E_FAIL;
	}

	// Fetch an EGL configuration that best matches the following 32-bit color surface settings.
	EGLint configAttribList[] =
	{
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 8,
		EGL_STENCIL_SIZE, 8,
		EGL_SAMPLE_BUFFERS, 0,
		EGL_NONE
	};
	EGLint eglConfigCount = 0;
	EGLConfig eglConfig = NULL;
	if (!eglGetConfigs(fEglDisplay, NULL, 0, &eglConfigCount))
	{
		return E_FAIL;
	}
	if (!eglChooseConfig(fEglDisplay, configAttribList, &eglConfig, 1, &eglConfigCount))
	{
		return E_FAIL;
	}
	fEglConfig = eglConfig;

	// Create an OpenGL ES 2.0 context.
	EGLint contextAttribs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE, EGL_NONE
	};
	fEglContext = eglCreateContext(fEglDisplay, fEglConfig, EGL_NO_CONTEXT, contextAttribs);
	if (EGL_NO_CONTEXT == fEglContext)
	{
		return E_FAIL;
	}

	// Create the OpenGL surface using the above configuration.
	return UpdateOpenGLSurface();
}

HRESULT Direct3DSurfaceAdapter::UpdateOpenGLSurface()
{
	// Do not continue if the Direct3D or OpenGL contexts have not been acquired yet.
	if (!fDirect3DDevice.Get() || !fDirect3DMainContext.Get() || !fDirect3DRenderTargetView.Get() || !fDirect3DWindow.Get())
	{
		return E_FAIL;
	}
	if (!fEglContext || !fEglDisplay || !fEglConfig)
	{
		return E_FAIL;
	}

	// Set up Angle to route OpenGL function calls to this surface's Direct3D context.
	fDirect3DWindow->Update(fDirect3DDevice.Get(), fDirect3DMainContext.Get(), fDirect3DRenderTargetView.Get());

	// Destroy the last OpenGL surface, if it exists.
	if (fEglSurface)
	{
		eglDestroySurface(fEglDisplay, fEglSurface);
		fEglSurface = nullptr;
	}

	// Create a new OpenGL surface using the Direct3D render target selected above.
	// Note: This needs to be done every time the render target's surface size has changed.
	EGLint surfaceAttribList[] =
	{
		EGL_NONE, EGL_NONE
	};
	fEglSurface = eglCreateWindowSurface(fEglDisplay, fEglConfig, fEglWindow, surfaceAttribList);
	if (EGL_NO_SURFACE == fEglSurface)
	{
		return E_FAIL;
	}

	// Make this Direct3D surface's OpenGL context and surface available to all OpenGL function calls.
	// Note: This only makes the context and its surface current on the thread it was called on.
	eglMakeCurrent(fEglDisplay, fEglSurface, fEglSurface, fEglContext);
	return S_OK;
}

void Direct3DSurfaceAdapter::DestroyOpenGLContext()
{
	if (fEglDisplay)
	{
		if (fEglSurface)
		{
			eglDestroySurface(fEglDisplay, fEglSurface);
			fEglSurface = nullptr;
		}
		if (fEglContext)
		{
			eglDestroyContext(fEglDisplay, fEglContext);
			fEglContext = nullptr;
		}
		eglTerminate(fEglDisplay);
		fEglDisplay = nullptr;
	}
	if (fDirect3DWindow)
	{
		fDirect3DWindow->Update(nullptr, nullptr, nullptr);
		fDirect3DWindow = nullptr;
	}
	fEglConfig = nullptr;
	fEglWindow = nullptr;
	eglMakeCurrent(EGL_NO_DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

#pragma endregion


#pragma region Internal BaseContentProvider Class Methods
Direct3DSurfaceAdapter::BaseContentProvider::BaseContentProvider(Direct3DSurfaceAdapter^ surfaceAdapter)
:	fSurfaceAdapterWeakReference(surfaceAdapter)
{
	if (nullptr == surfaceAdapter)
	{
		throw ref new Platform::NullReferenceException();
	}
}

Direct3DSurfaceAdapter^ Direct3DSurfaceAdapter::BaseContentProvider::GetSurfaceAdapter()
{
	return fSurfaceAdapterWeakReference.Resolve<Direct3DSurfaceAdapter>();
}

#pragma endregion


#pragma region Internal BackgroundContentProvider Class Methods
Direct3DSurfaceAdapter::BackgroundContentProvider::BackgroundContentProvider(Direct3DSurfaceAdapter^ surfaceAdapter)
:	BaseContentProvider(surfaceAdapter)
{
}

HRESULT Direct3DSurfaceAdapter::BackgroundContentProvider::Connect(
	_In_ IDrawingSurfaceRuntimeHostNative* host, _In_ ID3D11Device1* device)
{
	// Fetch the surface adapter.
	auto surfaceAdapter = GetSurfaceAdapter();
	if (!surfaceAdapter)
	{
		return E_FAIL;
	}

	// Notify the adapter that the Direct3D host has been received.
	// This in turn notifies the system that the adapter is ready to render to the surface.
	surfaceAdapter->OnDirect3DHostReceived(host);
	return S_OK;
}

void Direct3DSurfaceAdapter::BackgroundContentProvider::Disconnect()
{
	auto surfaceAdapter = GetSurfaceAdapter();
	if (surfaceAdapter)
	{
		surfaceAdapter->OnDirect3DHostLost();
	}
}

HRESULT Direct3DSurfaceAdapter::BackgroundContentProvider::PrepareResources(
	_In_ const LARGE_INTEGER* presentTargetTime, _Inout_ DrawingSurfaceSizeF* desiredRenderTargetSize)
{
	return S_OK;
}

HRESULT Direct3DSurfaceAdapter::BackgroundContentProvider::Draw(
	_In_ ID3D11Device1* device, _In_ ID3D11DeviceContext1* context, _In_ ID3D11RenderTargetView* renderTargetView)
{
	HRESULT result = S_OK;

	// Fetch the surface adapter.
	auto surfaceAdapter = GetSurfaceAdapter();
	if (!surfaceAdapter)
	{
		return E_FAIL;
	}

	// If a new device context has been received, then inform the adapter now.
	if (surfaceAdapter->fDirect3DDevice.Get() != device)
	{
		surfaceAdapter->OnDirect3DThreadNewContextReceived();
	}

	// Synchronize with the main UI thread, if requested.
	bool didDraw = false;
	if (surfaceAdapter->fAtomicIsWaitingToSynchronize.load())
	{
		bool isSynchronizing = false;
		if (kRenderThreadTask_Wait == surfaceAdapter->fAtomicRenderThreadTask)
		{
			isSynchronizing = true;
		}
		else
		{
			surfaceAdapter->fRenderThreadBlocker.Enable();
			auto unblockResult = surfaceAdapter->fRenderThreadBlocker.BlockUntil(std::chrono::milliseconds(20));
			isSynchronizing = unblockResult.WasUnblockedManually();
		}
		if (isSynchronizing && (kRenderThreadTask_Wait == surfaceAdapter->fAtomicRenderThreadTask))
		{
			surfaceAdapter->fRenderThreadBlocker.Enable();
			surfaceAdapter->fMainThreadBlocker.Disable();
			surfaceAdapter->fRenderThreadBlocker.Block();
			while (surfaceAdapter->fAtomicRenderThreadTask != kRenderThreadTask_None)
			{
				// Execute the next task requested by the main UI thread.
				switch (surfaceAdapter->fAtomicRenderThreadTask)
				{
					case kRenderThreadTask_AcquireContext:
					{
						// Optimization: Use the last Direct3D rendert view we initialized with, if it exists.
						//               Prevents core code from creating a new OpenGL surface unnecessarily.
						ID3D11RenderTargetView* lastRenderTargetView = renderTargetView;
						if ((surfaceAdapter->fDirect3DDevice.Get() == device) &&
						    (surfaceAdapter->fDirect3DRenderTargetView.Get() != nullptr))
						{
							lastRenderTargetView = surfaceAdapter->fDirect3DRenderTargetView.Get();
						}

						// Notify the system that we've acquired a Direct3D context and render target.
						surfaceAdapter->OnDirect3DThreadAcquiredContext(device, context, lastRenderTargetView);
						break;
					}
					case kRenderThreadTask_ExecuteDeferredCommands:
					{
						// Render to the surface by execute the queued Direct3d/OpenGL commands.
						if (surfaceAdapter->fDirect3DDeferredCommandList.Get())
						{
							surfaceAdapter->fDirect3DMainContext->ExecuteCommandList(
									surfaceAdapter->fDirect3DDeferredCommandList.Get(), FALSE);
							surfaceAdapter->fDirect3DWindow->Update(device, context, renderTargetView);
							if (eglGetCurrentContext() != surfaceAdapter->fEglContext)
							{
								eglMakeCurrent(
										surfaceAdapter->fEglDisplay, surfaceAdapter->fEglSurface,
										surfaceAdapter->fEglSurface, surfaceAdapter->fEglContext);
							}
							eglSwapBuffers(surfaceAdapter->fEglDisplay, surfaceAdapter->fEglSurface);
							didDraw = true;
						}
						break;
					}
				}

				// Wait for the main UI thread to send this thread another task.
				surfaceAdapter->fRenderThreadBlocker.Enable();
				surfaceAdapter->fMainThreadBlocker.Disable();
				surfaceAdapter->fRenderThreadBlocker.Block();
			}
		}
	}

	// Check if the main UI thread did not tell this thread to draw up above.
	if (false == didDraw)
	{
		if ((surfaceAdapter->fOrientation != Corona::WinRT::Interop::RelativeOrientation2D::Unknown) &&
		    (surfaceAdapter->fDirect3DDeferredCommandList.Get() != nullptr))
		{
			// Draw the last frame that was rendered via the derferred context.
			// Note: We must draw something or else Direct3D will draw the frame before last, which looks really bad.
			surfaceAdapter->fDirect3DMainContext->ExecuteCommandList(
					surfaceAdapter->fDirect3DDeferredCommandList.Get(), FALSE);
			surfaceAdapter->fDirect3DWindow->Update(device, context, renderTargetView);
			if (eglGetCurrentContext() != surfaceAdapter->fEglContext)
			{
				eglMakeCurrent(
						surfaceAdapter->fEglDisplay, surfaceAdapter->fEglSurface,
						surfaceAdapter->fEglSurface, surfaceAdapter->fEglContext);
			}
			eglSwapBuffers(surfaceAdapter->fEglDisplay, surfaceAdapter->fEglSurface);
		}
		else
		{
			// The Corona runtime is most likely not started yet or has been terminated.
			// Render a black screen in this case until the Corona runtime has been started again.
			surfaceAdapter->fDirect3DDeferredCommandList = nullptr;
			float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			context->ClearRenderTargetView(renderTargetView, clearColor);
		}
		didDraw = true;
	}

	return result;
}

#pragma endregion


#pragma region Internal SurfaceContentProvider Class Methods
Direct3DSurfaceAdapter::SurfaceContentProvider::SurfaceContentProvider(Direct3DSurfaceAdapter^ surfaceAdapter)
:	BaseContentProvider(surfaceAdapter),
	fDirect3DDevice(nullptr),
	fDirect3DMainContext(nullptr),
	fDirect3DRenderTarget(nullptr),
	fDirect3DRenderTargetView(nullptr),
	fSynchronizedTexture(nullptr),
	fLastSurfaceWidth(0),
	fLastSurfaceHeight(0)
{
}

HRESULT Direct3DSurfaceAdapter::SurfaceContentProvider::Connect(_In_ IDrawingSurfaceRuntimeHostNative* host)
{
	// Fetch the surface adapter.
	auto surfaceAdapter = GetSurfaceAdapter();
	if (!surfaceAdapter)
	{
		return E_FAIL;
	}

	// Notify the adapter that the Direct3D host has been received.
	// This in turn notifies the system that the adapter is ready to render to the surface.
	surfaceAdapter->OnDirect3DHostReceived(host);
	return S_OK;
}

void Direct3DSurfaceAdapter::SurfaceContentProvider::Disconnect()
{
	// Destroy this content provider's Direct3D resources by dereferencing them.
	if (fDirect3DMainContext)
	{
		ID3D11RenderTargetView* nullViews[] = { nullptr };
		fDirect3DMainContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
	}
	fSynchronizedTexture = nullptr;
	fDirect3DRenderTargetView = nullptr;
	fDirect3DRenderTarget = nullptr;
	fDirect3DMainContext = nullptr;
	fDirect3DDevice = nullptr;
	fLastSurfaceWidth = 0;
	fLastSurfaceHeight = 0;

	// Notify the adapter that we can no longer render to the Direct3D surface.
	auto surfaceAdapter = GetSurfaceAdapter();
	if (surfaceAdapter)
	{
		surfaceAdapter->OnDirect3DHostLost();
	}
}

HRESULT Direct3DSurfaceAdapter::SurfaceContentProvider::PrepareResources(
	_In_ const LARGE_INTEGER* presentTargetTime, _Out_ BOOL *isContentDirty)
{
	// Determine if we need to draw new content to the surface.
	*isContentDirty = FALSE;
	auto surfaceAdapter = GetSurfaceAdapter();
	if (surfaceAdapter)
	{
		if (surfaceAdapter->fAtomicIsWaitingToSynchronize.load() || (fDirect3DDevice.Get() == nullptr))
		{
			// It's time to draw new content to the surface.
			// Notify the rendering system that we want the GetTexture() function to be called.
			*isContentDirty = TRUE;
		}
	}
	return S_OK;
}

HRESULT Direct3DSurfaceAdapter::SurfaceContentProvider::GetTexture(
	_In_ const DrawingSurfaceSizeF *surfaceSize,
	_Outptr_ IDrawingSurfaceSynchronizedTextureNative **synchronizedTexture,
	_Out_ DrawingSurfaceRectF *textureSubRectangle)
{
	HRESULT result = S_OK;
	bool wasSurfaceUpdated = false;

	// Fetch the surface adapter.
	auto surfaceAdapter = GetSurfaceAdapter();
	if (!surfaceAdapter)
	{
		return E_FAIL;
	}

	// Create a Direct3D device context, if not done already.
	if (fDirect3DDevice.Get() == nullptr)
	{
		Microsoft::WRL::ComPtr<ID3D11Device> device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
		D3D_FEATURE_LEVEL supportedFeatureLevel;
		UINT deviceFlags = 0;
		HRESULT result;

		// Set up the Direct3D color channel order to be compatible with Direct2D.
		deviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

		// Set the DirectX hardware feature levels this application supports.
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_9_3
		};

		// Create a Direct3D device and context.
		// Note: Unlike an IDrawingSurfaceBackgroundContentProvider, this provider will never be given a device or context.
		result = D3D11CreateDevice(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags, featureLevels, ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION, &device, &supportedFeatureLevel, &context);
		if (FAILED(result))
		{
			return result;
		}

		// Store the created device and context.
		result = device.As(&fDirect3DDevice);
		if (FAILED(result))
		{
			return result;
		}
		result = context.As(&fDirect3DMainContext);
		if (FAILED(result))
		{
			return result;
		}

		// Notify the adapter on the main UI thread that a new device context has been received.
		wasSurfaceUpdated = true;
	}

	// Check if the render target was resized or has a zero width/height.
	bool isSurfaceSizeInvalid = (surfaceSize->width <= 0) || (surfaceSize->height <= 0);
	bool wasSurfaceResized = (fLastSurfaceWidth != surfaceSize->width) || (fLastSurfaceHeight != surfaceSize->height);
	
	// Destroy the last render target if the surface has been resized.
	if (fDirect3DRenderTarget.Get())
	{
		if (isSurfaceSizeInvalid || wasSurfaceResized)
		{
			fSynchronizedTexture = nullptr;
			fDirect3DRenderTargetView = nullptr;
			fDirect3DRenderTarget = nullptr;
			fLastSurfaceWidth = 0;
			fLastSurfaceHeight = 0;
		}
	}

	// Do not continue if the render target has a zero width/height. Nothing to render in this case.
	if (isSurfaceSizeInvalid)
	{
		return S_OK;
	}

	// Create a render target, if not done already.
	if (fDirect3DRenderTarget.Get() == nullptr)
	{
		ID3D11RenderTargetView* nullViews[] = { nullptr };
		fDirect3DMainContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
		fDirect3DMainContext->Flush();
		fDirect3DRenderTargetView = nullptr;
		CD3D11_TEXTURE2D_DESC renderTargetDescriptor(
									DXGI_FORMAT_B8G8R8A8_UNORM,
									static_cast<UINT>(surfaceSize->width),
									static_cast<UINT>(surfaceSize->height),
									1, 1,
									D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
		renderTargetDescriptor.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
		result = fDirect3DDevice->CreateTexture2D(&renderTargetDescriptor, nullptr, &fDirect3DRenderTarget);
		if (FAILED(result))
		{
			return result;
		}
		fLastSurfaceWidth = surfaceSize->width;
		fLastSurfaceHeight = surfaceSize->height;
		wasSurfaceUpdated = true;
	}
	if (fDirect3DRenderTargetView.Get() == nullptr)
	{
		result = fDirect3DDevice->CreateRenderTargetView(fDirect3DRenderTarget.Get(), nullptr, &fDirect3DRenderTargetView);
		if (FAILED(result))
		{
			return result;
		}
	}
	if (fSynchronizedTexture.Get() == nullptr)
	{
		if (surfaceAdapter->fSurfaceRuntimeHost.Get() == nullptr)
		{
			return E_FAIL;
		}
		result = surfaceAdapter->fSurfaceRuntimeHost->CreateSynchronizedTexture(
								fDirect3DRenderTarget.Get(), &fSynchronizedTexture);
		if (FAILED(result))
		{
			return result;
		}
	}

	// Notify the adapter on the main UI thread if a new device context has been received or the surface was resized.
	if (wasSurfaceUpdated)
	{
		surfaceAdapter->OnDirect3DThreadNewContextReceived();
	}

	// Synchronize with the main UI thread, if requested.
	bool hasRequestedExecuteDeferredCommands = false;
	if (surfaceAdapter->fAtomicIsWaitingToSynchronize.load())
	{
		bool isSynchronizing = false;
		if (kRenderThreadTask_Wait == surfaceAdapter->fAtomicRenderThreadTask)
		{
			isSynchronizing = true;
		}
		else
		{
			surfaceAdapter->fRenderThreadBlocker.Enable();
			auto unblockResult = surfaceAdapter->fRenderThreadBlocker.BlockUntil(std::chrono::milliseconds(20));
			isSynchronizing = unblockResult.WasUnblockedManually();
		}
		if (isSynchronizing && (kRenderThreadTask_Wait == surfaceAdapter->fAtomicRenderThreadTask))
		{
			// Execute the next task requested by the main UI thread.
			surfaceAdapter->fRenderThreadBlocker.Enable();
			surfaceAdapter->fMainThreadBlocker.Disable();
			surfaceAdapter->fRenderThreadBlocker.Block();
			while (surfaceAdapter->fAtomicRenderThreadTask != kRenderThreadTask_None)
			{
				switch (surfaceAdapter->fAtomicRenderThreadTask)
				{
					case kRenderThreadTask_AcquireContext:
						result = surfaceAdapter->OnDirect3DThreadAcquiredContext(
										fDirect3DDevice.Get(), fDirect3DMainContext.Get(), fDirect3DRenderTargetView.Get());
						break;

					case kRenderThreadTask_ExecuteDeferredCommands:
						hasRequestedExecuteDeferredCommands = true;
						break;
				}

				// Wait for the main UI thread to send this thread another task.
				surfaceAdapter->fRenderThreadBlocker.Enable();
				surfaceAdapter->fMainThreadBlocker.Disable();
				surfaceAdapter->fRenderThreadBlocker.Block();
			}
		}
	}

	// If the main UI thread has queued deferred Direct3D operations up above, then execute them now.
	// Note: We can only draw to the below synchronized texture while the main UI thread is not blocked.
	if (hasRequestedExecuteDeferredCommands)
	{
		if (surfaceAdapter->fDirect3DDeferredCommandList.Get())
		{
			result = fSynchronizedTexture->BeginDraw();
			if (SUCCEEDED(result))
			{
				surfaceAdapter->fDirect3DMainContext->ExecuteCommandList(
						surfaceAdapter->fDirect3DDeferredCommandList.Get(), FALSE);
				surfaceAdapter->fDirect3DWindow->Update(
						fDirect3DDevice.Get(), fDirect3DMainContext.Get(), fDirect3DRenderTargetView.Get());
				if (eglGetCurrentContext() != surfaceAdapter->fEglContext)
				{
					eglMakeCurrent(
							surfaceAdapter->fEglDisplay, surfaceAdapter->fEglSurface,
							surfaceAdapter->fEglSurface, surfaceAdapter->fEglContext);
				}
				eglSwapBuffers(surfaceAdapter->fEglDisplay, surfaceAdapter->fEglSurface);
				result = fSynchronizedTexture->EndDraw();
				if (SUCCEEDED(result))
				{
					textureSubRectangle->left = 0.0f;
					textureSubRectangle->top = 0.0f;
					textureSubRectangle->right = surfaceSize->width;
					textureSubRectangle->bottom = surfaceSize->height;
					result = fSynchronizedTexture.CopyTo(synchronizedTexture);
				}
			}
			surfaceAdapter->fDirect3DDeferredCommandList = nullptr;
		}
	}

	return result;
}

#pragma endregion

} } } } } }	// namespace CoronaLabs::Corona::WinRT::Phone::Interop::Graphics
