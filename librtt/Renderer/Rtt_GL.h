//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_GL_H__
#define _Rtt_GL_H__

#include "Core/Rtt_Config.h"
#include "Renderer/Rtt_GPUResource.h"

#if defined( Rtt_OPENGLES )
	#if defined( Rtt_IPHONE_ENV ) || defined( Rtt_TVOS_ENV )
		#include <OpenGLES/ES2/gl.h>
		#include <OpenGLES/ES2/glext.h>
	#elif defined( Rtt_SYMBIAN_ENV )
		#include <gles/gl.h>
	#elif defined( Rtt_ANDROID_ENV ) || defined( Rtt_WEBOS_ENV ) || defined( Rtt_WIN_PHONE_ENV ) || defined( Rtt_EMSCRIPTEN_ENV )
		#include <GLES2/gl2.h>
		#include <GLES2/gl2ext.h>
	#elif defined( Rtt_POWERVR_ENV )
		#include <GLES/egl.h>
		#include <GLES/gl.h>
	#elif defined(Rtt_NXS_ENV)
		#include <GLES2/gl2.h>
		#include <GLES2/gl2ext.h>
		#include <GLES2/gl2ext_nv.h>
		#include <GLES2/gl2platform.h>
#else
		#error TODO: Add path to gl.h header
	#endif
#elif defined(Rtt_LINUX_ENV)
	#ifdef _WIN32
		#include <GL/glew.h>
	#else
		#include <GL/gl.h>
		#include <GL/glext.h>
	#endif
#elif defined( Rtt_WIN_ENV )
	#define WIN32_LEAN_AND_MEAN
	#include <GL/glew.h>
#elif defined(Rtt_NXS_ENV)
	#include "../../platform/switch/Solar2D/Rtt_NX_GL.h"
#else
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// Convert an opaque handle to an OpenGL name
GLuint HandleToName( GPUResource::Handle handle );

// Convert an OpenGL name to an opaque handle
GPUResource::Handle NameToHandle( GLuint name );

// Check for and log any reported OpenGL errors
void GLCheckError( const char* file, int line );

// Send the given message to the logging system
void GLLogError( const char* message, const char* file, int line );

#ifdef Rtt_DEBUG
	#define GL_CHECK_ERROR() Rtt::GLCheckError( __FILE__, __LINE__ )
	#define GL_LOG_ERROR( message ) Rtt::GLLogError( message, __FILE__, __LINE__ )
#else
	#define GL_CHECK_ERROR()
	#define GL_LOG_ERROR( message )
#endif

// Hide any differences between OpenGL versions
#if defined( Rtt_OPENGLES )

	#if defined( Rtt_WIN_PHONE_ENV )
		#define Rtt_glBindVertexArray( id )
		#define Rtt_glDeleteVertexArrays( count, names )
		#define Rtt_glGenVertexArrays( count, names )
	#else
		#define Rtt_glBindVertexArray( id )								glBindVertexArrayOES( id )
		#define Rtt_glDeleteVertexArrays( count, names )				glDeleteVertexArraysOES( count, names )
		#define Rtt_glGenVertexArrays( count, names )					glGenVertexArraysOES( count, names )
	#endif

	#define Rtt_glBindFragDataLocation
	#define Rtt_glClearDepth			glClearDepthf
	#define Rtt_glDepthRange			glDepthRangef
	#define Rtt_glDisableMultisample()
	#define Rtt_glEnableMultisample()

#else

	#ifdef Rtt_OPENGL_EXT_APPLE
		#define Rtt_glBindVertexArray( id )								glBindVertexArrayAPPLE( id )
		#define Rtt_glDeleteVertexArrays( count, names )				glDeleteVertexArraysAPPLE( count, names )
		#define Rtt_glGenVertexArrays( count, names )					glGenVertexArraysAPPLE( count, names )
	#else
		#define Rtt_glBindVertexArray( id )								glBindVertexArray( id )
		#define Rtt_glDeleteVertexArrays( count, names )				glDeleteVertexArrays( count, names )
		#define Rtt_glGenVertexArrays( count, names )					glGenVertexArrays( count, names )
	#endif

	#define Rtt_glBindFragDataLocation( program, colorNumber, name )	glBindFragDataLocation( program, colorNumber, name )
	#define Rtt_glClearDepth											glClearDepth
	#define Rtt_glDepthRange											glDepthRange
	#define Rtt_glDisableMultisample()									glDisable( GL_MULTISAMPLE )
	#define Rtt_glEnableMultisample()									glEnable( GL_MULTISAMPLE )

#endif
    
// Enable GPU timer queries on supported platforms
#if defined( Rtt_WIN_ENV )
    #define ENABLE_GPU_TIMER_QUERIES
#endif
    
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GL_H__
