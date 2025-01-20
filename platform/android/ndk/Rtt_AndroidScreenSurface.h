//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidScreenSurface_H__
#define _Rtt_AndroidScreenSurface_H__

#include "Rtt_PlatformSurface.h"

#include <GLES2/gl2.h>

// ----------------------------------------------------------------------------

class AndroidGLContext;
class AndroidGLView;

namespace Rtt
{

// ----------------------------------------------------------------------------

class AndroidScreenSurface : public PlatformSurface
{
	Rtt_CLASS_NO_COPIES( AndroidScreenSurface )

	public:
		typedef PlatformSurface Super;

	public:
		AndroidScreenSurface( AndroidGLView* view, S32 approximateScreenDpi );
		virtual ~AndroidScreenSurface();

	public:
		virtual void SetCurrent() const;
		virtual void Flush() const;

	public:
		void* NativeWindow() const;
		virtual S32 Width() const;
		virtual S32 Height() const;

		AndroidGLContext* GetContext() const;

		virtual DeviceOrientation::Type GetOrientation() const;
		virtual S32 DeviceWidth() const;
		virtual S32 DeviceHeight() const;

		virtual S32 AdaptiveWidth() const;
		virtual S32 AdaptiveHeight() const;

	private:
		AndroidGLView* fView;
		GLuint fFramebuffer; // FBO id
		S32 fApproximateScreenDPI;
};

class AndroidOffscreenSurface : public PlatformSurface
{
	public:
		AndroidOffscreenSurface( const PlatformSurface& parent );
		virtual ~AndroidOffscreenSurface();

		virtual void SetCurrent() const;
		virtual void Flush() const;
		virtual S32 Width() const;
		virtual S32 Height() const;

		static bool IsSupported();

	private:
		S32 fWidth;
		S32 fHeight;

		GLuint fFramebuffer; // FBO id
		GLuint fTexture; // texture id
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidScreenSurface_H__
