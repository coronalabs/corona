//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_TesselatorRect.h"
#include "Display/Rtt_TesselatorLine.h"
#include "Rtt_Transform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// TODO: Move to separate file
TesselatorRectBase::TesselatorRectBase( Real w, Real h )
:	Super(),
	fHalfW( Rtt_RealDiv2( w ) ),
	fHalfH( Rtt_RealDiv2( h ) )
{
}

bool
TesselatorRectBase::SetSelfBounds( Real width, Real height )
{
	// TODO: We do not account for stroke width
	if ( !( width < Rtt_REAL_0 ) ) // (width >= 0)
	{
		SetWidth( width );
	}

	if ( !( height < Rtt_REAL_0 ) ) // (height >= 0)
	{
		SetHeight( height );
	}

	return true;
}

// ----------------------------------------------------------------------------

TesselatorRect::TesselatorRect( Real w, Real h )
:	Super( w, h ),
	fOffsetExists( 0 )
{
	// IEEE standard rep of 0.f is 0x0
	memset( fOffsets, 0, sizeof( fOffsets ) );
}

void
TesselatorRect::GenerateFill( ArrayVertex2& vertices )
{
	Rtt_ASSERT( vertices.Length() == 0 );

	if ( ! HasOffset() )
	{
		AppendRect( vertices, fHalfW, fHalfH );
	}
	else
	{
		// NOTE: Enum offsets are in counterclockwise fan order
		// while the x,y variables are in strip order
		Vertex2 quad[] =
		{
			{ -fHalfW + GetOffset( kX0 ), -fHalfH + GetOffset( kY0 ) },
			{ -fHalfW + GetOffset( kX1 ),  fHalfH + GetOffset( kY1 ) },
			{  fHalfW + GetOffset( kX3 ), -fHalfH + GetOffset( kY3 ) },
			{  fHalfW + GetOffset( kX2 ),  fHalfH + GetOffset( kY2 ) },
		};

		vertices.Append( quad[0] );
		vertices.Append( quad[1] );
		vertices.Append( quad[2] );
		vertices.Append( quad[3] );
	}
}

// Assumes points are passed in triangle fan order, not tri strip order
// Results also returned in fan order.
static void
getDgnlIntrsctP12andP34(
	float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
	float *outDiagonals )
{
	float xp2 = x2 - x1; // P2.x - P1.x ;
	float xp3 = x3 - x1; // P3.x - P1.x ;
	float xp4 = x4 - x1; // P4.x - P1.x ;
	float yp2 = y2 - y1; // P2.y - P1.y ;
	float yp3 = y3 - y1; // P3.y - P1.y ;
	float yp4 = y4 - y1; // P4.y - P1.y ;
	float x, y;

	// http://mathworld.wolfram.com/Line-LineIntersection.html
	// x = x1 + xp2*(xp3*yp4-xp4*yp3) / ((x3-x4)*yp2-xp2*(y3-y4))
	// y = y1 + (xp3*yp4-xp4*yp3)*yp2 / ((x3-x4)*yp2-xp2*(y3-y4))
	float x3y4mx4y3 = xp3 * yp4 - xp4 * yp3 ;
	float denominator = ( x3 - x4 ) * yp2 - xp2 * ( y3 - y4 ) ;
	// float denominator = ( P3.x - P4.x ) * yp2 - xp2 * ( P3.y - P4.y ) ;

	x = xp2 * x3y4mx4y3 / denominator ;
	y = yp2 * x3y4mx4y3 / denominator ;

	xp2 -= x ;
	xp3 -= x ;
	xp4 -= x ;
	yp2 -= y ;
	yp3 -= y ;
	yp4 -= y ;

	outDiagonals[ 0 ] = sqrt( x * x + y * y ) ;
	outDiagonals[ 1 ] = sqrt( xp2 * xp2 + yp2 * yp2 ) ;
	outDiagonals[ 2 ] = sqrt( xp3 * xp3 + yp3 * yp3 ) ;
	outDiagonals[ 3 ] = sqrt( xp4 * xp4 + yp4 * yp4 ) ;
// printf( "\t{%4.3g %4.3g %4.3g %4.3g}\n" , outDiagonals[ 0 ] , outDiagonals[ 1 ] , outDiagonals[ 2 ] , outDiagonals[ 3 ] ) ;
}

