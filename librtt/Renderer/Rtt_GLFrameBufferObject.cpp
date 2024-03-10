//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_GLFrameBufferObject.h"

#include "Renderer/Rtt_FrameBufferObject.h"
#include "Renderer/Rtt_Texture.h"
#include "Renderer/Rtt_GLTexture.h"
#include "Core/Rtt_Assert.h"

#if defined( Rtt_EGL )
	#include <EGL/egl.h>
#endif

#include <cstring>
#include "Rtt_Profiling.h"

// ----------------------------------------------------------------------------

#define ENABLE_DEBUG_PRINT	0

#if ENABLE_DEBUG_PRINT
	#define DEBUG_PRINT( ... ) Rtt_LogException( __VA_ARGS__ );
#else
	#define DEBUG_PRINT( ... )
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

void 
GLFrameBufferObject::Create( CPUResource* resource )
{
	Rtt_ASSERT( CPUResource::kFrameBufferObject == resource->GetType() );
//	FrameBufferObject* fbo = static_cast<FrameBufferObject*>( resource );
	SUMMED_TIMING( glfc, "Framebuffer GPU Resource: Create" );

	GLuint name;
	glGenFramebuffers( 1, &name );
	fHandle = NameToHandle( name );
	GL_CHECK_ERROR();

	Update( resource );

	DEBUG_PRINT( "%s : OpenGL name: %d\n",
					__FUNCTION__,
					name );
}

void 
GLFrameBufferObject::Update( CPUResource* resource )
{
	SUMMED_TIMING( glfu, "Framebuffer GPU Resource: Update" );

	// Query the bound FBO so that it can be restored. It may (or
	// may not) be worth passing this value in to avoid the query.
	GLint currentFBO;
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &currentFBO );
	{
		Rtt_ASSERT( CPUResource::kFrameBufferObject == resource->GetType() );
		FrameBufferObject* fbo = static_cast<FrameBufferObject*>( resource );

		// The Texture into which data will be rendered must be fully 
		// created prior to being attached to this FrameBufferObject.
		Texture* texture = fbo->GetTexture();
		Rtt_ASSERT( texture );

		GLTexture* gl_texture = static_cast< GLTexture * >( texture->GetGPUResource() );
		Rtt_ASSERT( gl_texture );

		GLuint texture_name = gl_texture->GetName();
		Rtt_ASSERT( texture_name );

		// Attach the destination Texture to this FrameBufferObject.
		glBindFramebuffer( GL_FRAMEBUFFER, GetName() );

		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_name, 0 );

		GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
		if( status != GL_FRAMEBUFFER_COMPLETE )
		{
			GL_LOG_ERROR( "Incomplete FrameBufferObject" );
		}
	}
	glBindFramebuffer( GL_FRAMEBUFFER, currentFBO );
	GL_CHECK_ERROR();
}

void 
GLFrameBufferObject::Destroy()
{
	GLuint name = GetName();
	if ( 0 != name )
	{
		glDeleteFramebuffers( 1, &name );
		GL_CHECK_ERROR();
		fHandle = 0;
	}

	DEBUG_PRINT( "%s : OpenGL name: %d\n",
					__FUNCTION__,
					name );
}

#if defined( Rtt_WIN_ENV )
	#define GL_TYPE_PREFIX GLAPIENTRY
#else
	#define GL_TYPE_PREFIX
#endif

typedef void (GL_TYPE_PREFIX *BlitFramebufferPtr)( GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter );
typedef void (GL_TYPE_PREFIX *BindFramebufferPtr)( GLenum target, GLuint framebuffer );

static BlitFramebufferPtr sBlitFramebuffer;
static BindFramebufferPtr sBindFramebuffer;
static GLenum sDrawBufferBinding;
static GLenum sReadBufferBinding;
static bool sCanScale;

void 
GLFrameBufferObject::Bind( bool asDrawBuffer )
{
	if (asDrawBuffer)
	{
		Rtt_ASSERT( HasFramebufferBlit( NULL ) );
		
		sBindFramebuffer( sDrawBufferBinding, GetName() );
		sBindFramebuffer( sReadBufferBinding, 0 );
	}
	
	else
	{
		glBindFramebuffer( GL_FRAMEBUFFER, GetName() );
	}

	GL_CHECK_ERROR();
}

