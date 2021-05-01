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
	#elif defined(Rtt_NINTENDO_ENV)
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
#elif defined(Rtt_NINTENDO_ENV)
	#include <GL/glew.h>
#else
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
#endif

// ----------------------------------------------------------------------------

#if defined(Rtt_NINTENDO_ENV)
#define glDeleteTextures s_glDeleteTextures
#define glGetError s_glGetError
#define glGetIntegerv s_glGetIntegerv
#define glGetString s_glGetString
#define glDisable s_glDisable
#define glEnable s_glEnable
#define glBlendFunc s_glBlendFunc
#define glViewport s_glViewport
#define glScissor s_glScissor
#define glClearColor s_glClearColor
#define glClear s_glClear
#define glDrawArrays s_glDrawArrays
#define glDrawElements s_glDrawElements
#define glGenTextures s_glGenTextures
#define glBindTexture s_glBindTexture
#define glTexParameteri s_glTexParameteri
#define glTexImage2D s_glTexImage2D
#define glTexSubImage2D s_glTexSubImage2D
#define glReadPixels s_glReadPixels


typedef void (GLAPIENTRY* TYPE_glClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (GLAPIENTRY* TYPE_glDeleteTextures) (GLsizei n, const GLuint* textures);
typedef GLenum(GLAPIENTRY* TYPE_glGetError) (void);
typedef void (GLAPIENTRY* TYPE_glGetIntegerv) (GLenum pname, GLint* params);
typedef const GLubyte* (GLAPIENTRY* TYPE_glGetString) (GLenum name);
typedef void (GLAPIENTRY* TYPE_glDisable) (GLenum cap);
typedef void (GLAPIENTRY* TYPE_glEnable) (GLenum cap);
typedef void (GLAPIENTRY* TYPE_glBlendFunc) (GLenum sfactor, GLenum dfactor);
typedef void (GLAPIENTRY* TYPE_glViewport) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GLAPIENTRY* TYPE_glScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GLAPIENTRY* TYPE_glClear) (GLbitfield mask);
typedef void (GLAPIENTRY* TYPE_glDrawArrays) (GLenum mode, GLint first, GLsizei count);
typedef void (GLAPIENTRY* TYPE_glDrawElements) (GLenum mode, GLsizei count, GLenum type, const void* indices);
typedef void (GLAPIENTRY* TYPE_glGenTextures) (GLsizei n, GLuint* textures);
typedef void (GLAPIENTRY* TYPE_glBindTexture) (GLenum target, GLuint texture);
typedef void (GLAPIENTRY* TYPE_glTexParameteri) (GLenum target, GLenum pname, GLint param);
typedef void (GLAPIENTRY* TYPE_glTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
typedef void (GLAPIENTRY* TYPE_glTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
typedef void (GLAPIENTRY* TYPE_glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);

extern TYPE_glClearColor s_glClearColor;
extern TYPE_glDeleteTextures s_glDeleteTextures;
extern TYPE_glGetError s_glGetError;
extern TYPE_glGetIntegerv s_glGetIntegerv;
extern TYPE_glGetString s_glGetString;
extern TYPE_glDisable s_glDisable;
extern TYPE_glEnable s_glEnable;
extern TYPE_glBlendFunc s_glBlendFunc;
extern TYPE_glViewport s_glViewport;
extern TYPE_glScissor s_glScissor;
extern TYPE_glClear s_glClear;
extern TYPE_glDrawArrays s_glDrawArrays;
extern TYPE_glDrawElements s_glDrawElements;
extern TYPE_glGenTextures s_glGenTextures;
extern TYPE_glBindTexture s_glBindTexture;
extern TYPE_glTexParameteri s_glTexParameteri;
extern TYPE_glTexImage2D s_glTexImage2D;
extern TYPE_glTexSubImage2D s_glTexSubImage2D;
extern TYPE_glReadPixels s_glReadPixels;



#endif


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
    //#define ENABLE_GPU_TIMER_QUERIES
#endif
    
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GL_H__
