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

#include "Rtt_MacViewSurface.h"

#import "GLView.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

MacViewSurface::MacViewSurface( GLView* view )
:	fView( view ),
	fDelegate( NULL ),
	fAdaptiveWidth( Super::kUninitializedVirtualLength ),
	fAdaptiveHeight( Super::kUninitializedVirtualLength )
{
}

MacViewSurface::MacViewSurface( GLView* view, S32 adaptiveWidth, S32 adaptiveHeight )
:	fView( view ),
	fDelegate( NULL ),
	fAdaptiveWidth( adaptiveWidth ),
	fAdaptiveHeight( adaptiveHeight )
{
}

MacViewSurface::~MacViewSurface()
{
//	[fView release];
}

void
MacViewSurface::SetCurrent() const
{

}
void
MacViewSurface::Flush() const
{
}

void
MacViewSurface::SetDelegate( Delegate* delegate )
{
	fDelegate = delegate;
}

S32
MacViewSurface::Width() const
{
	S32 result = [fView viewportWidth];
	return result;
}

S32
MacViewSurface::Height() const
{
	S32 result = [fView viewportHeight];
	return result;
}

DeviceOrientation::Type
MacViewSurface::GetOrientation() const
{
	return [fView orientation];
}

S32
MacViewSurface::AdaptiveWidth() const
{
	return ( fAdaptiveWidth > 0 ? fAdaptiveWidth : Super::AdaptiveWidth() );
}

S32
MacViewSurface::AdaptiveHeight() const
{
	return ( fAdaptiveHeight > 0 ? fAdaptiveHeight : Super::AdaptiveHeight() );
}

S32
MacViewSurface::DeviceWidth() const
{
	return [fView deviceWidth];
}

S32
MacViewSurface::DeviceHeight() const
{
	return [fView deviceHeight];
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