static const Vertex2 kDefaultUV[] =
{
	{ Rtt_REAL_0, Rtt_REAL_0 },
	{ Rtt_REAL_0, Rtt_REAL_1 },
	{ Rtt_REAL_1, Rtt_REAL_0 },
	{ Rtt_REAL_1, Rtt_REAL_1 },
};

void
TesselatorRect::GenerateFillTexture( ArrayVertex2& texCoords, const Transform& t )
{
	ArrayVertex2& vertices = texCoords;

	// Standard texture coordinates
	Quad uv;
	memcpy( uv, kDefaultUV, sizeof( Quad ) );

	// Transform uv's if necessary
	bool isIdentity = t.IsIdentity();
	bool shouldNormalize = ShouldNormalize();
	if ( ! isIdentity || shouldNormalize )
	{
		Matrix m;

		m.Translate( -Rtt_REAL_HALF, -Rtt_REAL_HALF ); // Center uv's about origin
		{
			if ( shouldNormalize )
			{
				// Apply scale *after* uv's are centered about origin
				// and *before* the texture transform t.
				Real sx = GetScaleX();
				if ( Rtt_RealIsZero( sx ) ) { sx = Rtt_REAL_1; }
				Real sy = GetScaleY();
				if ( Rtt_RealIsZero( sy ) ) { sy = Rtt_REAL_1; }
				m.Scale( sx, sy );
			}
			if ( ! isIdentity )
			{
				m.Scale( t.GetSx(), t.GetSy() );
				m.Rotate( - t.GetRotation() );
				m.Translate( t.GetX(), t.GetY() );
			}
		}
		m.Translate( Rtt_REAL_HALF, Rtt_REAL_HALF ); // Move uv's back

		m.Apply( uv, 4 );
	}

	if ( HasOffset() )
	{
		// TODO: Remove duplication of this code with GenerateFill() above

		// NOTE: Enum offsets are in counterclockwise fan order
		// while the x,y variables are in strip order
		const Vertex2 quad[] =
		{
			{ -fHalfW + GetOffset( kX0 ), -fHalfH + GetOffset( kY0 ) },
			{ -fHalfW + GetOffset( kX1 ),  fHalfH + GetOffset( kY1 ) },
			{  fHalfW + GetOffset( kX3 ), -fHalfH + GetOffset( kY3 ) },
			{  fHalfW + GetOffset( kX2 ),  fHalfH + GetOffset( kY2 ) },
		};

		float diagonals[4];
		// Note: This function expects P1 and P2 to be a diagonal, likewise for P3 and P4.
		// Therefore, we need to permute our points (tri strip order):
		//    P1 <= (x0,y0), P2 <= (x3,y3), P3 <= (x1,y1), P4 <= (x2,y2)
		getDgnlIntrsctP12andP34(
			quad[0].x, quad[0].y,
			quad[3].x, quad[3].y,
			quad[1].x, quad[1].y,
			quad[2].x, quad[2].y,
			diagonals );

		// The resulting diagonals come out in a corresponding order, so we need to
		// permute them back so the coefficients are in tri-strip order:
		//    c0 <= P1, c1 <= P3, c2 <= P4, c3 <= P2
		//    c0(d0,d1), c1(d2,d3), c2(d3,d2), c3(d1,d0)
		fTexCoeff[0] = 1 + diagonals[0] / diagonals[1];
		fTexCoeff[1] = 1 + diagonals[2] / diagonals[3];
		fTexCoeff[2] = 1 + diagonals[3] / diagonals[2];
		fTexCoeff[3] = 1 + diagonals[1] / diagonals[0];
	}

	vertices.Append( uv[0] );
	vertices.Append( uv[1] );
	vertices.Append( uv[2] );
	vertices.Append( uv[3] );
}

