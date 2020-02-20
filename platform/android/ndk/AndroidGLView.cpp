//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