GLuint
GLFrameBufferObject::GetName()
{
	return HandleToName( fHandle );
}

GLuint
GLFrameBufferObject::GetTextureName()
{
	// IMPORTANT: This returns the GL_COLOR_ATTACHMENT0 for the currently bound
	// fbo. The current fbo isn't necessarily the same as the fbo associated
	// with this class instance. This isn't a problem for now because we're
	// currently ONLY using this function after we bind the correct fbo for
	// this class instance.
	GLint param;
	glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER,
											GL_COLOR_ATTACHMENT0,
											GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
											&param );
	GL_CHECK_ERROR();

	return param;
}

#if defined( Rtt_OPENGLES )
	#define GL_GET_PROC(name, cap, suffix) (PFNGL ## cap ## suffix ## PROC) eglGetProcAddress( "gl" #name #suffix )
#else
	#define GL_GET_PROC(name, suffix) gl ## name ## suffix
#endif

bool
GLFrameBufferObject::HasFramebufferBlit( bool * canScale )
{
	static bool sIsInitialized;
	
	if (!sIsInitialized)
	{
		sIsInitialized = true;
		sBindFramebuffer = glBindFramebuffer;
	
	#if !defined( Rtt_OPENGLES )
		#if GL_ARB_framebuffer_object 
			sBlitFramebuffer = glBlitFramebuffer;
			sDrawBufferBinding = GL_DRAW_FRAMEBUFFER;
			sReadBufferBinding = GL_READ_FRAMEBUFFER;
		#endif
			
		#if GL_EXT_framebuffer_blit
			if (!sBlitFramebuffer)
			{
				sBlitFramebuffer = GL_GET_PROC( BlitFramebuffer, EXT );
				sBindFramebuffer = GL_GET_PROC( BindFramebuffer, EXT );
				sDrawBufferBinding = GL_DRAW_FRAMEBUFFER_EXT;
				sReadBufferBinding = GL_READ_FRAMEBUFFER_EXT;
			}
		#endif
		
		sCanScale = !!sBlitFramebuffer;
    #elif defined( GL_DRAW_FRAMEBUFFER_NV ) || defined( GL_DRAW_FRAMEBUFFER_ANGLE )
		const char * extensions = (const char *)glGetString( GL_EXTENSIONS );
		
		#if defined( GL_DRAW_FRAMEBUFFER_NV )
			if (strstr( extensions, "GL_NV_framebuffer_blit" ))
			{
				sBlitFramebuffer = GL_GET_PROC( BlitFramebuffer, BLITFRAMEBUFFER, NV );
				sDrawBufferBinding = GL_DRAW_FRAMEBUFFER_NV;
				sReadBufferBinding = GL_READ_FRAMEBUFFER_NV;
				sCanScale = !!sBlitFramebuffer;
			}
		#endif
		
		#if defined( GL_DRAW_FRAMEBUFFER_ANGLE )
			if (NULL == sBlitFramebuffer && strstr( extensions, "GL_ANGLE_framebuffer_blit" ))
			{
				sBlitFramebuffer = GL_GET_PROC( BlitFramebuffer, BLITFRAMEBUFFER, ANGLE );
				sDrawBufferBinding = GL_DRAW_FRAMEBUFFER_ANGLE;
				sReadBufferBinding = GL_READ_FRAMEBUFFER_ANGLE;
			}
		#endif
		
			if (NULL == sBlitFramebuffer && strstr( extensions, "GL_EXT_blit_framebuffer_params" ) )
			{
				// ? query for EXT? for raw?
				// read 0x8CA8
				// draw 0x8CA9
				// scalable??
			}

		// TODO? there are also *_APPLE variants, but related to multisampling...
	#endif
	}

	if (canScale)
	{
		// TODO: see comment in CaptureObject::Draw()
		*canScale = false; // sCanScale;
	}
	
	return NULL != sBlitFramebuffer;
}

#undef GL_GET_PROC
#undef GL_TYPE_PREFIX

void
GLFrameBufferObject::Blit( int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1, GLbitfield mask, GLenum filter )
{
	Rtt_ASSERT( sBlitFramebuffer );

	if (!sCanScale)
	{
		dstX1 = dstX0 + (srcX1 - srcX0);
		dstY1 = dstY0 + (srcY1 - srcY0);
		filter = GL_NEAREST;
	}
	
	sBlitFramebuffer( srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
