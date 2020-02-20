//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_Allocator.h"
#include "Display/Rtt_DisplayObject.h"
#include "Rtt_SimpleCachedPath.h"
#include "Rtt_FillTesselatorStream.h"
#include "Display/Rtt_ClosedPath.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

SimpleCachedPath::SimpleCachedPath( Rtt_Allocator* pAllocator, ClosedPath& path )
:	Super( path ),
	fCache( pAllocator )
{
}

void
SimpleCachedPath::Update( RenderData& data, const Matrix& srcToDstSpace )
{
#ifdef OLD_GRAPHICS
	// Super::Build( srcToDstSpace );

	// Generate Vertices
	FillTesselatorStream tesselator( & srcToDstSpace, fCache );

	// Ensure that Draw() goes through the non-cached path
//	Rtt_ASSERT( ! OriginalPath().IsCached() );
	OriginalPath().Draw( tesselator, NULL );
#endif
}

void
SimpleCachedPath::Translate( Real dx, Real dy )
{
	ArrayVertex2& vertices = fCache.Vertices();
	Rtt_ASSERT( ( vertices.Length() == 0 ) == ( fCache.Counts().Length() == 0 ) );

	if ( vertices.Length() > 0 )
	{
		Vertex2_Translate( vertices.WriteAccess(), vertices.Length(), dx, dy );
	}
}

void
SimpleCachedPath::Draw( Renderer& renderer ) const
{
#ifdef OLD_GRAPHICS
	Rtt_ASSERT( object );
	Quad stageQuad;
	const Quad* pSubmitQuad = object->GetSubmitQuad( stageQuad );

	const ArrayS32& counts = fCache.Counts();
	rStream.WillSubmitArray( * pSubmitQuad );
	rStream.Submit(
		RenderTypes::kFan,
		fCache.Vertices().ReadAccess(),
		counts.ReadAccess(),
		counts.Length() );
	rStream.DidSubmitArray();
#endif
}

void
SimpleCachedPath::Invalidate()
{
	fCache.Vertices().Empty();
	fCache.Counts().Empty();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
