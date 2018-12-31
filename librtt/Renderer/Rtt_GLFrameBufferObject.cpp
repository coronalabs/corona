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

#include "Renderer/Rtt_GLFrameBufferObject.h"

#include "Renderer/Rtt_FrameBufferObject.h"
#include "Renderer/Rtt_Texture.h"
#include "Renderer/Rtt_GLTexture.h"
#include "Core/Rtt_Assert.h"

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

void 
GLFrameBufferObject::Bind()
{
	glBindFramebuffer( GL_FRAMEBUFFER, GetName() );
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

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
