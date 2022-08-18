//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RenderSurfaceControl.h"
#include "Core\Rtt_Assert.h"
#include "WinString.h"
#include <exception>
#include <GL\glew.h>
#include <GL\wglew.h>
#include <GL\gl.h>
#include <GL\glu.h>


namespace Interop { namespace UI {

#pragma region Constructors/Destructors
RenderSurfaceControl::RenderSurfaceControl(HWND windowHandle)
:	Control(windowHandle),
	fReceivedMessageEventHandler(this, &RenderSurfaceControl::OnReceivedMessage),
	fRenderFrameEventHandlerPointer(nullptr),
	fMainDeviceContextHandle(nullptr),
	fRenderingContextHandle(nullptr)
{
	// Add event handlers.
	GetReceivedMessageEventHandlers().Add(&fReceivedMessageEventHandler);

	// Create an OpenGL context and bind it to the given control.
	CreateContext();
}

RenderSurfaceControl::~RenderSurfaceControl()
{
	// Remove event handlers.
	GetReceivedMessageEventHandlers().Remove(&fReceivedMessageEventHandler);

	// Destroy the OpenGL context.
	DestroyContext();
}

#pragma endregion


#pragma region Public Methods
bool RenderSurfaceControl::CanRender() const
{
	return (fRenderingContextHandle != nullptr);
}

RenderSurfaceControl::Version RenderSurfaceControl::GetRendererVersion() const
{
	return fRendererVersion;
}

void RenderSurfaceControl::SetRenderFrameHandler(RenderSurfaceControl::RenderFrameEvent::Handler *handlerPointer)
{
	fRenderFrameEventHandlerPointer = handlerPointer;
}

void RenderSurfaceControl::SelectRenderingContext()
{
	// Attempt to select this surface's rendering context.
	BOOL wasSelected = FALSE;
	if (fRenderingContextHandle)
	{
		if (fMainDeviceContextHandle)
		{
			wasSelected = ::wglMakeCurrent(fMainDeviceContextHandle, fRenderingContextHandle);
		}

		// Log an error if we've failed to select a rendering context.
		// Note: This can happen while the control is being destroyed.
		if (!wasSelected)
		{
			LPWSTR utf16Buffer;
			auto errorCode = ::GetLastError();
			::FormatMessageW(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					nullptr, errorCode,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPWSTR)&utf16Buffer, 0, nullptr);
			if (utf16Buffer && utf16Buffer[0])
			{
				WinString stringConverter;
				stringConverter.SetUTF16(utf16Buffer);
				Rtt_LogException(
						"Failed to select OpenGL rendering context. Reason:\r\n  %s\r\n", stringConverter.GetUTF8());
			}
			else
			{
				Rtt_LogException("Failed to select OpenGL rendering context.\r\n");
			}
			::LocalFree(utf16Buffer);
		}
	}

