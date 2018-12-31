//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_TesselatorStream.h"

#include "Rtt_Matrix.h"
#include "Display/Rtt_VertexCache.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// Unit circle
const Vertex2
TesselatorStream::kUnitCircleVertices[] =
{
	{ Rtt_REAL_0, Rtt_REAL_0 },
	{ Rtt_REAL_1, Rtt_REAL_0 },
	{ Rtt_REAL_0, Rtt_REAL_1 },
	{ Rtt_REAL_NEG_1, Rtt_REAL_0 },
	{ Rtt_REAL_0, Rtt_REAL_NEG_1 },
	{ Rtt_REAL_1, Rtt_REAL_0 }
};

TesselatorStream::TesselatorStream( const Matrix* srcToDstSpace, VertexCache& cache ) 
:	Super(),
	fSrcToDstSpace( srcToDstSpace ),
	fCache( cache ),
	fMaxSubdivideDepth( 0 )
{
}

/*
void
TesselatorStream::Submit( Mode mode, const Vertex2* vertices, const S32* counts, S32 numCounts )
{
	Rtt_ASSERT_NOT_REACHED();
}

void
TesselatorStream::Submit( const Vertex2 vertices[4] )
{
	Rtt_ASSERT_NOT_REACHED();
}

void
TesselatorStream::Submit( const Vertex2& origin, Real rectHalfW, Real rectHalfH, Real radius )
{
	Rtt_ASSERT_NOT_REACHED();
}

void
TesselatorStream::Submit( const Vertex2& origin, Real radius )
{
	Rtt_ASSERT_NOT_REACHED();
}

void
TesselatorStream::Submit( const Vertex2& origin, Real a, Real b )
{
	Rtt_ASSERT_NOT_REACHED();
}
*/

void
TesselatorStream::ApplyTransform( const Vertex2& origin )
{
	ArrayVertex2& vertices = fCache.Vertices();

	if ( fSrcToDstSpace )
	{
		fSrcToDstSpace->Apply( vertices.WriteAccess(), vertices.Length() );
	}
	else
	{
		Vertex2_Translate( vertices.WriteAccess(), vertices.Length(), origin.x, origin.y );
	}
}

static const Rtt_Real kUnitCircleScaleFactor[] =
{
#ifdef Rtt_REAL_FIXED
// TODO: will need better precision than this...
	0x16a09,
	0x1d906,
	0x1f629,
	0x1fd88,
	0x1ff62,
	0x1ffd8,
	0x1fff6,
	0x1fffd,
	0x1ffff,
	0x1ffff,
	0x1ffff,
	0x1ffff,
	0x20000,
	0x20000
#else
	1.414213562373095f,
	1.847759065022573f,
	1.961570560806461f,
	1.990369453344394f,
	1.997590912410345f,
	1.999397637392408f,
	1.999849403678289f,
	1.999962350565202f,
	1.999990587619152f,
	1.999997646903404f,
	1.999999411725764f,
	1.999999852931436f,
	1.999999963232858f,
	1.999999990808215f
#endif
};

