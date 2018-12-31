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


#include "Core/Rtt_Build.h"

#include "AndroidGLView.h"
#include "NativeToJavaBridge.h"

#include "Rtt_Runtime.h"
#include "Rtt_AndroidBitmap.h"
#include "Rtt_RenderingStream.h"

#include "Rtt_Event.h"
#include "Rtt_GPU.h"

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <android/log.h>

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

// At some point in the future, these could implement lower level GL drawing, but
// we're going to go with the default for now.

void
AndroidGLView::Flush()
{
	fNativeToJavaBridge->DisplayUpdate();
}

bool 
AndroidGLView::CreateFramebuffer( int width, int height, Rtt::DeviceOrientation::Type orientation )
{
#ifdef Rtt_DEBUG
	__android_log_print(ANDROID_LOG_INFO, "Corona", "AndroidGLView::CreateFramebuffer %d %d %d", width, height, orientation );
#endif

	fWidth = width;
	fHeight = height;
	fOrientation = orientation;

	switch ( orientation )
	{
		case Rtt::DeviceOrientation::kSidewaysLeft:
		case Rtt::DeviceOrientation::kSidewaysRight:
			fUprightWidth = height;
			fUprightHeight = width;
			break;
		default:
			fUprightWidth = width;
			fUprightHeight = height;
			break;
	}

	return true;
}

void 
AndroidGLView::DestroyFramebuffer()
{
}

void 
AndroidGLView::Render()
{
	if ( fRuntime )
	{
		(*fRuntime)();
	}
}

void
AndroidGLView::Resize( int width, int height )
{
	// NOTE: We assume caller also makes proper notification to Corona about
	// changes to the surface size including possible orientation changes.
	// This call is purely to resize the dimensions stored by the receiver.
	fWidth = width;
	fHeight = height;
	switch ( fOrientation )
	{
		case Rtt::DeviceOrientation::kSidewaysLeft:
		case Rtt::DeviceOrientation::kSidewaysRight:
			fUprightWidth = height;
			fUprightHeight = width;
			break;
		default:
			fUprightWidth = width;
			fUprightHeight = height;
			break;
	}
}

// TODO: Get rid of this. Used by old (broken) suspend/resume implementation
void 
AndroidGLView::ReinitializeRenderingStream()
{
	fReloadTextures = true;
}

/// Determines if this OpenGL view is set up with an alpha channel.
/// This is determined by calling glGetIntegerv(GL_ALPHA_BITS, value).
/// @return Returns true if this view supports alpha. Returns false if not.
bool
AndroidGLView::HasAlphaChannel()
{
	GLint alphaBits = 0;
	glGetIntegerv(GL_ALPHA_BITS, &alphaBits);
	return (alphaBits > 0);
}

// ----------------------------------------------------------------------------
	
// ----------------------------------------------------------------------------

