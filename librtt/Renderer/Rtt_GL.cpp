//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_GL.h"

#include "Core/Rtt_Assert.h"

#if defined(_MSC_VER) && _MSC_VER < 1600
	#ifdef  _WIN64
	typedef unsigned __int64 uintptr_t;
	#else
	typedef _W64 unsigned int uintptr_t;
	#endif
#else
	#include <stdint.h>
#endif
#if defined(Rtt_WIN_DESKTOP_ENV)
	#include <Windows.h>
	#include <WinGDI.h>
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

GLuint HandleToName( GPUResource::Handle handle )
{
	return static_cast<GLuint>( reinterpret_cast<uintptr_t>( handle ) );
}

GPUResource::Handle NameToHandle( GLuint name )
{
	return reinterpret_cast<void*>( static_cast<uintptr_t>( name ) );
}

void GLCheckError( const char* file, int line )
{
#if defined(Rtt_WIN_DESKTOP_ENV)
//TODO: We may want to move the following wglGetCurrentContext() function call to a separate cross-platform file.
	// On Windows, ignore OpenGL errors if a context has not been selected. They will safely no-op.
	// This is needed because we are unable to select an OpenGL context of a window/control that is
	// being destroyed, which means Corona GPUResource objects cannot destroy their resources anymore.
	// All we can do is destroy the window/control's OpenGL context itself instead.
	if (::wglGetCurrentContext() == NULL)
	{
		return;
	}
#endif
	GLenum error = glGetError();
	if( ! Rtt_VERIFY( GL_NO_ERROR == error ) )
	{
		switch( error )
		{
			case GL_INVALID_ENUM:
				GLLogError("GL_INVALID_ENUM (%s:%d)", file, line);
				break;
			case GL_INVALID_VALUE:
				GLLogError("GL_INVALID_VALUE (%s:%d)", file, line);
				break;
			case GL_INVALID_OPERATION:
				GLLogError("GL_INVALID_OPERATION (%s:%d)", file, line);
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				GLLogError("GL_INVALID_FRAMEBUFFER_OPERATION (%s:%d)", file, line);
				break;
			case GL_OUT_OF_MEMORY:
				GLLogError("GL_OUT_OF_MEMORY (%s:%d)", file, line);
				break;
			default:
				GLLogError("Unknown OpenGL error (%s:%d)", file, line);
				break;
		}
	}
}

void GLLogError( const char* message, const char* file, int line )
{
	Rtt_LogException( message, file, line );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

