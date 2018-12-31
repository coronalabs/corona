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

#include "Rtt_PlatformSurface.h"

#if defined( Rtt_IPHONE_ENV )
	#include <OpenGLES/ES1/glext.h>
#endif

// Starting to work on OffscreenGPUSurface for Windows
//#if defined( Rtt_WIN_ENV )
//#include "Rtt_GPU.h"
//#define GL_GLEXT_PROTOTYPES
//#include "opengl/glext.h"
//#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const S32 PlatformSurface::kDefaultVirtualDPI = 160;
const S32 PlatformSurface::kUninitializedVirtualLength = -1;

S32
PlatformSurface::CalculateVirtualLength( S32 virtualDPI, S32 screenDPI, S32 screenLength )
{
	float virtualLength = ((float)virtualDPI) * screenLength / screenDPI;

	S32 result = (S32)virtualLength;

	// TODO: Should we round to nearest multiple of 2?

	return result;
}

// ----------------------------------------------------------------------------

PlatformSurface::PlatformSurface()
{
}

PlatformSurface::~PlatformSurface()
{
}

DeviceOrientation::Type
PlatformSurface::GetOrientation() const
{
	// By default, assume that the screen physically rotates, but that the 
	// surface/receiver is fixed in orientation relative to the screen.
	// This means that Width()/Height() always return the same values as
	// DeviceWidth()/DeviceHeight()
	return DeviceOrientation::kUpright;
}

S32
PlatformSurface::ScaledWidth() const
{
	return Width();
}

S32
PlatformSurface::ScaledHeight() const
{
	return Height();
}

S32
PlatformSurface::DeviceWidth() const
{
	return Width();
}

S32
PlatformSurface::DeviceHeight() const
{
	return Height();
}

S32
PlatformSurface::AdaptiveWidth() const
{
	return DeviceWidth();
}

S32
PlatformSurface::AdaptiveHeight() const
{
	return DeviceHeight();
}

void*
PlatformSurface::NativeWindow() const
{
	return NULL;
}

void
PlatformSurface::SetDelegate( PlatformSurfaceDelegate* delegate )
{
}

// ----------------------------------------------------------------------------

// TODO: Replace platform ifdef's with a feature ifdef: Rtt_OFFSCREEN_SURFACE in Rtt_Config.h
#if ! defined( Rtt_ANDROID_ENV ) && !defined( Rtt_WIN_ENV ) && !defined( Rtt_EMSCRIPTEN_ENV ) && !defined( Rtt_NINTENDO_ENV )

OffscreenGPUSurface::OffscreenGPUSurface( const PlatformSurface& parent )
:	fWidth( parent.Width() ),
	fHeight( parent.Height() ),
	fTexture( 0 )
{
	if ( Rtt_VERIFY( Width() <= GL_MAX_TEXTURE_SIZE && Height() <= GL_MAX_TEXTURE_SIZE ) )
	{
		// Setting the framebuffer to 0 to disable is not a safe operation because of things like CAOpenGLLayer on Mac.
		// In general, we probably should be restoring the previous value instead of setting it to 0 anyway.
		// TODO: We should optimize this so we don't query the OpenGL state for the FBO.
		// This value should probably be passed as a parameter.
		// But this value can't necessarily be cached because this value may change. (See CAOpenGLLayer on Mac.)
		GLint savedfbo = 0;
		glGetIntegerv(Rtt_GL_FRAMEBUFFER_BINDING, &savedfbo);
		
		Rtt_glGenFramebuffers( 1, & fFramebuffer );
		Rtt_glBindFramebuffer( Rtt_GL_FRAMEBUFFER, fFramebuffer );

		GLuint texture;
		glGenTextures( 1, & texture ); GPUError();
		fTexture = texture;
		glBindTexture( GL_TEXTURE_2D, texture ); GPUError();

#ifdef Rtt_AUTHORING_SIMULATOR
		GLsizei w = Width();
		GLsizei h = Height();
#else
		GLsizei w = NextPowerOf2( Width() );
		GLsizei h = NextPowerOf2( Height() );
#endif
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL ); GPUError();
		Rtt_glFramebufferTexture2D( Rtt_GL_FRAMEBUFFER, Rtt_GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0 ); GPUError();

		if ( ! Rtt_VERIFY( Rtt_glCheckFramebufferStatus( Rtt_GL_FRAMEBUFFER ) == Rtt_GL_FRAMEBUFFER_COMPLETE ) )
		{
			Rtt_TRACE( ( "ERROR: Failed to make complete framebuffer object %x", Rtt_glCheckFramebufferStatus( Rtt_GL_FRAMEBUFFER ) ) );
		}
		
		// Unbind texture and framebuffer, so subsequent drawing is not drawn into FBO
		glBindTexture( GL_TEXTURE_2D, 0 ); GPUError();
		Rtt_glBindFramebuffer( Rtt_GL_FRAMEBUFFER, savedfbo); GPUError();
	}
}

OffscreenGPUSurface::~OffscreenGPUSurface()
{
	glDeleteTextures( 1, & fTexture );
	Rtt_glDeleteFramebuffers( 1, & fFramebuffer );
}

void
OffscreenGPUSurface::SetCurrent() const
{
	Rtt_glBindFramebuffer( Rtt_GL_FRAMEBUFFER, fFramebuffer );	
}

void
OffscreenGPUSurface::Flush() const
{
	// No-op b/c it's offscreen. We don't blit it anywhere
}

S32
OffscreenGPUSurface::Width() const
{
	return fWidth;
}

S32
OffscreenGPUSurface::Height() const
{
	return fHeight;
}

#endif // Rtt_ANDROID_ENV

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