// Subdivide circular segment --- assumes unit circle
void
TesselatorStream::SubdivideCircleSegment( const Vertex2& p1, const Vertex2& p2, int depth )
{
	// TODO: PERFORMANCE
	// Precalculate vertices on unit circle and group by depth.
	// These calculations are always the same, so we should just append them to 
	// the vertex array
	const int kSubdivideDepth = ( Rtt_VERIFY( fMaxSubdivideDepth > 0 ) ? fMaxSubdivideDepth : 2 );

	const Rtt_Real x1 = p1.x;
	const Rtt_Real y1 = p1.y;
	const Rtt_Real x2 = p2.x;
	const Rtt_Real y2 = p2.y;

	const Rtt_Real d0 = kUnitCircleScaleFactor[depth];
	const Rtt_Real xm0 = Rtt_RealDiv( (x1+x2), d0 );
	const Rtt_Real ym0 = Rtt_RealDiv( (y1+y2), d0 );

	const Vertex2 m0 = { xm0, ym0 };
	ArrayVertex2& vertices = fCache.Vertices();
	vertices.Append( m0 );

	++depth;
	bool shouldSubdivideAgain = ( depth < kSubdivideDepth );
	const Rtt_Real d1 = ( shouldSubdivideAgain ? kUnitCircleScaleFactor[depth] : Rtt_REAL_1 );

	Vertex2 m2;
	if ( shouldSubdivideAgain )
	{
		m2.x = Rtt_RealDiv( (x2+xm0), d1 );
		m2.y = Rtt_RealDiv( (y2+ym0), d1 );
		vertices.Append( m2 );
	}

	vertices.Append( p2 );
	vertices.Append( p1 );

	Vertex2 m1;
	if ( shouldSubdivideAgain )
	{
		m1.x = Rtt_RealDiv( (x1+xm0), d1 );
		m1.y = Rtt_RealDiv( (y1+ym0), d1 );
		vertices.Append( m1 );
	}

	ArrayS32& counts = fCache.Counts();
	counts.Append( shouldSubdivideAgain ? 5 : 3 ); // This fan has 5 vertices

	#if 0
		Rtt_TRACE( ( "\nEmitting Fan d[%d]=(%.15f)(%x)", depth - 1, d0, Rtt_FloatToFixed( d0 ) ) );
		if ( shouldSubdivideAgain )
		{
			Rtt_TRACE( ( " d[%d]=(%.15f)(%x)", depth, d1, Rtt_FloatToFixed( d1 ) ) );
		}
		Rtt_TRACE( ( ":\n" ) );
		Rtt_TRACE( ( "\t[m0] (%f,%f)\n", m0.x, m0.y ) );
		if ( shouldSubdivideAgain )
		{
			Rtt_TRACE( ( "\t[m2] (%f,%f)\n", m2.x, m2.y ) );
		}
		Rtt_TRACE( ( "\t[p2] (%f,%f)\n", x2, y2 ) );
		Rtt_TRACE( ( "\t[p1] (%f,%f)\n", x1, y1 ) );
		if ( shouldSubdivideAgain )
		{
			Rtt_TRACE( ( "\t[m1] (%f,%f)\n", m1.x, m1.y ) );
		}
	#endif
	
	if ( ++depth < kSubdivideDepth )
	{
		SubdivideCircleSegment( p1, m1, depth );
		SubdivideCircleSegment( m1, m0, depth );
		SubdivideCircleSegment( m0, m2, depth );
		SubdivideCircleSegment( m2, p2, depth );
	}
}

static void
ArcAppend( ArrayVertex2& vertices, const Vertex2& v, bool appendDuplicate )
{
	vertices.Append( v );
	if ( appendDuplicate )
	{
		vertices.Append( v );
	}
}

// Subdivide circular arc --- assumes unit circle.
// NOTE:
// * Does not append p2.
// * Does not update fCache.Counts().  Caller must update
void
TesselatorStream::SubdivideCircleArc( const Vertex2& p1, const Vertex2& p2, int depth, bool appendDuplicate )
{
	// TODO: PERFORMANCE
	// Precalculate vertices on unit circle and group by depth.
	// These calculations are always the same, so we should just append them to 
	// the vertex array
	const int kSubdivideDepth = ( Rtt_VERIFY( fMaxSubdivideDepth > 0 ) ? fMaxSubdivideDepth : 2 );

	const Rtt_Real x1 = p1.x;
	const Rtt_Real y1 = p1.y;
	const Rtt_Real x2 = p2.x;
	const Rtt_Real y2 = p2.y;

	const Rtt_Real d0 = kUnitCircleScaleFactor[depth];
	const Rtt_Real xm0 = Rtt_RealDiv( (x1+x2), d0 );
	const Rtt_Real ym0 = Rtt_RealDiv( (y1+y2), d0 );

	const Vertex2 m0 = { xm0, ym0 };
	ArrayVertex2& vertices = fCache.Vertices();
	if ( ++depth < kSubdivideDepth )
	{
		SubdivideCircleArc( p1, m0, depth, appendDuplicate );
		SubdivideCircleArc( m0, p2, depth, appendDuplicate );
	}
	else
	{
		ArcAppend( vertices, p1, appendDuplicate );
		ArcAppend( vertices, m0, appendDuplicate );
		/*
		vertices.Append( p1 );
		if ( appendDuplicate )
		{
			vertices.Append( p1 );
		}

		vertices.Append( m0 );
		if ( appendDuplicate )
		{
			vertices.Append( m0 );
		}
		*/
	}
}

