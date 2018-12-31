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

#ifndef _Rtt_PlatformSurface_H__
#define _Rtt_PlatformSurface_H__

#include "Rtt_DeviceOrientation.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class PlatformSurface;

// ----------------------------------------------------------------------------

class PlatformSurfaceDelegate
{
	public:
		virtual void WillFlush( const PlatformSurface& surface ) const = 0;
		virtual void DidFlush( const PlatformSurface& surface ) const = 0;
};

class PlatformSurface
{
	public:
		// The density of virtual pixels (in units of virtual pixels per inch)
		// for the virtual pixel lengths: AdaptiveWidth() and AdaptiveHeight()
		//
		// This is the density used in both iOS's point system (roughly)
		// and Android's device-independent pixel (dp) system.
		// http://developer.android.com/guide/topics/resources/more-resources.html#Dimension
		static const S32 kDefaultVirtualDPI;
		static const S32 kUninitializedVirtualLength;
		static S32 CalculateVirtualLength( S32 virtualDPI, S32 screenDPI, S32 screenLength );

	public:
		PlatformSurface();
		virtual ~PlatformSurface() = 0;

	public:
		virtual void SetCurrent() const = 0;
		virtual void Flush() const = 0;

	public:
		// Size in pixels of underlying surface
		virtual S32 Width() const = 0;
		virtual S32 Height() const = 0;

		// Size in native (platform-specific) units. By default, this is simply
		// the result of Width() and Height(). However, on some platforms, the
		// size of the screen is in scaled pixels, e.g. on iPhone, size is defined
		// in terms of "points" not actual pixels.
		virtual S32 ScaledWidth() const;
		virtual S32 ScaledHeight() const;

		// Return width and height of physical device in virtual pixels
		// that preserves the constant pixel density, kAdaptivePixelDensity.
		// The default just returns DeviceWidth() and DeviceHeight().
		// NOTE: The width/height are returned wrt the "upright" orientation.
		virtual S32 AdaptiveWidth() const;
		virtual S32 AdaptiveHeight() const;

	public:
		// Default implementation returns kPortrait to be consistent with
		// defaults for DeviceWidth/DeviceHeight()
		virtual DeviceOrientation::Type GetOrientation() const;

	public:
		// Return width and height of physical device in pixels. The default 
		// implementation just returns result of Width() and Height()
		// NOTE: The width/height are returned wrt the "upright" orientation.
		virtual S32 DeviceWidth() const;
		virtual S32 DeviceHeight() const;

	public:
		virtual void* NativeWindow() const;
		virtual void SetDelegate( PlatformSurfaceDelegate* delegate );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

// TODO: Remove this when OffscreenGPUSurface is moved to a separate file
#include "Rtt_GPU.h"

namespace Rtt
{

// ----------------------------------------------------------------------------

#if ! defined( Rtt_ANDROID_ENV ) && ! defined( Rtt_EMSCRIPTEN_ENV )

// TODO: Move to a separate file
// GPU-specific
class OffscreenGPUSurface : public PlatformSurface
{
	Rtt_CLASS_NO_COPIES( OffscreenGPUSurface )

	public:
		OffscreenGPUSurface( const PlatformSurface& parent );
		virtual ~OffscreenGPUSurface();

	public:
		virtual void SetCurrent() const;
		virtual void Flush() const;

	public:
		virtual S32 Width() const;
		virtual S32 Height() const;

	public:
		bool IsValid() const { return fTexture > 0; }

	protected:
		S32 fWidth;
		S32 fHeight;

		GLuint fFramebuffer; // FBO id
		GLuint fTexture; // texture id
};

#endif // defined( Rtt_ANDROID_ENV )

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformSurface_H__
