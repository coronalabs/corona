//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_SimpleCachedPath_H__
#define _Rtt_SimpleCachedPath_H__

#include "Rtt_CachedPath.h"

#include "Display/Rtt_VertexCache.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// Why do we need a SimpleCachedPath class?
// Maybe so we know we that all triangles actually contribute --- no stencil
// even/oddness to deal with.
//
// Or maybe for caching vertices?  But then how to represent cache?
// Maybe RenderingStream be responsible for associating paths with cache?
class SimpleCachedPath : public CachedPath
{
	public:
		typedef CachedPath Super;

	public:
		SimpleCachedPath( Rtt_Allocator* pAllocator, ClosedPath& path );

	public:
		virtual void Update( RenderData& data, const Matrix& srcToDstSpace );
		virtual void Translate( Real dx, Real dy );
		virtual void Draw( Renderer& renderer ) const;

	public:
		void Invalidate();

	protected:
		VertexCache fCache;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_SimpleCachedPath_H__
