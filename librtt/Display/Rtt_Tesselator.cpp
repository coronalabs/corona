//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_Tesselator.h"

#include "Rtt_Matrix.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// Unit circle
const Vertex2
Tesselator::kUnitCircleVertices[] =
{
	{ Rtt_REAL_0, Rtt_REAL_0 },
	{ Rtt_REAL_1, Rtt_REAL_0 },
	{ Rtt_REAL_0, Rtt_REAL_1 },
	{ Rtt_REAL_NEG_1, Rtt_REAL_0 },
	{ Rtt_REAL_0, Rtt_REAL_NEG_1 },
	{ Rtt_REAL_1, Rtt_REAL_0 }
};

// ----------------------------------------------------------------------------

Tesselator::Tesselator()
:	fMaxSubdivideDepth( 0 ),
	fInnerWidth( Rtt_REAL_0 ),
	fOuterWidth( Rtt_REAL_0 )
{
}

Tesselator::~Tesselator()
{
}

Geometry::PrimitiveType
Tesselator::GetStrokePrimitive() const
{
	return Geometry::kTriangleStrip;
}

void
Tesselator::SetInnerWidth( Real newValue )
{
	if ( newValue > Rtt_REAL_0 || Rtt_RealIsZero( newValue ) )
	{
		fInnerWidth = newValue;
	}
}

void
Tesselator::SetOuterWidth( Real newValue )
{
	if ( newValue > Rtt_REAL_0 || Rtt_RealIsZero( newValue ) )
	{
		fOuterWidth = newValue;
	}
}

void
Tesselator::SetWidth( Real newValue )
{
	Real halfW = Rtt_RealDiv2( newValue );
	SetInnerWidth( halfW );
	SetOuterWidth( halfW );
}

bool
Tesselator::HasStroke() const
{
	return ! Rtt_RealIsZero( fInnerWidth ) || Rtt_RealIsZero( fOuterWidth );
}

void
Tesselator::MoveTo( ArrayVertex2& vertices, const Vertex2& p )
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

