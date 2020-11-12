//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_VertexCache.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

VertexCache::VertexCache( Rtt_Allocator* pAllocator )
:	fVertices(pAllocator),
	fTexVertices(pAllocator),
	fColors(pAllocator),
	fCounts(pAllocator)
{
}

void
VertexCache::Invalidate()
{
	fVertices.Empty();
	fTexVertices.Empty();
	fCounts.Empty();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