	// If we've failed, then select a null context so that the caller won't clobber another rendering context by mistake.
	if (!wasSelected)
	{
		::wglMakeCurrent(nullptr, nullptr);
	}
}

void RenderSurfaceControl::SwapBuffers()
{
	if (fMainDeviceContextHandle)
	{
		::SwapBuffers(fMainDeviceContextHandle);
	}
}

void RenderSurfaceControl::RequestRender()
{
	auto windowHandle = GetWindowHandle();
	if (windowHandle)
	{
		::InvalidateRect(windowHandle, nullptr, FALSE);
	}
}

#pragma endregion


#pragma region Protected Methods
void RenderSurfaceControl::OnRaisedDestroyingEvent()
{
	// Destroy the rendering context before the control/window gets destroyed.
	DestroyContext();

	// Let the base class perform its final tasks.
	Control::OnRaisedDestroyingEvent();
}

#pragma endregion


#pragma region Private Methods
void RenderSurfaceControl::CreateContext()
{
	// Fetch this control's window handle.
	auto windowHandle = GetWindowHandle();
	if (!windowHandle)
	{
		return;
	}

	// Destroy the last OpenGL context that was created.
	DestroyContext();

	// Query the video hardware for multisampling result.
	auto multisampleTestResult = FetchMultisampleFormat();

	// Fetch the control's device context.
	fMainDeviceContextHandle = ::GetDC(windowHandle);
	if (!fMainDeviceContextHandle)
	{
		return;
	}

	// Select a good pixel format.
	PIXELFORMATDESCRIPTOR pixelFormatDescriptor {};
	pixelFormatDescriptor.nSize = sizeof(pixelFormatDescriptor);
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 24;
	pixelFormatDescriptor.cDepthBits = 16;
	pixelFormatDescriptor.iLayerType = PFD_MAIN_PLANE;
	int pixelFormatIndex = ::ChoosePixelFormat(fMainDeviceContextHandle, &pixelFormatDescriptor);
	if (0 == pixelFormatIndex)
	{
		DestroyContext();
		return;
	}

	// Assign a pixel format to the device context.
	if (multisampleTestResult.IsSupported)
	{
		pixelFormatIndex = multisampleTestResult.PixelFormatIndex;
	}
	BOOL wasFormatSet = ::SetPixelFormat(fMainDeviceContextHandle, pixelFormatIndex, &pixelFormatDescriptor);
	if (!wasFormatSet)
	{
		DestroyContext();
		return;
	}

	// Create and enable the OpenGL rendering context.
	fRenderingContextHandle = ::wglCreateContext(fMainDeviceContextHandle);
	if (!fRenderingContextHandle)
	{
		LPWSTR utf16Buffer;
		auto errorCode = ::GetLastError();
		::FormatMessageW(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				nullptr, errorCode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPWSTR)&utf16Buffer, 0, nullptr);
		if (utf16Buffer && utf16Buffer[0])
		{
			WinString stringConverter;
			stringConverter.SetUTF16(utf16Buffer);
			Rtt_LogException("Failed to create OpenGL rendering context. Reason:\r\n  %s\r\n", stringConverter.GetUTF8());
		}
		else
		{
			Rtt_LogException("Failed to create OpenGL rendering context.\r\n");
		}
		::LocalFree(utf16Buffer);
	}

	// Select the newly created OpenGL context.
	::wglMakeCurrent(fMainDeviceContextHandle, fRenderingContextHandle);

	// Load OpenGL extensions.
	glewInit();

	// Fetch the OpenGL driver's version.
	const char* versionString = (const char*)glGetString(GL_VERSION);
	fRendererVersion.SetString(versionString);
	fRendererVersion.SetMajorNumber(0);
	fRendererVersion.SetMinorNumber(0);
	if (versionString && (versionString[0] != '\0'))
	{
		try
		{
			int majorNumber = 0;
			int minorNumber = 0;
			sscanf_s(fRendererVersion.GetString(), "%d.%d", &majorNumber, &minorNumber);
			fRendererVersion.SetMajorNumber(majorNumber);
			fRendererVersion.SetMinorNumber(minorNumber);
		}
		catch (...) {}
	}
}

void RenderSurfaceControl::DestroyContext()
{
	// Fetch this control's window handle.
	auto windowHandle = GetWindowHandle();

	// Destroy the OpenGL context.
	::wglMakeCurrent(nullptr, nullptr);
	if (fRenderingContextHandle)
	{
		::wglDeleteContext(fRenderingContextHandle);
		fRenderingContextHandle = nullptr;
	}
	if (fMainDeviceContextHandle)
	{
		if (windowHandle)
		{
			::ReleaseDC(windowHandle, fMainDeviceContextHandle);
		}
		fMainDeviceContextHandle = nullptr;
	}
	fRendererVersion.SetString(nullptr);
	fRendererVersion.SetMajorNumber(0);
	fRendererVersion.SetMinorNumber(0);

	// Let the window re-paint itself now that we've detached from it.
	if (windowHandle)
	{
		::InvalidateRect(windowHandle, nullptr, FALSE);
	}
}

