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

#ifndef _Rtt_GPU_H__
#define _Rtt_GPU_H__

// ----------------------------------------------------------------------------

#if defined( Rtt_OPENGLES )
	#if defined( Rtt_IPHONE_ENV ) || defined( Rtt_TVOS_ENV )
		#include <OpenGLES/ES2/gl.h>
	#elif defined( Rtt_SYMBIAN_ENV )
		#include <gles/gl.h>
	#elif defined( Rtt_ANDROID_ENV ) || defined( Rtt_WEBOS_ENV )
		#include <GLES/gl.h>
	#elif defined( Rtt_WIN_PHONE_ENV ) || defined( Rtt_EMSCRIPTEN_ENV )
		#include <GLES2/gl2.h>
	#elif defined( Rtt_POWERVR_ENV )
		#include <GLES/egl.h>
		#include <GLES/gl.h>
	#else
		#error TODO: Add path to gl.h header
	#endif
#elif defined( Rtt_LINUX_ENV )
	#ifdef _WIN32
		#include <windows.h>
		#include <GL/glew.h>
	#else
		#include <GL/gl.h>
		#include <GL/glext.h>
	#endif
#elif defined( Rtt_WIN_ENV ) || defined( Rtt_NINTENDO_ENV )
  // TODO: Do we have to include windows.h here???
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>

	#ifdef Rtt_EMSCRIPTEN_ENV
		#include <GL/glew.h>
	#else
		#include <GL/gl.h>
		#include <GL/glu.h>
	#endif
#else
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
#endif

#include "Display/Rtt_PlatformBitmap.h"

// ----------------------------------------------------------------------------

#if defined( Rtt_OPENGLES )

	#if defined( Rtt_REAL_FIXED )
		#define Rtt_glColor4			glColor4x
		#define Rtt_glFrustum			glFrustumx
		#define Rtt_glMultMatrix( m )	glMultMatrixx( (const GLfixed*)(m) )
		#define Rtt_glOrtho				glOrthox
		#define Rtt_glRotate			glRotatex
		#define Rtt_glScale				glScalex
		#define Rtt_glTranslate			glTranslatex
	#else
		#define Rtt_glClearDepth	glClearDepthf
		#define Rtt_glColor4		glColor4f
		#define Rtt_glDepthRange	glDepthRangef
		#define Rtt_glFrustum		glFrustumf
		#define Rtt_glMultMatrix	glMultMatrixf
		#define Rtt_glOrtho			glOrthof
		#define Rtt_glRotate		glRotatef
		#define Rtt_glScale			glScalef
		#define Rtt_glTranslate		glTranslatef
	#endif

	#if GL_ES_VERSION_2_0
		#define Rtt_GL_COLOR_ATTACHMENT0		GL_COLOR_ATTACHMENT0
		#define Rtt_GL_FRAMEBUFFER				GL_FRAMEBUFFER
		#define Rtt_GL_FRAMEBUFFER_BINDING		GL_FRAMEBUFFER_BINDING
		#define Rtt_GL_FRAMEBUFFER_COMPLETE		GL_FRAMEBUFFER_COMPLETE
		#define Rtt_glBindFramebuffer			glBindFramebuffer
		#define Rtt_glCheckFramebufferStatus	glCheckFramebufferStatus
		#define Rtt_glDeleteFramebuffers		glDeleteFramebuffers
		#define Rtt_glFramebufferTexture2D		glFramebufferTexture2D
		#define Rtt_glGenFramebuffers			glGenFramebuffers
	#else
		#define Rtt_GL_COLOR_ATTACHMENT0		GL_COLOR_ATTACHMENT0_OES
		#define Rtt_GL_FRAMEBUFFER				GL_FRAMEBUFFER_OES
		#define Rtt_GL_FRAMEBUFFER_BINDING		GL_FRAMEBUFFER_BINDING_OES
		#define Rtt_GL_FRAMEBUFFER_COMPLETE		GL_FRAMEBUFFER_COMPLETE_OES
		#define Rtt_glBindFramebuffer			glBindFramebufferOES
		#define Rtt_glCheckFramebufferStatus	glCheckFramebufferStatusOES
		#define Rtt_glDeleteFramebuffers		glDeleteFramebuffersOES
		#define Rtt_glFramebufferTexture2D		glFramebufferTexture2DOES
		#define Rtt_glGenFramebuffers			glGenFramebuffersOES
	#endif
#else
	#define Rtt_GL_COLOR_ATTACHMENT0		GL_COLOR_ATTACHMENT0_EXT
	#define Rtt_GL_FRAMEBUFFER				GL_FRAMEBUFFER_EXT
	#define Rtt_GL_FRAMEBUFFER_BINDING		GL_FRAMEBUFFER_BINDING
	#define Rtt_GL_FRAMEBUFFER_COMPLETE		GL_FRAMEBUFFER_COMPLETE_EXT
	#define Rtt_glBindFramebuffer			glBindFramebufferEXT
	#define Rtt_glCheckFramebufferStatus	glCheckFramebufferStatusEXT
	#define Rtt_glClearDepth				glClearDepth
	#define Rtt_glColor4					glColor4f
	#define Rtt_glDeleteFramebuffers		glDeleteFramebuffersEXT
	#define Rtt_glDepthRange				glDepthRange
	#define Rtt_glFramebufferTexture2D		glFramebufferTexture2DEXT
	#define Rtt_glFrustum					glFrustum
	#define Rtt_glGenFramebuffers			glGenFramebuffersEXT
	#define Rtt_glMultMatrix				glMultMatrixf
	#define Rtt_glOrtho						glOrtho
	#define Rtt_glRotate					glRotatef
	#define Rtt_glScale						glScalef
	#define Rtt_glTranslate					glTranslatef

#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class GPU
{
#ifdef Rtt_DEBUG
	public:
		static void Error( const char filename[], int linenum )
		{
			const GLenum e = glGetError();

			if ( ! Rtt_VERIFY( e == GL_NO_ERROR ) )
			{
				const char kNull[] = "(null)";
				if ( ! filename ) { filename = kNull; }
				Rtt_TRACE( ( "%s(%d): GL Error(0x%x)\n", filename, linenum, e ) );
			}
		}
#endif

	// TODO: Following are implemented in Rtt_Color.cpp. Move to separate cpp file
	public:
		static GLenum GetInternalPixelFormat( PlatformBitmap::Format format );

		#if !defined( Rtt_OPENGLES )
			static GLenum GetPixelFormat( PlatformBitmap::Format format );
			static GLenum GetPixelType( PlatformBitmap::Format format );
		#endif
		
		static bool CheckIfContextSupportsExtension( const char * extension );
};

#ifdef Rtt_DEBUG
	#define GPUError()	::Rtt::GPU::Error( __FILE__, __LINE__ )
#else
	#define GPUError()	do {} while(0)
#endif

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GPU_H__