/// Gets the log2 of the given value.
/// Note: This function was taken from internal Lua function luaO_log2(), source file "lobject.c",
///       which we cannot access if Lua is provided in library form.
static int
GetLog2(unsigned int x)
{
	static const unsigned char log_2[256] = {
		0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
	};
	int l = -1;
	while (x >= 256) { l += 8; x >>= 8; }
	return l + log_2[x];
}

static int
DepthForRadius( Real radius )
{
	Rtt_ASSERT( radius > Rtt_REAL_0 );

	int r = Rtt_RealToInt( radius );

	// Quarter arc is pi*r/2 < r
	// And each depth halves the segment length, so at a depth of log2( r ),
	// the segment length is about a pixel
	int result = GetLog2( r ) - 1;

	if ( result <= 0 ) { result = 1; }

	// Max radius of 256 should satisfy most screen sizes, for now...
	if ( result > 8 ) { result = 8; }

	return result;
}

/*
static int
ArcDepthForRadius( Real radius )
{
	return SegmentDepthForRadius( radius );
}
*/

void
TesselatorStream::AppendCircle( Real radius, bool circularSegmentOnly )
{
	ArrayVertex2& vertices = fCache.Vertices();

	// TODO: Remove the assumption that fVertices is empty
	Rtt_ASSERT( vertices.Length() == 0 );

	if ( ! circularSegmentOnly )
	{
		const S32 count = ( sizeof( kUnitCircleVertices ) / sizeof( Vertex2 ) );
		for ( S32 i = 0; i < count; i++ )
		{
			vertices.Append( kUnitCircleVertices[i] );
		}

		fCache.Counts().Append( count );
	}

	fMaxSubdivideDepth = DepthForRadius( radius );
	SubdivideCircleSegment( kUnitCircleVertices[1], kUnitCircleVertices[2], 0 );
	SubdivideCircleSegment( kUnitCircleVertices[2], kUnitCircleVertices[3], 0 );
	SubdivideCircleSegment( kUnitCircleVertices[3], kUnitCircleVertices[4], 0 );
	SubdivideCircleSegment( kUnitCircleVertices[4], kUnitCircleVertices[5], 0 );
	fMaxSubdivideDepth = 0;

	Vertex2_Scale( vertices.WriteAccess(), vertices.Length(), radius, radius );
}

void
TesselatorStream::AppendCircleArc( Real radius, U32 options )
{
	ArrayVertex2& vertices = fCache.Vertices();

	// TODO: Remove the assumption that fVertices is empty
	Rtt_ASSERT( vertices.Length() == 0 );

	bool appendDuplicate = options & kAppendDuplicate;
	bool appendEndPoints = options & kAppendArcEndPoints;

	fMaxSubdivideDepth = DepthForRadius( radius );

	SubdivideCircleArc( kUnitCircleVertices[1], kUnitCircleVertices[2], 0, appendDuplicate );
	if ( appendEndPoints ) { ArcAppend( vertices, kUnitCircleVertices[2], appendDuplicate ); }

	SubdivideCircleArc( kUnitCircleVertices[2], kUnitCircleVertices[3], 0, appendDuplicate );
	if ( appendEndPoints ) { ArcAppend( vertices, kUnitCircleVertices[3], appendDuplicate ); }

	SubdivideCircleArc( kUnitCircleVertices[3], kUnitCircleVertices[4], 0, appendDuplicate );
	if ( appendEndPoints ) { ArcAppend( vertices, kUnitCircleVertices[4], appendDuplicate ); }

	SubdivideCircleArc( kUnitCircleVertices[4], kUnitCircleVertices[5], 0, appendDuplicate );
	if ( appendEndPoints ) { ArcAppend( vertices, kUnitCircleVertices[5], appendDuplicate ); }

	// Always append this endpoint to close the loop
	ArcAppend( vertices, kUnitCircleVertices[5], appendDuplicate );

	fMaxSubdivideDepth = 0;

	if ( ! (options&kNoScale) )
	{
		Vertex2_Scale( vertices.WriteAccess(), vertices.Length(), radius, radius );
	}
}

