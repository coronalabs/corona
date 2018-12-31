//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_EGLContext.h"

#include "Rtt_MPlatform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

EGLContext::EGLContext()
:	fEglDisplay( NULL ),
	fEglSurface( NULL ),
	fEglContext( NULL )
{
}

EGLContext::~EGLContext()
{
	eglMakeCurrent( fEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
	eglDestroySurface( fEglDisplay, fEglSurface ); 
	eglDestroyContext( fEglDisplay, fEglContext );
	eglTerminate( fEglDisplay );	// Release resources associated with EGL and OpenGL ES
}

bool
EGLContext::InitDisplay()
{
	bool result = false;

	fEglDisplay = eglGetDisplay( EGL_DEFAULT_DISPLAY );
	if ( Rtt_VERIFY( NULL != fEglDisplay ) )
	{
		EGLint major, minor;
		result = EGL_FALSE != eglInitialize( fEglDisplay, &major, &minor );
	}

	return result;
}

bool
EGLContext::Initialize( const MPlatform& platform, NativeWindowType window, int bitsPerPixel )
{
	bool result = false;
	EGLConfig config = 0;

	if ( ! InitDisplay() ) { goto exit_gracefully; }

	if ( ! InitConfig( config, bitsPerPixel ) ) { goto exit_gracefully; }

	fEglSurface = eglCreateWindowSurface( fEglDisplay, config, window, NULL );
	if ( ! Rtt_VERIFY( fEglSurface ) ) { goto exit_gracefully; }
	
	fEglContext = eglCreateContext( fEglDisplay, config, EGL_NO_CONTEXT, NULL );
	if ( ! Rtt_VERIFY( fEglContext ) ) { goto exit_gracefully; }

	result = ( EGL_FALSE != eglMakeCurrent( fEglDisplay, fEglSurface, fEglSurface, fEglContext ) );
	
exit_gracefully:
	if ( ! Rtt_VERIFY( result ) )
	{
		platform.RaiseError( MPlatform::kEGLContextInitError, NULL );
	}

	return result;
}

void
EGLContext::Flush() const
{
	eglSwapBuffers( fEglDisplay, fEglSurface );
}

bool
EGLContext::InitConfig( EGLConfig& rConfig, int bitsPerPixel )
{
	bool result = false;

	EGLint numConfigs;

	// Get the number of possible EGLConfigs 
	Rtt_ASSERT( eglGetConfigs( fEglDisplay, NULL, 0, &numConfigs ) != EGL_FALSE
			    && numConfigs > 0 );

	const int kBufferSize = bitsPerPixel;

	// Full-screen anti-aliasing
	const EGLint kAttributesFSAA[] =
	{
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BUFFER_SIZE, kBufferSize,
		EGL_DEPTH_SIZE, 16,
		EGL_SAMPLE_BUFFERS, 1,
		EGL_SAMPLES, 4,
		EGL_NONE
	};

	result = EGL_FALSE != eglChooseConfig(
								fEglDisplay, kAttributesFSAA,
								& rConfig, 1, & numConfigs )
			 && numConfigs > 0;

	if ( ! result )
	{
		// No anti-aliasing
		const EGLint kAttributesNoAA[] =
		{
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_BUFFER_SIZE, bitsPerPixel,
			EGL_DEPTH_SIZE, 16,
			EGL_NONE
		};

		result = Rtt_VERIFY( EGL_FALSE != eglChooseConfig(
											fEglDisplay, kAttributesNoAA,
											& rConfig, 1, & numConfigs )
							 && numConfigs > 0 );
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

