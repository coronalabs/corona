//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_EGLContext_H__
#define _Rtt_EGLContext_H__

#include <gles/egl.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

class MPlatform;

// ----------------------------------------------------------------------------

class EGLContext
{
	public:
		EGLContext();
		virtual ~EGLContext();

	public:
		bool Initialize( const MPlatform& platform, NativeWindowType window, int bitsPerPixel );
		void Flush() const;

	protected:
		bool InitDisplay();
		bool InitConfig( EGLConfig& rConfig, int bitsPerPixel );

	private:
		EGLDisplay fEglDisplay;
		EGLSurface fEglSurface;
		::EGLContext fEglContext;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_EGLContext_H__

// ----------------------------------------------------------------------------
