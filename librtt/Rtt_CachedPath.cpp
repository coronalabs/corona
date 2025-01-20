//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_CachedPath.h"
#include "Rtt_RenderingStream.h"
#include "Display/Rtt_ClosedPath.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

CachedPath::CachedPath( ClosedPath& path )
:	fPath( path )
{
}
/*
void
CachedPath::Build( const Matrix& srcToDstSpace )
{
//	Rtt_ASSERT( ! IsValid() );

	fPath.Build( srcToDstSpace );

//	SetValid();
}
*/
void
CachedPath::GetSelfBounds( Rect& rect ) const
{
	fPath.GetSelfBounds( rect );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

