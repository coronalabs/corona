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