static HMODULE GetLibraryModuleHandle()
{
	HMODULE moduleHandle = nullptr;
	DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
	::GetModuleHandleExW(flags, (LPCWSTR)GetLibraryModuleHandle, &moduleHandle);
	return moduleHandle;
}

RenderSurfaceControl::FetchMultisampleFormatResult RenderSurfaceControl::FetchMultisampleFormat()
{
	// Initialize a result value to "not supported".
	FetchMultisampleFormatResult result;
	result.IsSupported = false;

	// Fetch this control's window handle.
	auto windowHandle = GetWindowHandle();
	if (!windowHandle)
	{
		return result;
	}

	// Fetch a handle to this library.
	HMODULE moduleHandle = GetLibraryModuleHandle();

	// Hunt down a multisample format that is supported by this machine's video hardware.
	for (int sampleCount = 4; sampleCount > 0; sampleCount--)
	{
		// Create a temporary text label control.
		// We'll use it to bind a new OpenGL context to it below for testing purposes.
		auto controlHandle = ::CreateWindowEx(
				0, L"STATIC", nullptr, WS_CHILD, 0, 0, 0, 0, windowHandle, nullptr, moduleHandle, nullptr);
		if (!controlHandle)
		{
			return result;
		}

		// Fetch the temporary control's device context.
		HDC deviceContextHandle = ::GetDC(controlHandle);
		if (!deviceContextHandle)
		{
			::DestroyWindow(controlHandle);
			return result;
		}

		// Assign the below pixel format to the device context.
		PIXELFORMATDESCRIPTOR pixelFormatDescriptor {};
		pixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pixelFormatDescriptor.nVersion = 1;
		pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
		pixelFormatDescriptor.cColorBits = 24;
		pixelFormatDescriptor.cDepthBits = 16;
		pixelFormatDescriptor.iLayerType = PFD_MAIN_PLANE;
		int pixelFormatIndex = ::ChoosePixelFormat(deviceContextHandle, &pixelFormatDescriptor);
		if (0 == pixelFormatIndex)
		{
			::ReleaseDC(controlHandle, deviceContextHandle);
			::DestroyWindow(controlHandle);
			return result;
		}
		BOOL wasFormatSet = ::SetPixelFormat(deviceContextHandle, pixelFormatIndex, &pixelFormatDescriptor);
		if (!wasFormatSet)
		{
			::ReleaseDC(controlHandle, deviceContextHandle);
			::DestroyWindow(controlHandle);
			return result;
		}

		// Create an OpenGL rendering context.
		HGLRC renderingContextHandle = ::wglCreateContext(deviceContextHandle);
		if (!renderingContextHandle)
		{
			::ReleaseDC(controlHandle, deviceContextHandle);
			::DestroyWindow(controlHandle);
			return result;
		}

		// Select the newly created OpenGL context.
		BOOL wasContextSelected = ::wglMakeCurrent(deviceContextHandle, renderingContextHandle);
		if (!wasContextSelected)
		{
			::wglDeleteContext(renderingContextHandle);
			::ReleaseDC(controlHandle, deviceContextHandle);
			::DestroyWindow(controlHandle);
			return result;
		}

		// Load OpenGL extensions.
		if (glewInit() != GLEW_OK)
		{
			::wglDeleteContext(renderingContextHandle);
			::ReleaseDC(controlHandle, deviceContextHandle);
			::DestroyWindow(controlHandle);
			return result;
		}

		// Test for multisampling support.
		if (WGLEW_ARB_pixel_format && GLEW_ARB_multisample)
		{
			UINT formatCount = 0;
			int PFAttribs[] =
			{
				WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
				WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB, 24,
				WGL_DEPTH_BITS_ARB, 16,
				WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
				WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
				WGL_SAMPLES_ARB, sampleCount,
				0
			};
			pixelFormatIndex = 0;
			BOOL wasPixelFormatChosen = ::wglChoosePixelFormatARB(
					deviceContextHandle, PFAttribs, NULL, 1, &pixelFormatIndex, &formatCount);
			if (wasPixelFormatChosen && (formatCount > 0))
			{
				result.IsSupported = true;
				result.PixelFormatIndex = pixelFormatIndex;
			}
		}

		// Destroy the temporary control and OpenGL context.
		::wglDeleteContext(renderingContextHandle);
		::ReleaseDC(controlHandle, deviceContextHandle);
		::DestroyWindow(controlHandle);

		// Stop now if multisampling is supported.
		if (result.IsSupported)
		{
			break;
		}
	}

	// Return the final result.
	return result;
}

