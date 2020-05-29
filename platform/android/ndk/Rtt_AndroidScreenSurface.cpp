//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"

#include "Rtt_AndroidScreenSurface.h"

#include "AndroidGLView.h"
#include "Rtt_GPU.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AndroidScreenSurface::AndroidScreenSurface( AndroidGLView* view, S32 approximateScreenDpi )
:	fView( view ),
	fFramebuffer( 0 ),
	fApproximateScreenDPI( approximateScreenDpi )
{
// This gives error 0x502 invalid operation on Nexus One.
// 	if ( supportsScreenCapture() )
// 	{
// 		glGetFramebufferAttachmentParameterivOES(
// 			GL_FRAMEBUFFER_OES,
// 			GL_COLOR_ATTACHMENT0_OES,
// 			GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_OES,
// 			(GLint *) &fFramebuffer );
// 		GPUError();
// 	}
}

AndroidScreenSurface::~AndroidScreenSurface()
{
}

void
AndroidScreenSurface::SetCurrent() const
{
#if 0
// 	if ( fFramebuffer != 0 )
	{
//		glBindFramebufferOES( GL_FRAMEBUFFER_OES, fFramebuffer );

// Apparently this restores the previous framebuffer
		glBindFramebufferOES( GL_FRAMEBUFFER_OES, 0 );
		GPUError();
	}
#endif
}

void
AndroidScreenSurface::Flush() const
{
	fView->Flush();
}

void*
AndroidScreenSurface::NativeWindow() const
{
	return NULL;
}

S32
AndroidScreenSurface::Width() const
{
	return fView->Width();
}

S32
AndroidScreenSurface::Height() const
{
	return fView->Height();
}

S32
AndroidScreenSurface::AdaptiveWidth() const
{
	S32 pixelWidth = fView->DeviceWidth();
	return PlatformSurface::CalculateVirtualLength( PlatformSurface::kDefaultVirtualDPI, fApproximateScreenDPI, pixelWidth );
}

S32
AndroidScreenSurface::AdaptiveHeight() const
{
	S32 pixelHeight = fView->DeviceHeight();
	return PlatformSurface::CalculateVirtualLength( PlatformSurface::kDefaultVirtualDPI, fApproximateScreenDPI, pixelHeight );
}

AndroidGLContext*
AndroidScreenSurface::GetContext() const
{
	return NULL;
}

DeviceOrientation::Type
AndroidScreenSurface::GetOrientation() const
{
	return fView->GetOrientation();
}

S32
AndroidScreenSurface::DeviceWidth() const
{
	return fView->DeviceWidth();
}

S32
AndroidScreenSurface::DeviceHeight() const
{
	return fView->DeviceHeight();
}


// ----------------------------------------------------------------------------

AndroidOffscreenSurface::AndroidOffscreenSurface( const PlatformSurface& parent )
:	fWidth( parent.Width() ),
	fHeight( parent.Height() ),
	fTexture( 0 ),
	fFramebuffer( -1 )
{
#if 0
	if ( !AndroidOffscreenSurface::IsSupported() )
	{
		Rtt_TRACE( ( "ERROR: Device does not support screen capture" ) );
		return;
	}
	
	// This code is the same as IPhoneOffscreenSurface::IPhoneOffscreenSurface
	if ( Rtt_VERIFY( Width() <= GL_MAX_TEXTURE_SIZE && Height() <= GL_MAX_TEXTURE_SIZE ) )
	{
		glGenFramebuffersOES( 1, & fFramebuffer );
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, fFramebuffer);

		GLuint texture;
		glGenTextures( 1, & texture ); GPUError();
		fTexture = texture;
		glBindTexture( GL_TEXTURE_2D, texture ); GPUError();

		GLsizei w = NextPowerOf2( Width() );
		GLsizei h = NextPowerOf2( Height() );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL ); GPUError();
	//	glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 640, 960, GL_RGBA, GL_UNSIGNED_BYTE, texBuffer ); GPUError();
		glFramebufferTexture2DOES( GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, texture, 0 ); GPUError();

		// Unbind texture and framebuffer, so subsequent drawing is not drawn into FBO
		glBindTexture( GL_TEXTURE_2D, 0 ); GPUError();
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0); GPUError();

		if ( ! Rtt_VERIFY( glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) == GL_FRAMEBUFFER_COMPLETE_OES ) )
		{
			Rtt_TRACE( ( "ERROR: Failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) ) );
		}
	}
#endif
}

AndroidOffscreenSurface::~AndroidOffscreenSurface()
{
#if 0
	if ( fFramebuffer != -1 )
	{
		glDeleteTextures( 1, & fTexture );
		glDeleteFramebuffersOES( 1, & fFramebuffer );
	}
#endif
}

void
AndroidOffscreenSurface::SetCurrent() const
{
#if 0
	if ( fFramebuffer != -1 )
	{
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, fFramebuffer);	
	}
#endif
}

void
AndroidOffscreenSurface::Flush() const
{
	// No-op b/c it's offscreen. We don't blit it anywhere
}

S32
AndroidOffscreenSurface::Width() const
{
	return fWidth;
}

S32
AndroidOffscreenSurface::Height() const
{
	return fHeight;
}

bool
AndroidOffscreenSurface::IsSupported()
{
	static bool sHasChecked = false;
	static bool sIsSupported;
	
#if 0
	if (!sHasChecked)
	{
		sIsSupported = GPU::CheckIfContextSupportsExtension( "GL_OES_framebuffer_object" );
		sHasChecked = true;
	}
	return sIsSupported;
#else
	return false;
#endif
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

