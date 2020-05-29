//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_FillTesselatorStream_H__
#define _Rtt_FillTesselatorStream_H__

#include "Rtt_TesselatorStream.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class VertexCache;

// ----------------------------------------------------------------------------

// TODO: Rename this to FillTesselatorStream
class FillTesselatorStream : public TesselatorStream
{
	public:
		typedef TesselatorStream Super;

	public:
		// If non-NULL, srcToDstSpace is used to transform all generated
		// vertices. It overrides the "origin" parameter for the Submit() 
		// methods (i.e. rounded rectangle, circle, ellipse). If NULL, then
		// the origin parameter is used to displace (translate) the vertices.
		FillTesselatorStream( const Matrix* srcToDstSpace, VertexCache& cache );

	public:
		virtual void Submit( const VertexArray& vertices );
///		virtual void Submit( RenderTypes::Mode mode, const Vertex2* vertices, const S32* counts, S32 numCounts );
		virtual void Submit( const Quad vertices );
		virtual void Submit( const Vertex2& origin, Real rectHalfW, Real rectHalfH, Real radius );
		virtual void Submit( const Vertex2& origin, Real radius );
		virtual void Submit( const Vertex2& origin, Real a, Real b );

	private:
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_FillTesselatorStream_H__