void RenderSurfaceControl::OnReceivedMessage(UIComponent& sender, HandleMessageEventArgs& arguments)
{
	switch (arguments.GetMessageId())
	{
		case WM_ERASEBKGND:
		{
			// As an optimization, always handle the "Erase Background" message so that the operating system
			// won't automatically paint over the background. We'll just let OpenGL paint over the entire surface.
			// Handle the 
			arguments.SetHandled();
			arguments.SetReturnResult(1);
			break;
		}
		case WM_PAINT:
		{
			// Request the owner of this control to paint its content.
			OnPaint();

			// Flag that we've painted to the control's entire region.
			// Note: Windows will keep sending this control paint messages until we've flagged it as validated.
			ValidateRect(GetWindowHandle(), nullptr);

			// Flag that the paint message has been handled.
			arguments.SetHandled();
			arguments.SetReturnResult(0);
			break;
		}
	}
}

void RenderSurfaceControl::OnPaint()
{
	// Fetch this control's window handle.
	auto windowHandle = GetWindowHandle();
	if (!windowHandle)
	{
		return;
	}

	// Render to the control.
	if (fMainDeviceContextHandle && fRenderingContextHandle)
	{
		// Select this control's OpenGL context.
		SelectRenderingContext();

		// If the owner of this surface has provided a RenderFrameHandler, then use it to draw the next frame.
		// Otherwise, draw a black screen until a handler has been given to this surface.
		bool didDraw = false;
		if (fRenderFrameEventHandlerPointer)
		{
			HandledEventArgs arguments;
			try
			{
				fRenderFrameEventHandlerPointer->Invoke(*this, arguments);
				didDraw = arguments.WasHandled();
			}
			catch (std::exception ex) { }
		}
		if (false == didDraw)
		{
			::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			::glClear(GL_COLOR_BUFFER_BIT);
			SwapBuffers();
		}
	}
	else
	{
		// We were unable to set up an OpenGL context.
		// Render an error message to the control via GDI instead.
	}
}

#pragma endregion


#pragma region Version Class
RenderSurfaceControl::Version::Version()
:	Version(0, 0)
{
}

RenderSurfaceControl::Version::Version(int majorNumber, int minorNumber)
:	fMajorNumber(majorNumber),
	fMinorNumber(minorNumber)
{
	fVersionString = std::make_shared<std::string>("");
}

const char* RenderSurfaceControl::Version::GetString() const
{
	if (fVersionString)
	{
		return fVersionString->c_str();
	}
	return nullptr;
}

void RenderSurfaceControl::Version::SetString(const char* value)
{
	if (value)
	{
		fVersionString = std::make_shared<std::string>(value);
	}
	else
	{
		fVersionString = nullptr;
	}
}

int RenderSurfaceControl::Version::GetMajorNumber() const
{
	return fMajorNumber;
}

void RenderSurfaceControl::Version::SetMajorNumber(int value)
{
	fMajorNumber = value;
}

int RenderSurfaceControl::Version::GetMinorNumber() const
{
	return fMinorNumber;
}

void RenderSurfaceControl::Version::SetMinorNumber(int value)
{
	fMinorNumber = value;
}

int RenderSurfaceControl::Version::CompareTo(const RenderSurfaceControl::Version& version) const
{
	int x = (this->fMajorNumber * 100) + this->fMinorNumber;
	int y = (version.fMajorNumber * 100) + version.fMinorNumber;
	return x - y;
}

#pragma endregion

} }	// namespace Interop::UI
