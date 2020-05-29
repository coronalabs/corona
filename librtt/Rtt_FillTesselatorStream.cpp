//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_FillTesselatorStream.h"

#include "Rtt_Matrix.h"
#include "Display/Rtt_VertexCache.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

FillTesselatorStream::FillTesselatorStream( const Matrix* srcToDstSpace, VertexCache& cache )
:	Super( srcToDstSpace, cache )
{
}

void
FillTesselatorStream::Submit( const VertexArray& vertices )
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

void
FillTesselatorStream::Submit( const Quad vertices )
{
	Rtt_ASSERT_NOT_REACHED();
}

void
FillTesselatorStream::Submit( const Vertex2& origin, Real rectHalfW, Real rectHalfH, Real radius )
{
	if ( Rtt_RealIsZero( radius ) )
	{
		AppendRectangle( rectHalfW, rectHalfH );
	}
	else
	{
		ArrayVertex2& vertices = fCache.Vertices();

		Rtt_ASSERT( vertices.Length() == 0 );

		// The "corners" of the rounded rect are just circle segments 
		AppendCircle( radius, true );
		S32 length = vertices.Length(); Rtt_ASSERT( ( length & 0x3 ) == 0 );

		Real halfW = rectHalfW - radius;
		Real halfH = rectHalfH - radius;

		length = length >> 2;
		Vertex2* pVertices = vertices.WriteAccess();

		// Position origin of each circle segment appropriately
		Vertex2_Translate( pVertices, length, halfW, halfH ); pVertices += length;
		Vertex2_Translate( pVertices, length, -halfW, halfH ); pVertices += length;
		Vertex2_Translate( pVertices, length, -halfW, -halfH ); pVertices += length;
		Vertex2_Translate( pVertices, length, halfW, -halfH );

		// Add rest of rounded rectangle
		halfH = rectHalfH;

		const Vertex2 bottomLeft =	{ -halfW, -halfH };
		const Vertex2 upperLeft =	{ -halfW, halfH };
		const Vertex2 upperRight =	{ halfW, halfH };
		const Vertex2 bottomRight =	{ halfW, -halfH };

		// Middle rect
		vertices.Append( bottomLeft );
		vertices.Append( upperLeft );
		vertices.Append( upperRight );
		vertices.Append( bottomRight );

		ArrayS32& counts = fCache.Counts();
		counts.Append( 4 );

		halfW = rectHalfW;
		halfH -= radius;

		// Left trapezoid
		vertices.Append( bottomLeft );
		Vertex2 p = { -halfW, -halfH };
		vertices.Append( p );
		p.y = halfH;
		vertices.Append( p );
		vertices.Append( upperLeft );
		counts.Append( 4 );

		// Right trapezoid
		vertices.Append( bottomRight );
		vertices.Append( upperRight );
		p.x = halfW;
		vertices.Append( p );
		p.y = -halfH;
		vertices.Append( p );
		counts.Append( 4 );
	}

	ApplyTransform( origin );
}

void
FillTesselatorStream::Submit( const Vertex2& origin, Real a, Real b )
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

void
FillTesselatorStream::Submit( const Vertex2& origin, Real radius )
{
	// TODO: Everytime this is called, we subdivide the same exact unit circle
	// Precalculate this and put it into a constant table for lookup.
	// This would completely eliminate the need to call Subdivide
	// Also can be used for the rounded rects

	AppendCircle( radius, false );
	ApplyTransform( origin );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

