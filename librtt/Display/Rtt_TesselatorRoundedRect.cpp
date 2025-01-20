//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_TesselatorRoundedRect.h"

#include "Rtt_Matrix.h"
#include "Rtt_Transform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

TesselatorRoundedRect::TesselatorRoundedRect( Real w, Real h, Real radius )
:	Super( w, h ),
	fRadius( radius )
{
}

void
TesselatorRoundedRect::AppendRoundedRect( ArrayVertex2& vertices, Real halfW, Real halfH, Real originalRadius )
{
	Real radius = Clamp( originalRadius, 0.f, Min(halfW, halfH) );
	if ( Rtt_RealIsZero( radius ) )
	{
		AppendRect( vertices, halfW, halfH );
	}
	else
	{
		// The "corners" of the rounded rect are just circle segments 
		AppendCircleQuadrants( vertices, radius, 0 );

		S32 length = vertices.Length(); Rtt_ASSERT( ( length & 0x3 ) == 0 );

		// Origin of first circle quadrant
		Real x = halfW - radius;
		Real y = halfH - radius;

		length = length >> 2;
		Vertex2 *pVertices = vertices.WriteAccess();

		// Position origin of each circle quadrant appropriately
		Vertex2_Translate( pVertices, length,  x,  y ); pVertices += length;
		Vertex2_Translate( pVertices, length, -x,  y ); pVertices += length;
		Vertex2_Translate( pVertices, length, -x, -y ); pVertices += length;
		Vertex2_Translate( pVertices, length,  x, -y );

		// Circle quadrant origins (in fan order)
		const Vertex2 origin1 =	{  x,  y };
		const Vertex2 origin2 =	{ -x,  y };
		const Vertex2 origin3 =	{ -x, -y };
		const Vertex2 origin4 =	{  x, -y };

		// Complete border
		const Vertex2 p1 = { x + radius, y }; // perimeter pt of 1st quadrant, along x-direction
		vertices.Append( p1 );
		vertices.Append( origin1 );

		// Add interior rect.
		// NOTE: we add o1 again to create a degenerate triangle
		vertices.Append( origin1 );
		vertices.Append( origin2 );
		vertices.Append( origin4 ); // [1,2,4,3] is tri strip order
		vertices.Append( origin3 );
	}
}

void
TesselatorRoundedRect::GenerateFill( ArrayVertex2& vertices )
{
	Rtt_ASSERT( vertices.Length() == 0 );

	AppendRoundedRect( vertices, fHalfW, fHalfH, fRadius );
}

void
TesselatorRoundedRect::GenerateFillTexture( ArrayVertex2& texCoords, const Transform& t )
{
	ArrayVertex2& vertices = texCoords;

	Rtt_ASSERT( vertices.Length() == 0 );

	// Unit circle
	AppendRoundedRect( vertices, fHalfW, fHalfH, fRadius );
	Normalize( vertices );
	
	Real w = Rtt_RealMul2( fHalfW );
	Real h = Rtt_RealMul2( fHalfH );
	Real invW = Rtt_RealDivNonZeroAB( Rtt_REAL_1, w );
	Real invH = Rtt_RealDivNonZeroAB( Rtt_REAL_1, h );

	if ( t.IsIdentity() )
	{
		for ( int i = 0, iMax = vertices.Length(); i < iMax; i++ )
		{
			Vertex2& v = vertices[i];
			v.x = Rtt_RealMul( v.x + fHalfW, invW );
			v.y = Rtt_RealMul( v.y + fHalfH, invH );
		}
	}
	else
	{
		Matrix m;
		m.Scale( invW * t.GetSx(), invH * t.GetSy() );
		m.Rotate( - t.GetRotation() );
		m.Translate( t.GetX() + Rtt_REAL_HALF, t.GetY() + Rtt_REAL_HALF );
		m.Apply( vertices.WriteAccess(), vertices.Length() );
	}
}

void
TesselatorRoundedRect::GenerateStroke( ArrayVertex2& vertices )
{
	Real radius = fRadius;

	AppendCircleStroke( vertices, radius, fInnerWidth, fOuterWidth, true );

	Real halfW = fHalfW - radius;
	Real halfH = fHalfH - radius;

	const S32 numVertices = vertices.Length();

	// An add'l 2 vertices were added at the end to close the loop
	S32 length = numVertices - 2; Rtt_ASSERT( ( length & 0x3 ) == 0 );
	length = length >> 2;
	Vertex2* pVertices = vertices.WriteAccess();

	// Position origin of each circle segment appropriately
	Vertex2_Translate( pVertices, length, halfW, halfH ); pVertices += length;
	Vertex2_Translate( pVertices, length, -halfW, halfH ); pVertices += length;
	Vertex2_Translate( pVertices, length, -halfW, -halfH ); pVertices += length;
	Vertex2_Translate( pVertices, length, halfW, -halfH ); pVertices += length;

	// Last 2 vertices close the loop
	Vertex2_Translate( pVertices, 2, halfW, halfH );
}

void
TesselatorRoundedRect::GetSelfBounds( Rect& rect )
{
	rect.Initialize( fHalfW, fHalfH );
}

U32
TesselatorRoundedRect::FillVertexCount() const
{
	Real radius = Clamp( fRadius, 0.f, Min(fHalfW, fHalfH) );
	if ( Rtt_RealIsZero( radius ) )
	{
		return 4U;
	}
	else
	{ 
		return 6U + AppendCircleQuadrantsCount( radius, 0 );
	}
}

U32
TesselatorRoundedRect::StrokeVertexCount() const
{
	return AppendCircleStrokeCount( fRadius, true );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