void
Tesselator::ApplyTransform( ArrayVertex2& vertices, const Vertex2& origin )
{
	if ( ! Rtt_RealIsZero( origin.x ) || ! Rtt_RealIsZero( origin.y ) )
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

U32
Tesselator::CountForDepth( int depth )
{
	return (1U << depth) - 1U; // 2^depth - 1
}

// Subdivide circular sector --- assumes unit circle centered at (0,0)
// http://mathworld.wolfram.com/CircularSector.html
//
// Tesselation is a triangle fan of points along the circumference,
// but it's ordered as a triangle strip. This is why after each point along
// the circumference, the origin is appended to create degenerate tri's.
// This makes it easy to use in rounded rects.
//
// Use depth -1 to insert p1 at the first recursion. This will recurse to the
// same depth as passing depth = 0.
void
Tesselator::SubdivideCircleSector(
	ArrayVertex2& vertices, const Vertex2& p1, const Vertex2& p2, int depth )
{
	const Vertex2 kOrigin = { Rtt_REAL_0, Rtt_REAL_0 };

	// TODO: PERFORMANCE
	// Precalculate vertices on unit circle and group by depth.
	// These calculations are always the same, so we should just append them to 
	// the vertex array
	const int kSubdivideDepth = fMaxSubdivideDepth;

	const Rtt_Real x1 = p1.x;
	const Rtt_Real y1 = p1.y;
	const Rtt_Real x2 = p2.x;
	const Rtt_Real y2 = p2.y;

	const Rtt_Real d0 = kUnitCircleScaleFactor[depth];
	const Rtt_Real xm0 = Rtt_RealDiv( (x1+x2), d0 );
	const Rtt_Real ym0 = Rtt_RealDiv( (y1+y2), d0 );

	const Vertex2 m0 = { xm0, ym0 };

	++depth;
	bool shouldSubdivideAgain = ( depth < kSubdivideDepth );

	if ( shouldSubdivideAgain )
	{
		SubdivideCircleSector( vertices, p1, m0, depth );
	}

	vertices.Append( m0 );
	vertices.Append( kOrigin );

	if ( shouldSubdivideAgain )
	{
		SubdivideCircleSector( vertices, m0, p2, depth );
	}

	vertices.Append( p2 );
	vertices.Append( kOrigin );
}

static U32
SubdivideCircleSectorCount( int maxDepth )
{
	return 4U * Tesselator::CountForDepth( maxDepth );
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
Tesselator::SubdivideCircleArc( ArrayVertex2& vertices, const Vertex2& p1, const Vertex2& p2, int depth, bool appendDuplicate )
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

	if ( ++depth < kSubdivideDepth )
	{
		SubdivideCircleArc( vertices, p1, m0, depth, appendDuplicate );
		SubdivideCircleArc( vertices, m0, p2, depth, appendDuplicate );
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

static U32
SubdivideCircleArcCount( int maxDepth, bool appendDuplicate )
{
	U32 perArcCount = appendDuplicate ? 2U : 1U;

	return (Tesselator::CountForDepth( maxDepth ) + 1U) * perArcCount;
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
	int result = GetLog2( r );

	// For large circles, reduce subdivision depth
	if ( radius > 7 )
	{
		--result;
	}

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
Tesselator::AppendCircle( ArrayVertex2& vertices, Real radius, U32 options )
{
	// TODO: Remove the assumption that fVertices is empty
	Rtt_ASSERT( vertices.Length() == 0 );

	// By default, SubdivideCircleSector() does not add the first point,
	// so we add this in manually.
	vertices.Append( kUnitCircleVertices[1] ); // (-1,0)
	vertices.Append( kUnitCircleVertices[0] ); //  (0,0)

	fMaxSubdivideDepth = DepthForRadius( radius );
	SubdivideCircleSector( vertices, kUnitCircleVertices[1], kUnitCircleVertices[2], 0 );
	SubdivideCircleSector( vertices, kUnitCircleVertices[2], kUnitCircleVertices[3], 0 );
	SubdivideCircleSector( vertices, kUnitCircleVertices[3], kUnitCircleVertices[4], 0 );
	SubdivideCircleSector( vertices, kUnitCircleVertices[4], kUnitCircleVertices[5], 0 );
	fMaxSubdivideDepth = 0;

	if ( ! (options&kNoScale) )
	{
		Vertex2_Scale( vertices.WriteAccess(), vertices.Length(), radius, radius );
	}
}

void
Tesselator::AppendCircleQuadrants( ArrayVertex2& vertices, Real radius, U32 options )
{
	// TODO: Remove the assumption that fVertices is empty
	Rtt_ASSERT( vertices.Length() == 0 );

	fMaxSubdivideDepth = DepthForRadius( radius );

	// Insert first point of each quadrant (circle sector)
	vertices.Append( kUnitCircleVertices[1] ); // (-1,0)
	vertices.Append( kUnitCircleVertices[0] ); //  (0,0)
	SubdivideCircleSector( vertices, kUnitCircleVertices[1], kUnitCircleVertices[2], 0 );

	vertices.Append( kUnitCircleVertices[2] );
	vertices.Append( kUnitCircleVertices[0] );
	SubdivideCircleSector( vertices, kUnitCircleVertices[2], kUnitCircleVertices[3], 0 );

	vertices.Append( kUnitCircleVertices[3] );
	vertices.Append( kUnitCircleVertices[0] );
	SubdivideCircleSector( vertices, kUnitCircleVertices[3], kUnitCircleVertices[4], 0 );

	vertices.Append( kUnitCircleVertices[4] );
	vertices.Append( kUnitCircleVertices[0] );
	SubdivideCircleSector( vertices, kUnitCircleVertices[4], kUnitCircleVertices[5], 0 );
	fMaxSubdivideDepth = 0;

	if ( ! (options&kNoScale) )
	{
		Vertex2_Scale( vertices.WriteAccess(), vertices.Length(), radius, radius );
	}
}

void
Tesselator::AppendCircleArc( ArrayVertex2& vertices, Real radius, U32 options )
{
	// TODO: Remove the assumption that fVertices is empty
	Rtt_ASSERT( vertices.Length() == 0 );

	bool appendDuplicate = ( !! ( options & kAppendDuplicate ) );
	bool appendEndPoints = ( !! ( options & kAppendArcEndPoints ) );

	fMaxSubdivideDepth = DepthForRadius( radius );

	SubdivideCircleArc( vertices, kUnitCircleVertices[1], kUnitCircleVertices[2], 0, appendDuplicate );
	if ( appendEndPoints ) { ArcAppend( vertices, kUnitCircleVertices[2], appendDuplicate ); }

	SubdivideCircleArc( vertices, kUnitCircleVertices[2], kUnitCircleVertices[3], 0, appendDuplicate );
	if ( appendEndPoints ) { ArcAppend( vertices, kUnitCircleVertices[3], appendDuplicate ); }

	SubdivideCircleArc( vertices, kUnitCircleVertices[3], kUnitCircleVertices[4], 0, appendDuplicate );
	if ( appendEndPoints ) { ArcAppend( vertices, kUnitCircleVertices[4], appendDuplicate ); }

	SubdivideCircleArc( vertices, kUnitCircleVertices[4], kUnitCircleVertices[5], 0, appendDuplicate );
	if ( appendEndPoints ) { ArcAppend( vertices, kUnitCircleVertices[5], appendDuplicate ); }

	// Always append this endpoint to close the loop
	ArcAppend( vertices, kUnitCircleVertices[5], appendDuplicate );

	fMaxSubdivideDepth = 0;

	if ( ! (options&kNoScale) )
	{
		Vertex2_Scale( vertices.WriteAccess(), vertices.Length(), radius, radius );
	}
}

U32
Tesselator::AppendCircleCount( Real radius, U32 options ) const
{
	U32 maxDepth = DepthForRadius( radius );

	return 4U * SubdivideCircleSectorCount( maxDepth ) + 2U;
}

U32
Tesselator::AppendCircleQuadrantsCount( Real radius, U32 options ) const
{
	U32 maxDepth = DepthForRadius( radius );

	return 4U * SubdivideCircleSectorCount( maxDepth ) + 8U;
}

U32
Tesselator::AppendCircleArcCount( Real radius, U32 options ) const
{
	bool appendDuplicate = ( !! ( options & kAppendDuplicate ) );
	bool appendEndPoints = ( !! ( options & kAppendArcEndPoints ) );

	U32 maxDepth = DepthForRadius( radius ), perArcCount = appendDuplicate ? 2U : 1U;
	U32 count = SubdivideCircleArcCount( maxDepth, appendDuplicate );

	if (appendEndPoints)
	{
		count += perArcCount;
	}

	return 4U * count + perArcCount;
}

void
Tesselator::AppendRect( ArrayVertex2& vertices, Real halfW, Real halfH )
{
	// Append in tri-strip order
	const Vertex2 bottomLeft =	{ -halfW, -halfH };
	vertices.Append( bottomLeft );

	const Vertex2 upperLeft =	{ -halfW, halfH };
	vertices.Append( upperLeft );

	const Vertex2 bottomRight =	{ halfW, -halfH };
	vertices.Append( bottomRight );

	const Vertex2 upperRight =	{ halfW, halfH };
	vertices.Append( upperRight );
}

void
Tesselator::MoveCenterToOrigin( ArrayVertex2& vertices, Vertex2 currentCenter )
{
	for ( int i = 0, iMax = vertices.Length(); i < iMax; i++ )
	{
		Vertex2& v = vertices[i];
		v.x -= currentCenter.x;
		v.y -= currentCenter.y;
	}
}

void
Tesselator::AppendCircleStroke(
	ArrayVertex2& vertices,
	Real radius, Real innerWidth, Real outerWidth,
	bool appendEndPoints )
{
	const S32 oldLen = vertices.Length();

	// Generate vertices along perimeter of circle. Each vertex is added twice,
	// one for the inner and one for the outer radius.
	U32 options = kNoScale | kAppendDuplicate;
	if ( appendEndPoints )
	{
		options |= kAppendArcEndPoints;
	}

	const Real rInner = Max( radius - innerWidth, Rtt_REAL_0 ); // Ensure > 0
	const Real rOuter = radius + outerWidth;
	AppendCircleArc( vertices, rOuter, options );
	const S32 newLen = vertices.Length() - oldLen;

	// newLen should be multiple of 2 b/c each vertex was added twice
	Rtt_ASSERT( (newLen %2) == 0 );

	// Vertices are from a unit circle. Scale them out to the correct radius
	Vertex2* iVertices = vertices.WriteAccess() + oldLen;
	for ( S32 i = 0; i < newLen; i++ )
	{
		Rtt_ASSERT( (i+1) < newLen );

		Vertex2& inner = iVertices[i];
		Vertex2& outer = iVertices[++i];

		inner.x = Rtt_RealMul( inner.x, rInner );
		inner.y = Rtt_RealMul( inner.y, rInner );

		outer.x = Rtt_RealMul( outer.x, rOuter );
		outer.y = Rtt_RealMul( outer.y, rOuter );
	}
}

void
Tesselator::AppendStrokeTextureClosed( ArrayVertex2& vertices, int numVertices )
{
	// numVertices should be a multiple of 2 (one for inner; one for outer)
	Rtt_ASSERT( (numVertices % 2) == 0 );

	Rtt_ASSERT( vertices.Length() == 0 );

	const Vertex2 inner = { Rtt_REAL_HALF, Rtt_REAL_1 };
	const Vertex2 outer = { Rtt_REAL_HALF, Rtt_REAL_0 };

	// Number of points along the line are half the number of stroke vertices
	int numPoints = numVertices >> 1;
	for ( int i = 0; i < numPoints; i++ )
	{
		vertices.Append( inner );
		vertices.Append( outer );
	}
}

void
Tesselator::AppendStrokeTextureEndCap( ArrayVertex2& vertices, int numVertices )
{
	const Vertex2 innerStart = { Rtt_REAL_0, Rtt_REAL_1 };
	const Vertex2 outerStart = { Rtt_REAL_0, Rtt_REAL_0 };

	const Vertex2 innerEnd = { Rtt_REAL_1, Rtt_REAL_1 };
	const Vertex2 outerEnd = { Rtt_REAL_1, Rtt_REAL_0 };

	// Append end cap coordinates at start and end
	vertices.Append( innerStart );
	vertices.Append( outerStart );
	{
		Rtt_ASSERT( numVertices > 4 );
		numVertices -=4;

		AppendStrokeTextureClosed( vertices, numVertices );
	}
	vertices.Append( innerEnd );
	vertices.Append( outerEnd );
}

U32
Tesselator::AppendCircleStrokeCount( Real radius, bool appendEndPoints ) const
{
	U32 options = kNoScale | kAppendDuplicate;
	if ( appendEndPoints )
	{
		options |= kAppendArcEndPoints;
	}

	return AppendCircleArcCount( radius, options );
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