void
TesselatorRect::GenerateStroke( ArrayVertex2& vertices )
{
	const Real innerWidth = GetInnerWidth();
	const Real outerWidth = GetOuterWidth();

	if( ! HasOffset() )
	{
		Real halfW = fHalfW;
		Real halfH = fHalfH;

		// Src vertices in strip order
		const Vertex2 kSrc[] = {
			// 0.
			{ -halfW + innerWidth + GetOffset( kX0 ), -halfH + innerWidth + GetOffset( kY0 ) },
			{ -halfW - outerWidth + GetOffset( kX0 ), -halfH - outerWidth + GetOffset( kY0 ) },
			
			// 1.
			{ -halfW + innerWidth + GetOffset( kX1 ),  halfH - innerWidth + GetOffset( kY1 ) },
			{ -halfW - outerWidth + GetOffset( kX1 ),  halfH + outerWidth + GetOffset( kY1 ) },
			
			// 2.
			{  halfW - innerWidth + GetOffset( kX2 ),  halfH - innerWidth + GetOffset( kY2 ) },
			{  halfW + outerWidth + GetOffset( kX2 ),  halfH + outerWidth + GetOffset( kY2 ) },
			
			// 3.
			{  halfW - innerWidth + GetOffset( kX3 ), -halfH + innerWidth + GetOffset( kY3 ) },
			{  halfW + outerWidth + GetOffset( kX3 ), -halfH - outerWidth + GetOffset( kY3 ) },
			
			// 0. Repeat first point to close the loop
			{ -halfW + innerWidth + GetOffset( kX0 ), -halfH + innerWidth + GetOffset( kY0 ) },
			{ -halfW - outerWidth + GetOffset( kX0 ), -halfH - outerWidth + GetOffset( kY0 ) },
		};
		const int kNumSrc = sizeof( kSrc ) / sizeof( kSrc[0] );

		for ( int i = 0; i < kNumSrc; i++ )
		{
			vertices.Append( kSrc[i] );
		}
	}
	else
	{
		ArrayVertex2 rect( vertices.Allocator() );
		
		// polyline order
		Vertex2 quad[] =
		{
			{ -fHalfW + GetOffset( kX0 ), -fHalfH + GetOffset( kY0 ) },
			{ -fHalfW + GetOffset( kX1 ),  fHalfH + GetOffset( kY1 ) },
			{  fHalfW + GetOffset( kX2 ),  fHalfH + GetOffset( kY2 ) },
			{  fHalfW + GetOffset( kX3 ), -fHalfH + GetOffset( kY3 ) },
		};
		
		rect.Append( quad[0] );
		rect.Append( quad[1] );
		rect.Append( quad[2] );
		rect.Append( quad[3] );

		TesselatorLine t( rect , TesselatorLine::kLoopMode );
		t.SetInnerWidth( innerWidth );
		t.SetOuterWidth( outerWidth );
		
		t.GenerateStroke( vertices );
	}
}

void
TesselatorRect::GetSelfBounds( Rect& rect )
{
	rect.Initialize( fHalfW, fHalfH );
	
	if ( HasOffset() )
	{
		
		Vertex2 v1;
		v1.x = -fHalfW + GetOffset( kX0 );
		v1.y = -fHalfH + GetOffset( kY0 );
		
		Vertex2 v2;
		v2.x = -fHalfW + GetOffset( kX1 );
		v2.y = fHalfH + GetOffset( kY1 );
		
		
		Vertex2 v3;
		v3.x = fHalfW + GetOffset( kX3 );
		v3.y = -fHalfH + GetOffset( kY3 );
		
		Vertex2 v4;
		v4.x = fHalfW + GetOffset( kX2 );
		v4.y = fHalfH + GetOffset( kY2 );
		
		rect.Union(v1);
		rect.Union(v2);
		rect.Union(v3);
		rect.Union(v4);
	}
}

void
TesselatorRect::GetSelfBoundsForAnchor( Rect& rect ) const
{
	rect.Initialize( fHalfW, fHalfH );
}

U32
TesselatorRect::FillVertexCount() const
{
	return 4U;
}

U32
TesselatorRect::StrokeVertexCount() const
{
	return TesselatorLine::VertexCountFromPoints( 4U, true );
}

void
TesselatorRect::SetOffset( RectOffset offset, Real newValue )
{
	fOffsets[offset] = newValue;

	bool value = ! Rtt_RealIsZero( newValue ); // (offset newValue != 0 )
	const U8 mask = 1 << offset;
	const U8 p = fOffsetExists;
	fOffsetExists = ( value ? p | mask : p & ~mask );
///	Invalidate( kFillSourceTexture );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

