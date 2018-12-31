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

