//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifdef OLD_GRAPHICS

#include "Core/Rtt_Build.h"

#include "Rtt_EllipsePath.h"
#include "Rtt_RenderingStream.h"
#include "Display/Rtt_DisplayObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

EllipsePath::EllipsePath( Rtt_Allocator* pAllocator, Real a, Real b )
:	Super( pAllocator )
{
	fHalfW = a;
	fHalfH = b;
}

void
EllipsePath::Draw( Renderer& renderer, const DisplayObject* object ) const
{
	DisplayObjectSubmitContext context( rStream, object );

	if ( ! IsProperty( kIsCached ) || ! object )
	{
		Real a = fHalfW;
		Real b = fHalfH;
		if ( Rtt_RealEqual( a, b ) )
		{
			rStream.Submit( fDstOrigin, a );
		}
		else
		{
			rStream.Submit( fDstOrigin, a, b );
		}
	}
	else
	{
		fCache.Draw( rStream, object );
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif