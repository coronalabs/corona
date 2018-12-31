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

#include "Rtt_IPhoneScreenSurface.h"

#import <Availability.h>
#import <GLKit/GLKView.h>
#import <UIKit/UIScreen.h>
//#import <OpenGLES/EAGL.h>
#include "Rtt_GPU.h"

#ifdef Rtt_ORIENTATION
#include "Rtt_IPhoneDevice.h"
#endif

#import "CoronaOrientationProvider.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhoneScreenSurface::IPhoneScreenSurface( GLKView *view )
:	fView( view ) // NOTE: Weak ref b/c fView owns the Runtime instance that owns this.
{
	// NOTE: We assume CoronaView's didMoveToWindow is called already
	// which for iOS8, correctly sets this value to UIScreen's nativeScale value.
	fScale = view.contentScaleFactor;
}

IPhoneScreenSurface::~IPhoneScreenSurface()
{
}

void
IPhoneScreenSurface::SetCurrent() const
{
	Rtt_ASSERT( [EAGLContext currentContext] == fView.context );
//	[EAGLContext setCurrentContext:fView.context];

	//glBindFramebufferOES( GL_FRAMEBUFFER_OES, fView.viewFramebuffer );
}

void
IPhoneScreenSurface::Flush() const
{

}

DeviceOrientation::Type
IPhoneScreenSurface::GetOrientation() const
{
#ifdef Rtt_ORIENTATION
	UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
	return IPhoneDevice::ToOrientationTypeFromUIInterfaceOrientation( orientation );
#else
	// TODO: Pull this out to tvOS-specific code.
	#ifdef Rtt_TVOS_ENV
		return DeviceOrientation::kSidewaysRight;
	#else
		return DeviceOrientation::kUpright;
	#endif
#endif
}

S32
IPhoneScreenSurface::Width() const
{
	// Return size in pixels
	return fScale * fView.bounds.size.width;
}

S32
IPhoneScreenSurface::Height() const
{
	// Return size in pixels
	return fScale * fView.bounds.size.height;
}

S32
IPhoneScreenSurface::ScaledWidth() const
{
	// Return size in points
	return fView.bounds.size.width;
}

S32
IPhoneScreenSurface::ScaledHeight() const
{
	// Return size in points
	return fView.bounds.size.height;
}

S32
IPhoneScreenSurface::DeviceWidth() const
{
	return IsUpright() ? Width() : Height();
}

S32
IPhoneScreenSurface::DeviceHeight() const
{
	return IsUpright() ? Height() : Width();
}

S32
IPhoneScreenSurface::AdaptiveWidth() const
{
	return IsUpright() ? fView.bounds.size.width : fView.bounds.size.height;
}

S32
IPhoneScreenSurface::AdaptiveHeight() const
{
	return IsUpright() ? fView.bounds.size.height : fView.bounds.size.width;
}

bool
IPhoneScreenSurface::IsUpright() const
{
	bool result = true;

	if ( [fView conformsToProtocol:@protocol(CoronaOrientationProvider)] )
	{
		id <CoronaOrientationProvider> orientationProvider = (id <CoronaOrientationProvider>)fView;
		result = [orientationProvider isUpright];
	}
	return result;
}

EAGLContext*
IPhoneScreenSurface::GetContext() const
{
	return fView.context;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