void
TesselatorStream::AppendRectangle( Real halfW, Real halfH )
{
	ArrayVertex2& vertices = fCache.Vertices();

	const Vertex2 bottomLeft =	{ -halfW, -halfH };
	vertices.Append( bottomLeft );

	const Vertex2 upperLeft =	{ -halfW, halfH };
	vertices.Append( upperLeft );

	const Vertex2 upperRight =	{ halfW, halfH };
	vertices.Append( upperRight );

	const Vertex2 bottomRight =	{ halfW, -halfH };
	vertices.Append( bottomRight );

	ArrayS32& counts = fCache.Counts();
	counts.Append( 4 );
}

// ----------------------------------------------------------------------------

#if 0
const int kDepth = 4;
static double sD[kDepth];

#include <float.h>

static void setDepthValue( double d, int depth )
{
	Rtt_STATIC_ASSERT( (kDepth & 0x1) == 0 ); // kDepth must be an even int

	if ( sD[depth] < 0. )
	{
		sD[depth] = d;
	}
	else
	{
		Rtt_ASSERT( fabs( sD[depth] - d ) <= FLT_EPSILON );
	}
}

static void subdivideFans( double x1, double y1, double x2, double y2, int depth )
{
	double d0 = sqrt( 2. * ( 1. + x1*x2 + y1*y2 ) ); setDepthValue( d0, depth + 0 );
	double xm0 = (x1+x2) / d0;
	double ym0 = (y1+y2) / d0;

	double d1 = sqrt( 2. * ( 1. + x1*xm0 + y1*ym0 ) ); setDepthValue( d1, depth + 1 );
	double xm1 = (x1+xm0) / d1;
	double ym1 = (y1+ym0) / d1;

	double d2 = sqrt( 2. * ( 1. + x2*xm0 + y2*ym0 ) );
	Rtt_ASSERT( fabs( d2 - d1 ) <= FLT_EPSILON );

	double xm2 = (x2+xm0) / d1;
	double ym2 = (y2+ym0) / d1;

	Rtt_TRACE( ( "\nEmitting Fan d[%d]=(%.15f)(%x) d[%d]=(%.15f)(%x):\n",
		depth, d0, Rtt_FloatToFixed( d0 ), depth + 1, d1, Rtt_FloatToFixed( d1 ) ) );
	Rtt_TRACE( ( "\t[m0] (%f,%f)\n", xm0, ym0 ) );
	Rtt_TRACE( ( "\t[m2] (%f,%f)\n", xm2, ym2 ) );
	Rtt_TRACE( ( "\t[p2] (%f,%f)\n", x2, y2 ) );
	Rtt_TRACE( ( "\t[p1] (%f,%f)\n", x1, y1 ) );
	Rtt_TRACE( ( "\t[m1] (%f,%f)\n", xm1, ym1 ) );

	depth += 2;
	if ( depth < kDepth )
	{
		subdivideFans( x1, y1, xm1, ym1, depth );
		subdivideFans( xm1, ym1, xm0, ym0, depth );
		subdivideFans( xm0, ym0, xm2, ym2, depth );
		subdivideFans( xm2, ym2, x2, y2, depth );
	}
}



static void subdivide( double x1, double y1, double x2, double y2, int depth )
{
	double d = sqrt( 2. * ( 1. + x1*x2 + y1*y2 ) );
	double xm = (x1+x2) / d;
	double ym = (y1+y2) / d;

	if ( sD[depth] < 0. )
	{
		sD[depth] = d;
//		Rtt_TRACE( ( "d[%d]\t(%f,%f) = (%.15f) (%x)\t1/d = (%.15f) (%x)\n", 
//			depth, xm, ym, d, Rtt_FloatToFixed( d ), 1. / d, Rtt_FloatToFixed( 1. / d ) ) );
	}
	else
	{
		Rtt_ASSERT( fabs( sD[depth] - d ) <= FLT_EPSILON );
	}

//		Rtt_TRACE( ( "d[%d]\t(%f,%f) = (%.15f) (%x)\t1/d = (%.15f) (%x)\n",
//			depth, xm, ym, d, Rtt_FloatToFixed( d ), 1. / d, Rtt_FloatToFixed( 1. / d ) ) );

	if ( ++depth < kDepth )
	{
		subdivide( x1, y1, xm, ym, depth );
		subdivide( xm, ym, x2, y2, depth );
	}
}

Rtt_EXPORT void calcCircleConstants()
{
	for ( int i = 0; i < kDepth; i++ ) { sD[i] = -1.; }
	subdivideFans( 1., 0., 0., 1., 0 );
}
#endif

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

