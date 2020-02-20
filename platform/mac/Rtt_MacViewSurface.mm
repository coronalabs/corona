//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

